/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stdlib.h>

#include "macros.h"
#include "solver.h"

void normalise_timings(double *ntimings, uint64_t *timings,
	size_t ncache_lines, size_t npages)
{
	size_t x, y;
	uint64_t timing, lo, hi;
	double ntiming;

	for (y = 0; y < npages; ++y) {
		lo = UINT64_MAX;
		hi = 0;

		for (x = 0; x < ncache_lines; ++x) {
			timing = timings[y * ncache_lines + x];

			lo = min(lo, timing);
			hi = max(hi, timing);
		}

		for (x = 0; x < ncache_lines; ++x) {
			timing = timings[y * ncache_lines + x] - lo;

			if (hi == lo) {
				ntiming = 1.0;
			} else {
				ntiming = timing / (hi - lo);
			}

			ntimings[y * ncache_lines + x] = ntiming;
		}
	}
}

double solve_line(double *timings, size_t line, size_t page,
	size_t ncache_lines, size_t npages, size_t npages_per_line)
{
	/* calculate the score by taking the sum of all the points across
	 * a line, where timings is a matrix with timings, line is the x
	 * offset, page is the y offset, ncache_lines is the amount of cache
	 * lines, npages is the amount of pages and where npages_per_line is
	 * the expected amount of pages per line.
	 */

	double sum = 0;

	size_t row, col;
	for (row = 0; row < npages; ++row) {
		col = (line + (row + page) / npages_per_line) % ncache_lines;
		sum += timings[col + row * ncache_lines];
	}

	return sum;
}

void solve_lines(size_t *best_line, size_t *best_page,
	double *timings, size_t ncache_lines, size_t npages,
	size_t npages_per_line)
{
	/* solve all possibilities using solve_line and pick the best one
	 * and store it in best_line and best_page.
	 */

	size_t line, page;
	double line_sum;
	double best_sum = 0;

	for (line = 0; line < ncache_lines; ++line) {
		for (page = 0; page < npages_per_line; ++page) {
			line_sum = solve_line(timings, line, page, ncache_lines, npages, npages_per_line);

			if (line_sum > best_sum) {
				best_sum = line_sum;
				*best_line = line;
				*best_page = page;
			}
		}
	}
}
