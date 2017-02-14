/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdlib.h>

#include "macros.h"

struct cache {
	struct page_format *fmt;
	char *data; // eviction set
	size_t size; // eviction set size
	size_t cache_size;
	size_t line_size;
};

struct cache *new_cache(struct page_format *fmt, void *target,
	size_t cache_size, size_t line_size);
void del_cache(struct cache *cache);
void evict_cache_line(struct cache *cache, size_t table_size,
	size_t cache_line, size_t page_level);
