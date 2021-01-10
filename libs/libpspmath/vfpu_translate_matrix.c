#include "pspmath.h"

void vfpu_translate_matrix(ScePspFMatrix4 *m, float x, float y, float z)
{
	__asm__ volatile (
		"vmidt.q M000\n"
		"mtv        %1, S030\n"
		"mtv        %2, S031\n"
		"mtv        %3, S032\n"
		//"vmidt.q M100\n"
		//"lv.q   C200, %1\n"
		//"vmov.t  C130, C200\n"
		//"vmmul.q M200, M100, M000\n"
		"sv.q C000,  0 + %0\n"
		"sv.q C010, 16 + %0\n"
		"sv.q C020, 32 + %0\n"
		"sv.q C030, 48 + %0\n"
		: "=m"(*m) : "r"(x), "r"(y), "r"(z));
}
