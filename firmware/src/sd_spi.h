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

#ifndef SD_SPI_H
#define SD_SPI_H

#include <stdbool.h>
#include <stdint.h>

// microSD card over SPI1, the block device FatFs sits on. plain single and multiple block
// transfers, no DMA and no CRC: the card checks its own data with the CRC it appends, and the
// logger's worst case is one 512 byte write per few thousand CAN frames

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SD_OK = 0,
	SD_ERR_NO_CARD,		// the detect switch says the slot is empty
	SD_ERR_INIT,		// the card never left the idle state
	SD_ERR_TIMEOUT,		// the card stopped answering mid transfer
	SD_ERR_IO			// the card answered with an error token
} sd_result_t;

// brings up the GPIO and SPI1. does not touch the card
void sd_spi_hw_init(void);

// what the detect switch on GP7 says. advisory only: the card itself is the authority, so treat
// this as a hint for reporting and never as a reason not to try
bool sd_card_present(void);

// true once a card answered while sd_card_present() also said it was there. until then the detect
// line has not proven itself and must not be used to conclude the card is gone
bool sd_detect_reliable(void);

// runs the SPI mode initialization sequence, regardless of what the detect line says. call again
// after a card is swapped
sd_result_t sd_init(void);

bool sd_initialized(void);

// invalidates the initialized state, e.g. after the card was removed
void sd_reset_state(void);

sd_result_t sd_read_blocks(uint8_t *dst, uint32_t lba, uint32_t count);
sd_result_t sd_write_blocks(const uint8_t *src, uint32_t lba, uint32_t count);

// waits until the card has finished its internal programming
sd_result_t sd_sync(void);

// capacity in 512 byte sectors, from the CSD. 0 if it could not be read
uint32_t sd_sector_count(void);

#ifdef __cplusplus
}
#endif

#endif // SD_SPI_H
