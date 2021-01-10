#include "pspmath.h"

float vfpu_fabsf(float x) {
	float result;
	__asm__ volatile (
		"mtv     %1, S000\n"
		"vmov.s  S000, S000[|x|]\n"
		"mfv     %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}
