#include <stdio.h>

typedef unsigned int u32;

int lol[7];

void sceNetMCopyback(int *a0, int a1, int a2, int a3, int value)
{
	int *s6 = a0;
	u32 fp = 0;
	int s5 = a1;
	int *s3 = s6;
	int s2 = a2;
	int s1 = a3;

	if (a3 > 0) {
printf(" 1\n");
		u32 v1 = a3 + a2;
		u32 _a1 = v1 | a3;
		u32 _a2 = _a1 | a2;
		_a1 = _a2 & 0x80000000;

		if (((int)_a1 >= 0) && (v1 >= 0) && (s6 > 0)) {
printf(" 2\n");
			int s0 = s6[3];
printf("s5: 0x%08X\ns0: 0x%08X\n", s5, s0);
			if ((s5 > s0) && (s3 >= 0)) {
printf(" 3\n");
				int v0 = s3[0];
				fp += s0;
				s5 -= s0;
printf("s5: 0x%08X\nfp: 0x%08X\n", s5, fp);
				if (v0) {
printf(" 4\n");
					s0 = value;

					if (s5 > s0) {
						printf("loop back\n");
						return;
					}

					if (s2 <= 0) {
printf(" 5\n");
						s1 = s6[4] >> 16;

						if (s1 & 2) {
printf(" 6\n");
							int _s3 = s6[6];
printf("fp: 0x%08X\ns3: 0x%08X\n", fp, _s3);
							if ((int)fp > _s3)
								s6[6] = fp;
							else
								printf("no store\n");
						}
					}
				}
			}
		}
	}
}

int main()
{
	int val = 0x80000000;
	lol[0] = 0x88000000 - 12;
	lol[3] = 0x80000001;
	lol[4] = 0x20000;
	lol[6] = 0x80000000;
	sceNetMCopyback(lol, 0x8000000+0x80000001, 0, 1, val);
	printf("ret: 0x%08X\n\n", lol[6]);

	lol[6] = 0x80000000;
	sceNetMCopyback(lol, 0x8000001+0x80000001, 0, 1, val);
	printf("ret: 0x%08X\n\n", lol[6]);

	return 0;
}
