	.set noreorder

#include "pspimport.s"

	IMPORT_START "SystemCtrlPrivate",0x00090000
	IMPORT_FUNC  "SystemCtrlPrivate",0xBC7554DF,strcasecmp
	IMPORT_FUNC  "SystemCtrlPrivate",0xAC7554DF,strncasecmp
	IMPORT_FUNC  "SystemCtrlPrivate",0x983B00FB,lowerString
	IMPORT_FUNC  "SystemCtrlPrivate",0x272984FD,isSystemBooted
	IMPORT_FUNC  "SystemCtrlPrivate",0x5D7817CF,msstorCacheInit