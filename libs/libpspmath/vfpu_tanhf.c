#include "pspmath.h"

float vfpu_tanhf(float x) {
	float result;
	//y = exp(x+x);
	//return (y-1)/(y+1);
	__asm__ volatile (
		"mtv      %0, S000\n"
		"vadd.s   S000, S000, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmul.s   S000, S000, S001\n"
        "vexp2.s  S000, S000\n"
        "vone.s   S001\n"
        "vbfy1.p  C002, C000\n"
        "vdiv.s   S000, S003, S002\n"
        "mfv      %0, S000\n"
	: "=r"(result): "r"(x));
	return result;
}
