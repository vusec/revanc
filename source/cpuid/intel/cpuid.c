/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <macros.h>

#include <cpuid/cpuid.h>

const char *models[] = {
	[0x1a] = "Nehalem",
	[0x1d] = "Dunnington",
	[0x1e] = "Nehalem",
	[0x1f] = "Nehalem",
	[0x25] = "Westmere",
	[0x2a] = "Sandy Bridge",
	[0x2c] = "Westmere",
	[0x2d] = "Sandy Bridge",
	[0x2e] = "Nehalem",
	[0x2f] = "Westmere",
	[0x37] = "Baytrail",
	[0x3a] = "Ivy Bridge",
	[0x3c] = "Haswell",
	[0x3d] = "Broadwell",
	[0x3e] = "Ivy Bridge",
	[0x3f] = "Haswell",
	[0x45] = "Haswell",
	[0x46] = "Haswell",
	[0x47] = "Broadwell",
	[0x4d] = "Avoton",
	[0x4e] = "Skylake",
	[0x4f] = "Broadwell",
	[0x55] = "Skylake",
	[0x56] = "Broadwell",
	[0x5c] = "Apollo Lake",
	[0x5e] = "Skylake",
	[0x5f] = "Denverton",
	[0x8e] = "Kaby Lake",
	[0x9e] = "Kaby Lake",
};

const char *intel_get_cpu_model(void)
{
	unsigned model;
	uint32_t reg;

	asm volatile(
		"cpuid" :
		"=a" (reg) :
		"a" (0x00000001) :
		"%ebx", "%ecx", "%edx");

	model = (EXTRACT(reg, 16, 4) << 4) | EXTRACT(reg, 4, 4);

	return models[model];
}
