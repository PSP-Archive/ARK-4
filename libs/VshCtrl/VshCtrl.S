	.set noreorder

#include "pspimport.s"

// Build files
// VshCtrl_XXXX.o

#ifdef F_VshCtrl_0000
	IMPORT_START "VshCtrlLib",0x40090000
#endif
#ifdef F_VshCtrl_0001
	IMPORT_FUNC  "VshCtrlLib",0xFD26DA72,vctrlVSHRegisterVshMenu
#endif
#ifdef F_VshCtrl_0002
	IMPORT_FUNC  "VshCtrlLib",0xCD6B3913,vctrlVSHExitVSHMenu
#endif
#ifdef F_VshCtrl_0003
	IMPORT_FUNC  "VshCtrlLib",0xC1BE3884,vctrlVSHUpdateConfig
#endif
#ifdef F_VshCtrl_0004
	IMPORT_FUNC  "VshCtrlLib",0xC1FED158,vshDetectDiscType
#endif
#ifdef F_VshCtrl_0005
	IMPORT_FUNC  "VshCtrlLib",0x12DC7CF4,fakeParamInexistance
#endif
#ifdef F_VshCtrl_0006
	IMPORT_FUNC  "VshCtrlLib",0x36197FB3,vshCtrlDeleteHibernation
#endif
#ifdef F_VshCtrl_0007
	IMPORT_FUNC  "VshCtrlLib",0xF1E6A46D,isoOpen
#endif
#ifdef F_VshCtrl_0008
	IMPORT_FUNC  "VshCtrlLib",0xC4B3078E,isoRead
#endif
#ifdef F_VshCtrl_0009
	IMPORT_FUNC  "VshCtrlLib",0x4DA81CDD,isoClose
#endif
#ifdef F_VshCtrl_0010
	IMPORT_FUNC  "VshCtrlLib",0xE2498CEC,isoGetFileInfo
#endif
#ifdef F_VshCtrl_0011
	IMPORT_FUNC  "VshCtrlLib",0x4BDA734F,isoGetTotalSectorSize
#endif
#ifdef F_VshCtrl_0012
	IMPORT_FUNC  "VshCtrlLib",0x29154CE0,has_prometheus_module
#endif
#ifdef F_VshCtrl_0013
	IMPORT_FUNC  "VshCtrlLib",0x39F5F78E,has_update_file
#endif
