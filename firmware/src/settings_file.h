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

#ifndef SETTINGS_FILE_H
#define SETTINGS_FILE_H

#include "settings.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SETTINGS_FILE_LOADED = 0,	// read from the card
	SETTINGS_FILE_CREATED,		// none existed, one was written with the defaults
	SETTINGS_FILE_ERROR			// card trouble. the defaults are in effect
} settings_file_result_t;

// reads the settings file, or creates it with the defaults when the card has none. s is filled
// with the defaults first, so it is always usable whatever happens. bad_lines counts the lines
// that could not be used, which the caller reports
settings_file_result_t settings_file_load(settings_t *s, int *bad_lines);

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_FILE_H
