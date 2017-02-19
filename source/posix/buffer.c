/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include "buffer.h"
#include "macros.h"
#include "paging.h"

#ifndef MAP_NORESERVE
#define MAP_NORESERVE 0
#endif /* MAP_NORESERVE */

struct buffer *new_buffer(struct page_format *fmt, void *target)
{
	struct buffer *buffer;
	struct page_level *level;
	size_t stride = 0;
	size_t i;
	unsigned flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE;

	if (target)
		flags |= MAP_FIXED;

	if (!(buffer = calloc(1,sizeof *buffer)))
		return NULL;

	for (i = 0, level = fmt->levels; i < fmt->nlevels; ++i, ++level) {
		stride = level->page_size;
		buffer->size = max(buffer->size, level->npages * stride);
	}

	if ((buffer->data = mmap(target, buffer->size, PROT_READ | PROT_WRITE,
		flags, -1, 0)) == MAP_FAILED) {
		dperror();
		goto err_free_buffer;	
	}

	return buffer;

err_free_buffer:
	free(buffer);
	return NULL;
}

void del_buffer(struct buffer *buffer)
{
	munmap(buffer->data, buffer->size);
	free(buffer);
}
