#include "pspmath.h"

void vfpu_quaternion_sample_linear(ScePspQuatMatrix *qout, ScePspQuatMatrix *a, ScePspQuatMatrix *b, float t) {
	__asm__ volatile (
		"lv.q     C000, 0 + %1\n"
		"lv.q     C010, 0 + %2\n"
		"mtv      %3, S020\n"
		"vocp.s   S021, S020\n"
		"vscl.q   C000, C000, S021\n"
		"vscl.q   C010, C010, S020\n"
		"vadd.q   C000, C000, C010\n"
		"sv.q     C000, 0 + %0\n"
	: "=m"(*qout) : "m"(*a), "m"(*b), "r"(t));
}
