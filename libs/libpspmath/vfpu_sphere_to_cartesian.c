#include "pspmath.h"

void vfpu_sphere_to_cartesian(float az, float ze, float rad, float *x, float *y, float *z) {
	__asm__ volatile (
		"mtv      %3, S000\n"
		"mtv      %4, S001\n"
		"mtv      %5, S002\n"
		"vcst.s   S003, VFPU_2_PI\n" 							// C000 = [az, ze, rad, 2/pi]
		"vscl.p   C000, C000, S003\n"							// C000 = [az*2/pi, ze*2/pi, rad, 2/pi]
		"vrot.p   C010, S000, [s, c]\n"							// C010 = [sin(az), cos(az), ?, ?]
		"vrot.p   C012, S001, [s, c]\n"							// C010 = [sin(az), cos(az), sin(ze), cos(ze)]
		"vmul.q   C020, C010[y, 1, x, 0], C010[z, w, z, 0]\n"   // C020 = [0, cos(az)*sin(ez), cos(ze), sin(az)*sin(ze)]
		"vscl.t   C020, C020, S002\n"							// C020 = [0, r*cos(az)*sin(ez), r*cos(ze), r*sin(az)*sin(ze)]
		//"sv.q     C020, 0 + %0\n"
		"sv.s     S020, %0\n"
		"sv.s     S021, %1\n"
		"sv.s     S022, %2\n"
	:"+m"(*x), "+m"(*y), "+m"(*z)
	:"r"(az), "r"(ze), "r"(rad));
}
