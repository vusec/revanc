/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <macros.h>

#include <cpuid/cpuid.h>

#define AMD_FAMILY_K8			0x0f
#define AMD_FAMILY_K10			0x10
#define AMD_FAMILY_K8L			0x11
#define AMD_FAMILY_FUSION		0x12
#define AMD_FAMILY_BOBCAT		0x14
#define AMD_FAMILY_JAGUAR		0x16
#define AMD_FAMILY_BULLDOZER	0x15

const char *amd_get_cpu_model(void)
{
	unsigned family, model;
	uint32_t reg;

	asm volatile(
		"cpuid" :
		"=a" (reg) :
		"a" (0x00000001) :
		"%ebx", "%ecx", "%edx");

	family = EXTRACT(reg, 8, 4) + EXTRACT(reg, 20, 8);
	model = (EXTRACT(reg, 16, 4) << 4) | EXTRACT(reg, 4, 4);

	switch (family) {
	case AMD_FAMILY_K8: return "Hammer";
	case AMD_FAMILY_K10: return "K10";
	case AMD_FAMILY_K8L: return "K8L";
	case AMD_FAMILY_FUSION: return "Fusion";
	case AMD_FAMILY_BOBCAT: return "Bobcat";
	case AMD_FAMILY_JAGUAR: {
		switch (model) {
		case 0x00 ... 0x0f: return "Jaguar";
		case 0x30 ... 0x3f: return "Puma";
		default: return NULL;
		}
	}
	case AMD_FAMILY_BULLDOZER: {
		switch (model) {
		case 0x00 ... 0x01: return "Bulldozer";
		case 0x02: return "Piledriver";
		case 0x10 ... 0x1f: return "Piledriver";
		case 0x30 ... 0x3f: return "Steamroller";
		case 0x60 ... 0x6f: return "Excavator";
		default: return NULL;
		}
	}
	default: return NULL;
	}
}
