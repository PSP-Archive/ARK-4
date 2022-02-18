	.set noreorder

#include "pspimport.s"

	IMPORT_START "scePaf",0x00090011
	IMPORT_FUNC  "scePaf",0xD9E2D6E1,memset
	IMPORT_FUNC  "scePaf",0xB4652CFE,memcpy
	IMPORT_FUNC  "scePaf",0x4CF09BA2,strcmp