#include "pspmath.h"

void vfpu_normalize_vector(ScePspFVector4 *v) {
   __asm__ volatile (
       "lv.q   C000, %0\n"
       "vdot.t S010, C000, C000\n"
       "vrsq.s S010, S010\n"
       "vscl.t C000, C000, S010\n"
       "sv.q   C000, %0\n"
       : "+m"(*v));
}
