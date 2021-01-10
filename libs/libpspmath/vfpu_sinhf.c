#include "pspmath.h"

float vfpu_sinhf(float x) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmov.s   S002, S000[|x|]\n"
		"vcmp.s   NE, S000, S002\n"
        "vmul.s   S002, S001, S002\n"
        "vexp2.s  S002, S002\n"
        "vrcp.s   S003, S002\n"
        "vsub.s   S002, S002, S003\n"
        "vmul.s   S002, S002, S002[1/2]\n"
        "vcmov.s  S002, S002[-x], 0\n"
        "mfv      %0, S002\n"
	: "=r"(result) : "r"(x));
	return result;
}
