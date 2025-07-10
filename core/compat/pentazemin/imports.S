	.set noreorder

#include "pspimport.s"

	IMPORT_START "SystemCtrlPrivate",0x00090000
	IMPORT_FUNC  "SystemCtrlPrivate",0x5D7817CF,msstorCacheInit
	IMPORT_FUNC  "SystemCtrlPrivate",0x272984FD,isSystemBooted

	IMPORT_START "sceUsbCam",0x40090000
	IMPORT_FUNC  "sceUsbCam",0x3F0CF289,sceUsbCamSetupStill
	IMPORT_FUNC  "sceUsbCam",0x17F7B2FB,sceUsbCamSetupVideo
	IMPORT_FUNC  "sceUsbCam",0x03ED7A82,sceUsbCamSetupMic
	IMPORT_FUNC  "sceUsbCam",0x3DC0088E,sceUsbCamReadMic

	IMPORT_START "sceUsb_driver",0x00090011
	IMPORT_FUNC  "sceUsb_driver",0x63E55EBE,sceUsb_driver_0x63E55EBE
	IMPORT_FUNC  "sceUsb_driver",0xED8C8695,sceUsb_driver_0xED8C8695
