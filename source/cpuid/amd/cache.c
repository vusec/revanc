/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cpuid/cache.h>
#include <cpuid/cpuid.h>

#include "macros.h"

static size_t ext_nways[] = {
	[0x0] = 0,
	[0x1] = 1,
	[0x2] = 2,
	[0x3] = 3,
	[0x4] = 4,
	[0x5] = 6,
	[0x6] = 8,
	[0x8] = 16,
	[0xA] = 32,
	[0xB] = 48,
	[0xC] = 64,
	[0xD] = 96,
	[0xE] = 128,
	[0xF] = CACHE_FULLY_ASSOC,
};

size_t get_tlb_info_v1(union cache_desc *cache_desc, size_t size, uint32_t reg,
	size_t level, size_t page_size)
{
	size_t n = 0;

	if (EXTRACT(reg, 8, 8)) {
		if (cache_desc && n < size) {
			cache_desc->tlb.flags = CACHE_DESC_TLB |
				CACHE_DESC_CODE | CACHE_DESC_LEVEL(level);
			cache_desc->tlb.nways = EXTRACT(reg, 8, 8);
			cache_desc->tlb.nentries = EXTRACT(reg, 0, 8);
			cache_desc->tlb.page_size = page_size;
			++cache_desc;
		}

		++n;
	}

	if (EXTRACT(reg, 24, 8)) {
		if (cache_desc && n < size) {
			cache_desc->tlb.flags = CACHE_DESC_TLB |
				CACHE_DESC_DATA | CACHE_DESC_LEVEL(level);
			cache_desc->tlb.nways = EXTRACT(reg, 24, 8);
			cache_desc->tlb.nentries = EXTRACT(reg, 16, 8);
			cache_desc->tlb.page_size = page_size;
			++cache_desc;
		}

		++n;
	}

	return n;
}

size_t get_tlb_info_v2(union cache_desc *cache_desc, size_t size, uint32_t reg,
	size_t level, size_t page_size)
{
	size_t n = 0;

	/* Extract a unified TLB. */
	if (!EXTRACT(reg, 16, 16) && EXTRACT(reg, 12, 4)) {
		if (cache_desc && n < size) {
			cache_desc->tlb.flags = CACHE_DESC_TLB |
				CACHE_DESC_SHARED | CACHE_DESC_LEVEL(level);
			cache_desc->tlb.nways = ext_nways[EXTRACT(reg, 12, 4)];
			cache_desc->tlb.nentries = EXTRACT(reg, 0, 12);
			cache_desc->tlb.page_size = page_size;
			++cache_desc;
		}

		++n;

		return n;
	}
	
	/* Extract a separate code TLB. */
	if (EXTRACT(reg, 12, 4)) {
		if (cache_desc && n < size) {
			cache_desc->tlb.flags = CACHE_DESC_TLB |
				CACHE_DESC_CODE | CACHE_DESC_LEVEL(level);
			cache_desc->tlb.nways = ext_nways[EXTRACT(reg, 12, 4)];
			cache_desc->tlb.nentries = EXTRACT(reg, 0, 12);
			cache_desc->tlb.page_size = page_size;
			++cache_desc;
		}

		++n;
	}

	/* Extract a separate data TLB. */
	if (EXTRACT(reg, 28, 4)) {
		if (cache_desc && n < size) {
			cache_desc->tlb.flags = CACHE_DESC_TLB |
				CACHE_DESC_DATA | CACHE_DESC_LEVEL(level);
			cache_desc->tlb.nways = ext_nways[EXTRACT(reg, 28, 4)];
			cache_desc->tlb.nentries = EXTRACT(reg, 16, 12);
			cache_desc->tlb.page_size = page_size;
			++cache_desc;
		}

		++n;
	}

	return n;
}

size_t get_cache_info_v1(union cache_desc *cache_desc, size_t size,
	uint32_t reg, size_t level, unsigned flags)
{
	size_t n = 0;

	if (EXTRACT(reg, 16, 8)) {
		if (cache_desc && n < size) {
			cache_desc->cache.flags = CACHE_DESC_CACHE | flags |
				CACHE_DESC_LEVEL(level);
			cache_desc->cache.nways = EXTRACT(reg, 16, 8);
			cache_desc->cache.size = EXTRACT(reg, 24, 8) * KIB;
			cache_desc->cache.line_size = EXTRACT(reg, 0, 8);
			++cache_desc;
		}

		++n;
	}

	return n;
}

