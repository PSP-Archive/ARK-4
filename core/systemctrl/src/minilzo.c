/* minilzo.c -- mini subset of the LZO real-time data compression library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2015 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */

/*
 * NOTE:
 *   the full LZO package can be found at
 *   http://www.oberhumer.com/opensource/lzo/
 */

#define __LZO_IN_MINILZO 1

#if defined(LZO_CFG_FREESTANDING)
#  undef MINILZO_HAVE_CONFIG_H
#  define LZO_LIBC_FREESTANDING 1
#  define LZO_OS_FREESTANDING 1
#endif

#ifdef MINILZO_HAVE_CONFIG_H
#  include <config.h>
#endif
#include <limits.h>
#include <stddef.h>

#undef LZO_HAVE_CONFIG_H
#include "minilzo.h"

#ifndef __LZO_CONF_H
#define __LZO_CONF_H 1

#if !defined(__LZO_NOEXPORT1)
#  define __LZO_NOEXPORT1       /*empty*/
#endif
#if !defined(__LZO_NOEXPORT2)
#  define __LZO_NOEXPORT2       /*empty*/
#endif

#if 1
#  define LZO_PUBLIC_DECL(r)    LZO_EXTERN(r)
#endif
#if 1
#  define LZO_PUBLIC_IMPL(r)    LZO_PUBLIC(r)
#endif
#if !defined(LZO_LOCAL_DECL)
#  define LZO_LOCAL_DECL(r)     __LZO_EXTERN_C LZO_LOCAL_IMPL(r)
#endif
#if !defined(LZO_LOCAL_IMPL)
#  define LZO_LOCAL_IMPL(r)     __LZO_NOEXPORT1 r __LZO_NOEXPORT2 __LZO_CDECL
#endif
#if 1
#  define LZO_STATIC_DECL(r)    LZO_PRIVATE(r)
#endif
#if 1
#  define LZO_STATIC_IMPL(r)    LZO_PRIVATE(r)
#endif

#if 1 && !defined(LZO_CFG_FREESTANDING)
#if 1 && !defined(HAVE_STRING_H)
#define HAVE_STRING_H 1
#endif
#if 1 && !defined(HAVE_MEMCMP)
#define HAVE_MEMCMP 1
#endif
#if 1 && !defined(HAVE_MEMCPY)
#define HAVE_MEMCPY 1
#endif
#if 1 && !defined(HAVE_MEMMOVE)
#define HAVE_MEMMOVE 1
#endif
#if 1 && !defined(HAVE_MEMSET)
#define HAVE_MEMSET 1
#endif
#endif

#if 1 && defined(HAVE_STRING_H)
#include <string.h>
#endif

#if 1 || defined(lzo_int8_t) || defined(lzo_uint8_t)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_int8_t)  == 1)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_uint8_t) == 1)
#endif
#if 1 || defined(lzo_int16_t) || defined(lzo_uint16_t)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_int16_t)  == 2)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_uint16_t) == 2)
#endif
#if 1 || defined(lzo_int32_t) || defined(lzo_uint32_t)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_int32_t)  == 4)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_uint32_t) == 4)
#endif
#if defined(lzo_int64_t) || defined(lzo_uint64_t)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_int64_t)  == 8)
LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_uint64_t) == 8)
#endif

#if (LZO_CFG_FREESTANDING)
#  undef HAVE_MEMCMP
#  undef HAVE_MEMCPY
#  undef HAVE_MEMMOVE
#  undef HAVE_MEMSET
#endif

#if !(HAVE_MEMCMP)
#  undef memcmp
#  define memcmp(a,b,c)         lzo_memcmp(a,b,c)
#else
#  undef lzo_memcmp
#  define lzo_memcmp(a,b,c)     memcmp(a,b,c)
#endif
#if !(HAVE_MEMCPY)
#  undef memcpy
#  define memcpy(a,b,c)         lzo_memcpy(a,b,c)
#else
#  undef lzo_memcpy
#  define lzo_memcpy(a,b,c)     memcpy(a,b,c)
#endif
#if !(HAVE_MEMMOVE)
#  undef memmove
#  define memmove(a,b,c)        lzo_memmove(a,b,c)
#else
#  undef lzo_memmove
#  define lzo_memmove(a,b,c)    memmove(a,b,c)
#endif
#if !(HAVE_MEMSET)
#  undef memset
#  define memset(a,b,c)         lzo_memset(a,b,c)
#else
#  undef lzo_memset
#  define lzo_memset(a,b,c)     memset(a,b,c)
#endif

#undef NDEBUG
#if (LZO_CFG_FREESTANDING)
#  undef LZO_DEBUG
#  define NDEBUG 1
#  undef assert
#  define assert(e) ((void)0)
#else
#  if !defined(LZO_DEBUG)
#    define NDEBUG 1
#  endif
#  include <assert.h>
#endif

#if 0 && defined(__BOUNDS_CHECKING_ON)
#  include <unchecked.h>
#else
#  define BOUNDS_CHECKING_OFF_DURING(stmt)      stmt
#  define BOUNDS_CHECKING_OFF_IN_EXPR(expr)     (expr)
#endif

