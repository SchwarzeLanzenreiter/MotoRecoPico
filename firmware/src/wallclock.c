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

#include "config.h"
#include "wallclock.h"

static int64_t g_anchor_unix;	// UTC seconds at g_anchor_us
static uint64_t g_anchor_us;
static bool g_valid;
static int g_tz_offset_sec = DEFAULT_TZ_OFFSET_SEC;

void wallclock_init(void)
{
	g_anchor_unix = 0;
	g_anchor_us = 0;
	g_valid = false;
	g_tz_offset_sec = DEFAULT_TZ_OFFSET_SEC;
}

void wallclock_set_tz(int offset_sec)
{
	g_tz_offset_sec = offset_sec;
}

int wallclock_tz(void)
{
	return g_tz_offset_sec;
}

void wallclock_set(int64_t unix_utc, uint64_t monotonic_us)
{
	g_anchor_unix = unix_utc;
	g_anchor_us = monotonic_us;
	g_valid = true;
}

bool wallclock_valid(void)
{
	return g_valid;
}

// inverse of the days_from_civil used in the parser. Howard Hinnant's civil_from_days
static void civil_from_days(int64_t z, int *year, int *month, int *day)
{
	int64_t era, doe, yoe, y, doy, mp, d, m;

	z += 719468;
	era = ((z >= 0) ? z : z - 146096) / 146097;
	doe = z - era * 146097;								// [0, 146096]
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;	// [0, 399]
	y = yoe + era * 400;
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);		// [0, 365]
	mp = (5 * doy + 2) / 153;							// [0, 11]
	d = doy - (153 * mp + 2) / 5 + 1;					// [1, 31]
	m = mp + ((mp < 10) ? 3 : -9);						// [1, 12]

	*year = (int)(y + ((m <= 2) ? 1 : 0));
	*month = (int)m;
	*day = (int)d;
}

void wallclock_break_down(int64_t unix_time, wallclock_tm_t *out)
{
	int64_t days = unix_time / 86400;
	int64_t rem = unix_time % 86400;

	// C truncates toward zero, so a negative timestamp needs the day pulled back by one
	if (rem < 0) {
		rem += 86400;
		days -= 1;
	}

	civil_from_days(days, &out->year, &out->month, &out->day);

	out->hour = (int)(rem / 3600);
	out->minute = (int)((rem % 3600) / 60);
	out->second = (int)(rem % 60);
	out->millisecond = 0;
}

bool wallclock_local(uint64_t monotonic_us, wallclock_tm_t *out)
{
	int64_t elapsed_us;
	int64_t secs;
	int64_t frac_us;

	if (!g_valid) {
		return false;
	}

	// the timestamp asked about is regularly BEFORE the anchor: a trip file is named after the
	// moment it was created, and that is always earlier than the first GPS fix that set the
	// clock. an unsigned subtraction here wraps and produces a date in the year 5865800921,
	// which is exactly what one real log was named
	elapsed_us = (int64_t)monotonic_us - (int64_t)g_anchor_us;

	// C truncates division toward zero, so a negative remainder has to be carried by hand or the
	// seconds land one too high for any timestamp before the anchor
	secs = elapsed_us / 1000000;
	frac_us = elapsed_us % 1000000;

	if (frac_us < 0) {
		frac_us += 1000000;
		secs -= 1;
	}

	wallclock_break_down(g_anchor_unix + secs + g_tz_offset_sec, out);

	// the anchor comes from an RMC sentence, which carries whole seconds, so the fraction is
	// measured from it rather than invented. good enough to order two events inside one second
	out->millisecond = (int)(frac_us / 1000);

	return true;
}
