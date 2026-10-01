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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>
#include <stddef.h>

// the settings file on the card. one thing has to be changeable without a rebuild: the time zone
// the log file names are written in.
//
// the parsing here is deliberately free of any filesystem, so it can be tested on a PC.
// settings_file.c is the FatFs half

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int	tz_offset_sec;	// seconds to add to UTC for local time
} settings_t;

enum {
	SETTINGS_LINE_BAD     = -1,	// looked like a setting but could not be used
	SETTINGS_LINE_SKIPPED = 0,	// blank, or a comment starting with '#'
	SETTINGS_LINE_APPLIED = 1
};

void settings_defaults(settings_t *s);

// applies one line of the file. the caller reports SETTINGS_LINE_BAD, this does not log
int settings_parse_line(settings_t *s, const char *line);

// "+8", "-5", "+5:30", "9" -> seconds. false when it is not a time zone at all
bool settings_parse_tz(const char *value, int *out_sec);

// "+08:00" style, for logging back what was understood
void settings_format_tz(int offset_sec, char *out, size_t outsz);

// the file written when none exists. self documenting, because the only place anyone will look
// for the syntax is the file itself
const char *settings_default_file_text(void);

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
