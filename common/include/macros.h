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

#ifndef _MACROS_H_
#define _MACROS_H_

// Kernelify Address
#define KERNELIFY(f) (0x80000000 | ((unsigned int)(f)))

// j addr
#define JUMP(f) (0x08000000 | (((unsigned int)(f) >> 2) & 0x03ffffff))

// j addr getter (for kernel range, use in combination with KERNELIFY, works with j & jal)
#define JUMP_TARGET(i) (((unsigned int)(i) & 0x03ffffff) << 2)

// jal addr
#define JAL(f) (0x0C000000 | (((unsigned int)(f) >> 2) & 0x03ffffff))

#define MAKE_JUMP(a, f) _sw(JUMP(f), a);
#define MAKE_CALL(a, f) _sw(JAL(f), a);

// jal checker
#define IS_JAL(i) ((((unsigned int)i) & 0xFC000000) == 0x0C000000)

// syscall number
#define SYSCALL(n) ((n<<6)|12)

// nop
#define NOP 0

// jr ra
#define JR_RA 0x03E00008

// v0 result setter
#define LI_V0(n) ((0x2402 << 16) | ((n) & 0xFFFF))

#define MAKE_DUMMY_FUNCTION_RETURN_0(a) \
    _sw(JR_RA, a);\
    _sw(LI_V0(0), a + 4);\

#define MAKE_DUMMY_FUNCTION_RETURN_1(a) \
    _sw(JR_RA, a);\
    _sw(LI_V0(1), a + 4);\

// Array Element Counter
#define NELEMS(n) ((sizeof(n)) / sizeof(n[0]))

// is UID
#define IsUID(uid) ((uid > 0 && uid < 0x05000000) && ((uid & 1) == 1))

// Min & Max Macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#undef UNUSED
#define UNUSED(arg) ((void)(arg))

//by Bubbletune
#define U_EXTRACT_IMPORT(x) ((((u32)_lw((u32)x)) & ~0x08000000) << 2)
#define K_EXTRACT_IMPORT(x) (((((u32)_lw((u32)x)) & ~0x08000000) << 2) | 0x80000000)
#define U_EXTRACT_CALL(x) ((((u32)_lw((u32)x)) & ~0x0C000000) << 2)
#define K_EXTRACT_CALL(x) (((((u32)_lw((u32)x)) & ~0x0C000000) << 2) | 0x80000000)
#define K_EXTRACT_BRANCH(x) ((((((u32)_lw((u32)x)) & 0x0000FFFF) << 2) + x + 4) | 0x80000000)

// by Acid_Snake
// the opcode is filled with two 0's to the right and shifted to make it a byte long
#define GET_OPCODE(x) ((_lw(x) & 0xFC000000)>>24)
#define GET_FUNCTION_OPCODE(x) (_lw(x) & 0x3F)

#define MAKE_JUMP_PATCH(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a);
#define PTR_ALIGN_64(p) ((void*)((((u32)p)+64-1)&(~(64-1))))

//by Davee
#define HIJACK_FUNCTION(a, f, p) \
{ \
    static u32 _pb_[5]; \
    _sw(_lw((u32)(a)), (u32)_pb_); \
    _sw(_lw((u32)(a) + 4), (u32)_pb_ + 4);\
    _sw(NOP, (u32)_pb_ + 8);\
    _sw(NOP, (u32)_pb_ + 16);\
    MAKE_JUMP_PATCH((u32)_pb_ + 12, (u32)(a) + 8); \
    _sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), (u32)(a)); \
    _sw(0, (u32)(a) + 4); \
    p = (void *)_pb_; \
}

#define REDIRECT_SYSCALL(a, f) \
    _sw(JR_RA, a); \
    _sw(SYSCALL(sceKernelQuerySystemCall(f)), a + 4);

#define MAKE_DUMMY_FUNCTION(a, r) \
{ \
    u32 func = a; \
    if(r == 0) \
    { \
        _sw(JR_RA, func); \
        _sw(0x00001021, func + 4); \
    } \
    else \
    { \
        _sw(JR_RA, func); \
        _sw(0x24020000 | r, func + 4); \
    } \
}

#define REDIRECT_FUNCTION(a, f) \
{ \
    _sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), (u32)(a)); \
    _sw(0, (u32)(a) + 4); \
}

// from adrenaline
#define FW_TO_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)
#define FIRMWARE_TO_FW(f) ((((f >> 24) & 0xF) << 8) | (((f >> 16) & 0xF) << 4) | ((f >> 8) & 0xF))

#define MAKE_SYSCALL_FUNCTION(a, n) \
{ \
    _sw(JR_RA, (u32)(a)); \
    _sw(SYSCALL(n), (u32)(a) + 4); \
}


#define K_HIJACK_CALL(a, f, ptr) \
{ \
    ptr = (void *)K_EXTRACT_CALL(a); \
    MAKE_CALL(a, f); \
}

#endif

