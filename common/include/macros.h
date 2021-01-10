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

// Array Element Counter
#define NELEMS(n) ((sizeof(n)) / sizeof(n[0]))

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

// by Acid_Snake
// the opcode is filled with two 0's to the right and shifted to make it a byte long
#define GET_OPCODE(x) ((_lw(x) & 0xFC000000)>>24)
#define GET_FUNCTION_OPCODE(x) (_lw(x) & 0x3F)

#define MAKE_JUMP_PATCH(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a);

//by Davee
#define HIJACK_FUNCTION(a, f, ptr) \
{ \
	u32 func = a; \
	static u32 patch_buffer[3]; \
	_sw(_lw(func), (u32)patch_buffer); \
	_sw(_lw(func + 4), (u32)patch_buffer + 8);\
	MAKE_JUMP_PATCH((u32)patch_buffer + 4, func + 8); \
	_sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), func); \
	_sw(0, func + 4); \
	ptr = (void *)patch_buffer; \
}

#define MAKE_DUMMY_FUNCTION(a, r) \
{ \
	u32 func = a; \
	if(r == 0) \
	{ \
		_sw(0x03E00008, func); \
		_sw(0x00001021, func + 4); \
	} \
	else \
	{ \
		_sw(0x03E00008, func); \
		_sw(0x24020000 | r, func + 4); \
	} \
}

#define REDIRECT_FUNCTION(a, f) \
{ \
	u32 func = a; \
	_sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), func); \
	_sw(0, func + 4); \
}

#endif

