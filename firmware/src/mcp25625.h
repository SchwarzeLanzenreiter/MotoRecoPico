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

#ifndef MCP25625_H
#define MCP25625_H

#include <stdbool.h>
#include <stdint.h>

// MCP25625 (MCP2515 controller core plus transceiver) on SPI0.
//
// receiving happens in the interrupt handler, not in the main loop. the controller only has two
// receive buffers, so a single SD write stall of a few hundred milliseconds would overflow them
// several times over if frames were only collected between writes. the handler drains the chip
// into a ring buffer and the main loop takes its time emptying that

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint64_t	timestamp_us;	// time_us_64() when the interrupt was serviced
	uint32_t	id;
	uint8_t		dlc;
	uint8_t		extended;		// 29 bit id. the .dat format cannot hold these
	uint8_t		rtr;			// remote request, no payload
	uint8_t		data[8];
} can_frame_t;

// brings up SPI0 and the controller. loopback is the bench self test mode: frames sent with
// mcp25625_send() come straight back without a bus or an acknowledge
bool mcp25625_init(bool loopback);

// takes one frame out of the receive ring. false when the ring is empty
bool mcp25625_pop(can_frame_t *out);

// reads and clears the controller's receive overflow flags. returns how many new overflows were
// seen. this is the equivalent of the SO_RXQ_OVFL counter the raspberry pi version reported:
// frames lost this way leave no trace in the .dat file
uint32_t mcp25625_check_overflow(void);

// frames dropped because the ring was full, cumulative
uint32_t mcp25625_ring_drops(void);

// frames received since boot, cumulative
uint32_t mcp25625_rx_count(void);

// sends a standard frame. only used by the loopback self test
bool mcp25625_send(uint32_t id, const uint8_t *data, uint8_t dlc);

#ifdef __cplusplus
}
#endif

#endif // MCP25625_H
