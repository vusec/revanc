/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

enum cpuid_vendor_id {
	CPUID_VENDOR_NONE = -1,
	CPUID_VENDOR_INTEL,
	CPUID_VENDOR_AMD,
};

unsigned cpuid_get_max_leaf(void);
unsigned cpuid_get_max_ext_leaf(void);
enum cpuid_vendor_id cpuid_get_vendor_id(void);
const char *cpuid_get_vendor(void);
const char *cpuid_get_cpu_name(void);
const char *cpuid_get_cpu_model(void);
