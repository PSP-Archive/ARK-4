#include <pspsdk.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspkernel.h>
#include <pspge.h>
#include <pspdebug.h>
#include <pspaudio.h>
#include <psputility.h>
#include <pspumd.h>
#include <psptypes.h>
#include <pspimpose_driver.h>
#include <psputility.h>
#include <pspiofilemgr.h>

#ifndef VAL_SDK_H
#define VAL_SDK_H

// Typedefs
typedef unsigned char byte;

//
// Code
//
// MIPS opcodes
#define JR_RA_OPCODE 0x03E00008
#define NOP_OPCODE 0x00000000

// To distinguish between SYSCALL and JUMP stubs
#define SYSCALL_MASK_IMPORT 0x01000000
#define SYSCALL_MASK_RESOLVE 0xFFF00000

// This marks libraries that are not yet linked
#define SYSCALL_IMPORT_NOT_RESOLVED_YET 0x15

// Macros to deal with $gp register
#define GET_GP(gp) asm volatile ("move %0, $gp\n" : "=r" (gp))
#define SET_GP(gp) asm volatile ("move $gp, %0\n" :: "r" (gp))
#define BREAK asm volatile ("break\n")

// Macro to get the syscall number
#define GET_SYSCALL_NUMBER(sys) ((u32)(sys) >> 6)
// Macro to form syscal instruction
#define MAKE_SYSCALL(n) (0x03ffffff & (((u32)(n) << 6) | 0x0000000c))

//
// Memory
//
#define GAME_MEMORY_START 0x08800000
#define GAME_MEMORY_SIZE 0x1800000

// Max chars on module name
#define MAX_MODULE_NAME_LENGTH 30

// Max chars on library name
#define MAX_LIBRARY_NAME_LENGTH 30

/*
#ifndef ULONG
#define ULONG unsigned long
#endif
*/

#define EXIT sceKernelExitGame()
#define CLEAR_CACHE sceKernelDcacheWritebackInvalidateAll()
#define WAIT_SEMA sceKernelWaitSema

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

//Should be defined somewhere in the sdk ???
//SceUID sceKernelGetModuleIdByAddress(u32 address);

#endif

