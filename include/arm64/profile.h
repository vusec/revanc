/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdint.h>

#include "config.h"

typedef uint64_t cycles_t;

extern volatile cycles_t timer_cycles;

static inline void code_barrier(void)
{
	asm volatile("isb\n");
}

static inline void data_barrier(void)
{
	asm volatile("dsb sy\n" ::: "memory");
}

static inline cycles_t rdtsc(void)
{
	return timer_cycles;
}

