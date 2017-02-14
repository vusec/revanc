/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <string.h>

#include "macros.h"

void memswap(void *lhs_, void *rhs_, size_t n)
{
	char tmp, *lhs = lhs_, *rhs = rhs_;

	while (n--) {
		tmp = *lhs;
		*lhs++ = *rhs;
		*rhs++ = tmp;
	}
}

void shuffle(void *data, size_t n, size_t nmemb)
{
	size_t i;

	while (--n) {
		i = rand() % n;
		memswap((char *)data + i * nmemb,
			(char *)data + n * nmemb,
			nmemb);
	}
}

/* It appears that shuffling does not make a difference on the tested systems. */
void generate_indicies(size_t *indicies, size_t num)
{
	size_t i;

	for (i = 0; i < num; ++i)
		indicies[i] = i;
}
