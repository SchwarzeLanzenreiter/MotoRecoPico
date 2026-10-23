// MIT License
//
// Copyright (c) 2026 Schwarze Lanzenreiter
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// MotoRecoPico: the raspberry pi CAN + GPS logger (MotoRecoLogger/mrlogger.c) ported to a
// Pico 2 on the board described in docs/netlist.md. it writes the same .dat format, so the files
// open in MotoRecoViewer unchanged.
//
// everything runs in one loop. only two things are interrupt driven: CAN receive, because the
// controller has just two receive buffers and cannot wait for an SD write to finish, and the GPS
// UART, because 9600 baud does not survive being polled around a card that stalls for 200ms.

#include <stdio.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "config.h"
#include "gps_cmd.h"
#include "led.h"
#include "logger.h"
#include "mcp25625.h"
#include "motoreco.h"
#include "nmea.h"
#include "power.h"
#include "record.h"
#include "sd_spi.h"
#include "settings.h"
#include "settings_file.h"
#include "wallclock.h"

// ---------------------------------------------------------------------------
// GPS serial
// ---------------------------------------------------------------------------

static uint8_t g_gps_ring[GPS_RING_BYTES];
static volatile uint32_t g_gps_head;
static volatile uint32_t g_gps_tail;
static volatile uint32_t g_gps_drops;

static volatile uint32_t g_gps_bytes;	// raw bytes off the wire, for the throughput report
static volatile uint32_t g_gps_ring_peak;	// fullest the ring got in this rate window

// the time each '$' in the ring arrived, in the same order. a GPS record is stamped with when its
// sentence started arriving, the way a CAN frame is stamped by its interrupt: stamping it when it
// is written put every fix that waited out an SD stall on the same millisecond, 0.8s late.
//
// the two rings cannot drift apart: a '$' only goes into the byte ring together with its stamp,
// so every '$' the main loop pops has exactly one stamp waiting for it
#define GPS_STAMP_SLOTS		256		// sentences, not bytes. the byte ring holds ~130 at 61B each
static uint64_t g_gps_stamp[GPS_STAMP_SLOTS];
static volatile uint32_t g_stamp_head;
static volatile uint32_t g_stamp_tail;

static void on_uart_rx(void)
{
	while (uart_is_readable(uart0)) {
		uint8_t c = (uint8_t)uart_getc(uart0);
		uint32_t next = (g_gps_head + 1) % GPS_RING_BYTES;

		g_gps_bytes++;

		if (next == g_gps_tail) {
			g_gps_drops++;
			continue;
		}

		// no stamp slot, no '$'. the sentence then arrives headless and is dropped as bad, which
		// is what a lost '$' already did
		if (c == '$') {
			uint32_t stamp_next = (g_stamp_head + 1) % GPS_STAMP_SLOTS;

			if (stamp_next == g_stamp_tail) {
				g_gps_drops++;
				continue;
			}

			g_gps_stamp[g_stamp_head] = time_us_64();
			g_stamp_head = stamp_next;
		}

		g_gps_ring[g_gps_head] = c;
		g_gps_head = next;

		// how close the ring came to overflowing is the margin the loop stalls leave
		{
			uint32_t fill = (next + GPS_RING_BYTES - g_gps_tail) % GPS_RING_BYTES;

			if (fill > g_gps_ring_peak) {
				g_gps_ring_peak = fill;
			}
		}
	}
}

// rx_us is only written for a '$', with the time that sentence started arriving
static bool gps_pop(uint8_t *out, uint64_t *rx_us)
{
	uint32_t tail = g_gps_tail;

	if (tail == g_gps_head) {
		return false;
	}

	*out = g_gps_ring[tail];

	if (*out == '$' && g_stamp_tail != g_stamp_head) {
		*rx_us = g_gps_stamp[g_stamp_tail];
		g_stamp_tail = (g_stamp_tail + 1) % GPS_STAMP_SLOTS;
	}

	g_gps_tail = (tail + 1) % GPS_RING_BYTES;

	return true;
}

static void gps_uart_init(void)
{
	uart_init(uart0, GPS_UART_BAUD);
	gpio_set_function(PIN_GPS_TX, GPIO_FUNC_UART);
	gpio_set_function(PIN_GPS_RX, GPIO_FUNC_UART);
	uart_set_hw_flow(uart0, false, false);
	uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
	uart_set_fifo_enabled(uart0, true);

	irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
	irq_set_enabled(UART0_IRQ, true);
	uart_set_irq_enables(uart0, true, false);

	// PPS is wired but unused. leave it an input so nothing drives against the module
	gpio_init(PIN_GPS_PPS);
	gpio_set_dir(PIN_GPS_PPS, GPIO_IN);
}

// ---------------------------------------------------------------------------
// state
// ---------------------------------------------------------------------------

static nmea_fix_t g_fix;
static nmea_reader_t g_reader;
static settings_t g_settings;

static uint64_t g_time_base_us;		// monotonic time of the first record in the current file
static bool g_time_base_set;

static int32_t g_lon_prev;			// duplicate suppression, exactly as the pi version did it
static int32_t g_lat_prev;

// one shot reporting flags. these paths are hit on every loop iteration while the condition
// lasts, and an unthrottled debug log would be the slowest thing in the firmware
static bool g_flg_nofix;
static bool g_flg_noalt;
static bool g_flg_nonstd;
static bool g_flg_sd_err;

static uint32_t g_overflow_total;
static uint32_t g_nonstd_dropped;

// main loop stalls. the CAN ring and the GPS ring both only hold for as long as the loop comes
// back to drain them, and the one thing that keeps it away is the card holding a write
static uint32_t g_stall_count;			// iterations of LOOP_STALL_WARN_MS or more since power up
static uint32_t g_stall_max_ms;			// the longest of them
static uint64_t g_stall_reported_us;
static bool g_stall_reported;

// GPS diagnostics. a slow fix has several very different causes (nothing on the wire, wrong baud,
// no antenna, no sky, or simply a cold start) and they are indistinguishable from "not fixed yet"
// unless the receiver's own view of the world is recorded
static uint32_t g_gps_sentences;		// well formed sentences, all types
static uint32_t g_gps_bad;				// dropped on framing or checksum
static uint64_t g_gps_last_sentence_us;
static bool g_gps_seen_any;
static bool g_flg_gps_silent;
static uint64_t g_gps_search_start_us;	// when the current search began

// configuring the receiver, in the order it has to happen (see config.h for the why).
//
// the receiver refuses what it cannot do rather than failing quietly: asked for 100ms fixes at
// 9600 baud it answered $PMTK001,220,2, "valid command, action failed", and kept emitting 1Hz.
// so the baud goes up first, then the sentence set comes down, then the fix interval climbs a
// ladder until one is accepted -- and none of that is believed until PMTK400 reads back the
// interval the module actually holds
typedef enum {
	GPS_CFG_PROBE = 0,	// listening to work out which baud the module came up at
	GPS_CFG_BAUD,		// PMTK251 sent, waiting to hear NMEA at the new rate
	GPS_CFG_SENTENCES,	// PMTK314 sent, waiting for the ack
	GPS_CFG_RATE,		// PMTK220 sent, waiting for the ack. may climb the ladder
	GPS_CFG_VERIFY,		// PMTK400 sent, waiting for the PMTK500 that answers it
	GPS_CFG_DONE
} gps_cfg_step_t;

static gps_cfg_step_t g_gps_cfg_step;
static uint64_t g_gps_cfg_sent_us;
static int g_gps_cfg_tries;			// silence resends of the current attempt only. a refusal
									// starts a fresh attempt and must not consume these

static int g_gps_baud = GPS_UART_BAUD;	// what the pico's own uart is set to right now
static int g_gps_baud_prev;				// to fall back to when PMTK251 does not take
static int g_gps_probe_switches;
static bool g_gps_baud_switched;		// the pico has followed the module to the new rate

static int g_gps_rate_try_ms;			// the interval currently being attempted
static int g_gps_rate_accepted_ms;		// 0 until the receiver answers one with flag 3
static int g_gps_rate_effective_ms;		// what PMTK500 says the module really holds

