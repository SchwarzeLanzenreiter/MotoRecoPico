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

// tests for the parts that have no hardware in them: the NMEA parser, the .dat record encoding,
// the settings file, the ignition state machine and the wall clock. they are the only thing
// standing between a typo and a trip that MotoRecoViewer cannot open.
//
// two ways to run them:
//   - on the PC, with build.ps1 (MSVC) or the Makefile (gcc/clang)
//   - on any Pico, by building the motorecopico_tests target and reading the USB serial output.
//     no MotoRecoPico board needed, a bare Pico 2 is enough

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "gps_cmd.h"
#include "nmea.h"
#include "power.h"
#include "record.h"
#include "settings.h"
#include "wallclock.h"

static int g_failures = 0;
static int g_checks = 0;

static void check(int cond, const char *what)
{
	g_checks++;

	if (!cond) {
		g_failures++;
		printf("  FAIL: %s\n", what);
	}
}

static void check_close(double got, double expect, double tol, const char *what)
{
	g_checks++;

	if (!(fabs(got - expect) <= tol)) {
		g_failures++;
		printf("  FAIL: %s (got %.9f, expected %.9f)\n", what, got, expect);
	}
}

// ---------------------------------------------------------------------------
// MotoRecoViewer's side of the contract. these are DecodeRule.cs lines 400-422 transcribed, so
// the encoders are checked against the consumer instead of against themselves
// ---------------------------------------------------------------------------

static double viewer_longitude(const char d[8])
{
	const unsigned char *u = (const unsigned char *)d;
	return (u[3] * 16777216.0 + u[2] * 65536.0 + u[1] * 256.0 + u[0] - 180000000.0) / 1000000.0;
}

static double viewer_latitude(const char d[8])
{
	const unsigned char *u = (const unsigned char *)d;
	return (u[7] * 16777216.0 + u[6] * 65536.0 + u[5] * 256.0 + u[4] - 90000000.0) / 1000000.0;
}

static double viewer_altitude(const char d[8])
{
	const unsigned char *u = (const unsigned char *)d;
	return (u[3] * 16777216.0 + u[2] * 65536.0 + u[1] * 256.0 + u[0] - 10000000.0) / 10000.0;
}

static double viewer_speed_kmh(const char d[8])
{
	const unsigned char *u = (const unsigned char *)d;
	return (u[7] * 16777216.0 + u[6] * 65536.0 + u[5] * 256.0 + u[4]) / 1000000.0 * 3600 / 1000;
}

// ---------------------------------------------------------------------------

static void test_checksum(void)
{
	printf("checksum\n");

	check(nmea_checksum_ok("$GPGGA,123519,3540.8724,N,13946.0261,E,1,08,0.9,45.4,M,36.0,M,,*79"),
		"valid GGA checksum accepted");
	check(!nmea_checksum_ok("$GPGGA,123519,3540.8724,N,13946.0261,E,1,08,0.9,45.4,M,36.0,M,,*7A"),
		"wrong checksum rejected");
	check(!nmea_checksum_ok("$GPGGA,123519,3540.8724,N,13946.0261,E,1,08"),
		"missing checksum rejected");
	check(!nmea_checksum_ok("GPGGA,123519*79"),
		"missing dollar rejected");
	check(nmea_checksum_ok("$GNRMC,123519,A,3540.8724,N,13946.0261,E,022.4,084.4,230326,,,A*6E"),
		"GN talker accepted");
}

static void test_gga(void)
{
	nmea_fix_t fix;

	printf("GGA\n");

	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPGGA,123519,3540.8724,N,13946.0261,E,1,08,0.9,45.4,M,36.0,M,,*79")
		== NMEA_RESULT_GGA, "GGA recognised");
	check(fix.has_fix, "GGA quality 1 is a fix");
	check_close(fix.latitude, 35.0 + 40.8724 / 60.0, 1e-9, "GGA latitude");
	check_close(fix.longitude, 139.0 + 46.0261 / 60.0, 1e-9, "GGA longitude");
	check(fix.has_altitude, "GGA altitude present");
	check_close(fix.altitude, 45.4, 1e-9, "GGA altitude value");

	// quality 0 is the module talking before it has anything. this used to be gpsd's job to hide
	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPGGA,123519,,,,,0,00,,,M,,M,,*6B") == NMEA_RESULT_GGA,
		"unfixed GGA recognised");
	check(!fix.has_fix, "GGA quality 0 is not a fix");
	check(!fix.has_altitude, "no altitude without a fix");

	// 2D fix: position but no altitude field. the pi saw this as a NaN altitude from gpsd and
	// answered by skipping the 0x7FE frame, which is what has_altitude == 0 means here
	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPGGA,123519,3540.8724,N,13946.0261,E,1,04,2.1,,M,,M,,*7F")
		== NMEA_RESULT_GGA, "2D GGA recognised");
	check(fix.has_fix, "2D GGA still has a position");
	check(!fix.has_altitude, "2D GGA has no altitude");

	// southern and western hemispheres. sign handling is the classic place to get this wrong
	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPGGA,010203,3351.7000,S,15112.5000,W,2,10,0.8,12.5,M,20.0,M,,*73")
		== NMEA_RESULT_GGA, "southwest GGA recognised");
	check_close(fix.latitude, -(33.0 + 51.7 / 60.0), 1e-9, "south latitude is negative");
	check_close(fix.longitude, -(151.0 + 12.5 / 60.0), 1e-9, "west longitude is negative");
	check(fix.has_fix, "DGPS quality 2 counts as a fix");
}

