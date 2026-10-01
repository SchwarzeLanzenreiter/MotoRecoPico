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

#ifndef MOTORECO_H
#define MOTORECO_H

#include <stddef.h>
#include <stdint.h>

// the on disk record of the .dat trip log. this is the raspberry pi version's struct
// (MotoRecoLogger/motoreco.h) copied verbatim, because MotoRecoViewer reads these files as a flat
// sequence of 16 byte little endian records with no header (see Io/CanDataFile.cs). the field
// types must stay exactly as they are, misspelling included, or the two loggers stop producing
// interchangeable files
struct CANData {
	unsigned int		second;
	unsigned short int 	mirisecond;
	unsigned short int 	id;
	char 				data[8];
};

// the pi built this with arm gcc where the layout happens to be dense. nothing guarantees that on
// another compiler, so state it. these are what MotoRecoViewer's ParseRecord() indexes into
#if defined(__cplusplus)
	#define MR_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(_MSC_VER) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
	#define MR_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
	#define MR_STATIC_ASSERT(cond, msg) typedef char mr_static_assert_##__LINE__[(cond) ? 1 : -1]
#endif

MR_STATIC_ASSERT(sizeof(struct CANData) == 16,        "CANData must stay 16 bytes");
MR_STATIC_ASSERT(sizeof(unsigned int) == 4,           "CANData.second must stay 4 bytes");
MR_STATIC_ASSERT(sizeof(unsigned short int) == 2,     "CANData.id must stay 2 bytes");
MR_STATIC_ASSERT(offsetof(struct CANData, second) == 0,      "second must be at offset 0");
MR_STATIC_ASSERT(offsetof(struct CANData, mirisecond) == 4,  "mirisecond must be at offset 4");
MR_STATIC_ASSERT(offsetof(struct CANData, id) == 6,          "id must be at offset 6");
MR_STATIC_ASSERT(offsetof(struct CANData, data) == 8,        "data must be at offset 8");

// virtual CAN ids the pi version reserves for GPS. real bus traffic must not use them
#define GPS_CAN_ID_NUM1 2047	// longitude and latitude. 2047 = "7FF"
#define GPS_CAN_ID_NUM2 2046	// altitude and speed.    2046 = "7FE"

#endif // MOTORECO_H
