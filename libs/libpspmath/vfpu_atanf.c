#include "pspmath.h"

float vfpu_atanf(float x) {
	float result;
	// result = asinf(x/sqrt(x*x+1))
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vmul.s   S001, S000, S000\n"
		"vadd.s   S001, S001, S001[1]\n"
		"vrsq.s   S001, S001\n"
		"vmul.s   S000, S000, S001\n"
		"vasin.s  S000, S000\n"
		"vcst.s   S001, VFPU_PI_2\n"
		"vmul.s   S000, S000, S001\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}


#define PI   3.14159265358979f
#define PI_2 1.57079632679489f

inline static float fabsf(float x) {
	float r;
	__asm__ volatile( "abs.s %0, %1" : "=f"(r) :"f"(x):"memory");
	return r;
}

/*	double a;

	if (fabs(x) >= fabs(y)) {
		a = atan(y/x) ;
		if (x < 0.0) {
			if (y >= 0.0) a += _pi ;
			else a -= _pi ;
		}
	}
	else {
		a = -atan(x/y) ;
		if (y < 0.0) a -= _halfpi ;
		else     a += _halfpi ;
	}
	return a ;*/

float vfpu_atan2f(float y, float x) {
	float r;

	if (fabsf(x) >= fabsf(y)) {
		r = vfpu_atanf(y/x);
		if   (x < 0.0f) r += (y>=0.0f ? PI : -PI);
	} else {
		r = -vfpu_atanf(x/y);
		r += (y < 0.0f ? -PI_2 : PI_2);
	}
	return r;
}
