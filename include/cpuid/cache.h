/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

/* What the cache is used for. */
#define CACHE_DESC_NONE     0
#define CACHE_DESC_TLB      1
#define CACHE_DESC_CACHE    2
#define CACHE_DESC_PREFETCH 3
#define CACHE_DESC_TRACE    4

/* The level of the cache. */
#define CACHE_DESC_LEVEL(x) (((x & 0x3) << 3))

/* Whether the cache is an instruction, data or shared cache. */
#define CACHE_DESC_CODE   (1 << 5)
#define CACHE_DESC_DATA   (1 << 6)
#define CACHE_DESC_SHARED (CACHE_DESC_CODE | CACHE_DESC_DATA)

/* Whether the cache is inclusive of lower levels or not. */
#define CACHE_DESC_INCLUSIVE (1 << 8)

/* Common types. */
#define CACHE_DESC_NULL 0
#define CACHE_DESC_ITLB_L1 \
	(CACHE_DESC_TLB | CACHE_DESC_LEVEL(1) | CACHE_DESC_CODE)
#define CACHE_DESC_DTLB_L1 \
	(CACHE_DESC_TLB | CACHE_DESC_LEVEL(1) | CACHE_DESC_DATA)
#define CACHE_DESC_TLB_L1 \
	(CACHE_DESC_TLB | CACHE_DESC_LEVEL(1) | CACHE_DESC_SHARED)
#define CACHE_DESC_DTLB_L2 \
	(CACHE_DESC_TLB | CACHE_DESC_LEVEL(2) | CACHE_DESC_DATA)
#define CACHE_DESC_TLB_L2 \
	(CACHE_DESC_TLB | CACHE_DESC_LEVEL(2) | CACHE_DESC_SHARED)
#define CACHE_DESC_ICACHE_L1 \
	(CACHE_DESC_CACHE | CACHE_DESC_LEVEL(1) | CACHE_DESC_CODE)
#define CACHE_DESC_DCACHE_L1 \
	(CACHE_DESC_CACHE | CACHE_DESC_LEVEL(1) | CACHE_DESC_DATA)
#define CACHE_DESC_CACHE_L2 \
	(CACHE_DESC_CACHE | CACHE_DESC_LEVEL(2) | CACHE_DESC_SHARED)
#define CACHE_DESC_CACHE_L3 \
	(CACHE_DESC_CACHE | CACHE_DESC_LEVEL(3) | CACHE_DESC_SHARED)

/* Known constants for direct mapped and fully associative caches. */
#define CACHE_DIRECT_MAPPED 1
#define CACHE_FULLY_ASSOC (SIZE_MAX)

/* The granularity of the pages that are being cached by a TLB. */
#define TLB_4K_PAGE (1 << 0)
#define TLB_2M_PAGE (1 << 1)
#define TLB_4M_PAGE (1 << 2)
#define TLB_1G_PAGE (1 << 3)

/* The cache descriptor representing caches, TLBs, trace caches and prefetch
 * units.
 */
union cache_desc {
	struct {
		unsigned flags;
		size_t nways, size, line_size, lines_per_sector;
	} cache;
	struct {
		unsigned flags;
		size_t nways, nentries, page_size;
	} tlb;
	struct {
		unsigned flags;
		size_t size;
	} prefetch;
	unsigned flags;
};

static inline unsigned get_cache_desc_type(union cache_desc *cache_desc)
{
	return (cache_desc->flags & 0x07);
}

static inline size_t get_cache_desc_level(union cache_desc *cache_desc)
{
	return ((cache_desc->flags >> 3) & 0x03);
}

size_t copy_cache_desc(union cache_desc *cache_descs, size_t size,
	union cache_desc *cache_desc);
size_t set_tlb_desc(union cache_desc *cache_descs, size_t size,
	unsigned flags, size_t nways, size_t nentries, size_t page_size);
void print_cache_desc(union cache_desc *cache_desc);
size_t get_cache_descs(union cache_desc *cache_desc, size_t size);

