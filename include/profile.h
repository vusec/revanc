/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "macros.h"

#if defined(__x86_64__)
#include <x86-64/profile.h>
#elif defined(__aarch64__)
#include <arm64/profile.h>
#elif defined(__arm__)
#include <arm/profile.h>
#else
#error unsupported architecture.
#endif

int init_profiler(void);
uint64_t profile_access(volatile char *p);

void profile_page_table(
	uint64_t *timings,
	struct cache *cache,
	struct page_level *level,
	size_t n,
	size_t ncache_lines,
	size_t nrounds,
	volatile char *target,
	size_t stride);
void filter_signals(
	uint64_t *timings,
	struct page_format *fmt,
	volatile void *target,
	size_t npages,
	size_t ncache_lines,
	size_t npages_per_line,
	size_t nlevel);
unsigned profile_page_tables(
	unsigned *slot_error_distances,
	struct cache *cache,
	struct page_format *fmt,
	size_t nrounds,
	volatile void *target,
	size_t run,
	const char *output_path);
