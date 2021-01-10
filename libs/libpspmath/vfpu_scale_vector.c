#include "pspmath.h"

void vfpu_scale_vector(ScePspFVector4 *vout, ScePspFVector4 *vin, float scale) {
   __asm__ volatile (
       "lv.q    C000, %1\n"
       "mtv     %2, S010\n"
       "vscl.t  C000, C000, S010\n"
       "sv.q    C000, %0\n"
       : "=m"(*vout) : "m"(*vin), "r"(scale));
}