static void test_rmc(void)
{
	nmea_fix_t fix;

	printf("RMC\n");

	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPRMC,123519,A,3540.8724,N,13946.0261,E,022.4,084.4,230326,,,A*70")
		== NMEA_RESULT_RMC, "RMC recognised");
	check(fix.has_fix, "RMC status A is a fix");
	check(fix.has_speed, "RMC speed present");
	check_close(fix.speed, 22.4 * 1852.0 / 3600.0, 1e-9, "knots converted to m/s");
	check(fix.has_time, "RMC time present");
	check(fix.time_from_fix, "a fixed RMC's date is backed by a fix");
	// 2026-03-23 12:35:19 UTC
	check(fix.unix_time == 1774269319LL, "RMC unix time");
	check(fix.time_ms == 0, "no fraction, no milliseconds");

	// a 10Hz receiver sends the fraction, and the wall clock has to keep it
	nmea_init(&fix);
	check(nmea_parse(&fix, "$GNRMC,123519.300,A,3540.8724,N,13946.0261,E,022.4,084.4,230326,,,A*73")
		== NMEA_RESULT_RMC, "RMC with milliseconds");
	check(fix.unix_time == 1774269319LL && fix.time_ms == 300, "123519.300 is :19 and 300ms");

	nmea_init(&fix);
	check(nmea_parse(&fix, "$GNRMC,123519.05,A,3540.8724,N,13946.0261,E,022.4,084.4,230326,,,A*45")
		== NMEA_RESULT_RMC, "RMC with two fraction digits");
	check(fix.time_ms == 50, "123519.05 is 50ms, not 5");

	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPRMC,123519,V,,,,,,,230326,,,N*58") == NMEA_RESULT_RMC,
		"void RMC recognised");
	check(!fix.has_fix, "RMC status V is not a fix");
	check(!fix.has_speed, "no speed without a fix");
	check(fix.has_time, "void RMC still carries the date");
	check(!fix.time_from_fix, "but a void RMC's date is not backed by a fix");

	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPRMC,000000,A,3540.8724,N,13946.0261,E,000.0,000.0,010126,,,A*73")
		== NMEA_RESULT_RMC, "standing still RMC recognised");
	check(fix.has_speed, "zero speed is still a speed");
	check_close(fix.speed, 0.0, 1e-12, "zero speed value");
	check(fix.unix_time == 1767225600LL, "midnight new year unix time");

	// the real failure this flag exists for: GGA does not carry a date, but has_time is sticky,
	// so every GGA used to re-anchor the wall clock with the previous RMC's UTC against the
	// current monotonic time. only an RMC may move the clock, and only a fixed one
	printf("RMC/GGA clock ownership\n");
	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPRMC,123519,A,3540.8724,N,13946.0261,E,022.4,084.4,230326,,,A*70")
		== NMEA_RESULT_RMC, "fixed RMC first");
	check(fix.time_from_fix, "clock may be set");

	check(nmea_parse(&fix, "$GPGGA,123519,3540.8724,N,13946.0261,E,1,08,0.9,45.4,M,36.0,M,,*79")
		== NMEA_RESULT_GGA, "then a GGA");
	check(fix.time_from_fix, "a GGA neither grants nor revokes the clock");
	check(fix.unix_time == 1774269319LL, "and a GGA does not move the timestamp");

	check(nmea_parse(&fix, "$GPRMC,123519,V,,,,,,,230326,,,N*58") == NMEA_RESULT_RMC,
		"then the fix is lost");
	check(fix.has_time, "the date survives losing the fix");
	check(!fix.time_from_fix, "but it may no longer set the clock");

	printf("time conversion\n");
	check(nmea_to_unix(1970, 1, 1, 0, 0, 0) == 0, "epoch");
	check(nmea_to_unix(2000, 3, 1, 0, 0, 0) == 951868800LL, "leap year 2000");
	check(nmea_to_unix(2024, 2, 29, 23, 59, 59) == 1709251199LL, "leap day 2024");
}

// the window that stops a receiver's cold start guess from naming a trip. the value in the
// "2080" check is the one this module really emitted on the bike, not an invented one
static void test_time_plausible(void)
{
	printf("time plausibility\n");

	check(!nmea_time_plausible(nmea_to_unix(2080, 1, 6, 8, 59, 42)),
		"the 2080 date the receiver reported during a cold search is rejected");
	check(nmea_time_plausible(nmea_to_unix(2026, 10, 13, 11, 50, 21)),
		"the real fix from the same run is accepted");
	check(!nmea_time_plausible(0), "the epoch is not a plausible fix time");
	check(!nmea_time_plausible(-1), "nor is anything before it");
	check(nmea_time_plausible(nmea_to_unix(GPS_YEAR_MIN, 1, 1, 0, 0, 0)),
		"the first instant of the window is inside it");
	check(!nmea_time_plausible(nmea_to_unix(GPS_YEAR_MIN, 1, 1, 0, 0, 0) - 1),
		"one second earlier is outside");
	check(nmea_time_plausible(nmea_to_unix(GPS_YEAR_MAX, 12, 31, 23, 59, 59)),
		"the last second of the window is inside it");
	check(!nmea_time_plausible(nmea_to_unix(GPS_YEAR_MAX + 1, 1, 1, 0, 0, 0)),
		"the year after the window is outside");
}

