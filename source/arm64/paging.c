/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include "macros.h"
#include "paging.h"

struct page_level arm64_4k_page_levels[] = {
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

struct page_level arm64_16k_page_levels[] = {
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 2048,
		.table_size = 2048 * sizeof(uint64_t),
		.page_size = 16 * KIB,
		.slot_mask = 0x7ff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 2048,
		.table_size = 2048 * sizeof(uint64_t),
		.page_size = 32 * MIB,
		.slot_mask = 0x7ff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 2048,
		.table_size = 2048 * sizeof(uint64_t),
		.page_size = 64 * GIB,
		.slot_mask = 0x7ff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 2,
		.table_size = 2 * sizeof(uint64_t),
		.page_size = 128 * TIB,
	},
};

struct page_level arm64_64k_page_levels[] = {
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 8192,
		.table_size = 8192 * sizeof(uint64_t),
		.page_size = 64 * KIB,
		.slot_mask = 0x1fff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 8192,
		.table_size = 8192 * sizeof(uint64_t),
		.page_size = 512 * MIB,
		.slot_mask = 0x1fff,
	},
	{
		.entry_size = sizeof(uint64_t),
		.nentries = 64,
		.table_size = 64 * sizeof(uint64_t),
		.page_size = 4 * TIB,
		.slot_mask = 0x3f,
	},
};

struct page_format page_formats[] = {
	{ "arm64-4K-va39", arm64_4k_page_levels, 3, PAGE_FORMAT_FILTER },
	{ "arm64-4K-va48", arm64_4k_page_levels, 4, PAGE_FORMAT_FILTER },
	{ "arm64-16K", arm64_16k_page_levels, 4, PAGE_FORMAT_FILTER },
	{ "arm64-64K", arm64_64k_page_levels, 3, PAGE_FORMAT_FILTER },
	{ NULL, 0, 0, 0 },
};

