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

#ifndef GPS_CMD_H
#define GPS_CMD_H

#include <stdbool.h>
#include <stddef.h>

// PMTK commands: the MediaTek host protocol the GT-502MGG-N speaks. the receiver's own guide is
// docs/MT33xx_GNSS_PMTK_Users_Guide.
//
// the four used here, in the order main.c sends them. the first three are "Keep status after
// reboot -> Yes", so they are sent once per power up and the module remembers them:
//
//   PMTK251  the NMEA baud rate. 9600 cannot carry 10Hz, so this has to go first
//   PMTK314  which NMEA sentences to emit, and at what fraction of the fix rate
//   PMTK220  the position fix interval in milliseconds, 100 to 10000
//   PMTK400  query the interval actually in force. answered with a PMTK500 sentence
//
// the receiver refuses what it cannot do (PMTK001 flag 2) instead of failing silently, and it
// refused 100ms at 9600 on real hardware. so an ack is never taken as proof: PMTK400 reads the
// interval back, and main.c also measures the sentence rate on the wire.
//
// no hardware in this file: it builds the strings and reads the answers, main.c puts the bytes
// on the wire

#ifdef __cplusplus
extern "C" {
#endif

// longest command built here is PMTK314 with 22 fields
#define GPS_CMD_MAX 96

// wraps body ("PMTK220,100") into "$PMTK220,100*2F\r\n". returns false if it would not fit
bool gps_cmd_build(char *out, size_t outsz, const char *body);

// PMTK001 is the acknowledgement: "$PMTK001,<cmd>,<flag>". returns false if the sentence is not
// one. per the receiver's guide the flags are
//
//   0  invalid command or packet
//   1  unsupported command or packet type
//   2  valid command, but the action failed
//   3  valid command, and the action succeeded
//
// only 3 is success. 2 is what a fix interval the line cannot carry comes back as
bool gps_cmd_parse_ack(const char *sentence, int *cmd, int *flag);

#define GPS_ACK_OK 3

// PMTK500 answers PMTK400 and is the only honest account of the fix interval in force:
// "$PMTK500,FixInterval,Duration,RunInterval,HAcc,VAcc*CS" (the guide's own example is
// "$PMTK500,1000,0,0,0,0*1A")
typedef struct {
	int		fix_interval_ms;	// field 1. the number this whole exercise is about
	int		duration_ms;		// field 2. non-zero means the module sleeps between fixes
	int		run_interval_ms;	// field 3
	double	hacc_m;				// field 4, one sigma horizontal accuracy mask
	double	vacc_m;				// field 5
} gps_fix_ctl_t;

// the checksum is NOT verified here: nmea_parse has already validated it before a sentence gets
// this far. returns false when it is not a PMTK500, when a field is missing or unparseable, or
// when FixInterval falls outside the guide's 100..10000 -- a mangled line must not be able to
// report that the module holds 0ms
bool gps_cmd_parse_fix_ctl(const char *sentence, gps_fix_ctl_t *out);

// the next interval to try after current_ms was refused: the smallest rung of GPS_RATE_LADDER
// strictly above it, or 0 when the ladder is exhausted. a configured value that is not on the
// ladder (300, say) is honoured as the first attempt and then joins the ladder above itself
int gps_cmd_next_fix_interval(int current_ms);

// one of the bauds the guide's PMTK251 table lists, or 0 for "leave the module's baud alone"
bool gps_cmd_baud_allowed(int baud);

#ifdef __cplusplus
}
#endif

#endif // GPS_CMD_H
