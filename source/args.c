/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "args.h"
#include "paging.h"

int parse_addr(uintptr_t *addr, const char *s)
{
	if (*s++ != '0')
		return -1;

	if (*s++ != 'x')
		return -1;

	*addr = (uintptr_t)strtoull(s, NULL, 16);

	return 0;
}

int parse_size(size_t *size, const char *s)
{
	char *end;

	*size = strtoul(s, &end, 10);

	switch (toupper(*end)) {
#if defined(TIB)
	case 'T': *size *= 1024;
#endif
	case 'G': *size *= 1024;
	case 'M': *size *= 1024;
	case 'K': *size *= 1024;
	case 'B': ++end; break;
	case '\0': return 0;
	default: return -1;
	}

	if (*end)
		return -1;

	return 0;
}

int parse_array(size_t *values, size_t nvalues, const char *s)
{
	const char *p = s;
	char *end;
	size_t i;

	for (i = 0; i < nvalues; ++i) {
		p += strspn(p, " ");

		if (*p == '-') {
			p++;
			continue;
		}

		values[i] = strtoul(p, &end, 10);
		p = end;
		p += strspn(p, " ");

		if (*p == '\0')
			break;

		if (*p != ',')
			return -1;

		p++;
	}

	return 0;
}

void print_size(FILE *f, size_t size)
{
	if (size == 0) {
		fprintf(f, "0");
#if defined(TIB)
	} else if (size % TIB == 0) {
		fprintf(f, "%zuT", size / TIB);
#endif
	} else if (size % GIB == 0) {
		fprintf(f, "%zuG", size / GIB);
	} else if (size % MIB == 0) {
		fprintf(f, "%zuM", size / MIB);
	} else if (size % KIB == 0) {
		fprintf(f, "%zuK", size / KIB);
	} else {
		fprintf(f, "%zuB", size);
	}
}

void show_usage(const char *prog_name)
{
	fprintf(stderr,
		"\n------------------------\n\n"
		"Usage: %s [<arguments>]\n"
		"On x86, the tuning arguments are auto-detected, but can be "
		"overridden by providing the corresponding arguments. On ARM, "
		"there is no detection.\n"
		"\n"
		"More help:\n"
		" --list-page-formats: shows the supported page formats for "
		"the current architecture.\n"
		"\n"
		"General arguments:\n"
		" -h, --help: shows this help message\n"
		" -o, --output <path>: path to the directory in which to store the "
		"results (default './results')\n"
		" -n, --runs <value>: number of runs to perform with the same VA and "
		"eviction buffers (default 1)\n"
		" -r, --rounds <value>: number of measurement rounds (median "
		"is chosen, default 10)\n"
		"\n"
		"Tuning arguments:\n"
		" -s, --cache-size <value>: total cache size to evict (LLC "
		"size if include, otherwise sum of the size of all caches), "
		"for example 2M\n"
		" -l, --line-size <value>: LLC line length in bytes (default "
		"64)\n"
		" -f, --page-format <value>: the page format to use for the "
		"current architecture (see --list-page-formats)\n"
		" --target <addr>: the address to allocate the target buffer "
		"at.\n"
		"\n"
		"Per-page level tuning arguments:\n"
		" --pl[1-4]-entries <value>: number of entries to access to "
		"effectively evict the page structure caches or the TLB\n"
		" --pl[1-4]-pages <value>: number of pages to evict cache "
		"lines for.\n",
		prog_name);
}

#if defined(__i386__) || defined(__x86_64__)
#include <cpuid/cache.h>
#include <cpuid/cpuid.h>

void detect_args(struct args *args)
{
	union cache_desc cache_descs[32], *cache_desc;
	size_t nentries[4] = {0, 0, 0, 0};
	size_t ncache_descs, i;

	ncache_descs = get_cache_descs(cache_descs, 32);

	args->cache_size = 0;
	args->line_size = 0;

	for (i = 0; i < ncache_descs; ++i) {
		cache_desc = cache_descs + i;

		print_cache_desc(cache_desc);

		switch (get_cache_desc_type(cache_desc)) {
		case CACHE_DESC_TLB:
			if (!(cache_desc->flags & CACHE_DESC_DATA))
				break;

			if ((cache_desc->tlb.page_size & TLB_4K_PAGE)) {
				nentries[0] += cache_desc->tlb.nentries;
			}

			if ((cache_desc->tlb.page_size & TLB_2M_PAGE)) {
				nentries[1] += cache_desc->tlb.nentries;
			}

			if ((cache_desc->tlb.page_size & TLB_1G_PAGE)) {
				nentries[2] += cache_desc->tlb.nentries;
			}

			break;
		case CACHE_DESC_CACHE:
			if (!(cache_desc->flags & CACHE_DESC_DATA))
				break;

			args->line_size = max(args->line_size,
				cache_desc->cache.line_size);
			args->cache_size = max(args->cache_size,
				cache_desc->cache.size);
			break;
		default: break;
		}
	}

	for (i = 0; i < 4; ++i) {
		if (args->nentries[i] == SIZE_MAX)
			args->nentries[i] = nentries[i];
	}
}
#else
void detect_args(struct args *args)
{
	size_t i;

	for (i = 0; i < 4; ++i) {
		if (args->nentries[i] == SIZE_MAX)
			args->nentries[i] = 0;
	}
}
#endif

