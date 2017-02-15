/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

int pin_cpu(size_t i)
{
	cpu_set_t cpu_set;
	pthread_t thread;

	thread = pthread_self();

	CPU_ZERO(&cpu_set);
	CPU_SET(i, &cpu_set);

	return pthread_setaffinity_np(thread, sizeof cpu_set, &cpu_set);
}