// throughput over the last window. this is the measurement the fix rate experiment turns on:
// 9600 8N1 carries 960 bytes/s and nothing the receiver is told can change that
static uint64_t g_rate_window_us;
static uint32_t g_rate_bytes0;
static uint32_t g_rate_sentences0;
static uint32_t g_rate_bad0;
static uint32_t g_rate_drops0;

// GGA carries the altitude and RMC the speed, so at the very first fix exactly one half of a
// position has arrived and the other is legitimately missing for one sentence
static bool g_seen_fixed_gga;
static bool g_seen_fixed_rmc;
static bool g_flg_badtime;	// the receiver offered a date it cannot possibly have known

static uint64_t g_last_elapsed_us;	// what the previous record was stamped with
static uint32_t g_clamped_records;	// how often the monotonic guard in stamp_record had to act
static bool g_flg_clamped;

static void reset_log_state(void)
{
	// the pi cleared these on every key on, so each trip's file is self contained. a restart at
	// the same spot would otherwise have its first fix suppressed as a duplicate
	g_time_base_set = false;
	g_last_elapsed_us = 0;	// elapsed restarts at 0 with the base, so the guard has to as well
	g_lon_prev = 0;
	g_lat_prev = 0;
	g_flg_noalt = false;
	g_flg_nonstd = false;
	g_flg_badtime = false;
	g_flg_clamped = false;
	g_seen_fixed_gga = false;
	g_seen_fixed_rmc = false;

	// g_flg_nofix is deliberately not touched here. it tracks whether the receiver currently has
	// a fix, not whether something was reported, so opening a new file must not disturb it
}

// every record in the file passes through here. both kinds carry the time an interrupt saw them
// arrive: a CAN frame its capture, a GPS record the '$' of its sentence. they used to differ --
// GPS was stamped when written, which ran backwards against CAN 4 times in 27846 records at 10Hz
// (worst 2ms), and after an SD stall put every waiting fix on the same millisecond. now the two
// are merged in time order before they get here (see write_gps_pending).
//
// the guard below stays as the guarantee rather than the fix: a record never carries a time
// earlier than the record before it; one that would is pinned to that previous value instead.
// the merge leaves it almost nothing to do, so a clamped: count that climbs means it is broken.
//
// the same comparison also guards the subtraction. if such a frame is the very first record
// stamped it sets the base ahead of a now_us that follows, and an unsigned wrap there would put
// a nonsense second count into the file rather than a 2ms wobble
static void stamp_record(struct CANData *rec, uint64_t now_us)
{
	uint64_t elapsed;

	if (!g_time_base_set) {
		g_time_base_set = true;
		g_time_base_us = now_us;
	}

	elapsed = (now_us > g_time_base_us) ? (now_us - g_time_base_us) : 0;

	if (elapsed < g_last_elapsed_us) {
		if (!g_flg_clamped) {
			g_flg_clamped = true;
			logger_debug("record time went backwards by %uus and was pinned to the previous "
				"record. can and gps records are merged in time order, so this one slipped past "
				"the merge",
				(unsigned int)(g_last_elapsed_us - elapsed));
		}

		g_clamped_records++;
		elapsed = g_last_elapsed_us;
	}

	g_last_elapsed_us = elapsed;

	record_set_elapsed(rec, elapsed);
}

// ---------------------------------------------------------------------------
// ignition
//
// this is the one deadline in the firmware. the board keeps itself alive for a guaranteed 0.4s
// after the key goes off (docs/netlist.md section 4.7), and the log has to be flushed, closed and
// renamed inside it. sampling therefore runs off a timer rather than the main loop: an SD write
// can stall the loop for a couple of hundred milliseconds, and spending the budget on noticing
// would leave none for acting
// ---------------------------------------------------------------------------

static power_state_t g_power;
static repeating_timer_t g_ig_timer;
static volatile power_event_t g_power_event;	// posted by the timer, consumed by the main loop
static volatile int g_ig_mv;
static bool g_powered_off;		// ignition gone and the log closed. stay idle until it returns

static int read_ig_mv(void)
{
	adc_select_input(IG_ADC_INPUT);

	// 12 bit against the 3.3V reference. no divider maths: since v3 this pin is a clamped 0.65V
	// logic level, not a scaled battery voltage
	return (int)((uint32_t)adc_read() * 3300u / 4095u);
}

static bool ig_sample(repeating_timer_t *timer)
{
	power_event_t event;

	(void)timer;

	g_ig_mv = read_ig_mv();
	event = power_eval(&g_power, g_ig_mv);

	if (event != POWER_EVENT_NONE) {
		g_power_event = event;
	}

	return true;
}

// ---------------------------------------------------------------------------
// GPS records waiting for their place in the file
//
// a GPS record carries the time its sentence arrived, which can be older than CAN frames still
// sitting in the CAN ring -- after an SD stall by up to the length of the stall. the GPS side is
// therefore parsed first and its records held here, and handle_can_frames writes each one just
// before the first CAN frame captured after it, so the file comes out in time order
// ---------------------------------------------------------------------------

#define GPS_PENDING_SLOTS	128		// three records per fix. 3s of a 10Hz receiver is 90

typedef struct {
	struct CANData	rec;		// id and data filled in. the time is set when it is written
	uint64_t		rx_us;		// when its sentence started arriving
} gps_pending_t;

static gps_pending_t g_gps_pending[GPS_PENDING_SLOTS];
static uint32_t g_pending_head;		// main loop only, both ends
static uint32_t g_pending_tail;

static uint32_t gps_pending_free(void)
{
	return (g_pending_tail + GPS_PENDING_SLOTS - g_pending_head - 1) % GPS_PENDING_SLOTS;
}

// handle_gps stops parsing while there is no room, so this does not fail
static void gps_pending_push(const struct CANData *rec, uint64_t rx_us)
{
	g_gps_pending[g_pending_head].rec = *rec;
	g_gps_pending[g_pending_head].rx_us = rx_us;
	g_pending_head = (g_pending_head + 1) % GPS_PENDING_SLOTS;
}

// writes every held record that arrived at or before until_us. with no card they go nowhere,
// exactly as a GPS record written straight away did
static bool write_gps_pending(uint64_t until_us)
{
	bool ok = true;

	while (g_pending_tail != g_pending_head && g_gps_pending[g_pending_tail].rx_us <= until_us) {
		gps_pending_t *p = &g_gps_pending[g_pending_tail];

		g_pending_tail = (g_pending_tail + 1) % GPS_PENDING_SLOTS;

		stamp_record(&p->rec, p->rx_us);

		if (!logger_write(&p->rec)) {
			ok = false;
		}
	}

	return ok;
}

static uint64_t g_sentence_rx_us;	// arrival of the sentence the reader is assembling

// is there a sentence that started arriving at or before when_us and has not been parsed yet?
// its records do not exist yet, so nothing captured after its '$' may be written before it is.
// measured on the bench: without this, frames newer than a sentence still in the UART ring went
// to the card first, and the sentence's records were then clamped onto them -- 82 times in one
// replay, and all three fixes that waited out an 895ms stall landed on the same millisecond.
//
// two places such a sentence can be: whole or in part in the ring, where the stamp ring holds its
// '$', or already half way into the reader. the second is bounded, because a module that stops
// talking mid sentence would otherwise hold back every CAN frame from then on
static bool gps_unparsed_before(uint64_t when_us)
{
	uint32_t tail = g_stamp_tail;

	if (tail != g_stamp_head && g_gps_stamp[tail] <= when_us) {
		return true;
	}

	return g_reader.len > 0 && g_sentence_rx_us <= when_us &&
	       time_us_64() - g_sentence_rx_us < (uint64_t)GPS_SENTENCE_MAX_MS * 1000ULL;
}

// ---------------------------------------------------------------------------
// CAN
// ---------------------------------------------------------------------------

