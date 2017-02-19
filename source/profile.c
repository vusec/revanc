/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "cache.h"
#include "paging.h"
#include "profile.h"
#include "shuffle.h"
#include "solver.h"

#define PRIxPTR_WIDTH ((int)(2 * sizeof(uintptr_t)))

static pthread_t timer_thread;
volatile cycles_t timer_cycles;

static int cmp_uint64(const void *lhs_, const void *rhs_)
{
	const uint64_t *lhs = lhs_, *rhs = rhs_;

	if (*lhs < *rhs)
		return -1;

	if (*lhs > *rhs)
		return 1;

	return 0;
}

static FILE *vfopenf(const char *fname, const char *mode, va_list ap)
{
	FILE *f;
	char *s;

	if (vasprintf(&s, fname, ap) < 0)
		return NULL;

	f = fopen(s, mode);
	free(s);

	return f;
}

static FILE *fopenf(const char *fname, const char *mode, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, mode);
	f = vfopenf(fname, mode, ap);
	va_end(ap);

	return f;
}

static void *increment_cycles(void *data)
{
	(void)data;

	for (;;)
		++timer_cycles;

	return NULL;
}

int init_profiler(void)
{
	timer_cycles = 0;

	return pthread_create(&timer_thread, NULL, increment_cycles, NULL);
}

uint64_t profile_access(volatile char *p)
{
	uint64_t past, now;

	data_barrier();
	code_barrier();
	past = rdtsc();
	data_barrier();

	*p = 0x5A;
	
	data_barrier();
	now = rdtsc();
	code_barrier();
	data_barrier();

	return now - past;
}

void evict_cache_line(struct cache *cache, size_t table_size,
	size_t cache_line, size_t page_level)
{
	struct page_format *fmt = cache->fmt;
	struct page_level *level;
	volatile char *p = cache->data + cache_line * cache->line_size;
	size_t stride = 0;
	size_t i, j;
	
	/* Flush the given cache line from the data cache for every page table. */
	for (; p < cache->data + cache->cache_size; p += table_size) {
		*p = 0x5A;
	}

	/* Flush the TLBs and page structure caches. */
	for (j = 0, level = fmt->levels; j <= page_level; ++level, ++j) {
		stride = max(level->page_size, table_size);
		p = cache->data + cache_line * cache->line_size;

		for (i = 0; i < level->ncache_entries; ++i) {
			*p = 0x5A;
			p += stride;
		}
	}
}

static void profile_cache_lines(uint64_t *timings, struct cache *cache,
	struct page_level *level, size_t page_level, size_t *cache_lines,
	size_t ncache_lines, size_t nrounds, volatile char *page)
{
	volatile char *p;
	uint64_t timing;
	size_t cache_line;
	size_t i, j;

	for (i = 0; i < ncache_lines; ++i) {
		cache_line = cache_lines[i];
		p = page + cache_line * cache->line_size;

		for (j = 0; j < nrounds; ++j) {
			timing = UINT64_MAX;

			while (timing >= 1000) {
				evict_cache_line(cache, level->table_size, cache_line, page_level);
				timing = profile_access(p);
			}

			timings[cache_line * nrounds + j] = timing;
		}
	}
}

void profile_page_table(uint64_t *timings, struct cache *cache,
	struct page_level *level, size_t n, size_t ncache_lines, size_t nrounds,
	volatile char *target, size_t stride)
{
	volatile char *page;
	size_t *cache_lines;
	uint64_t *line_timings;
	uint64_t timing;
	size_t i, j;

	if (!(line_timings = malloc(ncache_lines * nrounds * sizeof *line_timings)))
		return;

	if (!(cache_lines = malloc(ncache_lines * sizeof *cache_lines)))
		goto err_free_line_timings;

	generate_indicies(cache_lines, ncache_lines);

	page = target;

	for (j = 0; j < level->npages; ++j) {
		profile_cache_lines(line_timings, cache, level, n,
			cache_lines, ncache_lines, nrounds, page);

		for (i = 0; i < ncache_lines; ++i) {
			qsort(line_timings + i * nrounds, nrounds,
				sizeof *line_timings, cmp_uint64);
			timing = line_timings[i * nrounds + nrounds / 2];

			timings[j * ncache_lines + i] = timing;
		}

		page += stride;
	}

err_free_line_timings:
	free(line_timings);
}

