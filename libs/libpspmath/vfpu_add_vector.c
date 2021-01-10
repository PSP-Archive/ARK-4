#include "pspmath.h"

void vfpu_add_vector(ScePspFVector4 *vout, ScePspFVector4 *va, ScePspFVector4 *vb) {
   __asm__ volatile (
       "lv.q    C000, %1\n"
       "lv.q    C010, %2\n"
       "vadd.t  C020, C000, C010\n"
       "sv.q    C020, %0\n"
       : "+m"(*vout): "m"(*va), "m"(*vb));
}