static bool handle_can_frames(void)
{
	can_frame_t frame;
	struct CANData rec;
	bool ok = true;
	int budget = CAN_RING_RECORDS;

	// bounded, so a bus busy enough to refill the ring as fast as it drains cannot starve the
	// GPS and housekeeping below
	while (budget-- > 0) {
		uint64_t frame_us;

		if (!mcp25625_peek_timestamp(&frame_us)) {
			break;
		}

		// GPS records that arrived before this frame go first
		if (!write_gps_pending(frame_us)) {
			ok = false;
		}

		// and a sentence that arrived before it but is still unparsed has to be parsed first. the
		// frame stays in the ring and the main loop comes straight back round to handle_gps. the
		// pending queue was just emptied up to here, so handle_gps has the room to do it
		if (gps_unparsed_before(frame_us)) {
			break;
		}

		mcp25625_pop(&frame);

		// struct CANData.id is 16 bits wide and has no room for a 29 bit id or a remote flag.
		// the pi version dropped these rather than truncating them into a wrong standard id
		if (frame.extended || frame.rtr) {
			g_nonstd_dropped++;

			if (!g_flg_nonstd) {
				g_flg_nonstd = true;
				logger_debug("non standard can frame is dropped. id:%08X ext:%d rtr:%d",
					(unsigned int)frame.id, frame.extended, frame.rtr);
			}

			continue;
		}

		stamp_record(&rec, frame.timestamp_us);
		rec.id = (unsigned short int)(frame.id & 0x7FF);

		// no DLC field in the record, so a short frame is zero filled and becomes
		// indistinguishable from a padded one. same as the pi version
		memset(rec.data, 0, sizeof(rec.data));
		memcpy(rec.data, frame.data, (frame.dlc > 8) ? 8 : frame.dlc);

		if (!logger_write(&rec)) {
			ok = false;
		}
	}

	// the rest: all of it when no frame is waiting, otherwise only what came before the next one.
	// a frame captured after this point is newer than every sentence already parsed, and an
	// unparsed sentence is newer than every pending record, so neither can be overtaken
	{
		uint64_t next_us;

		if (!mcp25625_peek_timestamp(&next_us)) {
			next_us = UINT64_MAX;
		}

		if (!write_gps_pending(next_us)) {
			ok = false;
		}
	}

	return ok;
}

// ---------------------------------------------------------------------------
// GPS
// ---------------------------------------------------------------------------

// hdop in tenths, for printing it as two integers instead of with %.1f.
//
// the SDK's printf cannot be trusted with this particular number. pico_printf's _ftoa() rounds
// the scaled fraction, and when that fraction lands exactly on .5 it takes a branch that
// increments without the "frac >= pow10[prec]" rollover guard the neighbouring branch has. frac
// then equals 10, the fractional loop runs one pass too many, its unsigned counter wraps to
// UINT_MAX, and the "add extra 0s" loop fills the whole 32 byte buffer. the decimal point and
// the integer part are never written at all.
//
// a real ride produced "hdop:00000000000000000000000000000010" four times, on lines that were
// otherwise perfect and reported excellent fixes. the trigger is an hdop of exactly 0.95 or
// 1.95 -- ordinary values for a good fix, and the only two in 0..100 where the double arithmetic
// lands precisely on the half. the Hz figures elsewhere are derived from the fix interval and
// none of the reachable values can land there, so they keep using %.1f
static int hdop_tenths(double hdop)
{
	if (hdop < 0.0) {
		return 0;
	}

	// the receiver tops out at 99.99, but a mangled sentence must not be able to print
	// something that reads like a plausible number
	if (hdop > 999.0) {
		return 9999;
	}

	return (int)(hdop * 10.0 + 0.5);
}

// one line describing what the receiver currently sees. used for the periodic search report, the
// moment a fix appears, and the moment one is lost
static void log_gps_state(const char *what, uint64_t now_us)
{
	uint32_t elapsed_s = (uint32_t)((now_us - g_gps_search_start_us) / 1000000ULL);
	int hdop10 = hdop_tenths(g_fix.hdop);

	logger_debug("gps %s after %us: quality:%d mode:%dD sats used:%d view:%d tracked:%d "
		"best snr:%d hdop:%d.%d sentences:%u bad:%u",
		what, (unsigned int)elapsed_s,
		g_fix.quality, g_fix.fix_mode, g_fix.sats_used,
		nmea_sats_in_view(&g_fix), nmea_sats_tracked(&g_fix), nmea_best_snr(&g_fix),
		hdop10 / 10, hdop10 % 10,
		(unsigned int)g_gps_sentences, (unsigned int)g_gps_bad);
}

// puts one PMTK command on the wire. a 60 byte command at 9600 baud occupies the UART for about
// 60ms, which is why this only ever runs during start up
static void gps_send(const char *body)
{
	char cmd[GPS_CMD_MAX];

	if (!gps_cmd_build(cmd, sizeof(cmd), body)) {
		logger_debug("gps command too long to send: %s", body);
		return;
	}

	uart_write_blocking(uart0, (const uint8_t *)cmd, strlen(cmd));

	// wait for the last bit to actually leave. PMTK251 is the reason: the module changes its
	// own rate the moment it has received the command, so anything still sitting in the FIFO
	// would go out at a rate nobody is listening at
	uart_tx_wait_blocking(uart0);
}

// moves the pico's side of the link. the receive ring and the sentence reader have to be thrown
// away with it: bytes captured at the old rate are framing noise at the new one, and a half
// finished sentence would otherwise be completed with bytes from the other rate
static void gps_set_baud(int baud)
{
	irq_set_enabled(UART0_IRQ, false);

	uart_set_baudrate(uart0, (uint)baud);
	g_gps_tail = g_gps_head;
	g_stamp_tail = g_stamp_head;
	nmea_reader_init(&g_reader);

	irq_set_enabled(UART0_IRQ, true);

	g_gps_baud = baud;
}

// the rate to listen at when the current one is silent. deliberately not taken from the settings
// alone, because the two cases that would break are the two that matter most: gps_baud = 0 means
// "leave the module alone", and a module an earlier card moved to 115200 still has to be found
// before it can be left alone. gps_baud = 9600 means "put it back", which likewise starts by
// finding it where it actually is
static int gps_other_baud(void)
{
	int fast = (g_settings.gps_baud != 0 && g_settings.gps_baud != GPS_UART_BAUD)
		? g_settings.gps_baud : DEFAULT_GPS_BAUD;

	if (fast == GPS_UART_BAUD) {
		return GPS_UART_BAUD;	// nothing to alternate with
	}

	return (g_gps_baud == GPS_UART_BAUD) ? fast : GPS_UART_BAUD;
}

static const char *gps_cfg_command_name(gps_cfg_step_t step)
{
	switch (step) {
	case GPS_CFG_BAUD:		return "PMTK251";
	case GPS_CFG_SENTENCES:	return "PMTK314";
	case GPS_CFG_RATE:		return "PMTK220";
	case GPS_CFG_VERIFY:	return "PMTK400";
	default:				return "nothing";
	}
}

// drives the configuration forward. called when the module is first heard, when an answer
// arrives, and from the main loop for the timeouts
static void gps_cfg_advance(gps_cfg_step_t step, uint64_t now_us)
{
	char body[GPS_CMD_MAX];

	g_gps_cfg_step = step;
	g_gps_cfg_sent_us = now_us;
	g_gps_cfg_tries++;

	switch (step) {
	case GPS_CFG_BAUD:
		g_gps_baud_switched = false;
		snprintf(body, sizeof(body), "PMTK251,%d", g_settings.gps_baud);
		gps_send(body);
		break;

	case GPS_CFG_SENTENCES:
		gps_send(GPS_SENTENCES_WANTED);
		break;

	case GPS_CFG_RATE:
		snprintf(body, sizeof(body), "PMTK220,%d", g_gps_rate_try_ms);
		gps_send(body);
		break;

	case GPS_CFG_VERIFY:
		gps_send("PMTK400");
		break;

	default:
		break;
	}
}

