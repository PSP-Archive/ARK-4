#include "pspmath.h"

void vfpu_identity_matrix(ScePspFMatrix4 *m) {
	__asm__ volatile (
		"vmidt.q	M000\n"
		"sv.q		C000, 0  + %0\n"
		"sv.q		C010, 16 + %0\n"
		"sv.q		C020, 32 + %0\n"
		"sv.q		C030, 48 + %0\n"
	:"=m"(*m));
}
