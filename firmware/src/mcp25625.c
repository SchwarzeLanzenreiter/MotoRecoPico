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
#include "mcp25625.h"

// SPI instructions
#define CMD_RESET		0xC0
#define CMD_READ		0x03
#define CMD_WRITE		0x02
#define CMD_READ_RX0	0x90	// starts at RXB0SIDH, clears RX0IF when CS goes high
#define CMD_READ_RX1	0x94	// starts at RXB1SIDH, clears RX1IF when CS goes high
#define CMD_RTS_TX0		0x81
#define CMD_READ_STATUS	0xA0
#define CMD_BIT_MODIFY	0x05

// registers
#define REG_CANSTAT		0x0E
#define REG_CANCTRL		0x0F
#define REG_EFLG		0x2D
#define REG_CANINTE		0x2B
#define REG_CANINTF		0x2C
#define REG_CNF3		0x28
#define REG_CNF2		0x29
#define REG_CNF1		0x2A
#define REG_TXB0CTRL	0x30
#define REG_TXB0SIDH	0x31
#define REG_RXB0CTRL	0x60
#define REG_RXB1CTRL	0x70

// board v4.1 moved the controller from SPI0 to SPI1 (docs/netlist.md section 3.7). the asserts
// below are what catches a pin edit that forgets to bring the peripheral along
#define CAN_SPI	spi1

_Static_assert(SPI_INSTANCE_OF(PIN_CAN_SCK) == 1 && SPI_FUNC_OF(PIN_CAN_SCK) == SPI_FUNC_SCK,
	"PIN_CAN_SCK is not an SPI1 SCK pin");
_Static_assert(SPI_INSTANCE_OF(PIN_CAN_MOSI) == 1 && SPI_FUNC_OF(PIN_CAN_MOSI) == SPI_FUNC_TX,
	"PIN_CAN_MOSI is not an SPI1 TX pin");
_Static_assert(SPI_INSTANCE_OF(PIN_CAN_MISO) == 1 && SPI_FUNC_OF(PIN_CAN_MISO) == SPI_FUNC_RX,
	"PIN_CAN_MISO is not an SPI1 RX pin");

#define CANCTRL_REQOP_MASK		0xE0
#define CANCTRL_REQOP_NORMAL	0x00
#define CANCTRL_REQOP_LOOPBACK	0x40
#define CANCTRL_REQOP_CONFIG		0x80

#define EFLG_RX0OVR	0x40
#define EFLG_RX1OVR	0x80

#define STATUS_RX0IF	0x01
#define STATUS_RX1IF	0x02

// see config.h for how these are derived. index with CAN_BITRATE_*
static const uint8_t CNF_TABLE[4][3] = {
	{ 0x00, 0xB3, 0x03 },	// 500k: BRP=0, 16TQ, sample at 75%
	{ 0x01, 0xB3, 0x03 },	// 250k: BRP=1
	{ 0x03, 0xB3, 0x03 },	// 125k: BRP=3
	{ 0x00, 0x91, 0x01 }	// 1M:   BRP=0, 8TQ
};

static can_frame_t g_ring[CAN_RING_RECORDS];
static volatile uint32_t g_head;	// written by the producer (interrupt), read by the consumer
static volatile uint32_t g_tail;	// written by the consumer (main loop)
static volatile uint32_t g_rx_count;
static volatile uint32_t g_ring_drops;
static uint32_t g_overflow_count;