// everything after the baud is settled: trim the sentences, then climb to a fix interval
static void gps_cfg_after_baud(uint64_t now_us)
{
	g_gps_cfg_tries = 0;

	// a rate of 0 means "leave the receiver alone", for comparing against its own defaults. the
	// read back still runs: this is precisely the mode in which nobody knows what it holds
	if (g_settings.gps_rate_ms == 0) {
		logger_debug("gps: gps_rate is 0, leaving the receiver's own output alone");
		gps_cfg_advance(GPS_CFG_VERIFY, now_us);
		return;
	}

	logger_debug("gps: configuring for %dms fixes (%.1fHz), GGA+RMC every fix, "
		"GSA+GSV every %d (%.1fHz)", g_settings.gps_rate_ms,
		(double)1000.0 / (double)g_settings.gps_rate_ms, GPS_DIAG_DIVIDER,
		(double)1000.0 / ((double)g_settings.gps_rate_ms * (double)GPS_DIAG_DIVIDER));

	g_gps_rate_try_ms = g_settings.gps_rate_ms;
	gps_cfg_advance(GPS_CFG_SENTENCES, now_us);
}

// the module is talking and we know at what rate. raise it first if asked: running the fix
// interval ladder first would find the ceiling of a line we are about to widen and settle there
static void gps_cfg_begin(uint64_t now_us)
{
	if (g_settings.gps_baud != 0 && g_settings.gps_baud != g_gps_baud) {
		logger_debug("gps: raising the link from %d to %d baud", g_gps_baud,
			g_settings.gps_baud);
		g_gps_baud_prev = g_gps_baud;
		g_gps_cfg_tries = 0;
		gps_cfg_advance(GPS_CFG_BAUD, now_us);
		return;
	}

	gps_cfg_after_baud(now_us);
}

// PMTK251 did not take: either the module was built without "allow change of baudrate", or it
// never received the command. carry on at the rate it is actually speaking
static void gps_cfg_baud_give_up(uint64_t now_us)
{
	if (g_gps_baud != g_gps_baud_prev) {
		gps_set_baud(g_gps_baud_prev);
	}

	logger_debug("gps: PMTK251,%d did not take, staying at %d baud", g_settings.gps_baud,
		g_gps_baud);
	gps_cfg_after_baud(now_us);
}

static void gps_cfg_on_sentences_ack(int flag, uint64_t now_us)
{
	if (flag == GPS_ACK_OK) {
		logger_debug("gps: sentence set applied (GGA+RMC every fix, GSA+GSV every %d)",
			GPS_DIAG_DIVIDER);
	} else {
		// there is no ladder for a sentence set, and putting the module's default set back
		// (PMTK314,-1) would only confirm it. the module's own set emits GLL and VTG as well,
		// and all of it on every fix, so the rate ladder below will settle on a slower rung --
		// and PMTK400 will say which
		logger_debug("gps: PMTK314 refused (flag %d). the receiver keeps its own sentence set, "
			"so GLL/VTG come back and everything runs at the full fix rate. the rate ladder "
			"will settle slower", flag);
	}

	g_gps_cfg_tries = 0;
	gps_cfg_advance(GPS_CFG_RATE, now_us);
}

static void gps_cfg_on_rate_ack(int flag, uint64_t now_us)
{
	int next;

	if (flag == GPS_ACK_OK) {
		g_gps_rate_accepted_ms = g_gps_rate_try_ms;
		logger_debug("gps: fix interval accepted (%dms, %.1fHz)", g_gps_rate_try_ms,
			(double)1000.0 / (double)g_gps_rate_try_ms);
		g_gps_cfg_tries = 0;
		gps_cfg_advance(GPS_CFG_VERIFY, now_us);
		return;
	}

	// flag 0 is "invalid command" and flag 1 is "unsupported". neither becomes supported at a
	// slower rate, so there is nothing to sweep. only flag 2, "action failed", is the receiver
	// saying it could not do this particular interval
	if (flag != 2) {
		logger_debug("gps: PMTK220 unsupported (flag %d). not sweeping", flag);
		g_gps_cfg_tries = 0;
		gps_cfg_advance(GPS_CFG_VERIFY, now_us);
		return;
	}

	next = gps_cmd_next_fix_interval(g_gps_rate_try_ms);

	if (next == 0) {
		logger_debug("gps: every interval from %dms to the end of the ladder was refused. "
			"the receiver keeps its own rate", g_settings.gps_rate_ms);
		g_gps_cfg_tries = 0;
		gps_cfg_advance(GPS_CFG_VERIFY, now_us);
		return;
	}

	logger_debug("gps: PMTK220,%d refused (flag 2), backing off to %dms", g_gps_rate_try_ms,
		next);

	g_gps_rate_try_ms = next;
	g_gps_cfg_tries = 0;	// a refusal is a new attempt, not a resend of the old one
	gps_cfg_advance(GPS_CFG_RATE, now_us);
}

// the only honest account of the fix interval: what the module says it holds, not what it
// acknowledged. the whole point of the read back
static void gps_cfg_on_fix_ctl(const gps_fix_ctl_t *ctl)
{
	int asked = g_gps_rate_accepted_ms ? g_gps_rate_accepted_ms : g_settings.gps_rate_ms;

	g_gps_rate_effective_ms = ctl->fix_interval_ms;

	if (g_settings.gps_rate_ms == 0) {
		logger_debug("gps: module holds %dms fixes (%.1fHz), we sent nothing",
			ctl->fix_interval_ms, (double)1000.0 / (double)ctl->fix_interval_ms);
	} else if (ctl->fix_interval_ms == asked) {
		logger_debug("gps: module holds %dms fixes (%.1fHz), as asked", ctl->fix_interval_ms,
			(double)1000.0 / (double)ctl->fix_interval_ms);
	} else {
		logger_debug("gps: module holds %dms fixes, we asked for %dms. PMTK220 did not take",
			ctl->fix_interval_ms, asked);
	}

	// a non-zero duration or run interval means the module sleeps between fixes, and then the
	// fix interval does not mean what it looks like
	if (ctl->duration_ms != 0 || ctl->run_interval_ms != 0) {
		logger_debug("gps: PMTK500 duration:%d run:%d", ctl->duration_ms, ctl->run_interval_ms);
	}

	g_gps_cfg_step = GPS_CFG_DONE;
}

// consumes anything that is an answer to one of our own commands: PMTK001 and PMTK500. returns
// true when the sentence was ours, so it never reaches the navigation handling.
//
// the flag is read by the step that was waiting for it, never by a shared dispatcher. that is
// the bug this replaces: a refusal used to fall through into "applied"
static bool gps_cfg_consume(const char *sentence, uint64_t now_us)
{
	gps_fix_ctl_t ctl;
	int cmd, flag;

	if (gps_cmd_parse_fix_ctl(sentence, &ctl)) {
		if (g_gps_cfg_step == GPS_CFG_VERIFY) {
			gps_cfg_on_fix_ctl(&ctl);
		}

		return true;
	}

	if (!gps_cmd_parse_ack(sentence, &cmd, &flag)) {
		return false;
	}

	if (g_gps_cfg_step == GPS_CFG_BAUD && cmd == 251) {
		// the ack is not what proves this one worked: the module switches rate the moment it has
		// the command, so its own acknowledgement can arrive garbled or not at all. hearing NMEA
		// at the new rate is the proof. a clean refusal, though, is worth acting on at once
		if (flag != GPS_ACK_OK) {
			logger_debug("gps: PMTK251 refused (flag %d). the module was built without "
				"\"allow change of baudrate\"", flag);
			gps_cfg_baud_give_up(now_us);
		}

		return true;
	}

	if (g_gps_cfg_step == GPS_CFG_SENTENCES && cmd == 314) {
		gps_cfg_on_sentences_ack(flag, now_us);
		return true;
	}

	if (g_gps_cfg_step == GPS_CFG_RATE && cmd == 220) {
		gps_cfg_on_rate_ack(flag, now_us);
		return true;
	}

	// PMTK400 is answered twice: an ack for the query, then the PMTK500 carrying the answer. the
	// PMTK500 is the completion event, so a successful ack here is just noise
	if (g_gps_cfg_step == GPS_CFG_VERIFY && cmd == 400) {
		if (flag != GPS_ACK_OK) {
			logger_debug("gps: PMTK400 refused (flag %d). the effective fix interval stays "
				"unverified", flag);
			g_gps_cfg_step = GPS_CFG_DONE;
		}

		return true;
	}

	// an answer to something we are not waiting on: a late duplicate after a resend, or the
	// module's own start up chatter. worth a line, but it must not drive the machine
	logger_debug("gps: stray PMTK%03d answer, flag %d", cmd, flag);

	return true;
}

