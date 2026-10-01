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

#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include <stdint.h>

#include "motoreco.h"

// the .dat trip log on the card, plus the debug log next to it.
//
// file life cycle, which is where this differs from the raspberry pi version:
//
//   power on (= ignition on)   open TMPnnnnn.DAT and start recording immediately
//   first GPS fix              close, rename to the trip's start time, reopen appending
//   ignition off               flush, sync, close, rename to the finish time, stay closed
//   abnormal power cut         whatever the last f_sync wrote is on the card
//
// the pi could close the file on key off because it kept running afterwards. here the ignition
// takes the supply with it, and the board's 0.4s of borrowed life is what the close has to fit
// into (docs/netlist.md section 4.7)

#ifdef __cplusplus
extern "C" {
#endif

bool logger_mount(void);		// mounts the card and opens a new trip file
void logger_unmount(void);

bool logger_is_open(void);
bool logger_has_temp_name(void);	// still waiting for GPS to say what day it is

// appends one record. buffered, so this is cheap enough to call per CAN frame
bool logger_write(const struct CANData *rec);

// pushes the buffer out and syncs the directory entry. call on LOG_FLUSH_INTERVAL_MS
bool logger_sync(void);

// renames the placeholder to the trip's start time. call once the wall clock becomes valid
bool logger_name_from_clock(void);

// what the close actually did. the caller logs it, because "the ignition went off" does not say
// whether the trip ended up under its own name or is still sitting there as a placeholder
typedef enum {
	LOGGER_FINALIZE_DONE = 0,	// synced and renamed
	LOGGER_FINALIZE_NO_CLOCK,	// synced, but GPS has not said what time it is yet
	LOGGER_FINALIZE_CLOSED,		// no log was open
	LOGGER_FINALIZE_ERROR		// the card refused
} logger_finalize_t;

const char *logger_finalize_text(logger_finalize_t result);

// the ignition is gone and the supply is running on C13's borrowed 0.4s. flush, sync, close,
// rename, and stay closed: reopening would only spend time that may not be there.
//
// the card stays mounted so the caller can still record what happened with logger_debug().
// call logger_unmount() once that is written
logger_finalize_t logger_shutdown(void);

// the debug log: same idea as the pi version's debug_log(), a line per event with a timestamp,
// rotated at 1MB with one generation kept. also mirrored to USB serial
void logger_debug(const char *fmt, ...);

const char *logger_filename(void);
uint32_t logger_records_written(void);

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H
