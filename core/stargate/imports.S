	.set noreorder

#include "pspimport.s"

	IMPORT_START "SystemCtrlPrivate",0x00090000
	IMPORT_FUNC  "SystemCtrlPrivate",0xBC7554DF,strcasecmp
	IMPORT_FUNC  "SystemCtrlPrivate",0xAC7554DF,strncasecmp
	IMPORT_FUNC  "SystemCtrlPrivate",0x34D6168F,getGameId
	IMPORT_FUNC  "SystemCtrlPrivate",0x272984FD,isSystemBooted