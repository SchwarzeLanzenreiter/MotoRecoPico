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

#ifndef POWER_H
#define POWER_H

// ignition detection on GP28.
//
// docs/netlist.md section 4.7 is the contract. the board gives the firmware a guaranteed 0.4s
// (typically 0.5s) of life after the key is turned off: C13 holds Q2's gate charge so the high
// side switch stays on while the gate discharges through R1. in that window the log has to be
// flushed, closed and renamed.
//
// what the pin actually carries changed in v3 and is easy to get wrong. it is not a divided
// battery voltage any more. R15 taps Q1's base node, which Q1's own base-emitter junction clamps
// to roughly 0.65V while the ignition is live, and 0V when it is not. so this reads ignition
// on/off, never the battery voltage, and it has to be the ADC because 0.65V is well under the
// digital input's threshold.
//
// no hardware here: the caller samples the ADC and feeds millivolts in, which keeps the state
// machine testable on a PC

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	POWER_EVENT_NONE = 0,
	POWER_EVENT_OFF,		// ignition lost. close the log now, the supply is on borrowed time
	POWER_EVENT_RESTORED	// ignition back while still alive (bench on USB, or a key flicked back)
} power_event_t;

typedef struct {
	int	armed;		// the ignition has been seen live at least once
	int	off;		// currently considered off
	int	low_count;
	int	high_count;
	int	mv;			// last sample, for reporting
} power_state_t;

void power_init(power_state_t *st);

// feed one ADC sample, in millivolts at the pin
power_event_t power_eval(power_state_t *st, int mv);

#ifdef __cplusplus
}
#endif

#endif // POWER_H
