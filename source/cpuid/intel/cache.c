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

/* This hardcoded look-up table is used to map all known cache descriptors to
 * actual cache descriptions.
 */
union cache_desc leaf2_cache_descs[] = {
	[0x00] = { .flags = CACHE_DESC_NONE },
	[0x01] = { .tlb = { CACHE_DESC_ITLB_L1, 4, 32, TLB_4K_PAGE }},
	[0x02] = { .tlb = { CACHE_DESC_ITLB_L1, CACHE_FULLY_ASSOC, 2,
		TLB_4M_PAGE }},
	[0x03] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 64, TLB_4K_PAGE }},
	[0x04] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 8, TLB_4M_PAGE }},
	[0x05] = { .tlb = { CACHE_DESC_DTLB_L2, 4, 32, TLB_4M_PAGE }},
	[0x06] = { .cache = { CACHE_DESC_ICACHE_L1, 4, 8 * KIB, 32, 1 }},
	[0x08] = { .cache = { CACHE_DESC_ICACHE_L1, 4, 16 * KIB, 32, 1 }},
	[0x09] = { .cache = { CACHE_DESC_ICACHE_L1, 4, 32 * KIB, 64, 1 }},
	[0x0A] = { .cache = { CACHE_DESC_DCACHE_L1, 2, 8 * KIB, 32, 1 }},
	[0x0B] = { .tlb = { CACHE_DESC_ITLB_L1, 4, 4, TLB_4M_PAGE }},
	[0x0C] = { .cache = { CACHE_DESC_DCACHE_L1, 4, 16 * KIB, 32, 1 }},
	[0x0D] = { .cache = { CACHE_DESC_DCACHE_L1, 4, 16 * KIB, 64, 1 }},
	[0x0E] = { .cache = { CACHE_DESC_DCACHE_L1, 6, 24 * KIB, 64, 1 }},
	[0x1D] = { .cache = { CACHE_DESC_CACHE_L2, 2, 128 * KIB, 64, 1 }},
	[0x21] = { .cache = { CACHE_DESC_CACHE_L2, 8, 256 * KIB, 64, 1 }},
	[0x22] = { .cache = { CACHE_DESC_CACHE_L3, 4, 512 * KIB, 64, 2 }},
	[0x23] = { .cache = { CACHE_DESC_CACHE_L3, 8, 1 * MIB, 64, 2 }},
	[0x24] = { .cache = { CACHE_DESC_CACHE_L2, 16, 1 * MIB, 64, 1 }},
	[0x25] = { .cache = { CACHE_DESC_CACHE_L3, 8, 2 * MIB, 64, 2 }},
	[0x29] = { .cache = { CACHE_DESC_CACHE_L3, 8, 4 * MIB, 64, 2 }},
	[0x2C] = { .cache = { CACHE_DESC_DCACHE_L1, 8, 32 * KIB, 64, 1}},
	[0x30] = { .cache = { CACHE_DESC_ICACHE_L1, 8, 32 * KIB, 64, 1}},
	[0x40] = { .flags = CACHE_DESC_NONE },
	[0x41] = { .cache = { CACHE_DESC_CACHE_L2, 4, 128 * KIB, 32, 1 }},
	[0x42] = { .cache = { CACHE_DESC_CACHE_L2, 4, 256 * KIB, 32, 1 }},
	[0x43] = { .cache = { CACHE_DESC_CACHE_L2, 4, 512 * KIB, 32, 1 }},
	[0x44] = { .cache = { CACHE_DESC_CACHE_L2, 4, 1 * MIB, 32, 1 }},
	[0x45] = { .cache = { CACHE_DESC_CACHE_L2, 4, 2 * MIB, 32, 1 }},
	[0x46] = { .cache = { CACHE_DESC_CACHE_L3, 4, 4 * MIB, 64, 1 }},
	[0x47] = { .cache = { CACHE_DESC_CACHE_L3, 8, 8 * MIB, 64, 1 }},
	[0x48] = { .cache = { CACHE_DESC_CACHE_L2, 12, 3 * MIB, 64, 1 }},
	/* TODO: L3 cache 4 MiB 16-way 64-byte if Intel Xeon processor family
	 * 0x0F, model 0x06. */
	[0x49] = { .cache = { CACHE_DESC_CACHE_L2, 16, 4 * MIB, 64, 1 }},
	[0x4A] = { .cache = { CACHE_DESC_CACHE_L3, 12, 6 * MIB, 64, 1 }},
	[0x4B] = { .cache = { CACHE_DESC_CACHE_L3, 16, 8 * MIB, 64, 1 }},
	[0x4C] = { .cache = { CACHE_DESC_CACHE_L3, 12, 12 * MIB, 64, 1 }},
	[0x4D] = { .cache = { CACHE_DESC_CACHE_L3, 16, 16 * MIB, 64, 1 }},
	[0x4E] = { .cache = { CACHE_DESC_CACHE_L2, 24, 6 * MIB, 64, 1 }},
	[0x4F] = { .tlb = { CACHE_DESC_ITLB_L1, 0, 32, TLB_4K_PAGE }},
	[0x50] = { .tlb = { CACHE_DESC_ITLB_L1, 0, 64,
		TLB_4K_PAGE | TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x51] = { .tlb = { CACHE_DESC_ITLB_L1, 0, 128,
		TLB_4K_PAGE | TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x52] = { .tlb = { CACHE_DESC_ITLB_L1, 0, 256,
		TLB_4K_PAGE | TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x55] = { .tlb = { CACHE_DESC_ITLB_L1,
		CACHE_FULLY_ASSOC, 7, TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x56] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 16, TLB_4M_PAGE }},
	[0x57] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 16, TLB_4K_PAGE }},
	[0x59] = { .tlb = { CACHE_DESC_DTLB_L1, CACHE_FULLY_ASSOC, 16,
		TLB_4K_PAGE }},
	[0x5A] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 32,
		TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x5B] = { .tlb = { CACHE_DESC_DTLB_L1, 0, 64,
		TLB_4K_PAGE | TLB_4M_PAGE }},
	[0x5C] = { .tlb = { CACHE_DESC_DTLB_L1, 0, 128,
		TLB_4K_PAGE | TLB_4M_PAGE }},
	[0x5D] = { .tlb = { CACHE_DESC_DTLB_L1, 0, 256,
		TLB_4K_PAGE | TLB_4M_PAGE }},
	[0x60] = { .cache = { CACHE_DESC_DCACHE_L1, 8, 16 * KIB, 64, 1 }},
	[0x61] = { .tlb = { CACHE_DESC_ITLB_L1, CACHE_FULLY_ASSOC, 48,
		TLB_4K_PAGE }},
	[0x63] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 32,
		TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x64] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 512, TLB_4K_PAGE }},
	[0x66] = { .cache = { CACHE_DESC_DCACHE_L1, 4, 8 * KIB, 64, 1 }},
	[0x67] = { .cache = { CACHE_DESC_DCACHE_L1, 4, 16 * KIB, 64, 1 }},
	[0x68] = { .cache = { CACHE_DESC_DCACHE_L1, 4, 32 * KIB, 64, 1 }},
	[0x6A] = { .tlb = { CACHE_DESC_TLB_L1, 8, 64, TLB_4K_PAGE }},
	[0x6B] = { .tlb = { CACHE_DESC_DTLB_L1, 8, 256, TLB_4K_PAGE }},
	[0x6C] = { .tlb = { CACHE_DESC_DTLB_L1, 8, 128,
		TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x6D] = { .tlb = { CACHE_DESC_DTLB_L1, CACHE_FULLY_ASSOC, 16,
		TLB_1G_PAGE }},
	[0x76] = { .tlb = { CACHE_DESC_ITLB_L1, 0, 8,
		TLB_2M_PAGE | TLB_4M_PAGE }},
	[0x78] = { .cache = { CACHE_DESC_CACHE_L2, 4, 1 * MIB, 64, 1 }},
	[0x79] = { .cache = { CACHE_DESC_CACHE_L2, 8, 128 * KIB, 64, 2 }},
	[0x7A] = { .cache = { CACHE_DESC_CACHE_L2, 8, 256 * KIB, 64, 2 }},
	[0x7B] = { .cache = { CACHE_DESC_CACHE_L2, 8, 512 * KIB, 64, 2 }},
	[0x7C] = { .cache = { CACHE_DESC_CACHE_L2, 8, 1 * MIB, 64, 2 }},
	[0x7D] = { .cache = { CACHE_DESC_CACHE_L2, 8, 2 * MIB, 64, 1 }},
	[0x7F] = { .cache = { CACHE_DESC_CACHE_L2, 2, 512 * KIB, 64, 1 }},
	[0x80] = { .cache = { CACHE_DESC_CACHE_L2, 8, 512 * KIB, 64, 1 }},
	[0x82] = { .cache = { CACHE_DESC_CACHE_L2, 8, 256 * KIB, 32, 1 }},
	[0x83] = { .cache = { CACHE_DESC_CACHE_L2, 8, 512 * KIB, 32, 1 }},
	[0x84] = { .cache = { CACHE_DESC_CACHE_L2, 8, 1 * MIB, 32, 1 }},
	[0x85] = { .cache = { CACHE_DESC_CACHE_L2, 8, 2 * MIB, 32, 1 }},
	[0x86] = { .cache = { CACHE_DESC_CACHE_L2, 4, 512 * KIB, 64, 1 }},
	[0x87] = { .cache = { CACHE_DESC_CACHE_L2, 8, 1 * MIB, 64, 1 }},
	[0xA0] = { .tlb = { CACHE_DESC_DTLB_L1, 0, 32, TLB_4K_PAGE }},
	[0xB0] = { .tlb = { CACHE_DESC_ITLB_L1, 4, 128, TLB_4K_PAGE }},
	/* TODO: 0xB1: also 4M pages, 4-way, 4 entries */
	[0xB1] = { .tlb = { CACHE_DESC_ITLB_L1, 4, 8, TLB_2M_PAGE }},
	[0xB2] = { .tlb = { CACHE_DESC_ITLB_L1, 4, 64, TLB_4K_PAGE }},
	[0xB3] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 128, TLB_4K_PAGE }},
	[0xB4] = { .tlb = { CACHE_DESC_DTLB_L2, 4, 256, TLB_4K_PAGE }},
	[0xB5] = { .tlb = { CACHE_DESC_ITLB_L1, 8, 64, TLB_4K_PAGE }},
	[0xB6] = { .tlb = { CACHE_DESC_ITLB_L1, 8, 128, TLB_4K_PAGE }},
	[0xBA] = { .tlb = { CACHE_DESC_DTLB_L2, 4, 64, TLB_4K_PAGE }},
	[0xC0] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 8,
		TLB_4K_PAGE | TLB_4M_PAGE }},
	[0xC1] = { .tlb = { CACHE_DESC_TLB_L2, 8, 1024,
		TLB_4K_PAGE | TLB_2M_PAGE }},
	[0xC2] = { .tlb = { CACHE_DESC_DTLB_L1, 4, 16,
		TLB_4K_PAGE | TLB_2M_PAGE }},
	[0xC3] = { .tlb = { CACHE_DESC_TLB_L2, 6, 1536,
		TLB_4K_PAGE | TLB_2M_PAGE }},
	[0xC4] = { .tlb = { CACHE_DESC_TLB_L1, 4, 32,
		TLB_2M_PAGE | TLB_4M_PAGE }},
	[0xCA] = { .tlb = { CACHE_DESC_TLB_L2, 4, 512, TLB_4K_PAGE }},
	[0xD0] = { .cache = { CACHE_DESC_CACHE_L3, 4, 512 * KIB, 64, 1 }},
	[0xD1] = { .cache = { CACHE_DESC_CACHE_L3, 4, 1 * MIB, 64, 1 }},
	[0xD2] = { .cache = { CACHE_DESC_CACHE_L3, 4, 2 * MIB, 64, 1 }},
	[0xD6] = { .cache = { CACHE_DESC_CACHE_L3, 8, 1 * MIB, 64, 1 }},
	[0xD7] = { .cache = { CACHE_DESC_CACHE_L3, 8, 2 * MIB, 64, 1 }},
	[0xD8] = { .cache = { CACHE_DESC_CACHE_L3, 8, 4 * MIB, 64, 1 }},
	[0xDC] = { .cache = { CACHE_DESC_CACHE_L3, 12, 1536 * KIB, 64, 1 }},
	[0xDD] = { .cache = { CACHE_DESC_CACHE_L3, 12, 3 * MIB, 64, 1 }},
	[0xDE] = { .cache = { CACHE_DESC_CACHE_L3, 12, 6 * MIB, 64, 1 }},
	[0xE2] = { .cache = { CACHE_DESC_CACHE_L3, 16, 2 * MIB, 64, 1 }},
	[0xE3] = { .cache = { CACHE_DESC_CACHE_L3, 16, 4 * MIB, 64, 1 }},
	[0xE4] = { .cache = { CACHE_DESC_CACHE_L3, 16, 8 * MIB, 64, 1 }},
	[0xEA] = { .cache = { CACHE_DESC_CACHE_L3, 24, 12 * MIB, 64, 1 }},
	[0xEB] = { .cache = { CACHE_DESC_CACHE_L3, 24, 18 * MIB, 64, 1 }},
	[0xEC] = { .cache = { CACHE_DESC_CACHE_L3, 24, 24 * MIB, 64, 1 }},
	[0xF0] = { .prefetch = { CACHE_DESC_PREFETCH, 64 }},
	[0xF1] = { .prefetch = { CACHE_DESC_PREFETCH, 128 }},
	[0xFF] = { .flags = CACHE_DESC_NONE },
};

