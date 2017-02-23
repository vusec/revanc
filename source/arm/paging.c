/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include "macros.h"
#include "paging.h"

struct page_level arm_page_levels[] = {
	{
		.entry_size = sizeof(uint32_t),
		.nentries = 256,
		.table_size = 256 * sizeof(uint32_t),
		.page_size = 4 * KIB,
		.slot_mask = 0xff,
	},
	{
		.entry_size = sizeof(uint32_t),
		/* actually 4096. */
		.nentries = 1024,
		.table_size = 1024 * sizeof(uint32_t),
		.page_size = 1 * MIB,
		.slot_mask = 0x3ff,
	},
};

struct page_level arm_lpae_page_levels[] = {
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
		.nentries = 4,
		.table_size = 4 * sizeof(uint64_t),
		.page_size = 1 * GIB,
	},
};

struct page_format page_formats[] = {
	{ "arm", arm_page_levels, 2, 0 },
	{ "arm-lpae", arm_lpae_page_levels, 2, PAGE_FORMAT_FILTER },
	{ NULL, 0, 0, 0 },
};
