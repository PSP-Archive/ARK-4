#include "pspmath.h"

void vfpu_quaternion_multiply(ScePspQuatMatrix *qout, ScePspQuatMatrix *a, ScePspQuatMatrix *b) {
	__asm__ volatile (
		"lv.q    C000, %1\n"			// load quaternion a
		"lv.q    C010, %2\n"			// load quaternion b
		"vqmul.q C020, C000, C010\n"	// C000 = quat a * quat b (quaternion multiply)
		"sv.q    C020, %0\n"			// store result
		: "+m"(*qout) : "m"(*a), "m"(*b));
}