// the module's baud at power up is genuinely unknown: PMTK251 persists across a reboot but
// reverts on a full cold start or a standby entry, and an earlier card may have moved it. so
// listen before saying anything, and alternate between the two candidates until something
// intelligible arrives
static void gps_probe_tick(uint64_t now_us)
{
	int other;

	if (now_us - g_gps_cfg_sent_us < (uint64_t)GPS_PROBE_MS * 1000ULL) {
		return;
	}

	other = gps_other_baud();

	if (other == g_gps_baud || g_gps_probe_switches >= GPS_PROBE_MAX_SWITCHES) {
		// either there is only one rate worth trying, or both have been tried twice. the
		// commands below are harmless with nobody listening, and the silence warning in the
		// main loop is what will keep saying so
		logger_debug("gps: nothing intelligible at %d baud%s. carrying on anyway, check the "
			"GPS wiring and J3 power", g_gps_baud,
			(other == g_gps_baud) ? "" : " or the other candidate");

		if (g_gps_baud != GPS_UART_BAUD) {
			gps_set_baud(GPS_UART_BAUD);
		}

		gps_cfg_begin(now_us);
		return;
	}

	g_gps_probe_switches++;
	logger_debug("gps: nothing at %d baud, listening at %d", g_gps_baud, other);
	gps_set_baud(other);
	g_gps_cfg_sent_us = now_us;
}

static void gps_baud_tick(uint64_t now_us)
{
	if (!g_gps_baud_switched) {
		// the module needs a moment to act on the command before our side moves
		if (now_us - g_gps_cfg_sent_us < (uint64_t)GPS_BAUD_SETTLE_MS * 1000ULL) {
			return;
		}

		gps_set_baud(g_settings.gps_baud);
		g_gps_baud_switched = true;
		g_gps_cfg_sent_us = now_us;
		return;
	}

	// listening at the new rate. a sentence arriving here is handled in handle_gps_sentence,
	// which is what ends this step
	if (now_us - g_gps_cfg_sent_us < (uint64_t)GPS_PROBE_MS * 1000ULL) {
		return;
	}

	gps_cfg_baud_give_up(now_us);
}

// resends the current command if the module never answered
static void gps_cfg_tick(uint64_t now_us)
{
	switch (g_gps_cfg_step) {
	case GPS_CFG_PROBE:
		gps_probe_tick(now_us);
		return;

	case GPS_CFG_BAUD:
		gps_baud_tick(now_us);
		return;

	case GPS_CFG_SENTENCES:
	case GPS_CFG_RATE:
	case GPS_CFG_VERIFY:
		break;

	default:
		return;
	}

	if (now_us - g_gps_cfg_sent_us < (uint64_t)GPS_CFG_ACK_TIMEOUT_MS * 1000ULL) {
		return;
	}

	if (g_gps_cfg_tries < GPS_CFG_RETRIES) {
		gps_cfg_advance(g_gps_cfg_step, now_us);
		return;
	}

	logger_debug("gps: no ack for %s after %d tries", gps_cfg_command_name(g_gps_cfg_step),
		g_gps_cfg_tries);

	// a silent PMTK220 is still worth a read back. if PMTK400 answers, the TX line is fine and
	// the command was simply ignored; if it does not, the line to the module is dead
	if (g_gps_cfg_step == GPS_CFG_VERIFY) {
		logger_debug("gps: PMTK400 never answered, the effective fix interval is unverified");
		g_gps_cfg_step = GPS_CFG_DONE;
		return;
	}

	g_gps_cfg_tries = 0;
	gps_cfg_advance(GPS_CFG_VERIFY, now_us);
}

// what actually came off the wire in the last window, against what the UART can carry.
// this is the number the fix rate experiment is about
static void gps_rate_report(uint64_t now_us)
{
	uint32_t bytes = g_gps_bytes - g_rate_bytes0;
	uint32_t sentences = g_gps_sentences - g_rate_sentences0;
	uint32_t bad = g_gps_bad - g_rate_bad0;
	uint32_t drops = g_gps_drops - g_rate_drops0;
	uint32_t secs = (uint32_t)((now_us - g_rate_window_us) / 1000000ULL);
	uint32_t capacity;

	if (secs == 0) {
		return;
	}

	// 8N1 is 10 bits on the wire per byte
	capacity = (uint32_t)g_gps_baud / 10;

	// the trailing pair is what makes this line self checking: if the module is not holding the
	// interval we asked for, the two numbers disagree and the whole experiment is answered here
	logger_debug("gps rate: %u.%u sentences/s %u B/s (%u%% of %d baud) avg %u B/sentence "
		"bad:%u uart_lost:%u ring peak:%u/%u (want %dms, module %dms)",
		(unsigned int)(sentences * 10 / secs / 10), (unsigned int)(sentences * 10 / secs % 10),
		(unsigned int)(bytes / secs), (unsigned int)(bytes / secs * 100 / capacity),
		g_gps_baud,
		(unsigned int)(sentences ? bytes / sentences : 0),
		(unsigned int)bad, (unsigned int)drops,
		(unsigned int)g_gps_ring_peak, (unsigned int)GPS_RING_BYTES,
		g_gps_rate_accepted_ms ? g_gps_rate_accepted_ms : g_settings.gps_rate_ms,
		g_gps_rate_effective_ms);

	g_rate_window_us = now_us;
	g_rate_bytes0 = g_gps_bytes;
	g_rate_sentences0 = g_gps_sentences;
	g_rate_bad0 = g_gps_bad;
	g_rate_drops0 = g_gps_drops;
	g_gps_ring_peak = 0;
}

static void queue_position(uint64_t now_us, uint64_t rx_us);

