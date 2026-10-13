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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "gps_cmd.h"

bool gps_cmd_build(char *out, size_t outsz, const char *body)
{
	unsigned char sum = 0;
	size_t i;
	size_t len = strlen(body);

	// '$' + body + '*' + two hex digits + CR + LF + NUL
	if (len + 7 > outsz) {
		return false;
	}

	for (i = 0; i < len; i++) {
		sum ^= (unsigned char)body[i];
	}

	snprintf(out, outsz, "$%s*%02X\r\n", body, sum);

	return true;
}

bool gps_cmd_parse_ack(const char *sentence, int *cmd, int *flag)
{
	const char *p;
	char *end;

	if (strncmp(sentence, "$PMTK001,", 9) != 0) {
		return false;
	}

	p = sentence + 9;
	*cmd = (int)strtol(p, &end, 10);

	if (end == p || *end != ',') {
		return false;
	}

	p = end + 1;
	*flag = (int)strtol(p, &end, 10);

	return end != p;
}

bool gps_cmd_parse_fix_ctl(const char *sentence, gps_fix_ctl_t *out)
{
	static const char prefix[] = "$PMTK500,";
	gps_fix_ctl_t ctl;
	const char *p;
	char *end;
	int *fields[3];
	int i;

	if (strncmp(sentence, prefix, sizeof(prefix) - 1) != 0) {
		return false;
	}

	memset(&ctl, 0, sizeof(ctl));

	fields[0] = &ctl.fix_interval_ms;
	fields[1] = &ctl.duration_ms;
	fields[2] = &ctl.run_interval_ms;

	p = sentence + sizeof(prefix) - 1;

	// each millisecond field has to be followed by a separator, so a sentence that was cut short
	// cannot be read as a complete one with zeros in it
	for (i = 0; i < 3; i++) {
		*fields[i] = (int)strtol(p, &end, 10);

		if (end == p || *end != ',') {
			return false;
		}

		p = end + 1;
	}

	ctl.hacc_m = strtod(p, &end);

	if (end == p || *end != ',') {
		return false;
	}

	p = end + 1;
	ctl.vacc_m = strtod(p, &end);

	// the last field ends at the checksum, the line end or the NUL, and a newer firmware may add
	// fields after it. anything that parsed as a number is accepted
	if (end == p) {
		return false;
	}

	if (ctl.fix_interval_ms < GPS_FIX_INTERVAL_MIN_MS ||
	    ctl.fix_interval_ms > GPS_FIX_INTERVAL_MAX_MS) {
		return false;
	}

	*out = ctl;

	return true;
}

int gps_cmd_next_fix_interval(int current_ms)
{
	static const int ladder[] = GPS_RATE_LADDER;
	size_t i;

	for (i = 0; i < sizeof(ladder) / sizeof(ladder[0]); i++) {
		if (ladder[i] > current_ms) {
			return ladder[i];
		}
	}

	return 0;
}

bool gps_cmd_baud_allowed(int baud)
{
	// the guide's PMTK251 table verbatim, plus 0 for "do not touch it"
	static const int allowed[] = {
		0, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 230400, 460800, 921600
	};
	size_t i;

	for (i = 0; i < sizeof(allowed) / sizeof(allowed[0]); i++) {
		if (allowed[i] == baud) {
			return true;
		}
	}

	return false;
}
