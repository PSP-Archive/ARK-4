/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef KXPLOIT_H
#define KXPLOIT_H

#include <functions.h>

#define SYSMEM_OFFSET_CHECK SYSMEM_TEXT+0x00002FA8

#ifndef PRTSTR
#define PRTSTR11(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11) g_tbl->prtstr(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)
#define PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) PRTSTR11(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, 0)
#define PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, x9) PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, 0)
#define PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, x8) PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, 0)
#define PRTSTR7(text, x1, x2, x3, x4, x5, x6, x7) PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, 0)
#define PRTSTR6(text, x1, x2, x3, x4, x5, x6) PRTSTR7(text, x1, x2, x3, x4, x5, x6, 0)
#define PRTSTR5(text, x1, x2, x3, x4, x5) PRTSTR6(text, x1, x2, x3, x4, x5, 0)
#define PRTSTR4(text, x1, x2, x3, x4) PRTSTR5(text, x1, x2, x3, x4, 0)
#define PRTSTR3(text, x1, x2, x3) PRTSTR4(text, x1, x2, x3, 0)
#define PRTSTR2(text, x1, x2) PRTSTR3(text, x1, x2, 0)
#define PRTSTR1(text, x1) PRTSTR2(text, x1, 0)
#define PRTSTR(text) PRTSTR1(text, 0)
#endif

extern UserFunctions* g_tbl;
extern int kx_type;
extern u32 test_val;

extern u32 readKram(u32 addr);
extern int stubScanner(UserFunctions*);
extern int doExploit(void);
extern void executeKernel(u32 kernelContentFunction);
extern void repairInstruction(KernelFunctions*);

#endif