int save_timings(
	uint64_t *timings,
	struct page_level *level,
	size_t n,
	size_t ncache_lines,
	size_t run,
	const char *output_dir)
{
	uint64_t timing;
	FILE *f;
	size_t i, j;

	if (!(f = fopenf("%s/%zu-level%zu.csv", "w", output_dir, run, n + 1)))
		return -1;

	for (j = 0; j < level->npages; ++j) {
		for (i = 0; i < ncache_lines; ++i) {
			timing = timings[j * ncache_lines + i];
			fprintf(f, "%" PRIu64 " ", timing);
		}

		fprintf(f, "\n");
	}

	fclose(f);

	return 0;
}

void filter_signals(
	uint64_t *timings,
	struct page_format *fmt,
	volatile void *target,
	size_t npages,
	size_t ncache_lines,
	size_t npages_per_line,
	size_t nlevel)
{
	uint64_t timing;
	struct page_level *level;
	size_t i, slot, page, line;

	for (page = 0; page < npages; ++page) {
		timing = UINT64_MAX;

		for (line = 0; line < ncache_lines; ++line) {
			timing = min(timing, timings[page * ncache_lines + line]);
		}

		for (i = 0, level = fmt->levels; i < fmt->nlevels; ++i, ++level) {
			if (i == nlevel)
				continue;

			slot = ((uintptr_t)target / level->page_size) % level->nentries; 
			line = slot / npages_per_line;

			timings[page * ncache_lines + line] = timing;
		}
	}
}

unsigned profile_page_tables(
	unsigned *slot_error_distances,
	struct cache *cache,
	struct page_format *fmt,
	size_t nrounds,
	volatile void *target,
	size_t run,
	const char *output_dir)
{
	struct page_level *level;
	double *ntimings;
	uint64_t *timings;
	uintptr_t va = 0;
	size_t slot, page, line;
	size_t npages_per_line;
	size_t ncache_lines;
	size_t stride = 0;
	size_t i;
	size_t expected_slot, expected_page, expected_line;
	FILE *fsolutions;
	FILE *freference;
	unsigned slot_errors = 0;

	if (!(fsolutions = fopenf("%s/%zu-solutions.csv", "w", output_dir, run)))
		return 0;

	if (!(freference = fopenf("%s/%zu-reference.csv", "w", output_dir, run)))
		goto err_close_solutions;

	printf("level\tbest line\tbest page\tslot\texpected\tva\n");

	for (i = 0, level = fmt->levels; i < fmt->nlevels; ++i, ++level) {
		stride = level->page_size;

		ncache_lines = level->table_size / cache->line_size;
		npages_per_line = cache->line_size / level->entry_size;

		if (!(timings = malloc(level->npages * ncache_lines *
			sizeof *timings)))
			continue;

		if (!(ntimings = malloc(level->npages * ncache_lines *
			sizeof *ntimings))) {
			free(timings);
			continue;
		}

		profile_page_table(timings, cache, level, i, ncache_lines,
			nrounds, target, stride);
		filter_signals(timings, fmt, target, level->npages, ncache_lines,
			npages_per_line, i);
		save_timings(timings, level, i, ncache_lines, run, output_dir);
		normalise_timings(ntimings, timings, ncache_lines, level->npages);
		solve_lines(&line, &page, ntimings, ncache_lines, level->npages,
			npages_per_line);

		/* calculate the slot from the found line and page. */
		/* use the slot to calculate part of the virtual address. */
		slot = line * npages_per_line + page;
		va += slot * level->page_size;

		expected_slot = ((uintptr_t)target / level->page_size) % level->nentries;
		if (slot != expected_slot) {
			slot_error_distances[slot_errors++] = (unsigned)abs((int)slot - (int)expected_slot);
		}

		expected_line = expected_slot / npages_per_line;
		expected_page = expected_slot % npages_per_line;

		printf("%zu\t%zu\t\t%zu\t\t%zu\t%zu\t\t0x%0*" PRIxPTR " [%s]\n", i + 1, line, page,
			slot, expected_slot, PRIxPTR_WIDTH, va, slot == expected_slot ? "OK" : "!!");
		fflush(stdout);

		fprintf(fsolutions, "%zu %zu %zu\n", npages_per_line, line, page);
		fprintf(freference, "%zu %zu %zu\n", npages_per_line, expected_line, expected_page);

		free(ntimings);
		free(timings);
	}

	fclose(fsolutions);
	fclose(freference);

	printf("Guessed VA: %p\n", (void *)va);
	return slot_errors;

err_close_solutions:
	fclose(fsolutions);
	return 0;
}
