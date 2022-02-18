#include <pspmoduleexport.h>
#define NULL ((void *) 0)

extern void module_start;
extern void module_info;
static const unsigned int __syslib_exports[4] __attribute__((section(".rodata.sceResident"))) = {
	0xD632ACDB,
	0xF01D73A7,
	(unsigned int) &module_start,
	(unsigned int) &module_info,
};

extern void decompressData;
extern void _sceMeAudio_67CD7972;
extern void _sceMeAudio_DE630CD2;
extern void spuReadCallback;
extern void spuWriteCallback;
static const unsigned int __PopcornPrivate_exports[10] __attribute__((section(".rodata.sceResident"))) = {
	0x6F2B3898,
	0xB4584887,
	0x4B163044,
	0x75D91004,
	0x61088FB6,
	(unsigned int) &decompressData,
	(unsigned int) &_sceMeAudio_67CD7972,
	(unsigned int) &_sceMeAudio_DE630CD2,
	(unsigned int) &spuReadCallback,
	(unsigned int) &spuWriteCallback,
};

const struct _PspLibraryEntry __library_exports[2] __attribute__((section(".lib.ent"), used)) = {
	{ NULL, 0x0000, 0x8000, 4, 1, 1, &__syslib_exports },
	{ "PopcornPrivate", 0x0011, 0x4001, 4, 0, 5, &__PopcornPrivate_exports },
};
