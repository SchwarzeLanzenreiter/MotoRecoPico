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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "ff.h"

#include "config.h"
#include "logger.h"
#include "sd_spi.h"
#include "wallclock.h"

#define TRIP_NAME_MAX	32
#define WRITE_BUF_RECORDS	32		// 512 bytes, one card sector

static FATFS g_fs;
static FIL g_file;
static bool g_mounted;
static bool g_open;
static bool g_temp_name;
static char g_name[TRIP_NAME_MAX];
static uint64_t g_start_us;			// monotonic time the current file was created
static uint32_t g_records;			// records written into this file

static struct CANData g_buf[WRITE_BUF_RECORDS];
static size_t g_buf_count;

static bool open_new_file(void);

static void build_timestamp_name(char *out, size_t outsz, uint64_t monotonic_us, int suffix)
{
	wallclock_tm_t tm;

	wallclock_local(monotonic_us, &tm);

	if (suffix == 0) {
		snprintf(out, outsz, "%s%04d%02d%02d_%02d%02d%02d.dat", LOG_DIR,
			tm.year, tm.month, tm.day, tm.hour, tm.minute, tm.second);
	} else {
		// two trips can only collide if one lasted under a second, but a failed rename would
		// leave the log under the placeholder name for good, so give it somewhere to go
		snprintf(out, outsz, "%s%04d%02d%02d_%02d%02d%02d_%d.dat", LOG_DIR,
			tm.year, tm.month, tm.day, tm.hour, tm.minute, tm.second, suffix);
	}
}

