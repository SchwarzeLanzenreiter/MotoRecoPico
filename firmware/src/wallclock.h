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

#ifndef WALLCLOCK_H
#define WALLCLOCK_H

#include <stdbool.h>
#include <stdint.h>

// wall clock time, kept as an offset from the monotonic microsecond counter.
//
// there is no RTC and no battery on this board, so until the GPS reports a date the logger has no
// idea what day it is. that is why a trip starts under a placeholder file name and is renamed once
// the first fix arrives.
//
// the caller passes the monotonic time in, which keeps this module free of hardware and testable
// on the host

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int year;		// 1970..
	int month;		// 1..12
	int day;		// 1..31
	int hour;
	int minute;
	int second;
	int millisecond;	// measured from the anchor, so two log lines in the same second can be
						// told apart. zero when only a unix timestamp was broken down
} wallclock_tm_t;

void wallclock_init(void);

// the offset from UTC used by wallclock_local(). comes from the settings file on the card, so it
// can change once, shortly after boot, when that file is read
void wallclock_set_tz(int offset_sec);
int wallclock_tz(void);

// anchors UTC seconds to a monotonic timestamp. safe to call on every fix
void wallclock_set(int64_t unix_utc, uint64_t monotonic_us);

bool wallclock_valid(void);

// local time (UTC plus the configured offset) for the given monotonic timestamp. false until the
// clock has been set at least once
bool wallclock_local(uint64_t monotonic_us, wallclock_tm_t *out);

// breaks a unix timestamp down. exposed for the tests
void wallclock_break_down(int64_t unix_time, wallclock_tm_t *out);

#ifdef __cplusplus
}
#endif

#endif // WALLCLOCK_H
