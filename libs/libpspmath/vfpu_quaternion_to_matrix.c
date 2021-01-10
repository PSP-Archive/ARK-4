#include "pspmath.h"

void vfpu_quaternion_to_matrix(ScePspQuatMatrix *q, ScePspFMatrix4 *m) {
	__asm__ volatile (
       "lv.q      C000, %1\n"                               // C000 = [x,  y,  z,  w ]
       "vmul.q    C010, C000, C000\n"                       // C010 = [x2, y2, z2, w2]
       "vcrs.t    C020, C000, C000\n"                       // C020 = [yz, xz, xy ]
       "vmul.q    C030, C000[x,y,z,1], C000[w,w,w,2]\n"	    // C030 = [wx, wy, wz ]

       "vadd.q    C100, C020[0,z,y,0], C030[0,z,-y,0]\n"    // C100 = [0,     xy+wz, xz-wy]
       "vadd.s    S100, S011, S012\n"                       // C100 = [y2+z2, xy+wz, xz-wy]

       "vadd.q    C110, C020[z,0,x,0], C030[-z,0,x,0]\n"    // C110 = [xy-wz, 0,     yz+wx]
       "vadd.s    S111, S010, S012\n"                       // C110 = [xy-wz, x2+z2, yz+wx]

       "vadd.q    C120, C020[y,x,0,0], C030[y,-x,0,0]\n"    // C120 = [xz+wy, yz-wx, 0    ]
       "vadd.s    S122, S010, S011\n"                       // C120 = [xz+wy, yz-wx, x2+y2]

       "vmscl.t   M100, M100, S033\n"                       // C100 = [2*(y2+z2), 2*(xy+wz), 2*(xz-wy)]
                                                            // C110 = [2*(xy-wz), 2*(x2+z2), 2*(yz+wx)]
                                                            // C120 = [2*(xz+wy), 2*(yz-wx), 2*(x2+y2)]

       "vocp.s    S100, S100\n"                             // C100 = [1-2*(y2+z2), 2*(xy+wz),   2*(xz-wy)  ]
       "vocp.s    S111, S111\n"                             // C110 = [2*(xy-wz),   1-2*(x2+z2), 2*(yz+wx)  ]
       "vocp.s    S122, S122\n"                             // C120 = [2*(xz+wy),   2*(yz-wx),   1-2*(x2+y2)]

       "vidt.q    C130\n"                                   // C130 = [0, 0, 0, 1]

       "sv.q      R100, 0  + %0\n"
       "sv.q      R101, 16 + %0\n"
       "sv.q      R102, 32 + %0\n"
       "sv.q      R103, 48 + %0\n"
	: "=m"(*m) : "m"(*q));
}
