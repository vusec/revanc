/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <string.h>

#define WIN32_MEAN_AND_LEAN
#define NOMINMAX
#include <windows.h>

#include "buffer.h"
#include "macros.h"
#include "paging.h"

struct buffer *new_buffer(struct page_format *fmt, void *target)
{
	struct buffer *buffer;
	struct page_level *level;
	char *page;
	size_t stride = 0;
	size_t i, j;

	if (!(buffer = malloc(sizeof *buffer)))
		return NULL;

	buffer->size = 0;

	for (i = 0, level = fmt->levels; i < fmt->nlevels; ++i, ++level) {
		stride = level->page_size;
		buffer->size = max(buffer->size, level->npages * stride);
	}

	if (!(buffer->data = VirtualAlloc(target, buffer->size, MEM_RESERVE,
		PAGE_READWRITE)))
		goto err_free_buffer;

	for (j = 0, level = fmt->levels; j < fmt->nlevels; ++j, ++level) {
		page = buffer->data;

		for (i = 0; i < level->npages; ++i) {
			VirtualAlloc(page, 4 * KIB, MEM_COMMIT, PAGE_READWRITE);
			page += level->page_size;
		}
	}

	return buffer;

err_free_buffer:
	free(buffer);
	return NULL;
}

void del_buffer(struct buffer *buffer)
{
	VirtualFree(buffer->data, buffer->size, MEM_RELEASE);
	free(buffer);
}