size_t get_cache_info_v2(union cache_desc *cache_desc, size_t size,
	uint32_t reg, size_t level, unsigned flags)
{
	size_t n = 0;

	if (EXTRACT(reg, 12, 4)) {
		if (cache_desc && n < size) {
			cache_desc->cache.flags = CACHE_DESC_CACHE | flags |
				CACHE_DESC_LEVEL(level);
			cache_desc->cache.nways = ext_nways[EXTRACT(reg, 12, 4)];
			cache_desc->cache.size = EXTRACT(reg, 16, 16) * KIB;
			cache_desc->cache.line_size = EXTRACT(reg, 0, 8);
			++cache_desc;
		}

		++n;
	}

	return n;
}

size_t get_cache_info_v3(union cache_desc *cache_desc, size_t size,
	uint32_t reg, size_t level, unsigned flags)
{
	size_t n = 0;

	if (EXTRACT(reg, 12, 4)) {
		if (cache_desc && n < size) {
			cache_desc->cache.flags = CACHE_DESC_CACHE | flags |
				CACHE_DESC_LEVEL(level);
			cache_desc->cache.nways = ext_nways[EXTRACT(reg, 12, 4)];
			cache_desc->cache.size = EXTRACT(reg, 18, 14) * 512 * KIB;
			cache_desc->cache.line_size = EXTRACT(reg, 0, 8);
			++cache_desc;
		}

		++n;
	}

	return n;
}

size_t amd_get_cache_descs_v1(union cache_desc *cache_desc, size_t size)
{
	uint32_t regs[4];
	size_t n = 0;

	if (cpuid_get_max_ext_leaf() < 0x80000005)
		return 0;

	asm volatile(
		"cpuid\n" :
		"=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]),
		"=d" (regs[3]) :
		"a" (0x80000005));

	n += get_tlb_info_v1(cache_desc + n, size - min(size, n), regs[0], 1,
		TLB_2M_PAGE | TLB_4M_PAGE);
	n += get_tlb_info_v1(cache_desc + n, size - min(size, n), regs[1], 1,
		TLB_4K_PAGE);
	n += get_cache_info_v1(cache_desc + n, size - min(size, n), regs[2], 1,
		CACHE_DESC_DATA);
	n += get_cache_info_v1(cache_desc + n, size - min(size, n), regs[3], 1,
		CACHE_DESC_CODE);

	return n;
}

size_t amd_get_cache_descs_v2(union cache_desc *cache_desc, size_t size)
{
	uint32_t regs[4];
	size_t n = 0;

	if (cpuid_get_max_ext_leaf() < 0x80000006)
		return 0;

	asm volatile(
		"cpuid\n" :
		"=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]),
		"=d" (regs[3]) :
		"a" (0x80000006));

	n += get_tlb_info_v2(cache_desc + n, size - min(size, n), regs[0], 2,
		TLB_2M_PAGE | TLB_4M_PAGE);
	n += get_tlb_info_v2(cache_desc + n, size - min(size, n), regs[1], 2,
		TLB_4K_PAGE);
	n += get_cache_info_v2(cache_desc + n, size - min(size, n), regs[2], 2,
		CACHE_DESC_SHARED);
	n += get_cache_info_v3(cache_desc + n, size - min(size, n), regs[3], 3,
		CACHE_DESC_SHARED);

	return n;
}

size_t amd_get_cache_descs_v3(union cache_desc *cache_desc, size_t size)
{
	uint32_t regs[4];
	size_t n = 0;

	if (cpuid_get_max_ext_leaf() < 0x80000019)
		return 0;

	asm volatile(
		"cpuid\n" :
		"=a" (regs[0]), "=b" (regs[1]) :
		"a" (0x80000019) :
		"%ecx", "%edx");

	n += get_tlb_info_v2(cache_desc + n, size - min(size, n), regs[0], 2,
		TLB_1G_PAGE);
	n += get_tlb_info_v2(cache_desc + n, size - min(size, n), regs[1], 2,
		TLB_1G_PAGE);

	return n;
}

size_t amd_get_cache_descs(union cache_desc *cache_desc, size_t size)
{
	size_t n = 0;

	n += amd_get_cache_descs_v1(cache_desc + n, size - min(size, n));
	n += amd_get_cache_descs_v2(cache_desc + n, size - min(size, n));
	n += amd_get_cache_descs_v3(cache_desc + n, size - min(size, n));

	return n;
}
