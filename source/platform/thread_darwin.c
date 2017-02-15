/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <pthread.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>

int pin_cpu(size_t i)
{
	pthread_t thread = pthread_self();
        thread_port_t mach_thread = pthread_mach_thread_np(thread);

	thread_affinity_policy_data_t policy = { i };
        thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);

        return 0;
}
