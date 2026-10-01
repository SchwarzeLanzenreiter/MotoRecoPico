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

static void on_uart_rx(void)
{
	while (uart_is_readable(uart0)) {
		uint8_t c = (uint8_t)uart_getc(uart0);
		uint32_t next = (g_gps_head + 1) % GPS_RING_BYTES;

		if (next == g_gps_tail) {
			g_gps_drops++;
			continue;
		}

		g_gps_ring[g_gps_head] = c;
		g_gps_head = next;
	}
}

static bool gps_pop(uint8_t *out)
{
	uint32_t tail = g_gps_tail;

	if (tail == g_gps_head) {
		return false;
	}

	*out = g_gps_ring[tail];
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

// GPS diagnostics. a slow fix has several very different causes (nothing on the wire, wrong baud,
// no antenna, no sky, or simply a cold start) and they are indistinguishable from "not fixed yet"
// unless the receiver's own view of the world is recorded
static uint32_t g_gps_sentences;		// well formed sentences, all types
static uint32_t g_gps_bad;				// dropped on framing or checksum
static uint64_t g_gps_last_sentence_us;
static bool g_gps_seen_any;
static bool g_flg_gps_silent;
static uint64_t g_gps_search_start_us;	// when the current search began

static void reset_log_state(void)
{
	// the pi cleared these on every key on, so each trip's file is self contained. a restart at
	// the same spot would otherwise have its first fix suppressed as a duplicate
	g_time_base_set = false;
	g_lon_prev = 0;
	g_lat_prev = 0;
	g_flg_noalt = false;
	g_flg_nonstd = false;

	// g_flg_nofix is deliberately not touched here. it tracks whether the receiver currently has
	// a fix, not whether something was reported, so opening a new file must not disturb it
}

static void stamp_record(struct CANData *rec, uint64_t now_us)
{
	if (!g_time_base_set) {
		g_time_base_set = true;
		g_time_base_us = now_us;
	}

	record_set_elapsed(rec, now_us - g_time_base_us);
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
	while (budget-- > 0 && mcp25625_pop(&frame)) {
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

	return ok;
}

// ---------------------------------------------------------------------------
// GPS
// ---------------------------------------------------------------------------

// one line describing what the receiver currently sees. used for the periodic search report, the
// moment a fix appears, and the moment one is lost
static void log_gps_state(const char *what, uint64_t now_us)
{
	uint32_t elapsed_s = (uint32_t)((now_us - g_gps_search_start_us) / 1000000ULL);

	logger_debug("gps %s after %us: quality:%d mode:%dD sats used:%d view:%d tracked:%d "
		"best snr:%d hdop:%.1f sentences:%u bad:%u",
		what, (unsigned int)elapsed_s,
		g_fix.quality, g_fix.fix_mode, g_fix.sats_used,
		nmea_sats_in_view(&g_fix), nmea_sats_tracked(&g_fix), nmea_best_snr(&g_fix),
		(double)g_fix.hdop,
		(unsigned int)g_gps_sentences, (unsigned int)g_gps_bad);
}

static bool handle_gps_sentence(const char *sentence, uint64_t now_us)
{
	struct CANData rec;
	int32_t lon, lat;
	bool ok = true;
	int result = nmea_parse(&g_fix, sentence);

	if (result == NMEA_RESULT_INVALID) {
		// a few of these at power up are normal, the receiver may be mid sentence when the UART
		// starts. a steady stream of them means the baud rate is wrong
		g_gps_bad++;
		return true;
	}

	g_gps_sentences++;
	g_gps_last_sentence_us = now_us;
	g_flg_gps_silent = false;

	// the first sentence proves the wiring and the baud rate. worth its own line, because
	// everything that comes after assumes both
	if (!g_gps_seen_any) {
		char head[8];
		size_t i;

		g_gps_seen_any = true;

		for (i = 0; i < sizeof(head) - 1 && sentence[i] != '\0' && sentence[i] != ','; i++) {
			head[i] = sentence[i];
		}

		head[i] = '\0';
		logger_debug("gps talking: first sentence '%s' at %d baud", head, GPS_UART_BAUD);
	}

	if (result == NMEA_RESULT_IGNORED) {
		return true;
	}

	// the receiver is the only clock on the board. re-anchor on every sentence that carries a
	// date, which also corrects the drift of the pico's own oscillator over a long ride
	if (g_fix.has_time) {
		bool was_valid = wallclock_valid();

		wallclock_set(g_fix.unix_time, now_us);

		if (!was_valid) {
			logger_debug("gps time acquired");
		}

		// the trip started before we knew the date, so it is still under a placeholder name
		if (logger_has_temp_name() && logger_name_from_clock()) {
			logger_debug("log renamed to '%s'", logger_filename());
		}
	}

	if (!g_fix.has_fix) {
		// losing a fix mid ride (a tunnel, a bad antenna connection) restarts the search, and the
		// periodic report below then shows whether it is coming back
		if (!g_flg_nofix) {
			g_flg_nofix = true;
			log_gps_state("fix lost", now_us);
			g_gps_search_start_us = now_us;
		}

		return true;
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
		return true;
	}

	g_lon_prev = lon;
	g_lat_prev = lat;

	stamp_record(&rec, now_us);
	record_gps_position(&rec, g_fix.latitude, g_fix.longitude);

	if (!logger_write(&rec)) {
		ok = false;
	}

	// altitude and speed are missing while the fix is 2D only. the pi saw those as NaN from gpsd
	// and skipped this frame rather than logging garbage
	if (!g_fix.has_altitude || !g_fix.has_speed) {
		if (!g_flg_noalt) {
			g_flg_noalt = true;
			logger_debug("altitude or speed is not available. skip GPS data2");
		}

		return ok;
	}

	g_flg_noalt = false;

	stamp_record(&rec, now_us);
	record_gps_alt_speed(&rec, g_fix.altitude, g_fix.speed);

	if (!logger_write(&rec)) {
		ok = false;
	}

	return ok;
}

static bool handle_gps(uint64_t now_us)
{
	uint8_t c;
	bool ok = true;
	int budget = GPS_RING_BYTES;

	while (budget-- > 0 && gps_pop(&c)) {
		const char *sentence = nmea_reader_push(&g_reader, (char)c);

		if (sentence != NULL && !handle_gps_sentence(sentence, now_us)) {
			ok = false;
		}
	}

	return ok;
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
		now_us = time_us_64();

		// before anything else. every millisecond spent elsewhere comes out of the 0.4s the
		// board has left once this fires
		handle_ignition();

		if (logger_is_open()) {
			if (!handle_can_frames()) {
				drop_card("write failed");
			}

			if (logger_is_open() && !handle_gps(now_us)) {
				drop_card("write failed");
			}
		} else {
			// no card: still drain both sources, or the CAN ring fills and the UART overruns
			handle_can_frames();
			handle_gps(now_us);
		}

		if (now_us - last_sync_us >= (uint64_t)LOG_FLUSH_INTERVAL_MS * 1000ULL) {
			last_sync_us = now_us;

			if (logger_is_open() && !logger_sync()) {
				drop_card("sync failed");
			}

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
			logger_debug("status: rx:%u records:%u ig:%dmV(%s) | gps fix:%d mode:%dD "
				"sats used:%d view:%d snr:%d hdop:%.1f sentences:%u bad:%u uart_lost:%u",
				(unsigned int)mcp25625_rx_count(), (unsigned int)logger_records_written(),
				g_ig_mv, g_power.off ? "off" : "on",
				g_fix.has_fix, g_fix.fix_mode, g_fix.sats_used,
				nmea_sats_in_view(&g_fix), nmea_best_snr(&g_fix), (double)g_fix.hdop,
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

		// nothing to do until the next interrupt. this is not a fixed rate loop, it just yields
		// the core between bursts of work
		tight_loop_contents();
	}

	return 0;
}
