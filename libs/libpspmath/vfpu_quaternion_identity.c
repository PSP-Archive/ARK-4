#include "pspmath.h"

void vfpu_quaternion_identity(ScePspQuatMatrix *q) {
	__asm__ volatile (
		"vidt.q C030\n"		// column is important here, we need the w component set to 1
		"sv.q   C030, %0\n"
		: "+m"(*q));
}
