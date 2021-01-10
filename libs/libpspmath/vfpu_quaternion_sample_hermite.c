#include "pspmath.h"

// constants for the hermite curve functions
ScePspFMatrix4 hermite = {
	{  2, -3,  0,  1 },
	{ -2,  3,  0,  0 },
	{  1, -2,  1,  0 },
	{  1, -1,  0,  0 }
};

void vfpu_quaternion_sample_hermite(ScePspQuatMatrix *qout, ScePspQuatMatrix *a, ScePspQuatMatrix *b, ScePspQuatMatrix *at, ScePspQuatMatrix *bt, float t) {
	__asm__ volatile (
		// load hermite transform matrix
		"lv.q    C000,  0(%6)\n"
		"lv.q    C010, 16(%6)\n"
		"lv.q    C020, 32(%6)\n"
		"lv.q    C030, 48(%6)\n"

		// load a, b, at, bt
		"lv.q    C100,  0(%1)\n"
		"lv.q    C110,  0(%2)\n"
		"lv.q    C120,  0(%3)\n"
		"lv.q    C130,  0(%4)\n"

		// C200 = [ t^3, t^2, t, 1]
		"mtv     %5, S202\n"
		"vmul.s  S201, S202, S202\n"
		"vmul.s  S200, S202, S201\n"
		"vone.s  S203\n"

		// multiply M000 by C200
		// C000 = [  2*t^3, -3*t^2, 0, 1]
		// C010 = [ -2*t^3,  3*t^2, 0, 0]
		// C020 = [    t^3, -2*t^2, t, 0]
		// C030 = [    t^3,   -t^2, 0, 0]

		"vmul.q  C000, C000, C200\n"
		"vmul.q  C010, C010, C200\n"
		"vmul.q  C020, C020, C200\n"
		"vmul.q  C030, C030, C200\n"

		// sum the terms
		// S210 =  2*t^3 - 3*t^2 + 1
		// S211 = -2*t^3 + 3*t^2
		// S212 =    t^3 - 2*t^2 + t
		// S213 =    t^3 -   t^2

		"vfad.q  S210, C000\n"
		"vfad.q  S211, C010\n"
		"vfad.q  S212, C020\n"
		"vfad.q  S213, C030\n"

		// scale the qaternions with terms
		"vscl.q  C100, C100, S210\n"
		"vscl.q  C110, C110, S211\n"
		"vscl.q  C120, C120, S212\n"
		"vscl.q  C130, C130, S213\n"

		// sum the results
		"vadd.q  C100, C100, C110\n"
		"vadd.q  C100, C100, C120\n"
		"vadd.q  C100, C100, C130\n"

		// and return results
		"sv.q    C100, 0(%0)\n"
	:"+r"(qout): "r"(a), "r"(b), "r"(at), "r"(bt), "r"(t), "r"(&hermite));
}
