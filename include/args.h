/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "macros.h"
#include "paging.h"

enum {
	OPTION_HELP = 'h',
	OPTION_CPU = 'c',
	OPTION_LINE_SIZE = 'l',
	OPTION_CACHE_SIZE = 's',
	OPTION_ROUNDS = 'r',
	OPTION_PAGE_FORMAT = 'f',
	OPTION_RUNS = 'n',
	OPTION_PL_ENTRIES = 255,
	OPTION_PL1_ENTRIES,
	OPTION_PL2_ENTRIES,
	OPTION_PL3_ENTRIES,
	OPTION_PL4_ENTRIES,
	OPTION_PL_PAGES,
	OPTION_PL1_PAGES,
	OPTION_PL2_PAGES,
	OPTION_PL3_PAGES,
	OPTION_PL4_PAGES,
	OPTION_LIST_PAGE_FORMATS,
	OPTION_TARGET,
	OPTION_EVICT_TARGET,
	OPTION_THRESHOLD,
	OPTION_OUTPUT = 'o',
};

struct args {
	char *page_format;
	size_t npages[4];
	size_t nentries[4];
	size_t cache_size;
	size_t line_size;
	size_t nrounds;
	size_t nruns;
	float threshold;
	uintptr_t target;
	uintptr_t evict_target;
	char *output;
	unsigned int cpu;
};

int parse_size(size_t *size, const char *s);
void print_size(FILE *f, size_t size);
void show_usage(const char *prog_name);
void detect_args(struct args *args);
int parse_args(struct args *args, int argc, const char *argv[]);
void print_args(FILE *f, struct args *args, struct page_format *fmt);
struct page_format *get_page_format_from_args(struct args *args);
