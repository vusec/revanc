/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdint.h>

#include "macros.h"

void normalise_timings(double *ntimings, uint64_t *timings,
	size_t ncache_lines, size_t npages);
double solve_line(double *timings, size_t line, size_t page,
	size_t ncache_lines, size_t npages, size_t npages_per_line);
void solve_lines(size_t *best_line, size_t *best_page,
	double *timings, size_t ncache_lines, size_t npages,
	size_t npages_per_line);