// GSA and GSV carry no position, only the diagnostics that explain a slow fix
static void test_diagnostics(void)
{
	nmea_fix_t fix;

	printf("GSA / GSV\n");

	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39") == NMEA_RESULT_GSA,
		"GSA recognised");
	check(fix.fix_mode == 3, "GSA reports a 3D fix");
	check_close(fix.pdop, 2.5, 1e-9, "PDOP");
	check_close(fix.hdop, 1.3, 1e-9, "HDOP from GSA");
	check_close(fix.vdop, 2.1, 1e-9, "VDOP");

	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPGSA,A,1,,,,,,,,,,,,,,,*1E") == NMEA_RESULT_GSA,
		"unfixed GSA recognised");
	check(fix.fix_mode == 1, "GSA reports no fix");

	// GGA's HDOP wins over GSA's, so a receiver reporting both cannot end up with the older one
	nmea_init(&fix);
	nmea_parse(&fix, "$GPGGA,123519,3540.8724,N,13946.0261,E,1,08,0.9,45.4,M,36.0,M,,*79");
	check(fix.sats_used == 8, "GGA satellite count");
	check_close(fix.hdop, 0.9, 1e-9, "HDOP from GGA");
	nmea_parse(&fix, "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39");
	check_close(fix.hdop, 0.9, 1e-9, "GSA does not overwrite GGA's HDOP");

	// a searching receiver still reports what it can see. this is the case the log has to show
	nmea_init(&fix);
	nmea_parse(&fix, "$GPGGA,123519,,,,,0,03,,,M,,M,,*68");
	check(!fix.has_fix, "still searching");
	check(fix.sats_used == 3, "satellites reported while searching");
	check(fix.quality == 0, "quality 0 recorded");

	// a two sentence GSV cycle: 7 in view, 5 of them with a signal
	nmea_init(&fix);
	check(nmea_parse(&fix, "$GPGSV,2,1,07,04,45,120,32,05,30,200,28,09,10,050,,12,60,310,41*7B")
		== NMEA_RESULT_GSV, "GSV recognised");
	check(nmea_parse(&fix, "$GPGSV,2,2,07,24,20,090,19,25,05,150,,30,70,270,35*4A")
		== NMEA_RESULT_GSV, "second GSV recognised");
	check(nmea_sats_in_view(&fix) == 7, "satellites in view");
	check(nmea_sats_tracked(&fix) == 5, "satellites with a signal");
	check(nmea_best_snr(&fix) == 41, "best signal to noise ratio");

	// the next cycle must replace the last one, not add to it. a receiver losing satellites has
	// to show fewer, or the log would never reveal an antenna going bad
	check(nmea_parse(&fix, "$GPGSV,1,1,02,04,45,120,20,05,30,200,18*72") == NMEA_RESULT_GSV,
		"new GSV cycle");
	check(nmea_sats_in_view(&fix) == 2, "in view replaced, not accumulated");
	check(nmea_sats_tracked(&fix) == 2, "tracked replaced, not accumulated");
	check(nmea_best_snr(&fix) == 20, "best snr replaced");

	// two constellations are counted together, each with its own cycle
	nmea_init(&fix);
	nmea_parse(&fix, "$GPGSV,1,1,03,04,45,120,32,05,30,200,28,09,10,050,22*4E");
	nmea_parse(&fix, "$GLGSV,1,1,02,68,40,100,30,69,25,180,26*6A");
	check(nmea_sats_in_view(&fix) == 5, "in view summed across talkers");
	check(nmea_sats_tracked(&fix) == 5, "tracked summed across talkers");
	check(nmea_best_snr(&fix) == 32, "best snr across talkers");
}

static void test_reader(void)
{
	nmea_reader_t reader;
	const char *line = NULL;
	const char *stream = "$GPGGA,1*00\r\n$GPRMC,2*00\r\n";
	int lines = 0;
	size_t i;
	char big[NMEA_MAX_SENTENCE + 40];

	printf("reader\n");

	nmea_reader_init(&reader);

	for (i = 0; i < strlen(stream); i++) {
		line = nmea_reader_push(&reader, stream[i]);

		if (line != NULL) {
			lines++;

			if (lines == 1) {
				check(strcmp(line, "$GPGGA,1*00") == 0, "first sentence assembled");
			} else {
				check(strcmp(line, "$GPRMC,2*00") == 0, "second sentence assembled");
			}
		}
	}

	check(lines == 2, "two sentences from the stream");

	// a lost byte must not glue two sentences together into one plausible looking line
	nmea_reader_init(&reader);
	check(nmea_reader_push(&reader, '$') == NULL, "start of sentence");
	check(nmea_reader_push(&reader, 'A') == NULL, "partial sentence");
	check(nmea_reader_push(&reader, '$') == NULL, "restart on dollar");
	check(nmea_reader_push(&reader, 'B') == NULL, "partial sentence again");
	line = NULL;
	line = nmea_reader_push(&reader, '\n');
	check(line != NULL && strcmp(line, "$B") == 0, "resynced on the second dollar");

	// an oversized line is dropped whole rather than truncated into something parseable
	memset(big, 'X', sizeof(big));
	big[0] = '$';
	nmea_reader_init(&reader);

	for (i = 0; i < sizeof(big); i++) {
		check(nmea_reader_push(&reader, big[i]) == NULL, "no sentence from an overlong line");
	}

	check(nmea_reader_push(&reader, '\n') == NULL, "overlong line dropped at the terminator");
}

static uint64_t le48(const unsigned char *u)
{
	uint64_t v = 0;
	int i;

	for (i = 5; i >= 0; i--) {
		v = (v << 8) | u[i];
	}

	return v;
}

