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

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "config.h"
#include "led.h"

static led_state_t g_state = LED_ERROR;
static uint64_t g_last_toggle_us;
static bool g_lit;

static void write_led(bool lit)
{
	g_lit = lit;
	gpio_put(PIN_LED, lit ? 0 : 1);	// low side drive, so low is on
}

void led_init(void)
{
	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);
	write_led(false);

	g_last_toggle_us = 0;
	g_state = LED_ERROR;
}

void led_set(led_state_t state)
{
	if (state == g_state) {
		return;
	}

	g_state = state;

	// start every pattern lit, so a change is visible immediately rather than up to a blink
	// later. idle is the exception: it means there is nothing to show
	write_led(state != LED_IDLE);
	g_last_toggle_us = time_us_64();
}

void led_update(uint64_t now_us)
{
	uint32_t period_ms;

	if (g_state == LED_IDLE) {
		if (g_lit) {
			write_led(false);
		}
		return;
	}

	if (g_state == LED_LOGGING) {
		if (!g_lit) {
			write_led(true);
		}
		return;
	}

	period_ms = (g_state == LED_NO_FIX) ? LED_BLINK_NOFIX_MS : LED_BLINK_ERROR_MS;

	if (now_us - g_last_toggle_us >= (uint64_t)period_ms * 1000ULL) {
		g_last_toggle_us = now_us;
		write_led(!g_lit);
	}
}
