/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "args.h"
#include "buffer.h"
#include "cache.h"
#include "paging.h"
#include "profile.h"
#include "shuffle.h"
#include "solver.h"
#include "sysfs.h"
#include "thread.h"
#include "macros.h"
#include "path.h"

#if defined(__i386__) || defined(__x86_64__)
#include <cpuid/cache.h>
#include <cpuid/cpuid.h>
#endif

int brute_force_evict_set(struct page_format *fmt, void *evict_target,
	size_t cache_size, size_t line_size, size_t nrounds, size_t nruns,
	float threshold, volatile void *target)
{
	struct cache *cache;
	struct page_level *level;
	double *ntimings;
	uint64_t *timings;
	size_t ncache_lines, npages_per_line;
	size_t stride = 0;
	size_t slot, page, line;
	size_t expected_slot;
	size_t i;
	size_t mult2, mult3;

	for (i = 0, level = fmt->levels; i < fmt->nlevels; ++i, ++level) {
		if (level->npages == 0)
			continue;

		stride = level->page_size;

		ncache_lines = level->table_size / line_size;
		npages_per_line = line_size / level->entry_size;

		expected_slot = ((uintptr_t)target / level->page_size) % level->nentries;
		slot = SIZE_MAX;

		if (!(timings = malloc(level->npages * ncache_lines *
			sizeof *timings)))
			continue;

		if (!(ntimings = malloc(level->npages * ncache_lines *
			sizeof *ntimings))) {
			free(timings);
			continue;
		}

		mult2 = 1;
		mult3 = 3;
		size_t run = 0;
		size_t success = 0;
		float rate;

		for (;;) {
			if (!(cache = new_cache(fmt, evict_target, cache_size,
				line_size))) {
				dprintf("unable to allocate the eviction set.\n");
				free(timings);
				free(ntimings);
				return -1;
			}

			success = 0;

			printf("probing %zu [", level->ncache_entries);
			fflush(stdout);

			for (run = 0; run < nruns; ++run) {
				profile_page_table(timings, cache, level, i, ncache_lines,
					nrounds, target, stride);

				if (fmt->flags & PAGE_FORMAT_FILTER)
					filter_signals(timings, fmt, target, level->npages,
						ncache_lines, npages_per_line, i);
				normalise_timings(ntimings, timings, ncache_lines, level->npages);
				solve_lines(&line, &page, ntimings, ncache_lines, level->npages,
					npages_per_line);

				slot = line * npages_per_line + page;

				slot &= level->slot_mask;
				expected_slot &= level->slot_mask;

				if (fabs((float)slot - expected_slot) <= 1.0) {
					++success;
					putc('#', stdout);
				} else {
					putc('.', stdout);
				}

				fflush(stdout);
			}

			printf("]\n");
			del_cache(cache);

			rate = 100.0f * success / nruns;

			if (rate >= threshold) {
				break;
			}

			if (mult2 < mult3) {
				level->ncache_entries = mult2;
				mult2 *= 2;
			} else {
				level->ncache_entries = mult3;
				mult3 *= 2;
			}
		}

		printf("found PL%zu cache entries: %zu\n", (i + 1),
			level->ncache_entries);

		free(ntimings);
		free(timings);
	}

	return 0;
}

int main(int argc, const char *argv[])
{
	struct args args = {
		.npages = { 128, 128, 128, 128 },
		.nentries = { SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX },
		.nrounds = 10,
		.line_size = 64,
		.nruns = 1,
		.threshold = 70.0,
		.output = "results",
		.target = 0,
		.evict_target = 0,
	};
	struct buffer *buffer;
	struct page_format *page_format;

	if (check_transparent_hugepages()) {
		dprintf("transparent huge pages seem to be enabled.\n"
			"please run 'echo \"never\" > /sys/kernel/mm/transparent_hugepage/"
			"enabled' as root.\n");
		return -1;
	}

	if (parse_args(&args, argc, argv) < 0) {
		show_usage(argv[0]);
		return -1;
	}

	detect_args(&args);

	if (!args.line_size) {
		dprintf("unable to detect line size, please specify the cache "
			"line size using --line-size.\n");
		return -1;
	}

	if (!args.cache_size) {
		dprintf("unable to detect cache size, please specify the "
			"cache size using --cache-size.\n");
		return -1;
	}

	if (!(page_format = get_page_format_from_args(&args))) {
		dprintf("unknown page format '%s', please use "
			"--list-page-formats to list all available page "
			"formats and specify the page format using "
			"--page-format.\n", args.page_format);
		return -1;
	}

	if (init_profiler() < 0) {
		dprintf("unable to set up the profiler.\n");
		return -1;
	}

	if (pin_cpu(args.cpu) != 0) {
		dprintf("unable to pin the thread.\n");
		return -1;
	}

	if (!(buffer = new_buffer(page_format, (void *)args.target))) {
		dprintf("unable to allocate the target buffer.\n");
		return -1;
	}

#if defined(__i386__) || defined(__x86_64__)
	printf("Detected CPU name: %s (%s)\n\n", cpuid_get_cpu_name(), cpuid_get_cpu_model());
#endif

	srand(time(0));

	brute_force_evict_set(page_format, (void *)args.evict_target,
		args.cache_size, args.line_size, args.nrounds, args.nruns,
		args.threshold, buffer->data);

	del_buffer(buffer);

	if (args.page_format)
		free(args.page_format);

	return 0;
}
