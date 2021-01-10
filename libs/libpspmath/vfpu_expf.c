#include "pspmath.h"

float vfpu_expf(float x) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_LN2\n"
        "vrcp.s  S001, S001\n"
        "vmul.s  S000, S000, S001\n"
        "vexp2.s S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}
