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

#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------------------
// pin assignment - board v4.1
//
// docs/netlist.md section 3.7 is the authority here. the assignment was chosen to shorten the
// board traces, not for logical tidiness, so do not "clean it up" without changing the board.
//
// v4.1 turned the Pico 180 degrees on the board, which moved every header pin to its diagonal
// opposite and forced a full reassignment. the part that is easy to miss: the two SPI blocks
// swapped instances as well, SD is now SPI0 and CAN is SPI1
// ---------------------------------------------------------------------------

// microSD on SPI0
#define PIN_SD_SCK		2
#define PIN_SD_MOSI		3
#define PIN_SD_MISO		4
#define PIN_SD_CS		5	// driven as plain GPIO, not the SPI hardware CSn
#define PIN_SD_CD		7	// card detect switch, external 10k pull up (R13)

// GPS GT-502MGG-N on UART0
#define PIN_GPS_TX		0	// pico -> module RXD (green wire)
#define PIN_GPS_RX		1	// module TXD (orange wire) -> pico
#define PIN_GPS_PPS		6	// not used yet. kept so the pin is not repurposed by accident

// green LED. the only indicator on the board. driven low side from 5V, so LOW lights it
#define PIN_LED			8

// CAN controller MCP25625 on SPI1
#define PIN_CAN_INT		9	// active low, open drain on the MCP25625 side
#define PIN_CAN_SCK		10
#define PIN_CAN_MOSI	11
#define PIN_CAN_MISO	12
#define PIN_CAN_CS		13	// driven as plain GPIO, not the SPI hardware CSn
#define PIN_CAN_STBY	20	// high puts the transceiver in standby
#define PIN_CAN_RESET	21	// active low, has a 10k pull up (R6)

// ignition sense. unchanged by the v4.1 rotation
#define PIN_IG_SENSE	28
#define IG_ADC_INPUT	2	// GP28 is ADC2

// GP14 to GP19, GP22, GP26 and GP27 are tied to GND on this board. never drive them

// which SPI block and which function a GPIO carries is fixed by the silicon: instance flips every
// 8 pins, function repeats every 4. the drivers assert their pins against these, so moving a pin
// in this file without moving the peripheral fails the build instead of the bike
#define SPI_INSTANCE_OF(gpio)	(((gpio) / 8) % 2)	// 0 = spi0, 1 = spi1
#define SPI_FUNC_OF(gpio)		((gpio) % 4)		// 0 = RX, 1 = CSn, 2 = SCK, 3 = TX
#define SPI_FUNC_RX		0
#define SPI_FUNC_CSN	1
#define SPI_FUNC_SCK	2
#define SPI_FUNC_TX		3

// card detect polarity: the switch is assumed to close to GND on insertion, so the pin reads low.
// the DM3AT drawing does not state which way the contact works, so this is an assumption.
//
// nothing depends on it being right. the firmware decides a card is present by whether one
// answers on SPI, and only trusts the detect pin for the mid ride removal check after the two
// have agreed once (sd_detect_reliable). getting this backwards costs the removal check only
#define SD_CD_INSERTED_LEVEL 0

// ---------------------------------------------------------------------------
// peripheral settings
// ---------------------------------------------------------------------------

#define CAN_SPI_BAUD		10000000	// 10MHz. MCP25625 allows 10MHz
#define SD_SPI_BAUD_INIT	400000		// the card spec requires <=400kHz until it is initialized
#define SD_SPI_BAUD_RUN		12500000	// raised after initialization

#define GPS_UART_BAUD		9600		// GT-502MGG-N. confirmed against the module on hardware

// ---------------------------------------------------------------------------
// CAN bit timing for a 16MHz crystal (Y1)
//
// TQ = 2 * (BRP + 1) / 16MHz. one bit is SyncSeg(1) + PropSeg + PhaseSeg1 + PhaseSeg2 TQ.
// all of the entries below sample at 75%, which is what CANopen/J1939 style buses expect.
//
//   CNF1 = SJW-1 (bit 7:6) | BRP (bit 5:0)
//   CNF2 = BTLMODE(1) | SAM(0) | PHSEG1-1 (bit 5:3) | PRSEG-1 (bit 2:0)
//   CNF3 = PHSEG2-1 (bit 2:0)
//
// 500kbps: BRP=0 -> TQ=125ns, 16TQ/bit = 2us, seg = 1 + 4 + 7 + 4
// ---------------------------------------------------------------------------

#define CAN_BITRATE_500K	0
#define CAN_BITRATE_250K	1
#define CAN_BITRATE_125K	2
#define CAN_BITRATE_1M		3

// most motorcycle buses this logger targets run at 500kbps. if nothing is ever received, suspect
// this first, then R8 (docs/netlist.md warns the 120 ohm terminator is always fitted)
#ifndef CAN_BITRATE
#define CAN_BITRATE		CAN_BITRATE_500K
#endif

