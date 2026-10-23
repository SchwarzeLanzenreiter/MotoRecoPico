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

#ifndef NMEA_H
#define NMEA_H

#include <stdint.h>
#include <stddef.h>

// NMEA 0183 parser. the pi version got all of this from gpsd, which does not exist here, so GGA
// and RMC are parsed directly off the module's serial stream.
//
// GGA and RMC each carry a part of what the log needs, so the state below is the union of the two
// and is updated in place, the same way gpsd merged them into one fix

#ifdef __cplusplus
extern "C" {
#endif

// the standard caps a sentence at 82 bytes including "$" and CRLF. accept a little more so a
// chatty module does not make every sentence look broken
#define NMEA_MAX_SENTENCE 120

enum {
	NMEA_RESULT_INVALID = -1,	// bad framing or checksum. sentence was dropped
	NMEA_RESULT_IGNORED = 0,	// well formed, but not a sentence we use
	NMEA_RESULT_GGA     = 1,
	NMEA_RESULT_RMC     = 2,
	NMEA_RESULT_GSA     = 3,	// fix mode and dilution of precision
	NMEA_RESULT_GSV     = 4		// satellites in view and their signal strength
};

// GSV comes as a series of sentences per constellation, so the counts have to be accumulated
// across a cycle before they mean anything. one slot per talker (GP, GL, GA, ...).
//
// six, because a talker with no slot left is dropped from the totals in silence (see gsv_slot),
// and this receiver can report GP, GL, GA, GB and GQ at once -- QZSS is enabled by default on
// it. undercounting the satellites in view would quietly defeat the one thing GSV is kept for
#define NMEA_MAX_TALKERS 6

typedef struct {
	char	id[3];			// talker, e.g. "GP". empty slot while id[0] == 0
	int		in_view;		// last completed cycle
	int		tracked;		// of those, how many report a signal to noise ratio
	int		best_snr;
	int		acc_tracked;	// cycle in progress
	int		acc_best_snr;
} nmea_gsv_t;

typedef struct {
	int		has_fix;		// GGA quality > 0, or RMC status 'A'
	double	latitude;		// degrees, north positive
	double	longitude;		// degrees, east positive

	int		has_altitude;	// GGA carried an MSL altitude. false while the fix is 2D only
	double	altitude;		// metres above mean sea level

	int		has_speed;		// RMC carried a speed over ground
	double	speed;			// metres per second, like gpsd's fix.speed

	int		has_time;		// RMC carried a parseable date and time, fixed or not
	int		time_from_fix;	// ...and that RMC was status 'A'. ONLY this one may set the wall
							// clock or name a file. during a cold search this module emitted
							// 2080-01-06 in a void RMC, and a trip named from that is named
							// wrong for good, so has_time alone is a diagnostic and nothing more
	int64_t	unix_time;		// seconds since the epoch, UTC
	int		time_ms;		// and the fraction RMC gave with it ("hhmmss.sss"). 0 when it gave none.
							// at 10Hz the fixes are .000, .100 ... .900 of the same second, and
							// anchoring each of them as .000 is what set the debug log's clock
							// back by up to 0.9s inside every second

	int		has_gga_time;	// the last GGA carried a parseable time of day
	int32_t	gga_tod_ms;		// ...as milliseconds since UTC midnight. GGA has no date, so this
							// only becomes a full time through nmea_gga_unix_ms

	// everything below exists to answer "why is there still no fix": a receiver that sees no
	// satellites at all is a different problem from one that sees eight but cannot resolve them.
	// quality, sats_used, hdop and fix_mode also reach the .dat file, in 0x7FD
	int		quality;		// GGA field 6 as received: 0 none, 1 GPS, 2 DGPS, 4/5 RTK
	int		sats_used;		// GGA field 7
	double	hdop;			// GGA field 8. 0 when not reported
	int		fix_mode;		// GSA field 2: 1 none, 2 2D, 3 3D
	double	pdop;			// GSA
	double	vdop;			// GSA

	nmea_gsv_t gsv[NMEA_MAX_TALKERS];
} nmea_fix_t;

// totals across every constellation the receiver reports
int nmea_sats_in_view(const nmea_fix_t *fix);
int nmea_sats_tracked(const nmea_fix_t *fix);
int nmea_best_snr(const nmea_fix_t *fix);

// assembles bytes from the UART into complete sentences
typedef struct {
	char	buf[NMEA_MAX_SENTENCE + 1];
	size_t	len;
	int		overrun;		// current line was too long. drop it at the terminator
} nmea_reader_t;

void nmea_init(nmea_fix_t *fix);
void nmea_reader_init(nmea_reader_t *reader);

// feeds one received byte. returns a NUL terminated sentence when one completes, else NULL.
// the returned pointer is owned by the reader and stays valid until the next call
const char *nmea_reader_push(nmea_reader_t *reader, char c);

// parses one sentence into fix. returns one of NMEA_RESULT_*
int nmea_parse(nmea_fix_t *fix, const char *sentence);

// helpers exposed for the host tests
int nmea_checksum_ok(const char *sentence);
int64_t nmea_to_unix(int year, int month, int day, int hour, int min, int sec);

// is this timestamp one a working receiver could plausibly have produced? a receiver with a
// garbage almanac is not a few seconds out, it is decades out, and the damage lands in a file
// name that cannot be taken back. the window is GPS_YEAR_MIN..GPS_YEAR_MAX
int nmea_time_plausible(int64_t unix_time);

// how far the last RMC may lag the GGA whose date it supplies. a receiver sending GGA first is one
// fix interval behind, which is 10s at the slowest gps_rate
#define NMEA_GGA_RMC_MAX_MS 60000

// the UTC of the last GGA in unix milliseconds. GGA carries only the time of day, so the date comes
// from the last RMC, and only from one that may also set the wall clock: status 'A', a plausible
// date, and at most NMEA_GGA_RMC_MAX_MS before the GGA. UTC midnight between the two is accounted
// for. returns 0, leaving *unix_ms alone, when there is no such answer
int nmea_gga_unix_ms(const nmea_fix_t *fix, int64_t *unix_ms);

#ifdef __cplusplus
}
#endif

#endif // NMEA_H
