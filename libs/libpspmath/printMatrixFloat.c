#include <stdio.h>
#include "pspmath.h"

void printMatrixFloat(int matid) {
	float m[16];

	#define SV(N)					\
		asm("usv.q	R"#N"00,  0 + %0\n"	\
		    "usv.q	R"#N"01, 16 + %0\n"	\
		    "usv.q	R"#N"02, 32 + %0\n"	\
		    "usv.q	R"#N"03, 48 + %0\n"	\
		    : "=m"(m))

	switch (matid) {
		case 0:		SV(0); break;
		case 1:		SV(1); break;
		case 2:		SV(2); break;
		case 3:		SV(3); break;
		case 4:		SV(4); break;
		case 5:		SV(5); break;
		case 6:		SV(6); break;
		case 7:		SV(7); break;
	}

	printf("\n\n");
	printf("      C%d00    C%d10    C%d20    C%d30\n", matid, matid, matid, matid);
	printf("R%d00: %0.6f %0.6f %0.6f %0.6f\n", matid, m[0], m[1], m[2], m[3]);
	printf("R%d01: %0.6f %0.6f %0.6f %0.6f\n", matid, m[4], m[5], m[6], m[7]);
	printf("R%d02: %0.6f %0.6f %0.6f %0.6f\n", matid, m[8], m[9], m[10], m[11]);
	printf("R%d03: %0.6f %0.6f %0.6f %0.6f\n", matid, m[12], m[13], m[14], m[15]);
}