#if (LZO_CFG_PGO)
#  undef __lzo_likely
#  undef __lzo_unlikely
#  define __lzo_likely(e)       (e)
#  define __lzo_unlikely(e)     (e)
#endif

#undef _
#undef __
#undef ___
#undef ____
#undef _p0
#undef _p1
#undef _p2
#undef _p3
#undef _p4
#undef _s0
#undef _s1
#undef _s2
#undef _s3
#undef _s4
#undef _ww

#if 1
#  define LZO_BYTE(x)       ((unsigned char) (x))
#else
#  define LZO_BYTE(x)       ((unsigned char) ((x) & 0xff))
#endif

#define LZO_MAX(a,b)        ((a) >= (b) ? (a) : (b))
#define LZO_MIN(a,b)        ((a) <= (b) ? (a) : (b))
#define LZO_MAX3(a,b,c)     ((a) >= (b) ? LZO_MAX(a,c) : LZO_MAX(b,c))
#define LZO_MIN3(a,b,c)     ((a) <= (b) ? LZO_MIN(a,c) : LZO_MIN(b,c))

#define lzo_sizeof(type)    ((lzo_uint) (sizeof(type)))

#define LZO_HIGH(array)     ((lzo_uint) (sizeof(array)/sizeof(*(array))))

#define LZO_SIZE(bits)      (1u << (bits))
#define LZO_MASK(bits)      (LZO_SIZE(bits) - 1)

#define LZO_USIZE(bits)     ((lzo_uint) 1 << (bits))
#define LZO_UMASK(bits)     (LZO_USIZE(bits) - 1)

#if !defined(DMUL)
#if 0

#  define DMUL(a,b) ((lzo_xint) ((lzo_uint32_t)(a) * (lzo_uint32_t)(b)))
#else
#  define DMUL(a,b) ((lzo_xint) ((a) * (b)))
#endif
#endif

#ifndef UA_SET1
#define UA_SET1             LZO_MEMOPS_SET1
#endif
#ifndef UA_SET2
#define UA_SET2             LZO_MEMOPS_SET2
#endif
#ifndef UA_SET3
#define UA_SET3             LZO_MEMOPS_SET3
#endif
#ifndef UA_SET4
#define UA_SET4             LZO_MEMOPS_SET4
#endif
#ifndef UA_MOVE1
#define UA_MOVE1            LZO_MEMOPS_MOVE1
#endif
#ifndef UA_MOVE2
#define UA_MOVE2            LZO_MEMOPS_MOVE2
#endif
#ifndef UA_MOVE3
#define UA_MOVE3            LZO_MEMOPS_MOVE3
#endif
#ifndef UA_MOVE4
#define UA_MOVE4            LZO_MEMOPS_MOVE4
#endif
#ifndef UA_MOVE8
#define UA_MOVE8            LZO_MEMOPS_MOVE8
#endif
#ifndef UA_COPY1
#define UA_COPY1            LZO_MEMOPS_COPY1
#endif
#ifndef UA_COPY2
#define UA_COPY2            LZO_MEMOPS_COPY2
#endif
#ifndef UA_COPY3
#define UA_COPY3            LZO_MEMOPS_COPY3
#endif
#ifndef UA_COPY4
#define UA_COPY4            LZO_MEMOPS_COPY4
#endif
#ifndef UA_COPY8
#define UA_COPY8            LZO_MEMOPS_COPY8
#endif
#ifndef UA_COPYN
#define UA_COPYN            LZO_MEMOPS_COPYN
#endif
#ifndef UA_COPYN_X
#define UA_COPYN_X          LZO_MEMOPS_COPYN
#endif
#ifndef UA_GET_LE16
#define UA_GET_LE16         LZO_MEMOPS_GET_LE16
#endif
#ifndef UA_GET_LE32
#define UA_GET_LE32         LZO_MEMOPS_GET_LE32
#endif
#ifdef LZO_MEMOPS_GET_LE64
#ifndef UA_GET_LE64
#define UA_GET_LE64         LZO_MEMOPS_GET_LE64
#endif
#endif
#ifndef UA_GET_NE16
#define UA_GET_NE16         LZO_MEMOPS_GET_NE16
#endif
#ifndef UA_GET_NE32
#define UA_GET_NE32         LZO_MEMOPS_GET_NE32
#endif
#ifdef LZO_MEMOPS_GET_NE64
#ifndef UA_GET_NE64
#define UA_GET_NE64         LZO_MEMOPS_GET_NE64
#endif
#endif
#ifndef UA_PUT_LE16
#define UA_PUT_LE16         LZO_MEMOPS_PUT_LE16
#endif
#ifndef UA_PUT_LE32
#define UA_PUT_LE32         LZO_MEMOPS_PUT_LE32
#endif
#ifndef UA_PUT_NE16
#define UA_PUT_NE16         LZO_MEMOPS_PUT_NE16
#endif
#ifndef UA_PUT_NE32
#define UA_PUT_NE32         LZO_MEMOPS_PUT_NE32
#endif

