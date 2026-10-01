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

#include <string.h>

#include "config.h"
#include "power.h"

void power_init(power_state_t *st)
{
	memset(st, 0, sizeof(*st));
}

power_event_t power_eval(power_state_t *st, int mv)
{
	st->mv = mv;

	if (mv >= IG_ON_MV) {
		st->low_count = 0;

		if (st->high_count < IG_ON_SAMPLES) {
			st->high_count++;
		}

		if (st->high_count < IG_ON_SAMPLES) {
			return POWER_EVENT_NONE;
		}

		// the ignition has to be seen live before its loss means anything. on the bench, powered
		// from USB with nothing on GP28, this never arms and never fires
		if (!st->armed) {
			st->armed = 1;
			return POWER_EVENT_NONE;
		}

		if (st->off) {
			st->off = 0;
			return POWER_EVENT_RESTORED;
		}

		return POWER_EVENT_NONE;
	}

	if (mv >= IG_OFF_MV) {
		// between the thresholds. the pin sits at either 0.65V or 0V, so this band is only ever
		// crossed in transit. leave both counters alone rather than guessing
		return POWER_EVENT_NONE;
	}

	st->high_count = 0;

	if (st->low_count < IG_OFF_SAMPLES) {
		st->low_count++;
	}

	if (!st->armed || st->off || st->low_count < IG_OFF_SAMPLES) {
		return POWER_EVENT_NONE;
	}

	st->off = 1;

	return POWER_EVENT_OFF;
}