// renames the open file to something derived from monotonic_us. the file is closed for the
// rename because FatFs will not rename an open file, and reopened appending unless the caller is
// shutting down, in which case reopening would only spend time the supply may not have
static bool rename_open_file(uint64_t monotonic_us, bool reopen)
{
	char newname[TRIP_NAME_MAX];
	FRESULT fr;
	int suffix;

	if (!g_open || !wallclock_valid()) {
		return false;
	}

	if (f_close(&g_file) != FR_OK) {
		g_open = false;
		return false;
	}

	g_open = false;

	for (suffix = 0; suffix < 10; suffix++) {
		build_timestamp_name(newname, sizeof(newname), monotonic_us, suffix);

		fr = f_rename(g_name, newname);

		if (fr == FR_OK) {
			strncpy(g_name, newname, sizeof(g_name) - 1);
			g_name[sizeof(g_name) - 1] = '\0';
			g_temp_name = false;
			break;
		}

		if (fr != FR_EXIST) {
			break;
		}
	}

	if (!reopen) {
		return true;
	}

	// reopen whatever name the file ended up with. losing the rename is survivable, losing the
	// open file is not
	if (f_open(&g_file, g_name, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
		return false;
	}

	g_open = true;

	return true;
}

static bool flush_buffer(void)
{
	UINT written = 0;

	if (g_buf_count == 0) {
		return true;
	}

	if (!g_open) {
		g_buf_count = 0;
		return false;
	}

	if (f_write(&g_file, g_buf, g_buf_count * sizeof(struct CANData), &written) != FR_OK ||
	    written != g_buf_count * sizeof(struct CANData)) {
		g_buf_count = 0;
		return false;
	}

	g_buf_count = 0;

	return true;
}

// picks the first free TMPnnnnn.DAT. used until GPS tells us the date, and after a card swap
static bool open_temp_file(void)
{
	FILINFO info;
	int i;

	for (i = 1; i < 100000; i++) {
		snprintf(g_name, sizeof(g_name), "%s%s%05d.DAT", LOG_DIR, LOG_TMP_PREFIX, i);

		if (f_stat(g_name, &info) == FR_NO_FILE) {
			if (f_open(&g_file, g_name, FA_WRITE | FA_CREATE_NEW) != FR_OK) {
				return false;
			}

			g_temp_name = true;
			return true;
		}
	}

	return false;
}

static bool open_new_file(void)
{
	g_buf_count = 0;
	g_records = 0;
	g_start_us = time_us_64();

	// with a valid clock (a warm restart with the GPS still tracking) the file can be named
	// properly straight away and never needs the placeholder
	if (wallclock_valid()) {
		int suffix;

		for (suffix = 0; suffix < 10; suffix++) {
			build_timestamp_name(g_name, sizeof(g_name), g_start_us, suffix);

			if (f_open(&g_file, g_name, FA_WRITE | FA_CREATE_NEW) == FR_OK) {
				g_temp_name = false;
				g_open = true;
				return true;
			}
		}
	}

	if (!open_temp_file()) {
		return false;
	}

	g_open = true;

	return true;
}

bool logger_mount(void)
{
	if (g_mounted) {
		return g_open;
	}

	// no check of the detect line here on purpose: f_mount with the "mount now" flag runs
	// disk_initialize(), which asks the card directly and fails fast on an empty slot
	if (f_mount(&g_fs, "", 1) != FR_OK) {
		return false;
	}

	g_mounted = true;

	if (!open_new_file()) {
		f_mount(NULL, "", 0);
		g_mounted = false;
		return false;
	}

	return true;
}

void logger_unmount(void)
{
	if (g_open) {
		flush_buffer();
		f_close(&g_file);
		g_open = false;
	}

	if (g_mounted) {
		f_mount(NULL, "", 0);
		g_mounted = false;
	}

	g_buf_count = 0;
}

bool logger_is_open(void)
{
	return g_open;
}

bool logger_has_temp_name(void)
{
	return g_open && g_temp_name;
}

bool logger_write(const struct CANData *rec)
{
	if (!g_open) {
		return false;
	}

	g_buf[g_buf_count++] = *rec;
	g_records++;

	if (g_buf_count < WRITE_BUF_RECORDS) {
		return true;
	}

	return flush_buffer();
}

bool logger_sync(void)
{
	if (!g_open) {
		return false;
	}

	if (!flush_buffer()) {
		return false;
	}

	// f_sync is what makes the records reachable after a power cut: it pushes the file's own
	// buffer out and updates the directory entry with the new size
	return f_sync(&g_file) == FR_OK;
}

bool logger_name_from_clock(void)
{
	if (!g_open || !g_temp_name || !wallclock_valid()) {
		return false;
	}

	if (!flush_buffer()) {
		return false;
	}

	// name it after when the trip started, not after now. the clock maps monotonic time to wall
	// clock time, so the start timestamp resolves correctly even though it is in the past
	return rename_open_file(g_start_us, true);
}

const char *logger_finalize_text(logger_finalize_t result)
{
	switch (result) {
	case LOGGER_FINALIZE_DONE:      return "log closed and renamed";
	case LOGGER_FINALIZE_NO_CLOCK:  return "log synced but not renamed, no gps time yet";
	case LOGGER_FINALIZE_CLOSED:    return "no log was open";
	default:                        return "the card refused";
	}
}

logger_finalize_t logger_shutdown(void)
{
	bool renamed;

	// the filesystem is deliberately left mounted. the caller still has to record why the log
	// closed, and logger_debug() writes nothing once the card is unmounted, which is how the
	// ignition-off line went missing from motoreco.log the first time this ran on the bike.
	// the caller unmounts when it has finished writing
	if (!g_open) {
		return LOGGER_FINALIZE_CLOSED;
	}

	// order matters, and it is the order docs/netlist.md section 4.7 asks for. the sync is what
	// actually saves the ride, so it goes first and the cosmetic rename comes after: if the
	// supply gives out between the two, the data is already on the card under the old name.
	//
	flush_buffer();
	f_sync(&g_file);

	if (!wallclock_valid()) {
		f_close(&g_file);
		g_open = false;
		return LOGGER_FINALIZE_NO_CLOCK;
	}

	renamed = rename_open_file(time_us_64(), false);

	if (g_open) {
		f_close(&g_file);
		g_open = false;
	}

	return renamed ? LOGGER_FINALIZE_DONE : LOGGER_FINALIZE_ERROR;
}

const char *logger_filename(void)
{
	return g_name;
}

uint32_t logger_records_written(void)
{
	return g_records;
}

// ---------------------------------------------------------------------------
// debug log
// ---------------------------------------------------------------------------

void logger_debug(const char *fmt, ...)
{
	char line[192];
	char stamp[32];
	wallclock_tm_t tm;
	va_list args;
	int len;
	FIL dbg;
	UINT written;

	// milliseconds matter here: the engine stop and the ignition going off can land in the same
	// second, and which came first is exactly what the log is asked to settle
	if (wallclock_local(time_us_64(), &tm)) {
		snprintf(stamp, sizeof(stamp), "[%04d%02d%02d %02d%02d%02d.%03d] ",
			tm.year, tm.month, tm.day, tm.hour, tm.minute, tm.second, tm.millisecond);
	} else {
		// no fix yet, so timestamp against the only clock there is
		snprintf(stamp, sizeof(stamp), "[+%llu.%03llus] ",
			(unsigned long long)(time_us_64() / 1000000ULL),
			(unsigned long long)((time_us_64() / 1000ULL) % 1000ULL));
	}

	va_start(args, fmt);
	len = vsnprintf(line, sizeof(line), fmt, args);
	va_end(args);

	if (len < 0) {
		return;
	}

	// USB serial first: it is the only output that works with no card in the slot, which is
	// exactly the situation worth reporting
	printf("%s%s\n", stamp, line);

	if (!g_mounted) {
		return;
	}

	if (f_open(&dbg, LOG_DIR DEBUG_LOG_FILE, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
		return;
	}

	f_write(&dbg, stamp, (UINT)strlen(stamp), &written);
	f_write(&dbg, line, (UINT)strlen(line), &written);
	f_write(&dbg, "\n", 1, &written);

	// the debug log shares the card with the trip logs, so an unbounded one eventually takes the
	// trips down with it. one generation, same as the pi version
	if (f_size(&dbg) >= DEBUG_LOG_MAX_BYTES) {
		f_close(&dbg);
		f_unlink(LOG_DIR DEBUG_LOG_FILE_OLD);
		f_rename(LOG_DIR DEBUG_LOG_FILE, LOG_DIR DEBUG_LOG_FILE_OLD);
		return;
	}

	f_close(&dbg);
}
