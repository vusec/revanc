/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "path.h"

/* If the path does not exist yet, a directory is created for the given path.
 * Otherwise, it is checked whether the path already points to an existing
 * directory or whether it is a symbolic link that points to an existing
 * directory.
 */
static int do_mkdir(const char *path)
{
	struct stat st;
	char *rpath;
	int ret;

	if (!(rpath = realpath(path, NULL)))
		return mkdir(path, 0777);

	ret = stat(rpath, &st);
	free(rpath);

	if (ret < 0)
		return -1;

	if (!S_ISDIR(st.st_mode))
		return -1;

	return 0;
}

/* Recursively checks whether a directory exists or creates a directory for
 * each subpath of a given path.
 */
int mkpath(const char *path)
{
	char *fpath = strdup(path);
	char *p;
	int ret = 0;

	for (p = strchr(fpath + 1, '/'); p; p = strchr(p + 1, '/')) {
		*p = '\0';

		if ((ret = do_mkdir(fpath)) < 0)
			goto err_free_fpath;

		*p = '/';
	}

	ret = do_mkdir(fpath);

err_free_fpath:
	free(fpath);
	return ret;
}