static void test_record(void)
{
	struct CANData rec;
	const unsigned char *u = (const unsigned char *)rec.data;
	int32_t expect;

	printf("record encoding\n");

	memset(&rec, 0, sizeof(rec));
	record_set_elapsed(&rec, 0);
	check(rec.second == 0 && rec.mirisecond == 0, "zero elapsed");

	record_set_elapsed(&rec, 1234567ULL);
	check(rec.second == 1 && rec.mirisecond == 234, "elapsed truncates to whole ms");

	record_set_elapsed(&rec, 3599999999ULL);
	check(rec.second == 3599 && rec.mirisecond == 999, "elapsed near an hour");

	// values chosen to be exact in binary floating point, so the expected integer is unambiguous
	record_gps_position(&rec, 35.5, 139.25);
	check(rec.id == GPS_CAN_ID_NUM1, "position frame uses 0x7FF");

	expect = 139250000 + 180000000;
	check(u[0] == (expect & 0xFF) && u[1] == ((expect >> 8) & 0xFF) &&
	      u[2] == ((expect >> 16) & 0xFF) && u[3] == ((expect >> 24) & 0xFF),
		"longitude is little endian in bytes 0-3");

	expect = 35500000 + 90000000;
	check(u[4] == (expect & 0xFF) && u[5] == ((expect >> 8) & 0xFF) &&
	      u[6] == ((expect >> 16) & 0xFF) && u[7] == ((expect >> 24) & 0xFF),
		"latitude is little endian in bytes 4-7");

	check_close(viewer_longitude(rec.data), 139.25, 1e-9, "viewer decodes the longitude back");
	check_close(viewer_latitude(rec.data), 35.5, 1e-9, "viewer decodes the latitude back");

	record_gps_alt_speed(&rec, 40.0, 10.0);
	check(rec.id == GPS_CAN_ID_NUM2, "altitude frame uses 0x7FE");
	check_close(viewer_altitude(rec.data), 40.0, 1e-9, "viewer decodes the altitude back");
	check_close(viewer_speed_kmh(rec.data), 36.0, 1e-9, "10 m/s reads as 36 km/h");

	// the corners of the encoding. altitude uses a smaller factor precisely because 1000000 would
	// overflow int32 above 1147m, so make sure a real mountain pass still comes back
	record_gps_position(&rec, -33.5, -70.75);
	check_close(viewer_latitude(rec.data), -33.5, 1e-9, "southern latitude round trip");
	check_close(viewer_longitude(rec.data), -70.75, 1e-9, "western longitude round trip");

	record_gps_alt_speed(&rec, 2172.0, 0.0);
	check_close(viewer_altitude(rec.data), 2172.0, 1e-6, "high altitude round trip");
	check_close(viewer_speed_kmh(rec.data), 0.0, 1e-12, "zero speed round trip");

	record_gps_alt_speed(&rec, -410.0, 0.0);
	check_close(viewer_altitude(rec.data), -410.0, 1e-6, "below sea level round trip");

	// 2026-03-23 12:35:19.300 UTC, the RMC used throughout this file
	record_gps_time(&rec, 1774269319300LL, GPS_FIX_KIND_3D, 8, 0.9);
	check(rec.id == GPS_CAN_ID_NUM3, "time frame uses 0x7FD");
	check(rec.id == 0x7FD, "0x7FD is 2045");
	check(le48(u) == 1774269319300ULL, "fix time is 48 bit little endian in bytes 0-5");
	check(u[6] == 0x28, "3D in the high nibble, 8 satellites in the low one");
	check(u[7] == 9, "HDOP 0.9 is stored as 9");

	// the far end of 48 bits: year 10889, which is the point of six bytes
	record_gps_time(&rec, 0xFFFFFFFFFFFFLL, GPS_FIX_KIND_DIFF, 12, 1.25);
	check(le48(u) == 0xFFFFFFFFFFFFULL, "the whole 48 bits survive");
	check(u[6] == 0x3C, "differential, 12 satellites");
	check(u[7] == 13, "HDOP rounds to the nearest tenth");

	record_gps_time(&rec, 0, GPS_FIX_KIND_2D, 24, 40.0);
	check(u[6] == 0x1F, "satellites stop at 15");
	check(u[7] == 255, "HDOP stops at 25.5");

	record_gps_time(&rec, 0, GPS_FIX_KIND_2D, 4, 0.0);
	check(u[7] == 0, "HDOP not reported stays 0");

	record_gps_time(&rec, 0, GPS_FIX_KIND_2D, 4, 0.01);
	check(u[7] == 1, "a tiny HDOP does not turn into \"not reported\"");

	check(record_fix_kind(1, 3, 1) == GPS_FIX_KIND_3D, "GPS fix, GSA says 3D");
	check(record_fix_kind(1, 2, 1) == GPS_FIX_KIND_2D, "GSA's 2D wins over a stale altitude");
	check(record_fix_kind(2, 3, 1) == GPS_FIX_KIND_DIFF, "GGA quality 2 is differential");
	check(record_fix_kind(4, 3, 1) == GPS_FIX_KIND_DIFF, "RTK fixed counts as differential");
	check(record_fix_kind(1, 0, 1) == GPS_FIX_KIND_3D, "no GSA yet, an altitude means 3D");
	check(record_fix_kind(1, 0, 0) == GPS_FIX_KIND_2D, "no GSA yet, no altitude means 2D");
}

// ---------------------------------------------------------------------------
// settings file
// ---------------------------------------------------------------------------

