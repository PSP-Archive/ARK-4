/*
 * libccc.h
 * Character Code Conversion Library
 * Version 0.31 by BenHur - http://www.psp-programming.com/benhur
 *
 * This work is licensed under the Creative Commons Attribution-Share Alike 3.0 License.
 * See LICENSE for more details.
 *
 */

#ifndef __LIBCCC_H__
#define __LIBCCC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

/** @defgroup character code conversion library
 *  @{
 */

typedef unsigned short cccUCS2; 
typedef unsigned char  cccCode;

/* supported codepages */
#define CCC_CP000  0x00 //ASCII
#define CCC_CP437  0x01 //US
#define CCC_CP850  0x05 //Multilingual Latin I
#define CCC_CP866  0x0b //Russian
#define CCC_CP932  0x0d //S-JIS
#define CCC_CP936  0x0e //GBK
#define CCC_CP949  0x0f //Korean
#define CCC_CP950  0x10 //Big5
#define CCC_CP1251 0x12 //Cyrillic
#define CCC_CP1252 0x13 //Latin II
#define CCC_CPUTF8 0xff //UTF-8
#define CCC_N_CP   0x14 //number of codepages (for array definition)

/* error codes */
#define CCC_SUCCESS             0x00000000
#define CCC_ERROR_BUFFER_SIZE   0x80000104
#define CCC_ERROR_INPUT_STREAM  0x80000108
#define CCC_ERROR_MEM_ALLOC     0x800200D9
#define CCC_ERROR_FILE_READ     0x80020130
#define CCC_ERROR_UNSUPPORTED   0x80020325


/**
 * Character counting 
 *
 * @param str - zero terminated string
 *
 * @param cp - codepage 
 *
 * @returns number of characters in the string
 */
int cccStrlen(cccCode const * str);                 //for single byte character sets
int cccStrlenSJIS(cccCode const * str);
int cccStrlenGBK(cccCode const * str);
int cccStrlenKOR(cccCode const * str);
int cccStrlenBIG5(cccCode const * str);
int cccStrlenUTF8(cccCode const * str);
int cccStrlenCode(cccCode const * str, unsigned char cp);
int cccStrlenUCS2(cccUCS2 const * str); 

/**
 * Character code conversion 
 *
 * @param dst - output string 
 *
 * @param count - size of output buffer
 *
 * @param str - input string
 *
 * @param cp - codepage 
 *
 * @returns number of converted character codes
 */
int cccSJIStoUCS2(cccUCS2 * dst, size_t count, cccCode const * str);
int cccGBKtoUCS2 (cccUCS2 * dst, size_t count, cccCode const * str);
int cccKORtoUCS2 (cccUCS2 * dst, size_t count, cccCode const * str);
int cccBIG5toUCS2(cccUCS2 * dst, size_t count, cccCode const * str);
int cccUTF8toUCS2(cccUCS2 * dst, size_t count, cccCode const * str);
int cccCodetoUCS2(cccUCS2 * dst, size_t count, cccCode const * str, unsigned char cp); 

/**
 * Set error character (character that's used for code points where conversion failed)
 *
 * @param code - new error character (default: 0)
 *
 * @returns previous error character
 */
cccUCS2 cccSetErrorCharUCS2(cccUCS2 code);

/**
 * Shutdown the Character Code Conversion Library
 */
void cccShutDown(void);

/** @} */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LIBCCC_H__