#define MEMCPY8_DS(dest,src,len) \
    lzo_memcpy(dest,src,len); dest += len; src += len

#define BZERO8_PTR(s,l,n) \
    lzo_memset((lzo_voidp)(s),0,(lzo_uint)(l)*(n))

#define MEMCPY_DS(dest,src,len) \
    do *dest++ = *src++; while (--len > 0)


#ifndef __LZO_PTR_H
#define __LZO_PTR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#if (LZO_ARCH_I086)
#error "LZO_ARCH_I086 is unsupported"
#elif (LZO_MM_PVP)
#error "LZO_MM_PVP is unsupported"
#else
#define PTR(a)              ((lzo_uintptr_t) (a))
#define PTR_LINEAR(a)       PTR(a)
#define PTR_ALIGNED_4(a)    ((PTR_LINEAR(a) & 3) == 0)
#define PTR_ALIGNED_8(a)    ((PTR_LINEAR(a) & 7) == 0)
#define PTR_ALIGNED2_4(a,b) (((PTR_LINEAR(a) | PTR_LINEAR(b)) & 3) == 0)
#define PTR_ALIGNED2_8(a,b) (((PTR_LINEAR(a) | PTR_LINEAR(b)) & 7) == 0)
#endif

#define PTR_LT(a,b)         (PTR(a) < PTR(b))
#define PTR_GE(a,b)         (PTR(a) >= PTR(b))
#define PTR_DIFF(a,b)       (PTR(a) - PTR(b))
#define pd(a,b)             ((lzo_uint) ((a)-(b)))


typedef union
{
    char            a_char;
    unsigned char   a_uchar;
    short           a_short;
    unsigned short  a_ushort;
    int             a_int;
    unsigned int    a_uint;
    long            a_long;
    unsigned long   a_ulong;
    lzo_int         a_lzo_int;
    lzo_uint        a_lzo_uint;
    lzo_xint        a_lzo_xint;
    lzo_int16_t     a_lzo_int16_t;
    lzo_uint16_t    a_lzo_uint16_t;
    lzo_int32_t     a_lzo_int32_t;
    lzo_uint32_t    a_lzo_uint32_t;
#if defined(lzo_uint64_t)
    lzo_int64_t     a_lzo_int64_t;
    lzo_uint64_t    a_lzo_uint64_t;
#endif
    size_t          a_size_t;
    ptrdiff_t       a_ptrdiff_t;
    lzo_uintptr_t   a_lzo_uintptr_t;
    void *          a_void_p;
    char *          a_char_p;
    unsigned char * a_uchar_p;
    const void *          a_c_void_p;
    const char *          a_c_char_p;
    const unsigned char * a_c_uchar_p;
    lzo_voidp       a_lzo_voidp;
    lzo_bytep       a_lzo_bytep;
    const lzo_voidp a_c_lzo_voidp;
    const lzo_bytep a_c_lzo_bytep;
}
lzo_full_align_t;

#ifdef __cplusplus
}
#endif

#endif

#ifndef LZO_DETERMINISTIC
#define LZO_DETERMINISTIC 1
#endif

#ifndef LZO_DICT_USE_PTR
#define LZO_DICT_USE_PTR 1
#endif

#if (LZO_DICT_USE_PTR)
#  define lzo_dict_t    const lzo_bytep
#  define lzo_dict_p    lzo_dict_t *
#else
#  define lzo_dict_t    lzo_uint
#  define lzo_dict_p    lzo_dict_t *
#endif

#endif

#if !defined(MINILZO_CFG_SKIP_LZO_UTIL)


#define LZO_BASE 65521u
#define LZO_NMAX 5552

#define LZO_DO1(buf,i)  s1 += buf[i]; s2 += s1
#define LZO_DO2(buf,i)  LZO_DO1(buf,i); LZO_DO1(buf,i+1)
#define LZO_DO4(buf,i)  LZO_DO2(buf,i); LZO_DO2(buf,i+2)
#define LZO_DO8(buf,i)  LZO_DO4(buf,i); LZO_DO4(buf,i+4)
#define LZO_DO16(buf,i) LZO_DO8(buf,i); LZO_DO8(buf,i+8)

#undef LZO_DO1
#undef LZO_DO2
#undef LZO_DO4
#undef LZO_DO8
#undef LZO_DO16

#endif

#if !defined(MINILZO_CFG_SKIP_LZO_STRING)
#undef lzo_memcmp
#undef lzo_memcpy
#undef lzo_memmove
#undef lzo_memset
#if !defined(__LZO_MMODEL_HUGE)
#  undef LZO_HAVE_MM_HUGE_PTR
#endif
#define lzo_hsize_t             lzo_uint
#define lzo_hvoid_p             lzo_voidp
#define lzo_hbyte_p             lzo_bytep
#define LZOLIB_PUBLIC(r,f)      LZO_PUBLIC(r) f
#define lzo_hmemcmp             lzo_memcmp
#define lzo_hmemcpy             lzo_memcpy
#define lzo_hmemmove            lzo_memmove
#define lzo_hmemset             lzo_memset
#define __LZOLIB_HMEMCPY_CH_INCLUDED 1
#if !defined(LZOLIB_PUBLIC)
#  define LZOLIB_PUBLIC(r,f)    r __LZOLIB_FUNCNAME(f)
#endif

