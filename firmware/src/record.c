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

#include "record.h"

// the pi memcpy'd a native int into the payload. spell the byte order out instead, so the file is
// the same little endian layout no matter what this is compiled for
static void put_le32(char *dst, int32_t value)
{
	uint32_t u = (uint32_t)value;

	dst[0] = (char)(u & 0xFF);
	dst[1] = (char)((u >> 8) & 0xFF);
	dst[2] = (char)((u >> 16) & 0xFF);
	dst[3] = (char)((u >> 24) & 0xFF);
}

void record_set_elapsed(struct CANData *rec, uint64_t elapsed_us)
{
	rec->second     = (unsigned int)(elapsed_us / 1000000ULL);
	rec->mirisecond = (unsigned short int)((elapsed_us % 1000000ULL) / 1000ULL);
}

int32_t record_encode_lon(double lon_deg)
{
	return (int32_t)(lon_deg * 1000000.0 + 180000000.0);
}

int32_t record_encode_lat(double lat_deg)
{
	return (int32_t)(lat_deg * 1000000.0 + 90000000.0);
}

int32_t record_encode_alt(double alt_m)
{
	return (int32_t)(alt_m * 10000.0 + 10000000.0);
}

int32_t record_encode_speed(double speed_mps)
{
	return (int32_t)(speed_mps * 1000000.0);
}

void record_gps_position(struct CANData *rec, double lat_deg, double lon_deg)
{
	rec->id = GPS_CAN_ID_NUM1;
	memset(rec->data, 0, sizeof(rec->data));
	put_le32(&rec->data[0], record_encode_lon(lon_deg));
	put_le32(&rec->data[4], record_encode_lat(lat_deg));
}

void record_gps_alt_speed(struct CANData *rec, double alt_m, double speed_mps)
{
	rec->id = GPS_CAN_ID_NUM2;
	memset(rec->data, 0, sizeof(rec->data));
	put_le32(&rec->data[0], record_encode_alt(alt_m));
	put_le32(&rec->data[4], record_encode_speed(speed_mps));
}
