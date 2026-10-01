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

// glue between FatFs and sd_spi.c. there is exactly one drive

#include "pico/stdlib.h"

#include "ff.h"		// must come first: diskio.h uses the types ff.h defines
#include "diskio.h"

#include "sd_spi.h"
#include "wallclock.h"

// whether a card is there is decided by whether one answers, not by the detect switch. the switch
// is a single mechanical contact on the socket's smallest pads, and taking its word for it would
// make one bad solder joint look exactly like a dead card
DSTATUS disk_status(BYTE pdrv)
{
	if (pdrv != 0) {
		return STA_NOINIT;
	}

	return sd_initialized() ? 0 : STA_NOINIT;
}

DSTATUS disk_initialize(BYTE pdrv)
{
	if (pdrv != 0) {
		return STA_NOINIT;
	}

	if (sd_init() != SD_OK) {
		// STA_NODISK only when the detect line and the silence agree. it is reporting, nothing
		// downstream treats the two failures differently
		return sd_card_present() ? STA_NOINIT : (STA_NODISK | STA_NOINIT);
	}

	return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
	if (pdrv != 0) {
		return RES_PARERR;
	}

	if (!sd_initialized()) {
		return RES_NOTRDY;
	}

	return (sd_read_blocks(buff, (uint32_t)sector, count) == SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
	if (pdrv != 0) {
		return RES_PARERR;
	}

	if (!sd_initialized()) {
		return RES_NOTRDY;
	}

	return (sd_write_blocks(buff, (uint32_t)sector, count) == SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	if (pdrv != 0) {
		return RES_PARERR;
	}

	switch (cmd) {
	case CTRL_SYNC:
		return (sd_sync() == SD_OK) ? RES_OK : RES_ERROR;

	case GET_SECTOR_COUNT:
		*(LBA_t *)buff = sd_sector_count();
		return (*(LBA_t *)buff != 0) ? RES_OK : RES_ERROR;

	case GET_SECTOR_SIZE:
		*(WORD *)buff = 512;
		return RES_OK;

	case GET_BLOCK_SIZE:
		// erase block size in sectors. only f_mkfs uses it, and formatting is disabled
		*(DWORD *)buff = 1;
		return RES_OK;

	default:
		return RES_PARERR;
	}
}

// FatFs asks for the time whenever it creates or updates a file. before the first GPS fix there
// is no clock at all, so fall back to the fixed date from ffconf.h rather than inventing one
DWORD get_fattime(void)
{
	wallclock_tm_t tm;

	if (!wallclock_local(time_us_64(), &tm)) {
		return ((DWORD)(FF_NORTC_YEAR - 1980) << 25) |
		       ((DWORD)FF_NORTC_MON << 21) |
		       ((DWORD)FF_NORTC_MDAY << 16);
	}

	return ((DWORD)(tm.year - 1980) << 25) |
	       ((DWORD)tm.month << 21) |
	       ((DWORD)tm.day << 16) |
	       ((DWORD)tm.hour << 11) |
	       ((DWORD)tm.minute << 5) |
	       ((DWORD)(tm.second / 2));
}