// rx_us is when the sentence started arriving, and it is the time its records carry. now_us is the
// loop's clock and only feeds the diagnostics. nothing here writes a record: they are queued for
// handle_can_frames to put in time order, so this has no write to fail
static void handle_gps_sentence(const char *sentence, uint64_t now_us, uint64_t rx_us)
{
	struct CANData rec;
	int64_t unix_ms;
	int result = nmea_parse(&g_fix, sentence);

	if (result == NMEA_RESULT_INVALID) {
		// a few of these at power up are normal, the receiver may be mid sentence when the UART
		// starts. a steady stream of them means the baud rate is wrong
		g_gps_bad++;
		return;
	}

	g_gps_sentences++;
	g_gps_last_sentence_us = now_us;
	g_flg_gps_silent = false;

	// the first sentence proves the wiring and the baud rate. worth its own line, because
	// everything that comes after assumes both
	if (!g_gps_seen_any) {
		char head[16];
		size_t i;

		g_gps_seen_any = true;

		for (i = 0; i < sizeof(head) - 1 && sentence[i] != '\0' && sentence[i] != ','; i++) {
			head[i] = sentence[i];
		}

		head[i] = '\0';
		logger_debug("gps talking: first sentence '%s' at %d baud", head, g_gps_baud);
	}

	// a well formed sentence at the current rate is the only proof either listening step gets.
	// both stop here rather than falling through to gps_cfg_consume: this sentence arrived
	// before the command that the step below goes on to send, so it cannot be its answer, and a
	// stale ack left in the module's output buffer across a pico reset would otherwise be
	// credited to a PMTK314 sent microseconds ago. one sentence's worth of navigation data is
	// given up for that, once per power up, before there is any fix to lose
	if (g_gps_cfg_step == GPS_CFG_PROBE) {
		logger_debug("gps: module is at %d baud", g_gps_baud);
		gps_cfg_begin(now_us);
		return;
	}

	// only once our side has followed. while the settle window is still running the module is
	// legitimately still on the old rate, and what arrives may be the PMTK251 ack itself
	if (g_gps_cfg_step == GPS_CFG_BAUD && g_gps_baud_switched) {
		logger_debug("gps: PMTK251 applied, the link is now %d baud", g_gps_baud);
		gps_cfg_after_baud(now_us);
		return;
	}

	// PMTK001 and PMTK500 are answers to our own commands, not navigation data. this has to come
	// before the NMEA_RESULT_IGNORED return below, because a PMTK500 parses as ignored
	if (gps_cfg_consume(sentence, now_us)) {
		return;
	}

	if (result == NMEA_RESULT_IGNORED) {
		return;
	}

	if (result == NMEA_RESULT_GGA && g_fix.has_fix) {
		g_seen_fixed_gga = true;
	} else if (result == NMEA_RESULT_RMC && g_fix.has_fix) {
		g_seen_fixed_rmc = true;
	}

	// the receiver is the only clock on the board, and only a fixed RMC may set it. three
	// separate reasons, each of which cost a real trip:
	//   - RMC only, because g_fix.has_time is sticky. a GGA reaching this block re-anchored the
	//     previous RMC's UTC against the current monotonic time, so the debug log's timestamps
	//     walked backwards inside every fix interval
	//   - time_from_fix, because a void RMC's date is whatever the module is guessing
	//   - the year window, because a bad date names a .dat file wrong and nothing takes it back
	if (result == NMEA_RESULT_RMC && g_fix.time_from_fix &&
	    nmea_time_plausible(g_fix.unix_time)) {
		bool was_valid = wallclock_valid();
		uint64_t frac_us = (uint64_t)g_fix.time_ms * 1000ULL;

		// the sentence says "unix_time plus time_ms", and it started arriving at rx_us, so the
		// whole second fell frac_us before that. it was anchored to the loop's now_us with the
		// fraction dropped, which set the clock back by up to 0.9s at every 10Hz fix and walked
		// the debug log's timestamps backwards. the clock runs late by the receiver's output
		// delay, a constant few tens of ms. wallclock_sync also keeps up with the pico's own
		// oscillator over a long ride
		if (rx_us > frac_us) {
			wallclock_sync(g_fix.unix_time, rx_us - frac_us);
		}

		if (!was_valid) {
			logger_debug("gps time acquired");
		}

		// the trip started before we knew the date, so it is still under a placeholder name
		if (logger_has_temp_name() && logger_name_from_clock()) {
			logger_debug("log renamed to '%s'", logger_filename());
		}
	} else if (result == NMEA_RESULT_RMC && g_fix.has_time && !g_fix.time_from_fix &&
	           !g_flg_badtime) {
		// the line whose absence made a 2080 file name a mystery. spelled out as a date rather
		// than an epoch count, because reading "2080" is the entire point of it
		wallclock_tm_t bad;

		wallclock_break_down(g_fix.unix_time, &bad);
		g_flg_badtime = true;

		logger_debug("gps: the receiver reports %04d-%02d-%02d %02d:%02d:%02d with no fix%s. "
			"ignoring it for the clock", bad.year, bad.month, bad.day, bad.hour, bad.minute,
			bad.second,
			nmea_time_plausible(g_fix.unix_time) ? "" : ", and that is not a plausible date");
	}

	queue_position(now_us, rx_us);

	// 0x7FD goes out with the GGA, not the RMC. every fifth fix this module sends GSA and GSV
	// between the two, and a frame stamped at the RMC's arrival was 45ms late on exactly those
	// fixes (20261023_182158.dat). stamping it at the GGA's arrival from the RMC would not do
	// either: the CAN frames of those 45ms may be in the file by then, and the record would be
	// pinned forward. GGA also carries the sats and HDOP, so they belong to the same fix.
	//
	// after the position, so a reader can pair it with the 0x7FF at or before it. it goes in
	// whether or not that one did: a standing bike writes no position, and its time still moves.
	// the date is the last fixed RMC's, so nothing goes in before the first one
	if (result == NMEA_RESULT_GGA && g_fix.quality > 0 && nmea_gga_unix_ms(&g_fix, &unix_ms)) {
		record_gps_time(&rec, unix_ms,
			record_fix_kind(g_fix.quality, g_fix.fix_mode, g_fix.has_altitude),
			g_fix.sats_used, g_fix.hdop);
		gps_pending_push(&rec, rx_us);
	}
}

// the 0x7FF and 0x7FE records of the fix g_fix now holds, if it is one worth writing
static void queue_position(uint64_t now_us, uint64_t rx_us)
{
	struct CANData rec;
	int32_t lon, lat;

	if (!g_fix.has_fix) {
		// losing a fix mid ride (a tunnel, a bad antenna connection) restarts the search, and the
		// periodic report below then shows whether it is coming back
		if (!g_flg_nofix) {
			g_flg_nofix = true;
			log_gps_state("fix lost", now_us);
			g_gps_search_start_us = now_us;
		}

		return;
	}

	if (g_flg_nofix) {
		g_flg_nofix = false;
		log_gps_state("fixed", now_us);
	}

	lon = record_encode_lon(g_fix.longitude);
	lat = record_encode_lat(g_fix.latitude);

	// GGA and RMC both carry the position, so a standing bike would otherwise write the same
	// point twice a second forever
	if (lon == g_lon_prev && lat == g_lat_prev) {
		return;
	}

	g_lon_prev = lon;
	g_lat_prev = lat;

	// both records carry the sentence's arrival time, because they describe the same fix
	record_gps_position(&rec, g_fix.latitude, g_fix.longitude);
	gps_pending_push(&rec, rx_us);

	// altitude and speed are missing while the fix is 2D only. the pi saw those as NaN from gpsd
	// and skipped this frame rather than logging garbage.
	//
	// the skip itself is unconditional: suppressing the message must never turn into writing a
	// record with half a fix in it, because that would change what is in the .dat file. only the
	// message waits for a full GGA+RMC pair, because at the very first fix exactly one half has
	// arrived and the other is legitimately missing for one sentence -- which read like a
	// receiver fault in the log when it was nothing of the kind
	if (!g_fix.has_altitude || !g_fix.has_speed) {
		if (!g_flg_noalt && g_seen_fixed_gga && g_seen_fixed_rmc) {
			g_flg_noalt = true;
			logger_debug("altitude or speed is not available. skip GPS data2 "
				"(alt:%d speed:%d)", g_fix.has_altitude, g_fix.has_speed);
		}

		return;
	}

	g_flg_noalt = false;

	record_gps_alt_speed(&rec, g_fix.altitude, g_fix.speed);
	gps_pending_push(&rec, rx_us);
}

// parses whatever has arrived and queues its records. one sentence makes at most three, so it stops
// while there is not room for that and leaves the rest in the ring for the next pass
static void handle_gps(uint64_t now_us)
{
	uint8_t c;
	int budget = GPS_RING_BYTES;

	while (budget-- > 0 && gps_pending_free() >= 3 && gps_pop(&c, &g_sentence_rx_us)) {
		const char *sentence = nmea_reader_push(&g_reader, (char)c);

		if (sentence != NULL) {
			handle_gps_sentence(sentence, now_us, g_sentence_rx_us);
		}
	}
}

// ---------------------------------------------------------------------------
// loop stalls
// ---------------------------------------------------------------------------

