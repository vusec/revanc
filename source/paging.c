/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "paging.h"

extern struct page_format page_formats[];

struct page_format *get_page_format(const char *name)
{
	struct page_format *fmt;

	for (fmt = page_formats; fmt->name; ++fmt) {
		if (strcmp(name, fmt->name) == 0)
			return fmt;
	}

	return NULL;
}

struct page_format *get_default_page_format(void)
{
	return page_formats;
}

void list_page_formats(FILE *f)
{
	struct page_format *fmt;

	for (fmt = page_formats; fmt->name; ++fmt) {
		fprintf(f, "%s ", fmt->name);
	}
}
