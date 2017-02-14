/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cpuid/cache.h>
#include <cpuid/cpuid.h>

#include "args.h"
#include "macros.h"

#include "amd/cache.h"
#include "intel/cache.h"

static void print_associativity(size_t nways)
{
	switch (nways) {
	case CACHE_DIRECT_MAPPED: printf("direct mapped"); break;
	case CACHE_FULLY_ASSOC: printf("fully associative"); break;
	case 0: break;
	default: printf("%zu-way set associative", nways); break;
	}
}

size_t copy_cache_desc(union cache_desc *cache_descs, size_t size,
	union cache_desc *cache_desc)
{
	if (cache_descs && size) {
		memcpy(cache_descs, cache_desc, sizeof *cache_desc);
	}

	return 1;
}

size_t set_tlb_desc(union cache_desc *cache_descs, size_t size,
	unsigned flags, size_t nways, size_t nentries, size_t page_size)
{
	if (cache_descs && size) {
		cache_descs->tlb.flags = flags;
		cache_descs->tlb.nways = nways;
		cache_descs->tlb.nentries = nentries;
		cache_descs->tlb.page_size = page_size;
	}

	return 1;
}

void print_cache_desc(union cache_desc *cache_desc)
{
	switch (get_cache_desc_type(cache_desc)) {
	case CACHE_DESC_TLB:
		print_associativity(cache_desc->tlb.nways);

		if (cache_desc->tlb.nways)
			putchar(' ');

		printf("L%zu ", get_cache_desc_level(cache_desc));

		switch (cache_desc->flags & CACHE_DESC_SHARED) {
		case CACHE_DESC_CODE: printf("i-"); break;
		case CACHE_DESC_DATA: printf("d-"); break;
		default: break;
		}

		printf("TLB (%zu entries,", cache_desc->tlb.nentries);
	
		if (cache_desc->tlb.page_size & TLB_4K_PAGE)
			printf(" 4K page");

		if (cache_desc->tlb.page_size & TLB_2M_PAGE)
			printf(" 2M page");

		if (cache_desc->tlb.page_size & TLB_4M_PAGE)
			printf(" 4M page");

		if (cache_desc->tlb.page_size & TLB_1G_PAGE)
			printf(" 1G page");

		printf(")\n");
		break;
	case CACHE_DESC_CACHE:
		print_associativity(cache_desc->cache.nways);

		if (cache_desc->cache.nways)
			putchar(' ');

		printf("L%zu ", get_cache_desc_level(cache_desc));

		switch (cache_desc->flags & CACHE_DESC_SHARED) {
		case CACHE_DESC_CODE: printf("i-"); break;
		case CACHE_DESC_DATA: printf("d-"); break;
		default: break;
		}

		printf("cache (");

		print_size(stdout, cache_desc->cache.size);

		printf(", %zuB line size", cache_desc->cache.line_size);

		if (cache_desc->flags & CACHE_DESC_INCLUSIVE)
			printf(". inclusive");

		printf(")\n");
		break;
	case CACHE_DESC_PREFETCH:
		printf("%zuB prefetch\n", cache_desc->prefetch.size);
		break;
	default: break;
	}
}

size_t get_cache_descs(union cache_desc *cache_desc, size_t size)
{
	size_t n = 0;

	switch (cpuid_get_vendor_id()) {
	case CPUID_VENDOR_AMD:
		return amd_get_cache_descs(cache_desc + n, size - min(size, n));
	case CPUID_VENDOR_INTEL:
		return intel_get_cache_descs(cache_desc + n, size - min(size, n));
	default:
		return 0;
	}
}
