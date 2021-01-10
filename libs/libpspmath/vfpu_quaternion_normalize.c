#include "pspmath.h"

void vfpu_quaternion_normalize(ScePspQuatMatrix *res) {
	__asm__ volatile (
		"lv.q    C000, %0\n"			// load quaternion into C000
		"vdot.q  S010, C000, C000\n"	// S010 = x^2 + y^2 + z^2 + w^2
		"vrsq.s  S010, S010\n"			// S020 = 1.0 / sqrt(S100)
		"vscl.q  C000, C000, S010\n"	// C000 = C000 * S010 (normalized quaternion)
		"sv.q    C000, %0\n"			// store into quaternion result
		: "+m"(*res));
}