#undef LZOLIB_PUBLIC
#endif

#define LZO1X           1
#define LZO_EOF_CODE    1
#define M2_MAX_OFFSET   0x0800

#if !defined(MINILZO_CFG_SKIP_LZO1X_1_COMPRESS)

#if 1 && defined(UA_GET_LE32)
#undef  LZO_DICT_USE_PTR
#define LZO_DICT_USE_PTR 0
#undef  lzo_dict_t
#define lzo_dict_t lzo_uint16_t
#endif

#define LZO_NEED_DICT_H 1
#ifndef D_BITS
#define D_BITS          14
#endif
#define D_INDEX1(d,p)       d = DM(DMUL(0x21,DX3(p,5,5,6)) >> 5)
#define D_INDEX2(d,p)       d = (d & (D_MASK & 0x7ff)) ^ (D_HIGH | 0x1f)
#if 1
#define DINDEX(dv,p)        DM(((DMUL(0x1824429d,dv)) >> (32-D_BITS)))
#else
#define DINDEX(dv,p)        DM((dv) + ((dv) >> (32-D_BITS)))
#endif

#ifndef __LZO_CONFIG1X_H
#define __LZO_CONFIG1X_H 1

#if !defined(LZO1X) && !defined(LZO1Y) && !defined(LZO1Z)
#  define LZO1X 1
#endif

#if !defined(__LZO_IN_MINILZO)
#include <lzo/lzo1x.h>
#endif

#ifndef LZO_EOF_CODE
#define LZO_EOF_CODE 1
#endif
#undef LZO_DETERMINISTIC

#define M1_MAX_OFFSET   0x0400
#ifndef M2_MAX_OFFSET
#define M2_MAX_OFFSET   0x0800
#endif
#define M3_MAX_OFFSET   0x4000
#define M4_MAX_OFFSET   0xbfff

#define MX_MAX_OFFSET   (M1_MAX_OFFSET + M2_MAX_OFFSET)

#define M1_MIN_LEN      2
#define M1_MAX_LEN      2
#define M2_MIN_LEN      3
#ifndef M2_MAX_LEN
#define M2_MAX_LEN      8
#endif
#define M3_MIN_LEN      3
#define M3_MAX_LEN      33
#define M4_MIN_LEN      3
#define M4_MAX_LEN      9

#define M1_MARKER       0
#define M2_MARKER       64
#define M3_MARKER       32
#define M4_MARKER       16

#ifndef MIN_LOOKAHEAD
#define MIN_LOOKAHEAD       (M2_MAX_LEN + 1)
#endif

#if defined(LZO_NEED_DICT_H)

#ifndef LZO_HASH
#define LZO_HASH            LZO_HASH_LZO_INCREMENTAL_B
#endif
#define DL_MIN_LEN          M2_MIN_LEN

#ifndef __LZO_DICT_H
#define __LZO_DICT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(D_BITS) && defined(DBITS)
#  define D_BITS        DBITS
#endif
#if !defined(D_BITS)
#  error "D_BITS is not defined"
#endif
#if (D_BITS < 16)
#  define D_SIZE        LZO_SIZE(D_BITS)
#  define D_MASK        LZO_MASK(D_BITS)
#else
#  define D_SIZE        LZO_USIZE(D_BITS)
#  define D_MASK        LZO_UMASK(D_BITS)
#endif
#define D_HIGH          ((D_MASK >> 1) + 1)

#if !defined(DD_BITS)
#  define DD_BITS       0
#endif
#define DD_SIZE         LZO_SIZE(DD_BITS)
#define DD_MASK         LZO_MASK(DD_BITS)

#if !defined(DL_BITS)
#  define DL_BITS       (D_BITS - DD_BITS)
#endif
#if (DL_BITS < 16)
#  define DL_SIZE       LZO_SIZE(DL_BITS)
#  define DL_MASK       LZO_MASK(DL_BITS)
#else
#  define DL_SIZE       LZO_USIZE(DL_BITS)
#  define DL_MASK       LZO_UMASK(DL_BITS)
#endif

#if (D_BITS != DL_BITS + DD_BITS)
#  error "D_BITS does not match"
#endif
#if (D_BITS < 6 || D_BITS > 18)
#  error "invalid D_BITS"
#endif
#if (DL_BITS < 6 || DL_BITS > 20)
#  error "invalid DL_BITS"
#endif
#if (DD_BITS < 0 || DD_BITS > 6)
#  error "invalid DD_BITS"
#endif

#if !defined(DL_MIN_LEN)
#  define DL_MIN_LEN    3
#endif
#if !defined(DL_SHIFT)
#  define DL_SHIFT      ((DL_BITS + (DL_MIN_LEN - 1)) / DL_MIN_LEN)
#endif