static inline void cs_select(void)
{
	gpio_put(PIN_CAN_CS, 0);
	__asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(void)
{
	__asm volatile("nop \n nop \n nop");
	gpio_put(PIN_CAN_CS, 1);
}

static uint8_t mcp_read_reg(uint8_t reg)
{
	uint8_t tx[3] = { CMD_READ, reg, 0x00 };
	uint8_t rx[3];

	cs_select();
	spi_write_read_blocking(CAN_SPI, tx, rx, 3);
	cs_deselect();

	return rx[2];
}

static void mcp_write_reg(uint8_t reg, uint8_t value)
{
	uint8_t tx[3] = { CMD_WRITE, reg, value };

	cs_select();
	spi_write_blocking(CAN_SPI, tx, 3);
	cs_deselect();
}

static void mcp_write_regs(uint8_t reg, const uint8_t *values, size_t len)
{
	uint8_t hdr[2] = { CMD_WRITE, reg };

	cs_select();
	spi_write_blocking(CAN_SPI, hdr, 2);
	spi_write_blocking(CAN_SPI, values, len);
	cs_deselect();
}

static void mcp_bit_modify(uint8_t reg, uint8_t mask, uint8_t value)
{
	uint8_t tx[4] = { CMD_BIT_MODIFY, reg, mask, value };

	cs_select();
	spi_write_blocking(CAN_SPI, tx, 4);
	cs_deselect();
}

static uint8_t mcp_read_status(void)
{
	uint8_t tx[2] = { CMD_READ_STATUS, 0x00 };
	uint8_t rx[2];

	cs_select();
	spi_write_read_blocking(CAN_SPI, tx, rx, 2);
	cs_deselect();

	return rx[1];
}

// reads one receive buffer with the instruction that clears its interrupt flag on the way out.
// buf gets SIDH, SIDL, EID8, EID0, DLC and the eight data bytes
static void mcp_read_rx(uint8_t cmd, uint8_t *buf)
{
	cs_select();
	spi_write_blocking(CAN_SPI, &cmd, 1);
	spi_read_blocking(CAN_SPI, 0x00, buf, 13);
	cs_deselect();
}

static void push_frame(const uint8_t *buf, uint64_t now_us)
{
	uint32_t head = g_head;
	uint32_t next = (head + 1) % CAN_RING_RECORDS;
	can_frame_t *slot;
	uint8_t sidl = buf[1];
	uint8_t dlc = buf[4] & 0x0F;

	g_rx_count++;

	// full ring. drop the newest frame rather than the oldest, so what is already on its way to
	// the card stays a contiguous stretch of the trip
	if (next == g_tail) {
		g_ring_drops++;
		return;
	}

	slot = &g_ring[head];
	slot->timestamp_us = now_us;
	slot->extended = (sidl & 0x08) ? 1 : 0;

	if (slot->extended) {
		slot->id = ((uint32_t)buf[0] << 21) |
		           (((uint32_t)sidl & 0xE0) << 13) |
		           (((uint32_t)sidl & 0x03) << 16) |
		           ((uint32_t)buf[2] << 8) |
		           (uint32_t)buf[3];
		slot->rtr = (buf[4] & 0x40) ? 1 : 0;
	} else {
		slot->id = ((uint32_t)buf[0] << 3) | ((uint32_t)sidl >> 5);
		slot->rtr = (sidl & 0x10) ? 1 : 0;	// SRR
	}

	slot->dlc = (dlc > 8) ? 8 : dlc;
	memcpy(slot->data, &buf[5], 8);

	g_head = next;
}

// drains every filled receive buffer. called from the interrupt, and from the main loop while the
// interrupt is masked
static void service_rx(void)
{
	uint8_t buf[13];
	uint8_t status;
	int guard;

	// the INT pin stays asserted while any flag is set, so one interrupt can cover both buffers.
	// the guard is there because a wedged controller reporting a flag it never clears would
	// otherwise spin here forever
	for (guard = 0; guard < 4; guard++) {
		status = mcp_read_status();

		if (status & STATUS_RX0IF) {
			mcp_read_rx(CMD_READ_RX0, buf);
			push_frame(buf, time_us_64());
		} else if (status & STATUS_RX1IF) {
			mcp_read_rx(CMD_READ_RX1, buf);
			push_frame(buf, time_us_64());
		} else {
			return;
		}
	}
}

static void can_irq_handler(uint gpio, uint32_t events)
{
	(void)gpio;
	(void)events;

	service_rx();
}

// the main loop has to talk to the controller too (overflow flags, self test). SPI0 is not shared
// with anything else, but it is shared with this driver's own interrupt, so mask that first
static void mcp_lock(void)
{
	gpio_set_irq_enabled(PIN_CAN_INT, GPIO_IRQ_EDGE_FALL, false);
}

static void mcp_unlock(void)
{
	gpio_set_irq_enabled(PIN_CAN_INT, GPIO_IRQ_EDGE_FALL, true);

	// INT is a level, not a pulse. if a frame arrived while the interrupt was masked there is no
	// edge left to catch it, so pick it up here
	if (!gpio_get(PIN_CAN_INT)) {
		mcp_lock();
		service_rx();
		gpio_set_irq_enabled(PIN_CAN_INT, GPIO_IRQ_EDGE_FALL, true);
	}
}

static bool set_mode(uint8_t reqop)
{
	int i;

	mcp_bit_modify(REG_CANCTRL, CANCTRL_REQOP_MASK, reqop);

	// the controller only changes mode at a bus idle point, so this is not immediate
	for (i = 0; i < 100; i++) {
		if ((mcp_read_reg(REG_CANSTAT) & CANCTRL_REQOP_MASK) == reqop) {
			return true;
		}

		sleep_ms(1);
	}

	return false;
}

bool mcp25625_init(bool loopback)
{
	uint8_t cnf[3];

	g_head = 0;
	g_tail = 0;
	g_rx_count = 0;
	g_ring_drops = 0;
	g_overflow_count = 0;

	spi_init(CAN_SPI, CAN_SPI_BAUD);
	gpio_set_function(PIN_CAN_SCK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_CAN_MOSI, GPIO_FUNC_SPI);
	gpio_set_function(PIN_CAN_MISO, GPIO_FUNC_SPI);

	gpio_init(PIN_CAN_CS);
	gpio_set_dir(PIN_CAN_CS, GPIO_OUT);
	gpio_put(PIN_CAN_CS, 1);

	// STBY low keeps the transceiver awake. it has no pull down of its own on the board
	gpio_init(PIN_CAN_STBY);
	gpio_set_dir(PIN_CAN_STBY, GPIO_OUT);
	gpio_put(PIN_CAN_STBY, 0);

	gpio_init(PIN_CAN_INT);
	gpio_set_dir(PIN_CAN_INT, GPIO_IN);
	gpio_pull_up(PIN_CAN_INT);

	// R6 already pulls RESET up. drive it low briefly anyway, so a warm restart of the firmware
	// starts from the same state as a cold one
	gpio_init(PIN_CAN_RESET);
	gpio_set_dir(PIN_CAN_RESET, GPIO_OUT);
	gpio_put(PIN_CAN_RESET, 0);
	sleep_ms(1);
	gpio_put(PIN_CAN_RESET, 1);
	sleep_ms(10);

	cs_select();
	{
		uint8_t cmd = CMD_RESET;
		spi_write_blocking(CAN_SPI, &cmd, 1);
	}
	cs_deselect();
	sleep_ms(10);

	// after a reset the controller must be in configuration mode. if it is not, either the SPI
	// wiring is wrong or the chip is not powered, and there is no point configuring anything
	if ((mcp_read_reg(REG_CANSTAT) & CANCTRL_REQOP_MASK) != CANCTRL_REQOP_CONFIG) {
		return false;
	}

	memcpy(cnf, CNF_TABLE[CAN_BITRATE], sizeof(cnf));
	mcp_write_reg(REG_CNF1, cnf[0]);
	mcp_write_reg(REG_CNF2, cnf[1]);
	mcp_write_reg(REG_CNF3, cnf[2]);

	// receive everything. this is a logger, filtering would only lose data. BUKT lets a frame
	// that arrives while RXB0 is still full roll over into RXB1 instead of being dropped
	mcp_write_reg(REG_RXB0CTRL, 0x64);	// RXM = 11 (any), BUKT = 1
	mcp_write_reg(REG_RXB1CTRL, 0x60);	// RXM = 11 (any)

	// only the two receive flags drive the INT pin. error and overflow conditions are polled from
	// the main loop instead, so a noisy bus cannot bury the receive path in interrupts
	mcp_write_reg(REG_CANINTE, 0x03);
	mcp_write_reg(REG_CANINTF, 0x00);
	mcp_write_reg(REG_EFLG, 0x00);

	// CLKOUT stays off: the pin is not connected on this board (docs/netlist.md section 3.6)
	if (!set_mode(loopback ? CANCTRL_REQOP_LOOPBACK : CANCTRL_REQOP_NORMAL)) {
		return false;
	}

	gpio_set_irq_enabled_with_callback(PIN_CAN_INT, GPIO_IRQ_EDGE_FALL, true, &can_irq_handler);

	// a frame may already be waiting if the bus was live before we finished configuring
	mcp_unlock();

	return true;
}

bool mcp25625_pop(can_frame_t *out)
{
	uint32_t tail = g_tail;

	if (tail == g_head) {
		return false;
	}

	*out = g_ring[tail];
	g_tail = (tail + 1) % CAN_RING_RECORDS;

	return true;
}

uint32_t mcp25625_check_overflow(void)
{
	uint8_t eflg;
	uint32_t added = 0;

	mcp_lock();
	eflg = mcp_read_reg(REG_EFLG);

	if (eflg & (EFLG_RX0OVR | EFLG_RX1OVR)) {
		// the flags do not count, they only say "at least one frame was lost since you last
		// looked". clear them and count the episode
		mcp_bit_modify(REG_EFLG, EFLG_RX0OVR | EFLG_RX1OVR, 0x00);
		added = 1;
		g_overflow_count++;
	}

	mcp_unlock();

	return added;
}

uint32_t mcp25625_ring_drops(void)
{
	return g_ring_drops;
}

uint32_t mcp25625_rx_count(void)
{
	return g_rx_count;
}

bool mcp25625_send(uint32_t id, const uint8_t *data, uint8_t dlc)
{
	uint8_t buf[13];
	uint8_t cmd = CMD_RTS_TX0;
	int i;
	bool sent = false;

	if (dlc > 8) {
		return false;
	}

	memset(buf, 0, sizeof(buf));
	buf[0] = (uint8_t)(id >> 3);			// SIDH
	buf[1] = (uint8_t)((id & 0x07) << 5);	// SIDL, standard id only
	buf[4] = dlc;							// TXB0DLC
	memcpy(&buf[5], data, dlc);

	mcp_lock();

	mcp_write_regs(REG_TXB0SIDH, buf, 13);

	cs_select();
	spi_write_blocking(CAN_SPI, &cmd, 1);
	cs_deselect();

	// wait for the transmit request to clear. in loopback this is immediate, on a real bus it
	// waits for an acknowledge and can fail if nothing else is listening
	for (i = 0; i < 100; i++) {
		if ((mcp_read_reg(REG_TXB0CTRL) & 0x08) == 0) {
			sent = true;
			break;
		}

		sleep_ms(1);
	}

	mcp_unlock();

	return sent;
}