static void test_settings(void)
{
	settings_t s;
	int sec;
	char tz[12];

	printf("settings\n");

	settings_defaults(&s);
	check(s.tz_offset_sec == DEFAULT_TZ_OFFSET_SEC, "default timezone");
	check(s.gps_rate_ms == DEFAULT_GPS_RATE_MS, "default gps rate");
	check(s.gps_baud == DEFAULT_GPS_BAUD, "default gps baud");

	// the text written into a new file has to parse back into those same defaults, or a card
	// created by the logger would not agree with the logger
	{
		settings_t from_file;
		const char *text = settings_default_file_text();
		const char *p = text;
		char line[160];

		settings_defaults(&from_file);
		from_file.tz_offset_sec = 0;
		from_file.gps_rate_ms = -1;
		from_file.gps_baud = -1;

		while (*p != '\0') {
			const char *nl = strchr(p, '\n');
			size_t len = (nl != NULL) ? (size_t)(nl - p) : strlen(p);

			if (len >= sizeof(line)) {
				len = sizeof(line) - 1;
			}

			memcpy(line, p, len);
			line[len] = '\0';

			check(settings_parse_line(&from_file, line) != SETTINGS_LINE_BAD,
				"every line of the generated file parses");

			p = (nl != NULL) ? nl + 1 : p + strlen(p);
		}

		check(from_file.tz_offset_sec == DEFAULT_TZ_OFFSET_SEC,
			"generated file round trips the timezone");
		check(from_file.gps_rate_ms == DEFAULT_GPS_RATE_MS,
			"generated file round trips the gps rate");
		check(from_file.gps_baud == DEFAULT_GPS_BAUD,
			"generated file round trips the gps baud");
	}

	// comments and blank lines
	settings_defaults(&s);
	check(settings_parse_line(&s, "# timezone = +99") == SETTINGS_LINE_SKIPPED, "comment ignored");
	check(settings_parse_line(&s, "   # indented comment") == SETTINGS_LINE_SKIPPED,
		"indented comment ignored");
	check(settings_parse_line(&s, "") == SETTINGS_LINE_SKIPPED, "blank line ignored");
	check(settings_parse_line(&s, "   \t ") == SETTINGS_LINE_SKIPPED, "whitespace line ignored");
	check(s.tz_offset_sec == DEFAULT_TZ_OFFSET_SEC, "comments changed nothing");

	// the settings themselves, in the shapes someone might actually type
	settings_defaults(&s);
	check(settings_parse_line(&s, "timezone = +9") == SETTINGS_LINE_APPLIED, "timezone applied");
	check(s.tz_offset_sec == 9 * 3600, "+9 is nine hours");

	check(settings_parse_line(&s, "  TimeZone=-5  ") == SETTINGS_LINE_APPLIED,
		"key is case insensitive and whitespace tolerant");
	check(s.tz_offset_sec == -5 * 3600, "-5 is minus five hours");

	check(settings_parse_line(&s, "timezone 7") == SETTINGS_LINE_APPLIED,
		"the equals sign is optional");
	check(s.tz_offset_sec == 7 * 3600, "bare 7 means +7");

	// the fix rate is in the settings file so that sweeping it does not need a rebuild
	check(settings_parse_line(&s, "gps_rate = 100") == SETTINGS_LINE_APPLIED, "10Hz accepted");
	check(s.gps_rate_ms == 100, "100ms stored");
	check(settings_parse_line(&s, "gps_rate = 10000") == SETTINGS_LINE_APPLIED,
		"the slowest the guide allows");
	check(settings_parse_line(&s, "gps_rate = 0") == SETTINGS_LINE_APPLIED,
		"0 is allowed: leave the receiver alone");
	check(s.gps_rate_ms == 0, "0 stored");

	// outside PMTK220's documented range
	check(settings_parse_line(&s, "gps_rate = 99") == SETTINGS_LINE_BAD, "faster than 10Hz");
	check(settings_parse_line(&s, "gps_rate = 10001") == SETTINGS_LINE_BAD, "slower than 10s");
	check(s.gps_rate_ms == 0, "a rejected rate changes nothing");

	// the baud is in the file for the same reason, and because it is the way back: a module left
	// at 115200 is put back by writing 9600 here
	check(settings_parse_line(&s, "gps_baud = 115200") == SETTINGS_LINE_APPLIED,
		"115200 accepted");
	check(s.gps_baud == 115200, "115200 stored");
	check(settings_parse_line(&s, "gps_baud = 9600") == SETTINGS_LINE_APPLIED,
		"9600 accepted, the way back");
	check(s.gps_baud == 9600, "9600 stored");
	check(settings_parse_line(&s, "gps_baud = 0") == SETTINGS_LINE_APPLIED,
		"0 is allowed: leave the baud alone");
	check(s.gps_baud == 0, "0 stored");

	// a baud outside the guide's PMTK251 table can never be asked for, so it is a typo, not a
	// setting. accepting it would make the probe settle back on 9600 every boot and the log
	// would read as if the module were at fault
	check(settings_parse_line(&s, "gps_baud = 115201") == SETTINGS_LINE_BAD,
		"a baud the module cannot be asked for");
	check(settings_parse_line(&s, "gps_baud = -9600") == SETTINGS_LINE_BAD, "a negative baud");
	check(settings_parse_line(&s, "gps_baud = banana") == SETTINGS_LINE_BAD, "a non numeric baud");
	check(s.gps_baud == 0, "a rejected baud changes nothing");

	// 'vehicle' selected the engine rpm decode back when the trip was closed on the engine
	// stopping. cards written by that firmware still carry the line, so it has to stay quiet
	// rather than be reported as junk on every boot
	check(settings_parse_line(&s, "vehicle = K51") == SETTINGS_LINE_SKIPPED,
		"the obsolete vehicle key is skipped in silence");
	check(settings_parse_line(&s, "vehicle = anything") == SETTINGS_LINE_SKIPPED,
		"whatever it says");
	check(s.tz_offset_sec == 7 * 3600, "and it changes nothing");

	// malformed lines
	check(settings_parse_line(&s, "timezone = banana") == SETTINGS_LINE_BAD, "bad timezone");
	check(settings_parse_line(&s, "timezone =") == SETTINGS_LINE_BAD, "empty timezone");
	check(settings_parse_line(&s, "colour = red") == SETTINGS_LINE_BAD, "unknown key");
	check(settings_parse_line(&s, "nonsense") == SETTINGS_LINE_BAD, "line with no value");

	printf("timezone parsing\n");
	check(settings_parse_tz("+8", &sec) && sec == 8 * 3600, "+8");
	check(settings_parse_tz("8", &sec) && sec == 8 * 3600, "8 without a sign");
	check(settings_parse_tz("-11", &sec) && sec == -11 * 3600, "-11");
	check(settings_parse_tz("0", &sec) && sec == 0, "UTC");
	check(settings_parse_tz("+5:30", &sec) && sec == 5 * 3600 + 1800, "+5:30");
	check(settings_parse_tz("+5.5", &sec) && sec == 5 * 3600 + 1800, "+5.5 is half an hour");
	check(settings_parse_tz("+5.75", &sec) && sec == 5 * 3600 + 2700, "+5.75 is 45 minutes");
	check(settings_parse_tz("+14", &sec) && sec == 14 * 3600, "+14 exists (Kiritimati)");
	check(!settings_parse_tz("+15", &sec), "+15 does not exist");
	check(!settings_parse_tz("-13", &sec), "-13 does not exist");
	check(!settings_parse_tz("+5:70", &sec), "70 minutes rejected");
	check(!settings_parse_tz("+5:3", &sec), "single digit minutes rejected");
	check(!settings_parse_tz("", &sec), "empty rejected");
	check(!settings_parse_tz("+", &sec), "sign alone rejected");
	check(!settings_parse_tz("9h", &sec), "trailing junk rejected");
	check(!settings_parse_tz("+123", &sec), "three digit hours rejected");

	settings_format_tz(8 * 3600, tz, sizeof(tz));
	check(strcmp(tz, "+08:00") == 0, "formats +08:00");
	settings_format_tz(-(5 * 3600 + 1800), tz, sizeof(tz));
	check(strcmp(tz, "-05:30") == 0, "formats -05:30");
}

