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
#include "settings.h"

void settings_defaults(settings_t *s)
{
	memset(s, 0, sizeof(*s));

	s->tz_offset_sec = DEFAULT_TZ_OFFSET_SEC;
}

static int lower(int c)
{
	return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

static int equals_ci(const char *a, const char *b)
{
	while (*a != '\0' && *b != '\0') {
		if (lower((unsigned char)*a) != lower((unsigned char)*b)) {
			return 0;
		}

		a++;
		b++;
	}

	return *a == *b;
}

static int is_space(int c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// copies src into dst without the whitespace at either end
static void trim_into(char *dst, size_t dstsz, const char *src, size_t len)
{
	size_t start = 0;
	size_t n;

	while (start < len && is_space((unsigned char)src[start])) {
		start++;
	}

	while (len > start && is_space((unsigned char)src[len - 1])) {
		len--;
	}

	n = len - start;

	if (n > dstsz - 1) {
		n = dstsz - 1;
	}

	memcpy(dst, &src[start], n);
	dst[n] = '\0';
}

bool settings_parse_tz(const char *value, int *out_sec)
{
	int sign = 1;
	int hours = 0;
	int minutes = 0;
	int digits = 0;
	const char *p = value;

	if (*p == '+') {
		p++;
	} else if (*p == '-') {
		sign = -1;
		p++;
	}

	while (*p >= '0' && *p <= '9') {
		hours = hours * 10 + (*p - '0');
		digits++;
		p++;
	}

	if (digits == 0 || digits > 2) {
		return false;
	}

	// half hour and quarter hour zones exist (+5:30, +5:45), so accept them even though the
	// common case is a whole number of hours
	if (*p == ':' || *p == '.') {
		char sep = *p;
		int mdigits = 0;

		p++;

		while (*p >= '0' && *p <= '9') {
			minutes = minutes * 10 + (*p - '0');
			mdigits++;
			p++;
		}

		if (sep == '.') {
			// a decimal fraction of an hour: "+5.5" is 30 minutes, "+5.75" is 45
			if (mdigits == 1) {
				minutes *= 6;
			} else if (mdigits == 2) {
				minutes = minutes * 60 / 100;
			} else {
				return false;
			}
		} else if (mdigits != 2) {
			return false;	// ":MM" is always two digits
		}
	}

	if (*p != '\0' || minutes >= 60) {
		return false;
	}

	// the real world spans UTC-12 to UTC+14
	if (hours > 14 || (sign > 0 && hours == 14 && minutes > 0) || (sign < 0 && hours > 12)) {
		return false;
	}

	*out_sec = sign * (hours * 3600 + minutes * 60);

	return true;
}

void settings_format_tz(int offset_sec, char *out, size_t outsz)
{
	int total = (offset_sec < 0) ? -offset_sec : offset_sec;

	snprintf(out, outsz, "%c%02d:%02d", (offset_sec < 0) ? '-' : '+',
		total / 3600, (total % 3600) / 60);
}

int settings_parse_line(settings_t *s, const char *line)
{
	const char *p = line;
	const char *sep;
	char key[24];
	char value[32];

	while (is_space((unsigned char)*p)) {
		p++;
	}

	// '#' starts a comment line, and a blank line is not an error either
	if (*p == '\0' || *p == '#') {
		return SETTINGS_LINE_SKIPPED;
	}

	sep = strchr(p, '=');

	// "key value" without the '=' is accepted too. someone editing this on a phone should not
	// lose a trip to a missing character
	if (sep == NULL) {
		sep = p;

		while (*sep != '\0' && !is_space((unsigned char)*sep)) {
			sep++;
		}

		if (*sep == '\0') {
			return SETTINGS_LINE_BAD;
		}
	}

	trim_into(key, sizeof(key), p, (size_t)(sep - p));
	trim_into(value, sizeof(value), sep + 1, strlen(sep + 1));

	if (key[0] == '\0' || value[0] == '\0') {
		return SETTINGS_LINE_BAD;
	}

	if (equals_ci(key, "timezone")) {
		int sec;

		if (!settings_parse_tz(value, &sec)) {
			return SETTINGS_LINE_BAD;
		}

		s->tz_offset_sec = sec;
		return SETTINGS_LINE_APPLIED;
	}

	// obsolete: the trip used to be closed on the engine stopping, decoded from the tachometer
	// frame, which meant the firmware had to know the model. the ignition line does that job
	// now and says the same thing on every bike. cards written by an older firmware still
	// carry this line, so it is skipped in silence rather than reported as junk on every boot
	if (equals_ci(key, "vehicle")) {
		return SETTINGS_LINE_SKIPPED;
	}

	return SETTINGS_LINE_BAD;
}

const char *settings_default_file_text(void)
{
	// written verbatim when the card has no settings file. it documents its own syntax, because
	// this file is the only thing the rider will have in front of them
	return
		"# MotoRecoPico settings\n"
		"#\n"
		"# Lines starting with '#' are comments. Edit with any text editor.\n"
		"# This file is hidden; enable \"show hidden files\" to see it.\n"
		"#\n"
		"# timezone: offset from UTC, used for the .dat file names and the debug log.\n"
		"#           whole hours (+8, -5) or with minutes (+5:30).\n"
		"timezone = " DEFAULT_TZ_TEXT "\n";
}