size_t intel_get_cache_descs_from_descriptor(union cache_desc *cache_descs,
	size_t size, size_t desc)
{
	size_t n = 0;

	switch (desc) {
	case 0x63:
		n += set_tlb_desc(cache_descs + n, size - min(size, n),
			CACHE_DESC_DTLB_L1, 4, 4, TLB_1G_PAGE);
		break;
	case 0xB1:
		n += set_tlb_desc(cache_descs + n, size - min(size, n),
			CACHE_DESC_ITLB_L1, 4, 4, TLB_4M_PAGE);
		break;
	case 0xC3:
		n += set_tlb_desc(cache_descs + n, size - min(size, n),
			CACHE_DESC_TLB_L2, 4, 16, TLB_1G_PAGE);
		break;
	default: break;
	}

	n += copy_cache_desc(cache_descs + n, size - min(size, n),
		leaf2_cache_descs + desc);

	return n;
}

size_t intel_get_cache_descs_v1(int *use_leaf4, union cache_desc *cache_desc,
	size_t size)
{
	union reg regs[4], *reg;
	size_t ncalls = SIZE_MAX;
	size_t i, j, k, n = 0;
	int first_call = 1;

	if (cpuid_get_max_leaf() < 0x00000002)
		return 0;

	*use_leaf4 = 0;

	/* Get the cache descriptors. */
	while (--ncalls) {
		asm volatile(
			"cpuid" :
			"=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]),
			"=d" (regs[3]) :
			"a" (2));

		/* If this is the first call, check how many times CPUID has
		 * to be issued to get all cache descriptors.
		 */
		if (first_call) {
			first_call = 0;
			ncalls = regs[0].u8[0];
		}

		/* Iterate over the registers. */
		for (j = 0; j < 4; ++j) {
			reg = regs + j;

			/* Check if the register contains valid descriptors. */
			if (reg->u32 & (1 << 31))
				continue;

			/* Iterate over the cache descriptors. */
			for (i = j ? 0: 1; i < 4; ++i) {
				k = reg->u8[i];
			
				/* Skip null descriptors. */
				if (k == 0)
					continue;

				/* No cache information available, we should
				 * obtain this using CPUID leaf4.
			 	 */
				if (k == 0xFF) {
					*use_leaf4 = 1;
					continue;
				}

				/* Copy the corresponding cache description. */
				n += intel_get_cache_descs_from_descriptor(
					cache_desc + n, size - min(size, n),
					k);
			}
		}
	}

	return n;
}

