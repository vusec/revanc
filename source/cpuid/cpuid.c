/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cpuid/cpuid.h>

#include "amd/cpuid.h"
#include "intel/cpuid.h"

struct vendor {
	const char *key;
	enum cpuid_vendor_id value;
} vendors[] = {
	{ "AuthenticAMD", CPUID_VENDOR_AMD },
	{ "GenuineIntel", CPUID_VENDOR_INTEL },
};

unsigned cpuid_get_max_leaf(void)
{
	static unsigned max_leaf;

	if (max_leaf)
		return max_leaf;
	
	asm volatile(
		"cpuid" :
		"=a" (max_leaf) :
		"a" (0) :
		"%ebx", "%ecx", "%edx");

	return max_leaf;
}

unsigned cpuid_get_max_ext_leaf(void)
{
	static unsigned max_leaf;

	if (max_leaf)
		return max_leaf;

	asm volatile(
		"cpuid" :
		"=a" (max_leaf) :
		"a" (0x80000000) :
		"%ebx", "%ecx", "%edx");

	return max_leaf;
}

enum cpuid_vendor_id cpuid_get_vendor_id(void)
{
	static enum cpuid_vendor_id vendor_id = CPUID_VENDOR_NONE;
	const char *vendor;
	size_t i;

	if (vendor_id != CPUID_VENDOR_NONE)
		return vendor_id;

	vendor = cpuid_get_vendor();

	for (i = 0; i < sizeof(vendors) / sizeof(*vendors); ++i) {
		if (strcmp(vendor, vendors[i].key) == 0) {
			vendor_id = vendors[i].value;

			return vendor_id;
		}
	}

	return vendor_id;
}

const char *cpuid_get_vendor(void)
{
	uint32_t regs[3];
	static char buffer[13];
	static char *vendor;

	if (vendor)
		return vendor;

	asm volatile(
		"cpuid" :
		"=b" (regs[0]), "=c" (regs[2]), "=d" (regs[1]) :
		"a" (0));

	vendor = buffer;
	memcpy(vendor, regs, 12);

	return vendor;
}

const char *cpuid_get_cpu_name(void)
{
	static char buf[3 * 4 * sizeof(uint32_t) + 1];
	char *name = NULL, *p;
	uint32_t func;

	if (name)
		return name;

	if (cpuid_get_max_ext_leaf() < 0x80000004)
		return 0;

	for (p = buf, func = 0x80000002; func <= 0x80000004; p += 16, ++func) {
		asm volatile("cpuid\n" :
			"=a" (*(uint32_t *)p),
			"=b" (*(uint32_t *)(p + 4)),
			"=c" (*(uint32_t *)(p + 8)),
			"=d" (*(uint32_t *)(p + 12)) :
			"a" (func));
	}

	name = buf + strspn(buf, " ");

	return name;
}

const char *cpuid_get_cpu_model(void)
{
	switch (cpuid_get_vendor_id()) {
	case CPUID_VENDOR_AMD: return amd_get_cpu_model();
	case CPUID_VENDOR_INTEL: return intel_get_cpu_model();
	default: return NULL;
	}
}
