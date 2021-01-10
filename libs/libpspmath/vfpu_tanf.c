#include "pspmath.h"

float vfpu_tanf(float x) {
	float result;
	// result = sin(x)/cos(x);
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_2_PI\n"
        "vmul.s   S000, S000, S001\n"
        "vrot.p   C002, S000, [s, c]\n"
        "vdiv.s   S000, S002, S003\n"
        "mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}
