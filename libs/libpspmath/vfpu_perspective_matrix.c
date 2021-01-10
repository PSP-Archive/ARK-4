#include "pspmath.h"

void vfpu_perspective_matrix(ScePspFMatrix4 *m, float fovy, float aspect, float near, float far) {
	__asm__ volatile (
		"vmzero.q M100\n"					// set M100 to all zeros
		"mtv     %1, S000\n"				// S000 = fovy
		"viim.s  S001, 90\n"				// S002 = 90.0f
		"vrcp.s  S001, S001\n"				// S002 = 1/90
		"vmul.s  S000, S000, S000[1/2]\n"	// S000 = fovy * 0.5 = fovy/2
		"vmul.s  S000, S000, S001\n"		// S000 = (fovy/2)/90
		"vrot.p  C002, S000, [c, s]\n"		// S002 = cos(angle), S003 = sin(angle)
		"vdiv.s  S100, S002, S003\n"		// S100 = m->x.x = cotangent = cos(angle)/sin(angle)
		"mtv     %3, S001\n"				// S001 = near
		"mtv     %4, S002\n"				// S002 = far
		"vsub.s  S003, S001, S002\n"		// S003 = deltaz = near-far
		"vrcp.s  S003, S003\n"				// S003 = 1/deltaz
		"mtv     %2, S000\n"				// S000 = aspect
		"vmov.s  S111, S100\n"				// S111 = m->y.y = cotangent
		"vdiv.s  S100, S100, S000\n"		// S100 = m->x.x = cotangent / aspect
		"vadd.s  S122, S001, S002\n"        // S122 = m->z.z = far + near
		"vmul.s  S122, S122, S003\n"		// S122 = m->z.z = (far+near)/deltaz
		"vmul.s  S132, S001, S002\n"        // S132 = m->w.z = far * near
		"vmul.s  S132, S132, S132[2]\n"     // S132 = m->w.z = 2 * (far*near)
		"vmul.s  S132, S132, S003\n"        // S132 = m->w.z = 2 * (far*near) / deltaz
		"vsub.s   S123, S123, S123[1]\n"	// S123 = m->z.w = -1.0
		"sv.q	 C100, 0  + %0\n"
		"sv.q	 C110, 16 + %0\n"
		"sv.q	 C120, 32 + %0\n"
		"sv.q	 C130, 48 + %0\n"
	:"=m"(*m): "r"(fovy),"r"(aspect),"r"(near),"r"(far));
}
