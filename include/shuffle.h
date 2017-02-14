/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdlib.h>

#include "macros.h"

void memswap(void *lhs, void *rhs, size_t n);
void shuffle(void *data, size_t nmemb, size_t n);
void generate_indicies(size_t *indicies, size_t num);