#undef DM
#undef DX

#if (DL_MIN_LEN == 3)
#  define _DV2_A(p,shift1,shift2) \
        (((( (lzo_xint)((p)[0]) << shift1) ^ (p)[1]) << shift2) ^ (p)[2])
#  define _DV2_B(p,shift1,shift2) \
        (((( (lzo_xint)((p)[2]) << shift1) ^ (p)[1]) << shift2) ^ (p)[0])
#  define _DV3_B(p,shift1,shift2,shift3) \
        ((_DV2_B((p)+1,shift1,shift2) << (shift3)) ^ (p)[0])
#elif (DL_MIN_LEN == 2)
#  define _DV2_A(p,shift1,shift2) \
        (( (lzo_xint)(p[0]) << shift1) ^ p[1])
#  define _DV2_B(p,shift1,shift2) \
        (( (lzo_xint)(p[1]) << shift1) ^ p[2])
#else
#  error "invalid DL_MIN_LEN"
#endif
#define _DV_A(p,shift)      _DV2_A(p,shift,shift)
#define _DV_B(p,shift)      _DV2_B(p,shift,shift)
#define DA2(p,s1,s2) \
        (((((lzo_xint)((p)[2]) << (s2)) + (p)[1]) << (s1)) + (p)[0])
#define DS2(p,s1,s2) \
        (((((lzo_xint)((p)[2]) << (s2)) - (p)[1]) << (s1)) - (p)[0])
#define DX2(p,s1,s2) \
        (((((lzo_xint)((p)[2]) << (s2)) ^ (p)[1]) << (s1)) ^ (p)[0])
#define DA3(p,s1,s2,s3) ((DA2((p)+1,s2,s3) << (s1)) + (p)[0])
#define DS3(p,s1,s2,s3) ((DS2((p)+1,s2,s3) << (s1)) - (p)[0])
#define DX3(p,s1,s2,s3) ((DX2((p)+1,s2,s3) << (s1)) ^ (p)[0])
#define DMS(v,s)        ((lzo_uint) (((v) & (D_MASK >> (s))) << (s)))
#define DM(v)           DMS(v,0)

#ifndef DINDEX
#define DINDEX(dv,p)        ((lzo_uint)((_DINDEX(dv,p)) & DL_MASK) << DD_BITS)
#endif
#if !defined(DINDEX1) && defined(D_INDEX1)
#define DINDEX1             D_INDEX1
#endif
#if !defined(DINDEX2) && defined(D_INDEX2)
#define DINDEX2             D_INDEX2
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif

#endif

#define LZO_DETERMINISTIC !(LZO_DICT_USE_PTR)

#ifndef DO_COMPRESS
#define DO_COMPRESS     lzo1x_1_compress
#endif

#if 1 && defined(DO_COMPRESS) && !defined(do_compress)
#  define do_compress       LZO_PP_ECONCAT2(DO_COMPRESS,_core)
#endif

#endif

#undef do_compress
#undef DO_COMPRESS
#undef LZO_HASH

#undef LZO_TEST_OVERRUN
#undef DO_DECOMPRESS
#define DO_DECOMPRESS       lzo1x_decompress

#if !defined(MINILZO_CFG_SKIP_LZO1X_DECOMPRESS)

#if defined(LZO_TEST_OVERRUN)
#  if !defined(LZO_TEST_OVERRUN_INPUT)
#    define LZO_TEST_OVERRUN_INPUT       2
#  endif
#  if !defined(LZO_TEST_OVERRUN_OUTPUT)
#    define LZO_TEST_OVERRUN_OUTPUT      2
#  endif
#  if !defined(LZO_TEST_OVERRUN_LOOKBEHIND)
#    define LZO_TEST_OVERRUN_LOOKBEHIND  1
#  endif
#endif

#undef TEST_IP
#undef TEST_OP
#undef TEST_IP_AND_TEST_OP
#undef TEST_LB
#undef TEST_LBO
#undef NEED_IP
#undef NEED_OP
#undef TEST_IV
#undef TEST_OV
#undef HAVE_TEST_IP
#undef HAVE_TEST_OP
#undef HAVE_NEED_IP
#undef HAVE_NEED_OP
#undef HAVE_ANY_IP
#undef HAVE_ANY_OP

#if defined(LZO_TEST_OVERRUN_INPUT)
#  if (LZO_TEST_OVERRUN_INPUT >= 1)
#    define TEST_IP             (ip < ip_end)
#  endif
#  if (LZO_TEST_OVERRUN_INPUT >= 2)
#    define NEED_IP(x) \
            if ((lzo_uint)(ip_end - ip) < (lzo_uint)(x))  goto input_overrun
#    define TEST_IV(x)          if ((x) >  (lzo_uint)0 - (511)) goto input_overrun
#  endif
#endif