// ---------------------------------------------------------------------------
// ignition detection
//
// the pin is a clamped logic level: about 650mV while the ignition is live, 0mV when it is not.
// getting this wrong either loses the 0.4s shutdown window or closes the log mid ride
// ---------------------------------------------------------------------------

static void test_power(void)
{
	power_state_t p;
	int i;

	printf("ignition detection\n");

	// the normal sequence: ignition live, then gone
	power_init(&p);
	check(power_eval(&p, 650) == POWER_EVENT_NONE, "first live sample arms nothing yet");

	for (i = 0; i < 50; i++) {
		check(power_eval(&p, 650) == POWER_EVENT_NONE, "steady ignition is not an event");
	}

	check(p.armed, "armed after seeing the ignition live");
	check(power_eval(&p, 0) == POWER_EVENT_NONE, "one low sample is not enough");
	check(power_eval(&p, 0) == POWER_EVENT_OFF, "off after the second low sample");
	check(p.off, "state says off");

	// and it must fire exactly once, or every loop would try to close an already closed log
	for (i = 0; i < 50; i++) {
		check(power_eval(&p, 0) == POWER_EVENT_NONE, "off fires once");
	}

	// key back on while still alive: bench on USB, or the key flicked back
	check(power_eval(&p, 650) == POWER_EVENT_NONE, "one live sample is not enough to restore");
	check(power_eval(&p, 650) == POWER_EVENT_RESTORED, "restored after the second");
	check(!p.off, "state says on again");

	for (i = 0; i < 50; i++) {
		check(power_eval(&p, 650) == POWER_EVENT_NONE, "restore fires once");
	}

	check(power_eval(&p, 0) == POWER_EVENT_NONE, "second cycle needs two samples too");
	check(power_eval(&p, 0) == POWER_EVENT_OFF, "second key off detected");

	// on the bench with nothing driving the pin, it reads 0 from the start. that must never look
	// like a key off, or a developer plugging in USB would get a shutdown every boot
	power_init(&p);

	for (i = 0; i < 500; i++) {
		check(power_eval(&p, 0) == POWER_EVENT_NONE, "never armed, never fires");
	}

	check(!p.armed, "still not armed");
	check(!p.off, "and not marked off");

	// a single dropout between two live samples must not trip it. IG_OFF_SAMPLES is what buys
	// this, and the counter has to reset on the live sample
	power_init(&p);
	power_eval(&p, 650);
	power_eval(&p, 650);
	check(power_eval(&p, 0) == POWER_EVENT_NONE, "first glitch sample");
	check(power_eval(&p, 650) == POWER_EVENT_NONE, "recovered");
	check(power_eval(&p, 0) == POWER_EVENT_NONE, "glitch again, counter had reset");
	check(!p.off, "a single sample glitch never closes the log");

	// the band between the thresholds is only crossed in transit and must decide nothing
	power_init(&p);
	power_eval(&p, 650);
	power_eval(&p, 650);

	for (i = 0; i < 20; i++) {
		check(power_eval(&p, (IG_OFF_MV + IG_ON_MV) / 2) == POWER_EVENT_NONE,
			"the grey band alone changes nothing");
	}

	check(!p.off, "still on after sitting in the band");

	// exact threshold behaviour, since these are the numbers the hardware document specifies
	power_init(&p);
	power_eval(&p, IG_ON_MV);
	power_eval(&p, IG_ON_MV);
	check(p.armed, "IG_ON_MV itself counts as live");
	power_eval(&p, IG_OFF_MV);
	check(power_eval(&p, IG_OFF_MV) == POWER_EVENT_NONE, "IG_OFF_MV itself is not yet off");
	power_eval(&p, IG_OFF_MV - 1);
	check(power_eval(&p, IG_OFF_MV - 1) == POWER_EVENT_OFF, "just below IG_OFF_MV is off");

	check(p.mv == IG_OFF_MV - 1, "last sample is kept for reporting");
}

// ---------------------------------------------------------------------------
// wall clock
//
// the clock is anchored when GPS first reports the time, and then asked about moments both after
// and BEFORE that anchor: a trip file is named after when it was opened, which is always earlier
// than the first fix
// ---------------------------------------------------------------------------

