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