size_t intel_get_cache_descs_v2(union cache_desc *cache_desc, size_t size)
{
	size_t i, n = 0;
	uint32_t regs[4];
	size_t nways, nparts, line_size, nsets, cache_size;
	unsigned type;

	if (cpuid_get_max_leaf() < 0x00000004)
		return 0;

	/* Iterate the cache levels. */
	for (i = 0;; ++i) {
		asm volatile(
			"cpuid\n" :
			"=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]),
			"=d" (regs[3]) :
			"a" (4), "c" (i));

		type = EXTRACT(regs[0], 0, 4);

		if (!type)
			break;

		if (type > 3)
			continue;

		++n;

		if (!cache_desc || n >= size)
			continue;

		switch (type) {
		case 1: cache_desc->flags = CACHE_DESC_DATA; break;
		case 2: cache_desc->flags = CACHE_DESC_CODE; break;
		case 3: cache_desc->flags = CACHE_DESC_SHARED; break;
		default: break;
		}

		nways = EXTRACT(regs[1], 22, 10) + 1;
		nparts = EXTRACT(regs[1], 12, 10) + 1;
		line_size = EXTRACT(regs[1], 0, 12) + 1;
		nsets = regs[2] + 1;

		cache_size = nways * nparts * line_size * nsets;

		cache_desc->flags |= CACHE_DESC_CACHE |
			CACHE_DESC_LEVEL(EXTRACT(regs[0], 5, 2));

		if (regs[0] & (1 << 9)) {
			cache_desc->cache.nways = CACHE_FULLY_ASSOC;
		} else {
			cache_desc->cache.nways = nways;
		}

		if (regs[3] & (1 << 1))
			cache_desc->flags |= CACHE_DESC_INCLUSIVE;

		cache_desc->cache.line_size = line_size;
		cache_desc->cache.size = cache_size;
		++cache_desc;
	}

	return n;
}

size_t intel_get_cache_descs(union cache_desc *cache_desc, size_t size)
{
	size_t n = 0;
	int use_leaf4 = 0;

	n += intel_get_cache_descs_v1(&use_leaf4, cache_desc + n,
		size - min(size, n));
	
	if (use_leaf4)
		n += intel_get_cache_descs_v2(cache_desc + n, size - min(size, n));

	return n;
}