// split by phase, because the fix for a stall depends on where it is: record writes and the
// periodic f_sync go through different FatFs paths, and "other" is the debug log's own writes
static void check_loop_stall(uint64_t start_us, uint64_t end_us, uint64_t can_us,
	uint64_t gps_us, uint64_t sync_us)
{
	uint64_t total_us = end_us - start_us;
	uint64_t other_us;
	uint32_t total_ms = (uint32_t)(total_us / 1000ULL);

	if (total_ms < LOOP_STALL_WARN_MS) {
		return;
	}

	g_stall_count++;

	if (total_ms > g_stall_max_ms) {
		g_stall_max_ms = total_ms;
	}

	// rate limited, and the counts in the status line carry whatever this skips. the line is a
	// write to the same card, so an unthrottled one could keep a slow card stalling
	if (g_stall_reported &&
	    end_us - g_stall_reported_us < (uint64_t)LOOP_STALL_REPORT_MS * 1000ULL) {
		return;
	}

	g_stall_reported = true;
	g_stall_reported_us = end_us;

	other_us = total_us - can_us - gps_us - sync_us;

	logger_debug("main loop stalled %ums (records %u, gps %u, sync %u, other %u ms). "
		"gps ring peak:%u/%u. stalls so far:%u longest:%ums",
		(unsigned int)total_ms, (unsigned int)(can_us / 1000ULL),
		(unsigned int)(gps_us / 1000ULL), (unsigned int)(sync_us / 1000ULL),
		(unsigned int)(other_us / 1000ULL),
		(unsigned int)g_gps_ring_peak, (unsigned int)GPS_RING_BYTES,
		(unsigned int)g_stall_count, (unsigned int)g_stall_max_ms);
}

// ---------------------------------------------------------------------------
// card handling
// ---------------------------------------------------------------------------

// reads the settings off the card, or writes a default file if there is none. runs on every
// successful mount, so swapping in a card with a different settings file takes effect
static void load_settings(void)
{
	settings_file_result_t res;
	char tz[12];
	int bad = 0;

	res = settings_file_load(&g_settings, &bad);

	if (res == SETTINGS_FILE_ERROR) {
		logger_debug("settings: '%s' could not be read or created. using the built in defaults",
			SETTINGS_FILE);
	} else if (res == SETTINGS_FILE_CREATED) {
		logger_debug("settings: no '%s' on the card, wrote one with the defaults", SETTINGS_FILE);
	}

	// applying them is separate from reading them, so the defaults take effect even after an
	// unreadable card
	wallclock_set_tz(g_settings.tz_offset_sec);

	settings_format_tz(g_settings.tz_offset_sec, tz, sizeof(tz));

	logger_debug("settings: timezone %s%s", tz,
		(bad > 0) ? " [some lines were ignored]" : "");
}

static void mount_card(void);

// the trip is finalized in exactly one place: when the ignition goes away. an earlier version
// also watched the tachometer on the CAN bus, but that needed a decode rule and a set of
// thresholds per model, and the ignition line says the same thing on every bike.
//
// grep the debug log for "trigger:" to find it
// acts on what the ignition sampler found. this is the deadline path: on power off the log is
// closed for good, on a return the next trip starts in a new file
static void handle_ignition(void)
{
	power_event_t event = g_power_event;

	if (event == POWER_EVENT_NONE) {
		return;
	}

	g_power_event = POWER_EVENT_NONE;

	if (event == POWER_EVENT_OFF) {
#if IG_OFF_SURVIVAL_LOG
		// taken before the card work, so the measurement below covers the whole shutdown
		uint64_t now_us = time_us_64();
#endif
		logger_finalize_t result = logger_shutdown();

		g_powered_off = true;

		// the debug line is written after the card work, deliberately. it opens and closes
		// another file, and the trip log has first claim on the time that is left
		if (result == LOGGER_FINALIZE_DONE) {
			logger_debug("trigger: ignition off (%dmV). log closed and renamed as '%s'",
				g_ig_mv, logger_filename());
		} else {
			logger_debug("trigger: ignition off (%dmV). %s",
				g_ig_mv, logger_finalize_text(result));
		}

#if IG_OFF_SURVIVAL_LOG
		// how long the supply actually lasts is the number every deadline here is built on, and
		// it has only ever been calculated. keep writing until it gives out: whichever line makes
		// it onto the card last is the measurement. see config.h for turning this off
		{
			uint32_t step_ms = IG_OFF_SURVIVAL_STEP_MS;

			while (step_ms <= IG_OFF_SURVIVAL_MAX_MS) {
				while (time_us_64() - now_us < (uint64_t)step_ms * 1000ULL) {
					tight_loop_contents();
				}

				logger_debug("power still up %llums after ignition off",
					(unsigned long long)((time_us_64() - now_us) / 1000ULL));

				step_ms += IG_OFF_SURVIVAL_STEP_MS;
			}
		}
#endif

		// only now. logger_shutdown() leaves the card mounted precisely so the lines above can
		// reach motoreco.log rather than only the USB port nobody is plugged into on a bike
		logger_unmount();

		return;
	}

	// ignition back while we are still alive. that means the supply never actually went away,
	// which happens on the bench over USB, or if the key was flicked off and straight back on.
	// the closed file stays closed and the next trip gets its own. mount_card() also rereads the
	// settings for the new trip
	g_powered_off = false;
	logger_debug("ignition back on (%dmV). starting a new log", g_ig_mv);
	mount_card();
}

// the detect switch never gets a veto. it is one mechanical contact on the two smallest pads of
// the socket, and its polarity is an assumption the DM3AT drawing does not confirm, so a card is
// present when a card answers. the detect pin only colours the error message
static void mount_card(void)
{
	bool detected = sd_card_present();

	sd_reset_state();

	if (!logger_mount()) {
		if (!g_flg_sd_err) {
			g_flg_sd_err = true;

			if (!detected) {
				logger_debug("no card: the slot reads empty and nothing answered on the card spi");
			} else {
				logger_debug("a card is in the slot but no log file could be opened. "
					"exFAT or an unformatted card looks exactly like this, try FAT32");
			}
		}

		return;
	}

	g_flg_sd_err = false;
	reset_log_state();
	logger_debug("logging to '%s'", logger_filename());

	load_settings();

	// the card works and the detect line disagreed. say so once: everything keeps running, but
	// the removal check below is now off, and that pad is worth a look with a meter
	if (!detected) {
		logger_debug("note: card detect (GP7) reads empty with a working card. "
			"ignoring it. check pad 9/10 solder, or flip SD_CD_INSERTED_LEVEL");
	}
}

// a write failed, or the card was pulled. drop everything and let the retry timer bring it back.
// capture itself keeps running, the pi version made the same choice: losing the card must not
// take the rest of the logger down
static void drop_card(const char *why)
{
	logger_debug("sd error: %s", why);
	logger_unmount();
	sd_reset_state();
	g_flg_sd_err = true;
}

#ifdef CAN_SELF_TEST
static void can_self_test(void)
{
	const uint8_t payload[8] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04 };
	can_frame_t frame;
	int i;

	logger_debug("loopback self test: sending 0x123");

	if (!mcp25625_send(0x123, payload, 8)) {
		logger_debug("loopback self test: send failed");
		return;
	}

	for (i = 0; i < 100; i++) {
		if (mcp25625_pop(&frame)) {
			logger_debug("loopback self test: got id %03X dlc %d data %02X%02X%02X%02X",
				(unsigned int)frame.id, frame.dlc,
				frame.data[0], frame.data[1], frame.data[2], frame.data[3]);
			return;
		}

		sleep_ms(1);
	}

	logger_debug("loopback self test: nothing came back");
}
#endif

// ---------------------------------------------------------------------------

