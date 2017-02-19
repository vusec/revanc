/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Checks if transparent hugepages is enabled or disabled. */
int check_transparent_hugepages(void)
{
	FILE *f;
	char *p, *line = NULL;
	size_t n = 0;
	int ret = 0;

	if (!(f = fopen("/sys/kernel/mm/transparent_hugepage/enabled", "r")))
		return 0;

	if (getline(&line, &n, f) == -1)
		goto err_close;

	if (!(p = strchr(line, '[')))
		goto err_free_line;

	ret = (strncmp(p, "[always]", 7) == 0);

err_free_line:
	free(line);
err_close:
	fclose(f);
	return ret;
}

