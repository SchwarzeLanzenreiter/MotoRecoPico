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

// runs the tests on the target instead of on a PC.
//
// this exists because the development machine has the Pico toolchain but no host C compiler, and
// because running them on the real silicon also proves the things a PC cannot: that the record
// layout is 16 bytes when compiled for arm, and that the arm ABI packs it the way MotoRecoViewer
// reads it.
//
// flash motorecopico_tests.uf2 onto any Pico 2 and open its USB serial port.

#include <stdio.h>

#include "pico/stdio_usb.h"
#include "pico/stdlib.h"

int motoreco_run_tests(void);

int main(void)
{
	int failures;
	int i;

	stdio_init_all();

	// the output is worthless if it is printed before the port is opened. wait up to 10s for a
	// terminal, then run anyway so the board is not stuck waiting for one
	for (i = 0; i < 100 && !stdio_usb_connected(); i++) {
		sleep_ms(100);
	}

	sleep_ms(500);

	failures = motoreco_run_tests();

	// repeat the verdict forever, so it is still there whenever the port gets opened
	while (true) {
		printf("\n%s (%d failures). connect and re-read above for details.\n",
			(failures == 0) ? "ALL TESTS PASSED" : "TESTS FAILED", failures);
		sleep_ms(5000);
	}

	return 0;
}