int main(void)
{
	uint64_t now_us;
	uint64_t last_sync_us = 0;
	uint64_t last_status_us = 0;
	uint64_t last_retry_us = 0;
	uint64_t last_gps_report_us = 0;
	uint64_t last_overflow_us = 0;
	uint64_t last_drop_report_us = 0;
	uint32_t reported_ring_drops = 0;
	bool can_ok;

	stdio_init_all();

	led_init();
	led_set(LED_ERROR);

	wallclock_init();
	nmea_init(&g_fix);
	nmea_reader_init(&g_reader);
	// the built in default is in effect until the card is mounted and its settings file read,
	// which is a second or two later. nothing that happens in between depends on it
	settings_defaults(&g_settings);
	wallclock_set_tz(g_settings.tz_offset_sec);

	reset_log_state();

	// the receiver starts out unfixed, so the first fix counts as an acquisition and gets logged
	g_flg_nofix = true;

	adc_init();
	adc_gpio_init(PIN_IG_SENSE);
	power_init(&g_power);

	// negative period means "this often", measured from the end of the callback
	add_repeating_timer_ms(-IG_POLL_INTERVAL_MS, ig_sample, NULL, &g_ig_timer);

	sd_spi_hw_init();
	gps_uart_init();

	// start listening for the module's rate straight away. nothing is sent until something
	// intelligible has been heard, because the rate it comes up at cannot be assumed
	g_gps_cfg_sent_us = time_us_64();

	g_rate_window_us = time_us_64();

	logger_debug("MotoRecoPico starting");

	mount_card();

#ifdef CAN_SELF_TEST
	can_ok = mcp25625_init(true);
#else
	can_ok = mcp25625_init(false);
#endif

	if (!can_ok) {
		// nothing else to do about it: without the controller there is no CAN data at all. keep
		// running so the GPS track is still recorded and the failure is visible in the log
		logger_debug("mcp25625 did not respond. check spi wiring and power");
	} else {
		logger_debug("can controller ready");
	}

#ifdef CAN_SELF_TEST
	if (can_ok) {
		can_self_test();
	}
#endif

	while (true) {
		uint64_t phase_us, can_us, gps_us, sync_us = 0;

		now_us = time_us_64();

		// before anything else. every millisecond spent elsewhere comes out of the 0.4s the
		// board has left once this fires
		handle_ignition();

#ifdef STALL_TEST_MS
		// bench only: stand in for a card that holds a write, to see the GPS records keep their
		// spacing across a stall. never in a build that goes on the bike
		{
			static uint64_t last_stall_test_us;

			if (now_us - last_stall_test_us >= 10000000ULL) {
				last_stall_test_us = now_us;
				busy_wait_ms(STALL_TEST_MS);
			}
		}
#endif

		phase_us = time_us_64();

		// GPS first: parsing only queues its records, and the CAN drain below needs them queued
		// to slot them in between the frames in time order
		handle_gps(now_us);

		gps_us = time_us_64() - phase_us;
		phase_us += gps_us;

		// no card: still drain both sources, or the CAN ring fills and the UART overruns
		if (!handle_can_frames() && logger_is_open()) {
			drop_card("write failed");
		}

		can_us = time_us_64() - phase_us;

		if (now_us - last_sync_us >= (uint64_t)LOG_FLUSH_INTERVAL_MS * 1000ULL) {
			last_sync_us = now_us;
			phase_us = time_us_64();

			if (logger_is_open() && !logger_sync()) {
				drop_card("sync failed");
			}

			sync_us = time_us_64() - phase_us;

			// a card pulled mid ride is worth catching early, but only where the detect line has
			// proven itself by agreeing with a card that actually answered. on a board where it
			// is miswired or chattering from vibration, this check would throw away a perfectly
			// good card once a second. without it, a real removal still surfaces as a failed
			// write on the next flush
			if (logger_is_open() && sd_detect_reliable() && !sd_card_present()) {
				drop_card("card removed");
			}
		}

		if (now_us - last_overflow_us >= 1000000ULL) {
			last_overflow_us = now_us;

			// frames the controller had to drop leave no trace in the .dat file. this is the
			// only evidence, exactly like the pi version's SO_RXQ_OVFL reporting
			g_overflow_total += mcp25625_check_overflow();
		}

		if (now_us - last_drop_report_us >= (uint64_t)DROP_REPORT_INTERVAL_MS * 1000ULL) {
			uint32_t drops = mcp25625_ring_drops();

			last_drop_report_us = now_us;

			if (g_overflow_total > 0 || drops != reported_ring_drops) {
				logger_debug("can rx overflow. controller episodes:%u ring drops:%u (+%u)",
					(unsigned int)g_overflow_total, (unsigned int)drops,
					(unsigned int)(drops - reported_ring_drops));
				reported_ring_drops = drops;
				g_overflow_total = 0;
			}
		}

		// while there is no fix, say what the receiver can see. this is the difference between
		// "the antenna sees nothing" and "it sees eight satellites and is still resolving them",
		// which otherwise both look like a GPS that takes too long
		if (!g_fix.has_fix &&
		    now_us - last_gps_report_us >= (uint64_t)GPS_REPORT_INTERVAL_MS * 1000ULL) {
			last_gps_report_us = now_us;

			if (!g_gps_seen_any) {
				// the module talks as soon as it is powered, fix or no fix, so silence is a
				// wiring or power fault rather than a reception problem
				logger_debug("gps silent: nothing received on uart0 in %us. "
					"check the module's TXD reaches GP1, its power, and the common ground",
					(unsigned int)((now_us - g_gps_search_start_us) / 1000000ULL));
			} else {
				log_gps_state("searching", now_us);
			}
		}

		gps_cfg_tick(now_us);

		if (now_us - g_rate_window_us >= (uint64_t)GPS_RATE_REPORT_MS * 1000ULL) {
			gps_rate_report(now_us);
		}

		// the receiver went quiet after having talked: a broken wire, a browned out module, or a
		// baud rate that drifted. distinct from never having said anything at all
		if (g_gps_seen_any && !g_flg_gps_silent &&
		    now_us - g_gps_last_sentence_us >= (uint64_t)GPS_SILENT_WARN_MS * 1000ULL) {
			g_flg_gps_silent = true;
			logger_debug("gps went quiet: no sentence for %ums after %u good ones",
				(unsigned int)((now_us - g_gps_last_sentence_us) / 1000ULL),
				(unsigned int)g_gps_sentences);
		}

		// retry a missing or broken card, but not while the ignition is off: there the log was
		// closed on purpose, and reopening one would undo that a few seconds later
		if (!logger_is_open() && !g_powered_off &&
		    now_us - last_retry_us >= (uint64_t)SD_RETRY_INTERVAL_MS * 1000ULL) {
			last_retry_us = now_us;
			mount_card();
		}

		if (now_us - last_status_us >= (uint64_t)STATUS_INTERVAL_MS * 1000ULL) {
			last_status_us = now_us;
			// ig is the raw pin level, not a battery voltage. this pin cannot measure the
			// battery on this board, see config.h
			// clamped: records the monotonic guard in stamp_record had to pin to the previous
			// timestamp. expected to stay at or near 0 now that CAN and GPS are merged in time
			// order; a number that climbs means the merge is letting records through out of order
			logger_debug("status: rx:%u records:%u clamped:%u stalls:%u longest:%ums ig:%dmV(%s) "
				"| gps fix:%d mode:%dD sats used:%d view:%d snr:%d hdop:%d.%d sentences:%u "
				"bad:%u uart_lost:%u",
				(unsigned int)mcp25625_rx_count(), (unsigned int)logger_records_written(),
				(unsigned int)g_clamped_records,
				(unsigned int)g_stall_count, (unsigned int)g_stall_max_ms,
				g_ig_mv, g_power.off ? "off" : "on",
				g_fix.has_fix, g_fix.fix_mode, g_fix.sats_used,
				nmea_sats_in_view(&g_fix), nmea_best_snr(&g_fix),
				hdop_tenths(g_fix.hdop) / 10, hdop_tenths(g_fix.hdop) % 10,
				(unsigned int)g_gps_sentences, (unsigned int)g_gps_bad,
				(unsigned int)g_gps_drops);
		}

		if (g_powered_off) {
			// closed cleanly and waiting for the supply to go. a blink here would be a lie, and
			// on the bench it would look like a card fault
			led_set(LED_IDLE);
		} else if (!logger_is_open()) {
			led_set(LED_ERROR);
		} else if (!g_fix.has_fix) {
			led_set(LED_NO_FIX);
		} else {
			led_set(LED_LOGGING);
		}

		led_update(now_us);

		// only with a log open. without a card the loop spends whole seconds retrying the mount on
		// purpose, and nothing is being recorded for a stall to cost
		if (logger_is_open()) {
			check_loop_stall(now_us, time_us_64(), can_us, gps_us, sync_us);
		}

		// nothing to do until the next interrupt. this is not a fixed rate loop, it just yields
		// the core between bursts of work
		tight_loop_contents();
	}

	return 0;
}
