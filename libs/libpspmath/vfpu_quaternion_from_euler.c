#include "pspmath.h"

#define PI 3.14159265358979323846f
const float piover180 = PI/180.0f;

void vfpu_quaternion_from_euler(ScePspQuatMatrix *res, float x, float y, float z) {
	__asm__ volatile (
		"mtv	%1, S000\n\t"				// S000 = x
		"mtv	%2, S001\n\t"				// S001 = y
		"mtv	%3, S002\n\t"				// S002 = z
		"lv.s   S010, 0(%4)\n"
		"vscl.t C000, C000, S010\n\t"		// x *= pi/180, y *= pi/180, z *= pi/180

		"vfim.s S010, 0.5\n\t"				// S010 = 0.5
		"vscl.t C000, C000, S010\n\t"		// x *= 0.5, y *= 0.5, z *= 0.5
		"vcst.s S010, VFPU_2_PI\n\t"		// load 2/PI into S010, S011 and S012
		"vscl.t C000, C000, S010\n\t"		// x *= 2/PI, y *= 2/pi, z *= pi/2

		"vrot.p C010, S000, [s, c]\n\t"		// S010 = sr, S011 = cr
		"vrot.p C020, S001, [s, c]\n\t"		// S020 = sp, S021 = cp
		"vrot.p C030, S002, [s, c]\n\t"		// S030 = sy, S031 = cy

		// fear the madness of prefixes
		"vmul.q R100, C010[x,y,y,x], C020[y,x,x,y]\n"
		"vmul.q R100, R100,          C030[y,x,y,x]\n"
		"vmul.q R101, C010[y,x,y,x], C020[y,x,y,x]\n"
		"vmul.q R101, R101,          C030[x,y,y,x]\n"
		"vadd.q C000, R100[x,z,0,0], R100[-y,w,0,0]\n"
		"vadd.q C000, C000, R101[0,0,x,z]\n"
		"vadd.q C000, C000, R101[0,0,-y,w]\n"
		"usv.q  C000, %0\n\t"
		:"=m"(*res) : "r"(x), "r"(y), "r"(z), "r"(&piover180));
	vfpu_quaternion_normalize(res);
}
