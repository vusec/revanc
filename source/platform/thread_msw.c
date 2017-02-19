/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>

#define WIN32_MEAN_AND_LEAN
#define NOMINMAX
#include <windows.h>

int pin_cpu(size_t i)
{
	SetThreadAffinityMask(GetCurrentThread(), (1 << i));

	return 0;
}