#if defined(LZO_TEST_OVERRUN_OUTPUT)
#  if (LZO_TEST_OVERRUN_OUTPUT >= 1)
#    define TEST_OP             (op <= op_end)
#  endif
#  if (LZO_TEST_OVERRUN_OUTPUT >= 2)
#    undef TEST_OP
#    define NEED_OP(x) \
            if ((lzo_uint)(op_end - op) < (lzo_uint)(x))  goto output_overrun
#    define TEST_OV(x)          if ((x) >  (lzo_uint)0 - (511)) goto output_overrun
#  endif
#endif

#if defined(LZO_TEST_OVERRUN_LOOKBEHIND)
#  define TEST_LB(m_pos)        if (PTR_LT(m_pos,out) || PTR_GE(m_pos,op)) goto lookbehind_overrun
#  define TEST_LBO(m_pos,o)     if (PTR_LT(m_pos,out) || PTR_GE(m_pos,op-(o))) goto lookbehind_overrun
#else
#  define TEST_LB(m_pos)        ((void) 0)
#  define TEST_LBO(m_pos,o)     ((void) 0)
#endif

#if !defined(LZO_EOF_CODE) && !defined(TEST_IP)
#  define TEST_IP               (ip < ip_end)
#endif

#if defined(TEST_IP)
#  define HAVE_TEST_IP 1
#else
#  define TEST_IP               1
#endif
#if defined(TEST_OP)
#  define HAVE_TEST_OP 1
#else
#  define TEST_OP               1
#endif

#if defined(HAVE_TEST_IP) && defined(HAVE_TEST_OP)
#  define TEST_IP_AND_TEST_OP   (TEST_IP && TEST_OP)
#elif defined(HAVE_TEST_IP)
#  define TEST_IP_AND_TEST_OP   TEST_IP
#elif defined(HAVE_TEST_OP)
#  define TEST_IP_AND_TEST_OP   TEST_OP
#else
#  define TEST_IP_AND_TEST_OP   1
#endif

#if defined(NEED_IP)
#  define HAVE_NEED_IP 1
#else
#  define NEED_IP(x)            ((void) 0)
#  define TEST_IV(x)            ((void) 0)
#endif
#if defined(NEED_OP)
#  define HAVE_NEED_OP 1
#else
#  define NEED_OP(x)            ((void) 0)
#  define TEST_OV(x)            ((void) 0)
#endif

#if defined(HAVE_TEST_IP) || defined(HAVE_NEED_IP)
#  define HAVE_ANY_IP 1
#endif
#if defined(HAVE_TEST_OP) || defined(HAVE_NEED_OP)
#  define HAVE_ANY_OP 1
#endif

#if defined(DO_DECOMPRESS)
LZO_PUBLIC(int)
DO_DECOMPRESS  ( const lzo_bytep in , lzo_uint  in_len,
                       lzo_bytep out, lzo_uintp out_len,
                       lzo_voidp wrkmem )
