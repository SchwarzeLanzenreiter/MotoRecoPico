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

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "config.h"
#include "sd_spi.h"

// board v4.1 moved the card from SPI1 to SPI0 (docs/netlist.md section 3.7)
#define SD_SPI	spi0

_Static_assert(SPI_INSTANCE_OF(PIN_SD_SCK) == 0 && SPI_FUNC_OF(PIN_SD_SCK) == SPI_FUNC_SCK,
	"PIN_SD_SCK is not an SPI0 SCK pin");
_Static_assert(SPI_INSTANCE_OF(PIN_SD_MOSI) == 0 && SPI_FUNC_OF(PIN_SD_MOSI) == SPI_FUNC_TX,
	"PIN_SD_MOSI is not an SPI0 TX pin");
_Static_assert(SPI_INSTANCE_OF(PIN_SD_MISO) == 0 && SPI_FUNC_OF(PIN_SD_MISO) == SPI_FUNC_RX,
	"PIN_SD_MISO is not an SPI0 RX pin");

#define CMD0	0	// GO_IDLE_STATE
#define CMD8	8	// SEND_IF_COND
#define CMD9	9	// SEND_CSD
#define CMD12	12	// STOP_TRANSMISSION
#define CMD16	16	// SET_BLOCKLEN
#define CMD17	17	// READ_SINGLE_BLOCK
#define CMD24	24	// WRITE_BLOCK
#define CMD25	25	// WRITE_MULTIPLE_BLOCK
#define CMD55	55	// APP_CMD
#define CMD58	58	// READ_OCR
#define ACMD23	23	// SET_WR_BLK_ERASE_COUNT
#define ACMD41	41	// SD_SEND_OP_COND

#define R1_IDLE			0x01
#define R1_ILLEGAL_CMD	0x04

#define TOKEN_BLOCK		0xFE	// single block, and multiple block read
#define TOKEN_WRITE_MB	0xFC	// one block of a multiple block write
#define TOKEN_STOP_MB	0xFD	// end of a multiple block write

#define BUSY_TIMEOUT_MS		500		// the spec allows 250ms for a write, 100ms for a read
#define INIT_TIMEOUT_MS		1000

static bool g_initialized;
static bool g_block_addressing;		// SDHC/SDXC address in blocks, older cards in bytes
static bool g_detect_reliable;		// the detect line agreed with a card that actually answered

static inline void cs_select(void)
{
	gpio_put(PIN_SD_CS, 0);
}

static inline void cs_deselect(void)
{
	uint8_t ff = 0xFF;

	gpio_put(PIN_SD_CS, 1);

	// the card needs one more clocked byte after CS goes high to release the data line
	spi_write_blocking(SD_SPI, &ff, 1);
}

static uint8_t xchg(uint8_t out)
{
	uint8_t in = 0xFF;

	spi_write_read_blocking(SD_SPI, &out, &in, 1);

	return in;
}

// polls until the card stops holding the line low. this is where a worn out card spends its time
static bool wait_ready(uint32_t timeout_ms)
{
	absolute_time_t deadline = make_timeout_time_ms(timeout_ms);

	do {
		if (xchg(0xFF) == 0xFF) {
			return true;
		}
	} while (!time_reached(deadline));

	return false;
}

static uint8_t send_command(uint8_t cmd, uint32_t arg)
{
	uint8_t buf[6];
	uint8_t r1;
	int i;

	buf[0] = (uint8_t)(0x40 | cmd);
	buf[1] = (uint8_t)(arg >> 24);
	buf[2] = (uint8_t)(arg >> 16);
	buf[3] = (uint8_t)(arg >> 8);
	buf[4] = (uint8_t)arg;

	// CRC is only checked before the card leaves SPI idle, so only these two need a real one
	if (cmd == CMD0) {
		buf[5] = 0x95;
	} else if (cmd == CMD8) {
		buf[5] = 0x87;
	} else {
		buf[5] = 0x01;	// stop bit, any CRC
	}

	spi_write_blocking(SD_SPI, buf, sizeof(buf));

	// the answer arrives within 8 bytes. the first byte after CMD12 is a stuff byte and is skipped
	if (cmd == CMD12) {
		xchg(0xFF);
	}

	for (i = 0; i < 10; i++) {
		r1 = xchg(0xFF);

		if ((r1 & 0x80) == 0) {
			return r1;
		}
	}

	return 0xFF;
}

static uint8_t send_app_command(uint8_t cmd, uint32_t arg)
{
	uint8_t r1 = send_command(CMD55, 0);

	if (r1 > 1) {
		return r1;
	}

	return send_command(cmd, arg);
}

