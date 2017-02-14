/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>

#include <errno.h>

void dperror_ext(const char *fname, int line_no)
{
	fprintf(stderr, "%s:%d: ", fname, line_no);
	perror("");
}

