	.set noreorder

#include "pspimport.s"

// Build files
// libpspav_XXXX.o

#ifdef F_libpspav_0000
	IMPORT_START "pspav",0x40090000
#endif
#ifdef F_libpspav_0001
	IMPORT_FUNC  "pspav",0xEC38D2C9,pspavSetAt3Data
#endif
#ifdef F_libpspav_0002
	IMPORT_FUNC  "pspav",0xA405590A,pspavResetAt3Data
#endif
#ifdef F_libpspav_0003
	IMPORT_FUNC  "pspav",0xC52E1F12,pspavPlayAT3
#endif
#ifdef F_libpspav_0004
	IMPORT_FUNC  "pspav",0xAD5E3793,pspavPlayGamePMF
#endif
#ifdef F_libpspav_0005
	IMPORT_FUNC  "pspav",0x07EC66E6,pspavPlayVideoFile
#endif