#endif
{
    lzo_bytep op;
    const lzo_bytep ip;
    lzo_uint t;
#if defined(COPY_DICT)
    lzo_uint m_off;
    const lzo_bytep dict_end;
#else
    const lzo_bytep m_pos;
#endif

    const lzo_bytep const ip_end = in + in_len;
#if defined(HAVE_ANY_OP)
    lzo_bytep const op_end = out + *out_len;
#endif
#if defined(LZO1Z)
    lzo_uint last_m_off = 0;
#endif

    LZO_UNUSED(wrkmem);

#if defined(COPY_DICT)
    if (dict)
    {
        if (dict_len > M4_MAX_OFFSET)
        {
            dict += dict_len - M4_MAX_OFFSET;
            dict_len = M4_MAX_OFFSET;
        }
        dict_end = dict + dict_len;
    }
    else
    {
        dict_len = 0;
        dict_end = NULL;
    }
#endif

    *out_len = 0;

    op = out;
    ip = in;

    NEED_IP(1);
    if (*ip > 17)
    {
        t = *ip++ - 17;
        if (t < 4)
            goto match_next;
        assert(t > 0); NEED_OP(t); NEED_IP(t+3);
        do *op++ = *ip++; while (--t > 0);
        goto first_literal_run;
    }

    for (;;)
    {
        NEED_IP(3);
        t = *ip++;
        if (t >= 16)
            goto match;
        if (t == 0)
        {
            while (*ip == 0)
            {
                t += 255;
                ip++;
                TEST_IV(t);
                NEED_IP(1);
            }
            t += 15 + *ip++;
        }
        assert(t > 0); NEED_OP(t+3); NEED_IP(t+6);
#if (LZO_OPT_UNALIGNED64) && (LZO_OPT_UNALIGNED32)
        t += 3;
        if (t >= 8) do
        {
            UA_COPY8(op,ip);
            op += 8; ip += 8; t -= 8;
        } while (t >= 8);
        if (t >= 4)
        {
            UA_COPY4(op,ip);
            op += 4; ip += 4; t -= 4;
        }
        if (t > 0)
        {
            *op++ = *ip++;
            if (t > 1) { *op++ = *ip++; if (t > 2) { *op++ = *ip++; } }
        }
#elif (LZO_OPT_UNALIGNED32) || (LZO_ALIGNED_OK_4)
#if !(LZO_OPT_UNALIGNED32)
        if (PTR_ALIGNED2_4(op,ip))
        {
#endif
        UA_COPY4(op,ip);
        op += 4; ip += 4;
        if (--t > 0)
        {
            if (t >= 4)
            {
                do {
                    UA_COPY4(op,ip);
                    op += 4; ip += 4; t -= 4;
                } while (t >= 4);
                if (t > 0) do *op++ = *ip++; while (--t > 0);
            }
            else
                do *op++ = *ip++; while (--t > 0);
        }
#if !(LZO_OPT_UNALIGNED32)
        }
        else
#endif
#endif
#if !(LZO_OPT_UNALIGNED32)
        {
            *op++ = *ip++; *op++ = *ip++; *op++ = *ip++;
            do *op++ = *ip++; while (--t > 0);
        }
#endif

first_literal_run:

        t = *ip++;
        if (t >= 16)
            goto match;
#if defined(COPY_DICT)
#if defined(LZO1Z)
        m_off = (1 + M2_MAX_OFFSET) + (t << 6) + (*ip++ >> 2);
        last_m_off = m_off;
#else
        m_off = (1 + M2_MAX_OFFSET) + (t >> 2) + (*ip++ << 2);
#endif
        NEED_OP(3);
        t = 3; COPY_DICT(t,m_off)
#else
#if defined(LZO1Z)
        t = (1 + M2_MAX_OFFSET) + (t << 6) + (*ip++ >> 2);
        m_pos = op - t;
        last_m_off = t;
#else
        m_pos = op - (1 + M2_MAX_OFFSET);
        m_pos -= t >> 2;
        m_pos -= *ip++ << 2;
#endif
        TEST_LB(m_pos); NEED_OP(3);
        *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos;
#endif
        goto match_done;

        for (;;) {
match:
            if (t >= 64)
            {
#if defined(COPY_DICT)
#if defined(LZO1X)
                m_off = 1 + ((t >> 2) & 7) + (*ip++ << 3);
                t = (t >> 5) - 1;
#elif defined(LZO1Y)
                m_off = 1 + ((t >> 2) & 3) + (*ip++ << 2);
                t = (t >> 4) - 3;
#elif defined(LZO1Z)
                m_off = t & 0x1f;
                if (m_off >= 0x1c)
                    m_off = last_m_off;
                else
                {
                    m_off = 1 + (m_off << 6) + (*ip++ >> 2);
                    last_m_off = m_off;
                }
                t = (t >> 5) - 1;
#endif
#else
#if defined(LZO1X)
                m_pos = op - 1;
                m_pos -= (t >> 2) & 7;
                m_pos -= *ip++ << 3;
                t = (t >> 5) - 1;
#elif defined(LZO1Y)
                m_pos = op - 1;
                m_pos -= (t >> 2) & 3;
                m_pos -= *ip++ << 2;
                t = (t >> 4) - 3;
#elif defined(LZO1Z)
                {
                    lzo_uint off = t & 0x1f;
                    m_pos = op;
                    if (off >= 0x1c)
                    {
                        assert(last_m_off > 0);
                        m_pos -= last_m_off;
                    }
                    else
                    {
                        off = 1 + (off << 6) + (*ip++ >> 2);
                        m_pos -= off;
                        last_m_off = off;
                    }
                }
                t = (t >> 5) - 1;
#endif
                TEST_LB(m_pos); assert(t > 0); NEED_OP(t+3-1);
                goto copy_match;
#endif
            }
            else if (t >= 32)
            {
                t &= 31;
                if (t == 0)
                {
                    while (*ip == 0)
                    {
                        t += 255;
                        ip++;
                        TEST_OV(t);
                        NEED_IP(1);
                    }
                    t += 31 + *ip++;
                    NEED_IP(2);
                }
#if defined(COPY_DICT)
#if defined(LZO1Z)
                m_off = 1 + (ip[0] << 6) + (ip[1] >> 2);
                last_m_off = m_off;
#else
                m_off = 1 + (ip[0] >> 2) + (ip[1] << 6);
#endif
#else
#if defined(LZO1Z)
                {
                    lzo_uint off = 1 + (ip[0] << 6) + (ip[1] >> 2);
                    m_pos = op - off;
                    last_m_off = off;
                }
#elif (LZO_OPT_UNALIGNED16) && (LZO_ABI_LITTLE_ENDIAN)
                m_pos = op - 1;
                m_pos -= UA_GET_LE16(ip) >> 2;
#else
                m_pos = op - 1;
                m_pos -= (ip[0] >> 2) + (ip[1] << 6);
#endif
#endif
                ip += 2;
            }
            else if (t >= 16)
            {
#if defined(COPY_DICT)
                m_off = (t & 8) << 11;
#else
                m_pos = op;
                m_pos -= (t & 8) << 11;
#endif
                t &= 7;
                if (t == 0)
                {
                    while (*ip == 0)
                    {
                        t += 255;
                        ip++;
                        TEST_OV(t);
                        NEED_IP(1);
                    }
                    t += 7 + *ip++;
                    NEED_IP(2);
                }
#if defined(COPY_DICT)
#if defined(LZO1Z)
                m_off += (ip[0] << 6) + (ip[1] >> 2);
#else
                m_off += (ip[0] >> 2) + (ip[1] << 6);
#endif
                ip += 2;
                if (m_off == 0)
                    goto eof_found;
                m_off += 0x4000;
#if defined(LZO1Z)
                last_m_off = m_off;
#endif
#else
#if defined(LZO1Z)
                m_pos -= (ip[0] << 6) + (ip[1] >> 2);
#elif (LZO_OPT_UNALIGNED16) && (LZO_ABI_LITTLE_ENDIAN)
                m_pos -= UA_GET_LE16(ip) >> 2;
#else
                m_pos -= (ip[0] >> 2) + (ip[1] << 6);
#endif
                ip += 2;
                if (m_pos == op)
                    goto eof_found;
                m_pos -= 0x4000;
#if defined(LZO1Z)
                last_m_off = pd((const lzo_bytep)op, m_pos);
#endif
#endif
            }
            else
            {
#if defined(COPY_DICT)
#if defined(LZO1Z)
                m_off = 1 + (t << 6) + (*ip++ >> 2);
                last_m_off = m_off;
#else
                m_off = 1 + (t >> 2) + (*ip++ << 2);
#endif
                NEED_OP(2);
                t = 2; COPY_DICT(t,m_off)
#else
#if defined(LZO1Z)
                t = 1 + (t << 6) + (*ip++ >> 2);
                m_pos = op - t;
                last_m_off = t;
#else
                m_pos = op - 1;
                m_pos -= t >> 2;
                m_pos -= *ip++ << 2;
#endif
                TEST_LB(m_pos); NEED_OP(2);
                *op++ = *m_pos++; *op++ = *m_pos;
#endif
                goto match_done;
            }

#if defined(COPY_DICT)

            NEED_OP(t+3-1);
            t += 3-1; COPY_DICT(t,m_off)

#else

            TEST_LB(m_pos); assert(t > 0); NEED_OP(t+3-1);
#if (LZO_OPT_UNALIGNED64) && (LZO_OPT_UNALIGNED32)
            if (op - m_pos >= 8)
            {
                t += (3 - 1);
                if (t >= 8) do
                {
                    UA_COPY8(op,m_pos);
                    op += 8; m_pos += 8; t -= 8;
                } while (t >= 8);
                if (t >= 4)
                {
                    UA_COPY4(op,m_pos);
                    op += 4; m_pos += 4; t -= 4;
                }
                if (t > 0)
                {
                    *op++ = m_pos[0];
                    if (t > 1) { *op++ = m_pos[1]; if (t > 2) { *op++ = m_pos[2]; } }
                }
            }
            else
#elif (LZO_OPT_UNALIGNED32) || (LZO_ALIGNED_OK_4)
#if !(LZO_OPT_UNALIGNED32)
            if (t >= 2 * 4 - (3 - 1) && PTR_ALIGNED2_4(op,m_pos))
            {
                assert((op - m_pos) >= 4);
#else
            if (t >= 2 * 4 - (3 - 1) && (op - m_pos) >= 4)
            {
#endif
                UA_COPY4(op,m_pos);
                op += 4; m_pos += 4; t -= 4 - (3 - 1);
                do {
                    UA_COPY4(op,m_pos);
                    op += 4; m_pos += 4; t -= 4;
                } while (t >= 4);
                if (t > 0) do *op++ = *m_pos++; while (--t > 0);
            }
            else
#endif
            {
copy_match:
                *op++ = *m_pos++; *op++ = *m_pos++;
                do *op++ = *m_pos++; while (--t > 0);
            }

#endif

match_done:
#if defined(LZO1Z)
            t = ip[-1] & 3;
#else
            t = ip[-2] & 3;
#endif
            if (t == 0)
                break;

match_next:
            assert(t > 0); assert(t < 4); NEED_OP(t); NEED_IP(t+3);
#if 0
            do *op++ = *ip++; while (--t > 0);
#else
            *op++ = *ip++;
            if (t > 1) { *op++ = *ip++; if (t > 2) { *op++ = *ip++; } }
#endif
            t = *ip++;
        }
    }

eof_found:
    *out_len = pd(op, out);
    return (ip == ip_end ? LZO_E_OK :
           (ip < ip_end  ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));

#if defined(HAVE_NEED_IP)
input_overrun:
    *out_len = pd(op, out);
    return LZO_E_INPUT_OVERRUN;
#endif

#if defined(HAVE_NEED_OP)
output_overrun:
    *out_len = pd(op, out);
    return LZO_E_OUTPUT_OVERRUN;
#endif

#if defined(LZO_TEST_OVERRUN_LOOKBEHIND)
lookbehind_overrun:
    *out_len = pd(op, out);
    return LZO_E_LOOKBEHIND_OVERRUN;
#endif
}

#endif

/***** End of minilzo.c *****/
