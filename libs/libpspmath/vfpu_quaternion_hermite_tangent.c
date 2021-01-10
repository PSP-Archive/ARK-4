#include "pspmath.h"

void vfpu_quaternion_hermite_tangent(ScePspQuatMatrix *qout, ScePspQuatMatrix *p1, ScePspQuatMatrix *p2, float bias) {
	__asm__ volatile (
		// load p1 and p2
		"lv.q    C000, 0(%1)\n"
		"lv.q    C010, 0(%2)\n"

		// load bias
		"mtv     %3, S100\n"

		// C020 = C010 - C000
		"vsub.q  C020, C010, C000\n"

		// scale C020 by bias
		"vscl.q  C020, C020, S100\n"

		// store result
		"sv.q    C020, 0(%0)\n"
	:"+r"(qout): "r"(p1), "r"(p2), "r"(bias));
}
