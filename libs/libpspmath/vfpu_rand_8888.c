#include "pspmath.h"

unsigned int vfpu_rand_8888(int min, int max) {
	unsigned int result;
	__asm__ volatile (
		"mtv      %1, S020\n"
		"mtv      %2, S021\n"
		"vmov.t   C000, C020[x, x, x]\n"
		"vmov.t   C010, C020[y, y, y]\n"
		"vi2f.t   C000, C000, 0\n"
		"vi2f.t   C010, C010, 0\n"
		"vsub.t   C010, C010, C000\n"
		"vrndf1.t C020\n"
		"vsub.t   C020, C020, C020[1, 1, 1]\n"
		"vmul.t   C020, C020, C010\n"
		"vadd.t   C020, C020, C000\n"
		"vf2iz.t  C020, C020, 23\n"
		"viim.s   S023, 255\n"
		"vf2iz.s  S023, S023, 23\n"
		"vi2uc.q  S000, C020\n"
		"mfv      %0, S000\n"
	:"=r"(result): "r"(min), "r"(max));
	return result;
}
