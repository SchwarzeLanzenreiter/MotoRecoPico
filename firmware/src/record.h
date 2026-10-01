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

#ifndef RECORD_H
#define RECORD_H

#include <stdint.h>

#include "motoreco.h"

// builds .dat records. no hardware here on purpose, so the encoding can be regression tested on
// the host against the byte layout MotoRecoViewer expects

#ifdef __cplusplus
extern "C" {
#endif

// microseconds since the log started -> the second / mirisecond pair the pi version writes.
// the pi took nsec/1000000, i.e. it truncated, and so does this
void record_set_elapsed(struct CANData *rec, uint64_t elapsed_us);

// fixed point encodings copied from mrlogger.c. altitude uses a factor two digits smaller than
// the rest because 1000000 overflows int32 above 1147m
int32_t record_encode_lon(double lon_deg);		// factor 1000000, offset 180
int32_t record_encode_lat(double lat_deg);		// factor 1000000, offset 90
int32_t record_encode_alt(double alt_m);		// factor 10000,   offset 1000
int32_t record_encode_speed(double speed_mps);	// factor 1000000, no offset

// virtual frame 0x7FF: longitude in bytes 0-3, latitude in bytes 4-7
void record_gps_position(struct CANData *rec, double lat_deg, double lon_deg);

// virtual frame 0x7FE: altitude in bytes 0-3, speed in bytes 4-7
void record_gps_alt_speed(struct CANData *rec, double alt_m, double speed_mps);

#ifdef __cplusplus
}
#endif

#endif // RECORD_H