static void test_wallclock(void)
{
	wallclock_tm_t tm;

	printf("wall clock\n");

	wallclock_init();
	wallclock_set_tz(9 * 3600);
	check(!wallclock_valid(), "no clock before the first fix");
	check(!wallclock_local(1000000, &tm), "and nothing can be asked of it");

	// 2026-09-04 05:59:17 UTC seen 1.5s after boot. JST is nine hours ahead
	wallclock_set(1788501557LL, 1500000);
	check(wallclock_valid(), "clock is set");

	check(wallclock_local(1500000, &tm), "at the anchor");
	check(tm.year == 2026 && tm.month == 9 && tm.day == 4, "anchor date");
	check(tm.hour == 14 && tm.minute == 59 && tm.second == 17, "anchor time in JST");
	check(tm.millisecond == 0, "anchor has no fraction");

	// after the anchor
	check(wallclock_local(1500000 + 2500000, &tm), "2.5s later");
	check(tm.second == 19 && tm.millisecond == 500, "2.5s later reads 19.500");

	// before the anchor. this is the one that produced '5865800921_230104.dat' on a real ride,
	// because the subtraction was unsigned and wrapped
	check(wallclock_local(274000, &tm), "before the anchor");
	check(tm.year == 2026 && tm.month == 9 && tm.day == 4, "date before the anchor is sane");
	check(tm.hour == 14 && tm.minute == 59 && tm.second == 15, "1.226s before the anchor");
	check(tm.millisecond == 774, "and the fraction carries correctly");

	// far enough before the anchor to cross a minute, an hour and a day boundary backwards
	wallclock_init();
	wallclock_set_tz(9 * 3600);
	wallclock_set(1788501557LL, 60ULL * 60ULL * 1000000ULL);	// anchored an hour after boot
	check(wallclock_local(0, &tm), "at boot, an hour before the anchor");
	check(tm.hour == 13 && tm.minute == 59 && tm.second == 17, "an hour earlier");

	// UTC and a negative zone, since the offset is applied after the delta
	wallclock_init();
	wallclock_set_tz(0);
	wallclock_set(1788501557LL, 1000000);
	check(wallclock_local(1000000, &tm) && tm.hour == 5 && tm.minute == 59, "UTC");

	wallclock_set_tz(-5 * 3600);
	check(wallclock_local(1000000, &tm) && tm.hour == 0 && tm.minute == 59, "UTC-5");

	// a zone that pushes across midnight backwards
	wallclock_set_tz(-8 * 3600);
	check(wallclock_local(1000000, &tm), "UTC-8");
	check(tm.day == 3 && tm.hour == 21 && tm.minute == 59, "previous day in UTC-8");

	// sync: anchors an unset clock, then leaves it alone while the fixes agree with it
	printf("wall clock sync\n");
	wallclock_init();
	wallclock_set_tz(0);
	check(wallclock_sync(1788501557LL, 2000000), "an unset clock is anchored");
	check(wallclock_local(2000000, &tm) && tm.second == 17 && tm.millisecond == 0, "at :17.000");

	// the next second's fix arrives 7ms late, the sort of wobble a sentence has
	check(!wallclock_sync(1788501558LL, 3007000), "7ms of wobble does not re-anchor");
	check(wallclock_local(3007000, &tm) && tm.second == 18 && tm.millisecond == 7,
		"so the clock keeps running from the first anchor");

	// and early by as much: still left alone, so the log never steps backwards
	check(!wallclock_sync(1788501559LL, 3993000), "7ms early does not re-anchor either");

	// drifted past the tolerance: re-anchored
	check(wallclock_sync(1788501600LL, 45100000), "100ms of drift re-anchors");
	check(wallclock_local(45100000, &tm) && tm.minute == 0 && tm.second == 0 &&
		tm.millisecond == 0, "and the clock follows the fix");

	// a different second altogether, as after a long gap or a bad first fix, re-anchors at once
	check(wallclock_sync(1788509999LL, 46000000), "a clock seconds out is re-anchored");
}

// ---------------------------------------------------------------------------
// PMTK commands
//
// the expected strings are the examples printed in docs/MT33xx_GNSS_PMTK_Users_Guide, so these
// check the checksum against the receiver vendor's own arithmetic rather than against ours
// ---------------------------------------------------------------------------

