/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))

#define KIB ((size_t)1024)
#define MIB (1024 * KIB)
#define GIB (1024 * MIB)

#if SIZE_MAX == UINT64_MAX
#define TIB (1024 * GIB)
#endif

/* Used to extract a bit field. */
#define BIT(n) (1 << (n))
#define EXTRACT(x, k, n) ((x) >> (k) & ((1 << (n)) - 1))

/* Represents a register that is accessible using 8-bit, 16-bit and 32-bit
 * granularity.
 */
union reg {
	uint32_t u32;
	uint16_t u16[2];
	uint8_t u8[4];
};

#define dperror() dperror_ext(__FILE__, __LINE__)
#define dprintf(...) \
	do { \
		fprintf(stderr, "%s:%d: error: ", __FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
	} while(0)

void dperror_ext(const char *fname, int line_no);
