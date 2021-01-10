#include "pspmath.h"

void vfpu_quaternion_ln(ScePspQuatMatrix *qout, ScePspQuatMatrix *qin) {
	//float r  = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
	//float t  = r>0.00001f? atan2(r,a[3])/r: 0.f;
	//return quat(t*a[0],t*a[1],t*a[2],0.5*log(norm(a)));
	float r;
	__asm__ volatile (
		"lv.q     C010, 0 + %1\n"
		"vdot.t   S020, C010, C010\n"		// r = x^2 + y^2 + z^2
		"vsqrt.s  S020, S020\n"				// r = sqrt(x^2 + y^2 + z^2)
		"mfv      %0, S020\n"
	:"=r"(r): "m"(*qin));
	r = vfpu_atan2f(r, qin->w)/r;
	__asm__ volatile (
		"mtv      %1, S021\n"				// t = atan2(r, w) / r
		"vdot.q   S022, C010, C010\n"		// norm = x^2 + y^2 + z^2 + w^2
		"vcst.s   S023, VFPU_LOG2E\n"
        "vrcp.s   S023, S023\n"
        "vlog2.s  S013, S022\n"
        "vmul.s   S013, S013, S023\n"
		"vmul.s   S013, S013, S013[1/2]\n"
        "vscl.t   C010, C010, S021\n"
        "sv.q     C010, 0 + %0\n"
	: "=m"(*qout) : "r"(r));
}