static void test_gps_cmd(void)
{
	char out[GPS_CMD_MAX];
	int cmd, flag;

	printf("PMTK commands\n");

	check(gps_cmd_build(out, sizeof(out), "PMTK220,1000"), "built the 1Hz command");
	check(strcmp(out, "$PMTK220,1000*1F\r\n") == 0, "PMTK220,1000 checksum matches the guide");

	check(gps_cmd_build(out, sizeof(out), "PMTK400"), "built the query");
	check(strcmp(out, "$PMTK400*36\r\n") == 0, "PMTK400 checksum matches the guide");

	check(gps_cmd_build(out, sizeof(out), "PMTK251,38400"), "built the baud command");
	check(strcmp(out, "$PMTK251,38400*27\r\n") == 0, "PMTK251 checksum matches the guide");

	check(gps_cmd_build(out, sizeof(out), "PMTK314,-1"), "built the sentence reset");
	check(strcmp(out, "$PMTK314,-1*04\r\n") == 0, "PMTK314,-1 checksum matches the guide");

	// the set this firmware actually sends: GGA and RMC on every fix, GSA and GSV every fifth,
	// everything else off. field 4 is GSA and field 5 is GSV, per the guide's numbering, and 5
	// is the largest divider it defines
	check(gps_cmd_build(out, sizeof(out), GPS_SENTENCES_WANTED), "built the wanted set");
	check(strcmp(out,
		"$PMTK314,0,1,0,1,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*34\r\n") == 0,
		"wanted sentence set checksum");

	// the divider has to be one PMTK314 accepts, or the receiver refuses the whole sentence and
	// quietly keeps GLL/VTG at the full fix rate
	check(GPS_DIAG_DIVIDER >= 1 && GPS_DIAG_DIVIDER <= 5,
		"the diagnostic divider is within the guide's 1..5");


	// no room, no command. better than a truncated sentence on the wire
	check(!gps_cmd_build(out, 8, "PMTK220,1000"), "refuses to truncate");

	printf("PMTK acknowledgements\n");
	check(gps_cmd_parse_ack("$PMTK001,314,3*36", &cmd, &flag), "ack recognised");
	check(cmd == 314 && flag == 3, "ack carries the command and the flag");

	check(gps_cmd_parse_ack("$PMTK001,220,2*36", &cmd, &flag), "refusal recognised");
	check(cmd == 220 && flag == 2, "flag 2 is valid command, action failed");

	check(!gps_cmd_parse_ack("$GPGGA,123519,,,,,0,00,,,M,,M,,*6B", &cmd, &flag),
		"a GGA is not an ack");
	check(!gps_cmd_parse_ack("$PMTK011,MTKGPS*08", &cmd, &flag), "a startup message is not an ack");
	check(!gps_cmd_parse_ack("$PMTK001,314", &cmd, &flag), "a truncated ack is rejected");

	// the other flags the guide defines. none of them is success, and the firmware has to tell
	// them apart: flag 2 is worth retrying slower, flag 0 and 1 never will be
	check(gps_cmd_parse_ack("$PMTK001,220,0*34", &cmd, &flag), "flag 0 recognised");
	check(cmd == 220 && flag == 0, "flag 0 is an invalid command");
	check(gps_cmd_parse_ack("$PMTK001,251,1*36", &cmd, &flag), "flag 1 recognised");
	check(cmd == 251 && flag == 1, "flag 1 is an unsupported command");

	// the guide prints a three field variant for some commands. the extra field must not upset
	// the two the firmware reads
	check(gps_cmd_parse_ack("$PMTK001,449,3,1*24", &cmd, &flag), "a three field ack parses");
	check(cmd == 449 && flag == 3, "and still yields the command and the flag");

	printf("PMTK500 fix control\n");
	{
		gps_fix_ctl_t ctl;

		// the guide's own printed example
		check(gps_cmd_parse_fix_ctl("$PMTK500,1000,0,0,0,0*1A", &ctl), "the guide's example parses");
		check(ctl.fix_interval_ms == 1000, "fix interval read back");
		check(ctl.duration_ms == 0 && ctl.run_interval_ms == 0, "duration and run interval");
		check_close(ctl.hacc_m, 0.0, 1e-12, "horizontal accuracy mask");
		check_close(ctl.vacc_m, 0.0, 1e-12, "vertical accuracy mask");

		check(gps_cmd_parse_fix_ctl("$PMTK500,100,5000,10000,2.5,4.5*00", &ctl),
			"a sleeping configuration parses");
		check(ctl.fix_interval_ms == 100, "10Hz read back");
		check(ctl.duration_ms == 5000 && ctl.run_interval_ms == 10000, "the sleep fields");
		check_close(ctl.hacc_m, 2.5, 1e-12, "fractional hacc");
		check_close(ctl.vacc_m, 4.5, 1e-12, "fractional vacc");

		// nmea_parse validates the checksum before this ever sees a sentence, so this parser
		// must not duplicate that work. pinning it in a test rather than only in a comment
		check(gps_cmd_parse_fix_ctl("$PMTK500,200,0,0,0,0*XX", &ctl),
			"the checksum is nmea_parse's job, not this parser's");
		check(ctl.fix_interval_ms == 200, "and the fields still come out");

		// a newer firmware may append fields
		check(gps_cmd_parse_fix_ctl("$PMTK500,500,0,0,0,0,7,8*00", &ctl),
			"extra trailing fields are tolerated");
		check(ctl.fix_interval_ms == 500, "without disturbing the ones we read");

		check(!gps_cmd_parse_fix_ctl("$PMTK500", &ctl), "the bare command is not a reply");
		check(!gps_cmd_parse_fix_ctl("$PMTK500,1000,0,0*2A", &ctl), "too few fields rejected");
		check(!gps_cmd_parse_fix_ctl("$PMTK500,1000,0,0,0*2A", &ctl), "four fields is still too few");
		check(!gps_cmd_parse_fix_ctl("$PMTK500,abc,0,0,0,0*00", &ctl), "a non numeric interval");
		check(!gps_cmd_parse_fix_ctl("$PMTK001,220,2*36", &ctl), "an ack is not a PMTK500");
		check(!gps_cmd_parse_fix_ctl("$GPGGA,123519,,,,,0,00,,,M,,M,,*6B", &ctl),
			"a GGA is not a PMTK500");

		// outside the guide's range the number is not usable, and reporting "the module holds
		// 0ms" would be worse than reporting nothing at all
		check(!gps_cmd_parse_fix_ctl("$PMTK500,50,0,0,0,0*00", &ctl), "faster than the guide allows");
		check(!gps_cmd_parse_fix_ctl("$PMTK500,10001,0,0,0,0*00", &ctl), "slower than it allows");
		check(!gps_cmd_parse_fix_ctl("$PMTK500,0,0,0,0,0*00", &ctl), "zero rejected");
	}

	// the back off ladder. this is the only place the sweep's decisions are exercised at all:
	// on the bike they only happen once per power up, and only when the receiver refuses
	printf("PMTK220 back off ladder\n");
	check(gps_cmd_next_fix_interval(100) == 125, "100ms refused -> try 125ms");
	check(gps_cmd_next_fix_interval(125) == 200, "125 -> 200");
	check(gps_cmd_next_fix_interval(200) == 250, "200 -> 250");
	check(gps_cmd_next_fix_interval(250) == 500, "250 -> 500");
	check(gps_cmd_next_fix_interval(500) == 1000, "500 -> 1000");
	check(gps_cmd_next_fix_interval(1000) == 0, "1000ms is the last rung: the ladder ends");
	check(gps_cmd_next_fix_interval(300) == 500, "a configured value off the ladder joins above it");
	check(gps_cmd_next_fix_interval(99) == 100, "below the first rung climbs onto it");
	check(gps_cmd_next_fix_interval(5000) == 0, "above the last rung there is nowhere to go");

	printf("PMTK251 baud table\n");
	check(gps_cmd_baud_allowed(0), "0 means leave the module alone");
	check(gps_cmd_baud_allowed(9600), "9600, the module's default");
	check(gps_cmd_baud_allowed(115200), "115200, what 10Hz needs");
	check(gps_cmd_baud_allowed(921600), "the fastest the guide lists");
	check(!gps_cmd_baud_allowed(9601), "a rate the module cannot be asked for");
	check(!gps_cmd_baud_allowed(-1), "a negative rate");
	check(!gps_cmd_baud_allowed(1000000), "a rate off the end of the table");
}

int motoreco_run_tests(void)
{
	g_failures = 0;
	g_checks = 0;

	printf("MotoRecoPico tests\n\n");

	test_checksum();
	test_gga();
	test_rmc();
	test_time_plausible();
	test_diagnostics();
	test_gps_cmd();
	test_reader();
	test_record();
	test_power();
	test_settings();
	test_wallclock();

	printf("\n%d checks, %d failures\n", g_checks, g_failures);

	return g_failures;
}

// on the pico the entry point lives in test_pico_main.c, which has to bring USB up first
#ifndef TEST_ON_PICO
int main(void)
{
	return (motoreco_run_tests() == 0) ? 0 : 1;
}
#endif