int parse_args(struct args *args, int argc, const char *argv[])
{
	struct option options[] = {
		{ "help", no_argument, NULL, OPTION_HELP },
		{ "cpu", required_argument, 0, OPTION_CPU },
		{ "target", required_argument, 0, OPTION_TARGET },
		{ "evict-target", required_argument, 0, OPTION_EVICT_TARGET },
		{ "line-size", required_argument, 0, OPTION_LINE_SIZE },
		{ "cache-size", required_argument, 0, OPTION_CACHE_SIZE },
		{ "rounds", required_argument, 0, OPTION_ROUNDS },
		{ "tlb-entries", required_argument, 0, OPTION_PL1_ENTRIES },
		{ "pl-entries", required_argument, 0, OPTION_PL_ENTRIES },
		{ "pl1-entries", required_argument, 0, OPTION_PL1_ENTRIES },
		{ "pl2-entries", required_argument, 0, OPTION_PL2_ENTRIES },
		{ "pl3-entries", required_argument, 0, OPTION_PL3_ENTRIES },
		{ "pl4-entries", required_argument, 0, OPTION_PL4_ENTRIES },
		{ "pl-pages", required_argument, 0, OPTION_PL_PAGES },
		{ "pl1-pages", required_argument, 0, OPTION_PL1_PAGES },
		{ "pl2-pages", required_argument, 0, OPTION_PL2_PAGES },
		{ "pl3-pages", required_argument, 0, OPTION_PL3_PAGES },
		{ "pl4-pages", required_argument, 0, OPTION_PL4_PAGES },
		{ "page-format", required_argument, 0, OPTION_PAGE_FORMAT },
		{ "list-page-formats", no_argument, NULL,
			OPTION_LIST_PAGE_FORMATS },
		{ "runs", required_argument, 0, OPTION_RUNS },
		{ "threshold", required_argument, 0, OPTION_THRESHOLD },
		{ "output", required_argument, 0, OPTION_OUTPUT },
		{ NULL, 0, 0, 0 },
	};
	int ret;

	while ((ret = getopt_long(argc, (char * const *)argv, "hc:l:n:s:f:r:o:",
		options, NULL)) >= 0) {
		switch (ret) {
		case OPTION_HELP: return -1;
		case OPTION_CPU:
			args->cpu = strtoul(optarg, NULL, 10);
			break;
		case OPTION_TARGET:
			if ((parse_addr(&args->target, optarg)) < 0)
				return -1;

			break;
		case OPTION_EVICT_TARGET:
			if ((parse_addr(&args->evict_target, optarg)) < 0)
				return -1;

			break;
		case OPTION_LINE_SIZE:
			if ((parse_size(&args->line_size, optarg)) < 0)
				return -1;

			break;
		case OPTION_CACHE_SIZE:
			if ((parse_size(&args->cache_size, optarg)) < 0)
				return -1;

			break;
		case OPTION_ROUNDS:
			if ((parse_size(&args->nrounds, optarg)) < 0)
				return -1;

			break;
		case OPTION_PL_ENTRIES:
			if ((parse_array(args->nentries, 4, optarg)) < 0)
				return -1;
			break;
		case OPTION_PL1_ENTRIES:
		case OPTION_PL2_ENTRIES:
		case OPTION_PL3_ENTRIES:
		case OPTION_PL4_ENTRIES:
			if ((parse_size(args->nentries + ret -
				OPTION_PL1_ENTRIES, optarg)) < 0)
				return -1;
			break;
		case OPTION_PL_PAGES:
			if ((parse_array(args->npages,4, optarg)) < 0)
				return -1;
			break;
		case OPTION_PL1_PAGES:
		case OPTION_PL2_PAGES:
		case OPTION_PL3_PAGES:
		case OPTION_PL4_PAGES:
			if ((parse_size(args->npages + ret -
				OPTION_PL1_PAGES, optarg)) < 0)
				return -1;

			break;
		case OPTION_PAGE_FORMAT:
			args->page_format = strdup(optarg);
			break;
		case OPTION_LIST_PAGE_FORMATS:
			fprintf(stderr, "Supported page formats: ");
			list_page_formats(stderr);
			fprintf(stderr, "\n\n");
			return -1;
		case OPTION_RUNS:
			if ((parse_size(&args->nruns, optarg)) < 0)
				return -1;

			break;
		case OPTION_THRESHOLD:
			args->threshold = strtof(optarg, NULL);
		case OPTION_OUTPUT:
			args->output = strdup(optarg);
			break;
		default:
			break;
		}
	}

	return 0;
}

void print_args(FILE *f, struct args *args, struct page_format *fmt)
{
	(void)fmt;
	fprintf(f, "Settings:\n"
		"  runs: %zu\n"
		"  rounds: %zu\n"
		"  page format: %s\n"
		"  cache size: ",
		args->nruns,
		args->nrounds,
		args->page_format ? args->page_format : "default");
	print_size(f, args->cache_size);
	fprintf(f, "\n"
		"  cache line size: ");
	print_size(f, args->line_size);
	fprintf(f, "\n\n");
}

struct page_format *get_page_format_from_args(struct args *args)
{
	struct page_format *fmt = NULL;
	struct page_level *level;
	size_t i;

	if (args->page_format && !(fmt = get_page_format(args->page_format)))
		return NULL;

	if (!fmt)
		fmt = get_default_page_format();

	if (!fmt)
		return NULL;

	for (i = 0, level = fmt->levels; i < fmt->nlevels; ++level, ++i) {
		level->npages = args->npages[i];
		level->ncache_entries = args->nentries[i];
	}

	return fmt;
}
