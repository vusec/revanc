/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include "args.h"
#include "cache.h"
#include "paging.h"
#include "macros.h"

#ifndef MAP_NORESERVE
#define MAP_NORESERVE 0
#endif /* MAP_NORESERVE */

struct cache *new_cache(struct page_format *fmt, void *target,
	size_t cache_size, size_t line_size)
{
	struct cache *cache;
	struct page_level *level;
	size_t stride = 0;
	size_t i;
	unsigned flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE;

	if (target)
		flags |= MAP_FIXED;

	if (!(cache = malloc(sizeof *cache)))
		return NULL;

	cache->fmt = fmt;
	cache->cache_size = cache_size;
	cache->line_size = line_size;

	// calculate the buffer size needed to evict this cache
	cache->size = cache_size;

	for (i = 0, level = fmt->levels; i < fmt->nlevels; ++i, ++level) {
		stride = max(level->page_size, level->table_size);
		cache->size = max(cache->size, level->ncache_entries * stride);
	}

	if ((cache->data = mmap(target, cache->size, PROT_READ | PROT_WRITE, flags,
		-1, 0)) == MAP_FAILED) {
		dperror();
		goto err_free_cache;
	}

	return cache;

err_free_cache:
	free(cache);
	return NULL;
}

void del_cache(struct cache *cache)
{
	munmap(cache->data, cache->size);
	free(cache);
}
