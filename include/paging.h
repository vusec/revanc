/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <macros.h>

struct page_level {
	size_t entry_size;
	size_t nentries;
	size_t table_size;
	size_t page_size;
	size_t ncache_entries;
	size_t npages;
	size_t slot_mask;
};

struct page_format {
	const char *name;
	struct page_level *levels;
	size_t nlevels;
	int flags;
};

#define PAGE_FORMAT_FILTER BIT(0)

struct page_format *get_page_format(const char *name);
struct page_format *get_default_page_format(void);
void list_page_formats(FILE *f);
