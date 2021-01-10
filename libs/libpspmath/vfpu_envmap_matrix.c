#include "pspmath.h"

void vfpu_envmap_matrix(ScePspFVector4 *envmat, float r) {
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_2_PI\n"
		"vmul.s   S000, S000, S001\n"
		"vrot.q   C010, S000, [c, s, 0, 0]\n"
		"vrot.q   C020, S000, [-s, c, 0, 0]\n"
		"sv.q     C010, 0  + %0\n"
		"sv.q     C020, 16 + %0\n"
	:"=m"(*envmat):"r"(r));
}
