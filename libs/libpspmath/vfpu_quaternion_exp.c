#include "pspmath.h"

void vfpu_quaternion_exp(ScePspQuatMatrix *qout, ScePspQuatMatrix *qin) {
	//float r  = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
	//float et = exp(a[3]);
	//float s  = r>=0.00001f? et*sin(r)/r: 0.f;
	//return quat(s*a[0],s*a[1],s*a[2],et*cos(r));
	__asm__ volatile (
		"lv.q     C000, 0 + %1\n"			// C000 = [x, y, z, w]
		"vdot.t   S010, C000, C000\n"		// S010 = x^2 + y^2 + z^2
		"vsqrt.s  S010, S010\n"				// S010 = r = sqrt(x^2 + y^2 + z^2)
		"vcst.s   S011, VFPU_LN2\n"			// S011 = ln(2)
		"vrcp.s   S011, S011\n"				// S011 = 1/ln(2)
		"vmul.s   S011, S011, S003\n"		// S011 = w*(1/ln(2))
		"vexp2.s  S011, S011\n"				// S011 = et = exp(w)
		"vcst.s   S012, VFPU_2_PI\n"		// S012 = 2/PI
		"vmul.s   S012, S012, S010\n"		// S012 = r * 2/PI
		"vrot.p   R003, S012, [c,s]\n"		// S003 = cos(r), S013 = sin(r)
		"vdiv.s   S013, S013, S010\n"		// S013 = sin(r)/r
		"vscl.p   R003, R003, S011\n"  		// S003 = et * cos(r), S013 = et * sin(r)/r
		"vscl.t   C000, C000, S013\n"		// C000 = [s*x, s*y, s*z, et*cos(r)]
		"sv.q     C000, 0 + %0\n"
	: "=m"(*qout) : "m"(*qin));
}
