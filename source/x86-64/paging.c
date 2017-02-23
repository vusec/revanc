/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include "macros.h"
#include "paging.h"

struct page_level x86_64_page_levels[] = {
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 512,
		.table_size = 512 * sizeof(uint64_t),
		.page_size = 4 * KIB,
		.slot_mask = 0x1ff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 512,
		.table_size = 512 * sizeof(uint64_t),
		.page_size = 2 * MIB,
		.slot_mask = 0x1ff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 512,
		.table_size = 512 * sizeof(uint64_t),
		.page_size = 1 * GIB,
		.slot_mask = 0x1ff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 512,
		.table_size = 512 * sizeof(uint64_t),
		.page_size = 512 * GIB,
		.slot_mask = 0x1ff,
	},
};

struct page_format page_formats[] = {
	{ "x86-64", x86_64_page_levels, 4, PAGE_FORMAT_FILTER },
	{ NULL, 0, 0, 0 },
};