void sd_spi_hw_init(void)
{
	spi_init(SD_SPI, SD_SPI_BAUD_INIT);
	gpio_set_function(PIN_SD_SCK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_SD_MOSI, GPIO_FUNC_SPI);
	gpio_set_function(PIN_SD_MISO, GPIO_FUNC_SPI);

	gpio_init(PIN_SD_CS);
	gpio_set_dir(PIN_SD_CS, GPIO_OUT);
	gpio_put(PIN_SD_CS, 1);

	// R13 pulls the detect line up on the board. no internal pull: this line is only advisory
	// (see sd_detect_reliable), and an internal pull would hide a broken one
	gpio_init(PIN_SD_CD);
	gpio_set_dir(PIN_SD_CD, GPIO_IN);
	gpio_disable_pulls(PIN_SD_CD);

	g_initialized = false;
	g_detect_reliable = false;
}

bool sd_card_present(void)
{
	return gpio_get(PIN_SD_CD) == SD_CD_INSERTED_LEVEL;
}

bool sd_detect_reliable(void)
{
	return g_detect_reliable;
}

bool sd_initialized(void)
{
	return g_initialized;
}

void sd_reset_state(void)
{
	g_initialized = false;
}

sd_result_t sd_init(void)
{
	uint8_t r1;
	uint8_t ocr[4];
	int i;
	absolute_time_t deadline;
	bool detected = sd_card_present();

	g_initialized = false;

	// the detect switch is deliberately not a gate. it is one mechanical contact and two solder
	// joints on the smallest pads of the socket, and its polarity is an assumption
	// (SD_CD_INSERTED_LEVEL) that the DM3AT drawing does not spell out. asking the card itself is
	// both cheaper and more truthful: with an empty slot CMD0 gets no answer and this returns in
	// well under a millisecond
	//
	// the card only enters SPI mode if it sees at least 74 clocks with CS high
	spi_set_baudrate(SD_SPI, SD_SPI_BAUD_INIT);
	gpio_put(PIN_SD_CS, 1);

	for (i = 0; i < 10; i++) {
		xchg(0xFF);
	}

	cs_select();

	r1 = send_command(CMD0, 0);

	if (r1 != R1_IDLE) {
		cs_deselect();
		return SD_ERR_INIT;
	}

	r1 = send_command(CMD8, 0x000001AA);

	if (r1 == R1_IDLE) {
		// version 2 card. read the rest of R7 and check it echoed our voltage and pattern back
		for (i = 0; i < 4; i++) {
			ocr[i] = xchg(0xFF);
		}

		if (ocr[2] != 0x01 || ocr[3] != 0xAA) {
			cs_deselect();
			return SD_ERR_INIT;
		}

		deadline = make_timeout_time_ms(INIT_TIMEOUT_MS);

		do {
			r1 = send_app_command(ACMD41, 0x40000000);	// HCS: we can handle block addressing
		} while (r1 != 0 && !time_reached(deadline));

		if (r1 != 0) {
			cs_deselect();
			return SD_ERR_INIT;
		}

		if (send_command(CMD58, 0) != 0) {
			cs_deselect();
			return SD_ERR_INIT;
		}

		for (i = 0; i < 4; i++) {
			ocr[i] = xchg(0xFF);
		}

		// CCS in the OCR: set means the card takes block numbers, clear means byte offsets
		g_block_addressing = (ocr[0] & 0x40) != 0;
	} else {
		// version 1 card, or MMC. these are all byte addressed and small
		g_block_addressing = false;

		deadline = make_timeout_time_ms(INIT_TIMEOUT_MS);

		do {
			r1 = send_app_command(ACMD41, 0);
		} while (r1 != 0 && !time_reached(deadline));

		if (r1 != 0) {
			cs_deselect();
			return SD_ERR_INIT;
		}

		if (send_command(CMD16, 512) != 0) {
			cs_deselect();
			return SD_ERR_INIT;
		}
	}

	cs_deselect();

	spi_set_baudrate(SD_SPI, SD_SPI_BAUD_RUN);
	g_initialized = true;

	// a card answered. if the detect line claimed the slot was empty while that happened, it is
	// wired, soldered or polarised wrong, and nothing may act on it from here on
	g_detect_reliable = detected;

	return SD_OK;
}

// waits for a data token and reads len bytes plus the two CRC bytes that follow
static sd_result_t read_data_block(uint8_t *dst, uint32_t len)
{
	absolute_time_t deadline = make_timeout_time_ms(BUSY_TIMEOUT_MS);
	uint8_t token;

	do {
		token = xchg(0xFF);

		if (token != 0xFF) {
			break;
		}
	} while (!time_reached(deadline));

	if (token != TOKEN_BLOCK) {
		return (token == 0xFF) ? SD_ERR_TIMEOUT : SD_ERR_IO;
	}

	spi_read_blocking(SD_SPI, 0xFF, dst, len);

	xchg(0xFF);	// CRC, discarded. the card verifies its own data
	xchg(0xFF);

	return SD_OK;
}

