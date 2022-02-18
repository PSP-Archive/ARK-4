#ifndef COMMON_H
#define COMMON_H	

#include <pspkernel.h>
#include <psploadcore.h>
#include <pspaudio.h>

#define MAKE_JUMP(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a);
#define MAKE_CALL(a, f) _sw(0x0C000000 | (((u32)(f) >> 2) & 0x03FFFFFF), a);

//by Davee
#define HIJACK_FUNCTION(a, f, ptr) \
{ \
	u32 func = a; \
	static u32 patch_buffer[3]; \
	_sw(_lw(func), (u32)patch_buffer); \
	_sw(_lw(func + 4), (u32)patch_buffer + 8);\
	MAKE_JUMP((u32)patch_buffer + 4, func + 8); \
	_sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), func); \
	_sw(0, func + 4); \
	ptr = (void *)patch_buffer; \
}

typedef struct
{
	/* PEOPS SPU configuration */
	int enablepeopsspu;
	int volume;
	int reverb;
	int interpolation;
	int enablexaplaying;
	int changexapitch;
	int spuirqwait;
	int spuupdatemode;
	int sputhreadpriority;
} PeopsConfig;

#endif