// ---------------------------------------------------------------------------
// logging
// ---------------------------------------------------------------------------

#define LOG_DIR				""			// root of the card. keep the trailing slash if you set one
#define LOG_TMP_PREFIX		"TMP"		// TMPnnnnn.DAT until GPS gives us a wall clock
#define DEBUG_LOG_FILE		"motoreco.log"
#define DEBUG_LOG_FILE_OLD	"motoreco.log.1"
#define DEBUG_LOG_MAX_BYTES	1048576		// rotate at 1MB, one retained generation, as on the pi

#define LOG_FLUSH_INTERVAL_MS	1000	// f_sync interval. an abnormal power cut costs this much
#define DROP_REPORT_INTERVAL_MS	10000	// rate limit for the receive overflow report
#define SD_RETRY_INTERVAL_MS	5000	// how often a missing or broken card is retried
#define STATUS_INTERVAL_MS		30000	// periodic "still alive" line in the debug log.
										// 30s so that even a short test ride leaves at least one

// while there is no fix, report what the receiver can see this often. a cold start on this board
// is the normal case (J3 has no backup supply pin, so the module forgets its almanac at every key
// off) and can take tens of seconds, so the log has to show whether it is progressing or stuck
#define GPS_REPORT_INTERVAL_MS	10000
#define GPS_SILENT_WARN_MS		5000	// no sentence at all for this long means wiring or baud

// ---------------------------------------------------------------------------
// ignition off detection (docs/netlist.md section 4.7 is the contract)
//
// GP28 does NOT carry a divided battery voltage. since v3 it taps Q1's base node through R15,
// and Q1's base-emitter junction clamps that node to about 0.65V while the ignition is live,
// 0V when it is not. the battery voltage is not measurable on this board at all: what can be
// read is ignition on or off, and only with the ADC, because 0.65V is below the digital
// threshold of roughly 2V.
//
// the board guarantees 0.4s (typically 0.5s) of life after the key goes off, because C13 keeps
// Q2's gate charged while it bleeds off through R1. flush, close and rename have to fit in that,
// including an SD write stall of up to 250ms. the contract asks for a poll of 50ms or faster;
// this samples every 10ms from a timer interrupt so that a card stalled in the middle of a write
// cannot push the detection late as well
// ---------------------------------------------------------------------------

#define IG_OFF_MV				350		// 0.35V, the threshold netlist.md section 4.7 recommends
#define IG_ON_MV				500		// hysteresis. the live level is about 650mV
#define IG_POLL_INTERVAL_MS		10
#define IG_OFF_SAMPLES			2		// 20ms of the 400ms budget spent on being sure
#define IG_ON_SAMPLES			2

// measures how long the board really lives after the key goes off, by writing a line every so
// often until the supply gives out. the last line that survives is the answer.
//
// MEASURED on the bike 2026-09-04: the last line written was "power still up 550ms", and the
// next one at 600ms never made it, so the board lives roughly 0.55 to 0.6s. that confirms the
// 0.4s guarantee and the 0.5s typical figure in docs/netlist.md section 4.7, with margin.
//
// off again now that the number is known: it writes to the debug log while the supply is
// collapsing, and a torn write there can leave that one file needing a chkdsk. set it back to 1
// to re-measure, for instance after changing C13 or the load
#define IG_OFF_SURVIVAL_LOG		0
#define IG_OFF_SURVIVAL_STEP_MS	50
#define IG_OFF_SURVIVAL_MAX_MS	1000

// ---------------------------------------------------------------------------
// settings file
//
// the one thing that changes without a rebuild lives on the card instead of here: the time zone.
// see settings.c. the values below are only what gets written into a freshly created settings
// file, and what is used while no card is readable
// ---------------------------------------------------------------------------

#define SETTINGS_FILE		"MOTORECO.CFG"	// hidden, in the root of the card

// GPS reports UTC. the pi version named files in local time, so shift to keep them comparable
#define DEFAULT_TZ_TEXT		"+9"		// JST
#define DEFAULT_TZ_OFFSET_SEC	(9 * 3600)	// must match DEFAULT_TZ_TEXT


// receive ring between the CAN interrupt and the main loop. 4096 records is 64KB of the 520KB
// SRAM and covers about 2 seconds of a saturated 500kbps bus, which is far longer than any single
// SD write stall observed in practice
#define CAN_RING_RECORDS	4096

#define GPS_RING_BYTES		2048	// UART receive ring. one NMEA sentence is at most 82 bytes

// ---------------------------------------------------------------------------
// LED patterns (docs/netlist.md section 3.5)
// ---------------------------------------------------------------------------

#define LED_BLINK_NOFIX_MS	500		// 1Hz: logging, but GPS has no fix yet
#define LED_BLINK_ERROR_MS	100		// fast: no card or the card stopped accepting writes

#endif // CONFIG_H