sd_result_t sd_read_blocks(uint8_t *dst, uint32_t lba, uint32_t count)
{
	uint32_t i;
	sd_result_t res = SD_OK;

	if (!g_initialized) {
		return SD_ERR_NO_CARD;
	}

	cs_select();

	// single block reads in a loop rather than CMD18. reads are rare here (mount, directory
	// lookups, the odd cluster chain walk) and the multiple block path costs a stop token and its
	// own error handling for a speed the logger never needs
	for (i = 0; i < count; i++) {
		uint32_t addr = g_block_addressing ? (lba + i) : ((lba + i) * 512);

		if (!wait_ready(BUSY_TIMEOUT_MS)) {
			res = SD_ERR_TIMEOUT;
			break;
		}

		if (send_command(CMD17, addr) != 0) {
			res = SD_ERR_IO;
			break;
		}

		res = read_data_block(dst + i * 512, 512);

		if (res != SD_OK) {
			break;
		}
	}

	cs_deselect();

	return res;
}

static sd_result_t write_data_block(const uint8_t *src, uint8_t token)
{
	uint8_t resp;

	if (!wait_ready(BUSY_TIMEOUT_MS)) {
		return SD_ERR_TIMEOUT;
	}

	xchg(token);

	if (token == TOKEN_STOP_MB) {
		return SD_OK;
	}

	spi_write_blocking(SD_SPI, src, 512);

	xchg(0xFF);	// dummy CRC
	xchg(0xFF);

	resp = xchg(0xFF);

	// 0bxxx0sss1, sss == 010 means accepted. anything else is a CRC or write error
	if ((resp & 0x1F) != 0x05) {
		return SD_ERR_IO;
	}

	return SD_OK;
}

sd_result_t sd_write_blocks(const uint8_t *src, uint32_t lba, uint32_t count)
{
	uint32_t i;
	sd_result_t res = SD_OK;
	uint32_t addr;

	if (!g_initialized) {
		return SD_ERR_NO_CARD;
	}

	cs_select();

	addr = g_block_addressing ? lba : (lba * 512);

	if (count == 1) {
		if (send_command(CMD24, addr) != 0) {
			cs_deselect();
			return SD_ERR_IO;
		}

		res = write_data_block(src, TOKEN_BLOCK);

		if (res == SD_OK && !wait_ready(BUSY_TIMEOUT_MS)) {
			res = SD_ERR_TIMEOUT;
		}

		cs_deselect();

		return res;
	}

	// multiple block write. this is the path the trip log takes whenever FatFs has more than one
	// sector to flush, and it is the one worth having: the card erases the whole run at once
	// instead of once per sector
	send_app_command(ACMD23, count);	// pre erase hint. failure here is not fatal

	if (send_command(CMD25, addr) != 0) {
		cs_deselect();
		return SD_ERR_IO;
	}

	for (i = 0; i < count; i++) {
		res = write_data_block(src + i * 512, TOKEN_WRITE_MB);

		if (res != SD_OK) {
			break;
		}
	}

	// the stop token has to go out even after an error, or the card stays in write mode
	write_data_block(NULL, TOKEN_STOP_MB);

	if (!wait_ready(BUSY_TIMEOUT_MS) && res == SD_OK) {
		res = SD_ERR_TIMEOUT;
	}

	cs_deselect();

	return res;
}

sd_result_t sd_sync(void)
{
	sd_result_t res;

	if (!g_initialized) {
		return SD_ERR_NO_CARD;
	}

	cs_select();
	res = wait_ready(BUSY_TIMEOUT_MS) ? SD_OK : SD_ERR_TIMEOUT;
	cs_deselect();

	return res;
}

uint32_t sd_sector_count(void)
{
	uint8_t csd[16];
	uint32_t sectors = 0;

	if (!g_initialized) {
		return 0;
	}

	cs_select();

	if (send_command(CMD9, 0) == 0 && read_data_block(csd, 16) == SD_OK) {
		if ((csd[0] >> 6) == 1) {
			// CSD version 2: capacity is (C_SIZE + 1) * 512KB
			uint32_t c_size = (((uint32_t)csd[7] & 0x3F) << 16) | ((uint32_t)csd[8] << 8) | csd[9];
			sectors = (c_size + 1) * 1024;
		} else {
			// CSD version 1
			uint32_t c_size = (((uint32_t)csd[6] & 0x03) << 10) | ((uint32_t)csd[7] << 2) |
			                  ((uint32_t)csd[8] >> 6);
			uint32_t mult = 1u << ((((csd[9] & 0x03) << 1) | (csd[10] >> 7)) + 2);
			uint32_t read_bl_len = 1u << (csd[5] & 0x0F);

			sectors = (c_size + 1) * mult * (read_bl_len / 512);
		}
	}

	cs_deselect();

	return sectors;
}
