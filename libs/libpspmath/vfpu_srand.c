#include "pspmath.h"

void vfpu_srand(unsigned int x) {
	__asm__ volatile ( "mtv %0, S000\n vrnds.s S000" : "=r"(x));
}
