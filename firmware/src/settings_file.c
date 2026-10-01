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

// the filesystem half of the settings. kept apart from settings.c so the parsing can be tested
// on a PC without dragging FatFs along

#include <string.h>

#include "ff.h"

#include "config.h"
#include "logger.h"
#include "settings_file.h"

#define SETTINGS_LINE_MAX 128

// writes the documented default file and hides it, so it does not sit next to the trip logs in
// the rider's file manager. failing to hide it is not a failure: the settings still work
static bool write_default_file(void)
{
	FIL fp;
	const char *text = settings_default_file_text();
	UINT written = 0;
	UINT len = (UINT)strlen(text);

	if (f_open(&fp, LOG_DIR SETTINGS_FILE, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
		return false;
	}

	if (f_write(&fp, text, len, &written) != FR_OK || written != len) {
		f_close(&fp);
		return false;
	}

	f_close(&fp);
	f_chmod(LOG_DIR SETTINGS_FILE, AM_HID, AM_HID);

	return true;
}

settings_file_result_t settings_file_load(settings_t *s, int *bad_lines)
{
	FIL fp;
	FILINFO info;
	char line[SETTINGS_LINE_MAX];
	int line_no = 0;

	*bad_lines = 0;

	// start from the defaults every time, so a file that only sets one key leaves the other at a
	// known value rather than at whatever the previous card said
	settings_defaults(s);

	if (f_stat(LOG_DIR SETTINGS_FILE, &info) != FR_OK) {
		return write_default_file() ? SETTINGS_FILE_CREATED : SETTINGS_FILE_ERROR;
	}

	if (f_open(&fp, LOG_DIR SETTINGS_FILE, FA_READ) != FR_OK) {
		return SETTINGS_FILE_ERROR;
	}

	// f_gets stops at a newline and NUL terminates, which is exactly the unit the parser wants.
	// a line longer than the buffer comes back in pieces and its tail fails to parse, which is
	// reported as a bad line rather than silently swallowed
	while (f_gets(line, sizeof(line), &fp) != NULL) {
		line_no++;

		if (settings_parse_line(s, line) == SETTINGS_LINE_BAD) {
			(*bad_lines)++;
			logger_debug("settings: line %d ignored: %s", line_no, line);
		}
	}

	f_close(&fp);

	// an older file created before the attribute was set, or one copied back from a PC, gets
	// hidden again. harmless if it already is
	f_chmod(LOG_DIR SETTINGS_FILE, AM_HID, AM_HID);

	return SETTINGS_FILE_LOADED;
}
