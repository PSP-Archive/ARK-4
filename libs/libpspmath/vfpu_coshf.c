#include "pspmath.h"

float vfpu_coshf(float x) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmov.s   S002, S000[|x|]\n"
        "vmul.s   S002, S001, S002\n"
        "vexp2.s  S002, S002\n"
        "vrcp.s   S003, S002\n"
        "vadd.s   S002, S002, S003\n"
        "vmul.s   S002, S002, S002[1/2]\n"
        "mfv      %0, S002\n"
	: "=r"(result) : "r"(x));
	return result;
}
