	.set noreorder

#include "pspimport.s"

// Build files
// SystemCtrlForUser_XXXX.o

#ifdef F_SystemCtrlForUser_0000
	IMPORT_START "SystemCtrlForUser",0x40090000
#endif
#ifdef F_SystemCtrlForUser_0001
	IMPORT_FUNC  "SystemCtrlForUser",0x2D10FB28,sctrlKernelLoadExecVSHWithApitype
#endif
#ifdef F_SystemCtrlForUser_0002
	IMPORT_FUNC  "SystemCtrlForUser",0xEB74FE45,sctrlKernelSetUserLevel
#endif
#ifdef F_SystemCtrlForUser_0003
	IMPORT_FUNC  "SystemCtrlForUser",0xD8FF9B99,sctrlKernelSetDevkitVersion
#endif
#ifdef F_SystemCtrlForUser_0004
	IMPORT_FUNC  "SystemCtrlForUser",0x1090A2E1,sctrlHENGetVersion
#endif
#ifdef F_SystemCtrlForUser_0005
	IMPORT_FUNC  "SystemCtrlForUser",0x5328B431,sctrlHENGetMinorVersion
#endif
#ifdef F_SystemCtrlForUser_0006
	IMPORT_FUNC  "SystemCtrlForUser",0x159AF5CC,sctrlHENFindFunction
#endif
#ifdef F_SystemCtrlForUser_0007
	IMPORT_FUNC  "SystemCtrlForUser",0x5A35C948,sctrlSESetUmdFile
#endif
#ifdef F_SystemCtrlForUser_0008
	IMPORT_FUNC  "SystemCtrlForUser",0x8D5BE1F0,sctrlKernelSetInitApitype
#endif
#ifdef F_SystemCtrlForUser_0009
	IMPORT_FUNC  "SystemCtrlForUser",0x128112C3,sctrlKernelSetInitFileName
#endif
#ifdef F_SystemCtrlForUser_0010
	IMPORT_FUNC  "SystemCtrlForUser",0x62CAC4CF,sctrlPatchModule
#endif
#ifdef F_SystemCtrlForUser_0011
	IMPORT_FUNC  "SystemCtrlForUser",0x2A868045,sctrlModuleTextAddr
#endif
#ifdef F_SystemCtrlForUser_0012
	IMPORT_FUNC  "SystemCtrlForUser",0x603EE1D0,sctrlKernelSetNidResolver
#endif
#ifdef F_SystemCtrlForUser_0013
	IMPORT_FUNC  "SystemCtrlForUser",0xB364FBB4,sctrlKernelRand
#endif
#ifdef F_SystemCtrlForUser_0014
	IMPORT_FUNC  "SystemCtrlForUser",0x324DF3DD,flushCache
#endif
#ifdef F_SystemCtrlForUser_0015
	IMPORT_FUNC  "SystemCtrlForUser",0xBC939DC1,sctrlSESetBootConfFileIndex
#endif
#ifdef F_SystemCtrlForUser_0016
	IMPORT_FUNC  "SystemCtrlForUser",0x70B92B45,sctrlSEGetBootConfFileIndex
#endif
#ifdef F_SystemCtrlForUser_0017
	IMPORT_FUNC  "SystemCtrlForUser",0x5CB025F0,sctrlSESetBootConfFileIndex
#endif
#ifdef F_SystemCtrlForUser_0018
	IMPORT_FUNC  "SystemCtrlForUser",0x31C6160D,sctrlSESetDiscType
#endif
#ifdef F_SystemCtrlForUser_0019
	IMPORT_FUNC  "SystemCtrlForUser",0xABEF849B,sctrlSEGetDiscType
#endif
#ifdef F_SystemCtrlForUser_0020
	IMPORT_FUNC  "SystemCtrlForUser",0xCC9ADCF8,sctrlHENSetSpeed
#endif
#ifdef F_SystemCtrlForUser_0021
	IMPORT_FUNC  "SystemCtrlForUser",0x23833651,sctrlGetThreadUIDByName
#endif
#ifdef F_SystemCtrlForUser_0022
	IMPORT_FUNC  "SystemCtrlForUser",0xF93BEC5A,sctrlGetThreadContextByUID
#endif
#ifdef F_SystemCtrlForUser_0023
	IMPORT_FUNC  "SystemCtrlForUser",0x72D520D4,sctrlGetThreadContextByName
#endif
#ifdef F_SystemCtrlForUser_0024
	IMPORT_FUNC "SystemCtrlForUser",0xF462EE55,sctrlDeflateDecompress
#endif
#ifdef F_SystemCtrlForUser_0025
	IMPORT_FUNC  "SystemCtrlForUser",0x2BA770CD,sctrlKernelSetUMDEmuFile
#endif
#ifdef F_SystemCtrlForUser_0026
	IMPORT_FUNC "SystemCtrlForUser",0x1C90BECB,sctrlHENSetStartModuleHandler
#endif
#ifdef F_SystemCtrlForUser_0027
	IMPORT_FUNC  "SystemCtrlForUser",0x3DF7F7D8,printk
#endif
#ifdef F_SystemCtrlForUser_0028
	IMPORT_FUNC  "SystemCtrlForUser",0xCB739F42,printkCached
#endif
#ifdef F_SystemCtrlForUser_0029
	IMPORT_FUNC  "SystemCtrlForUser",0x5C94CB48,printkSync
#endif
#ifdef F_SystemCtrlForUser_0030
	IMPORT_FUNC  "SystemCtrlForUser",0xFCE44FB8,sctrlGetInitPARAM
#endif
#ifdef F_SystemCtrlForUser_0031
	IMPORT_FUNC  "SystemCtrlForUser",0x6EAFC02D,sctrlHENGetArkConfig
#endif
#ifdef F_SystemCtrlForUser_0032
	IMPORT_FUNC  "SystemCtrlForUser",0x2794CCF4,sctrlKernelExitVSH
#endif
#ifdef F_SystemCtrlForUser_0033
	IMPORT_FUNC  "SystemCtrlForUser",0xAD4D5EA5,sctrlSESetConfigEx
#endif
#ifdef F_SystemCtrlForUser_0034
	IMPORT_FUNC  "SystemCtrlForUser",0x8E426F09,sctrlSEGetConfigEx
#endif
#ifdef F_SystemCtrlForUser_0035
	IMPORT_FUNC  "SystemCtrlForUser",0x1DDDAD0C,sctrlSESetConfig
#endif
#ifdef F_SystemCtrlForUser_0036
	IMPORT_FUNC  "SystemCtrlForUser",0x16C3B7EE,sctrlSEGetConfig
#endif
#ifdef F_SystemCtrlForUser_0037
	IMPORT_FUNC  "SystemCtrlForUser",0x16100529,LZ4_decompress_fast
#endif
#ifdef F_SystemCtrlForUser_0038
    IMPORT_FUNC  "SystemCtrlForUser",0xAA6274F8,sctrlGetThreads
#endif
#ifdef F_SystemCtrlForUser_0039
    IMPORT_FUNC "SystemCtrlForUser",0x373C8373,sctrlSuspendThreads
#endif
#ifdef F_SystemCtrlForUser_0040
    IMPORT_FUNC "SystemCtrlForUser",0xE2A48BE5,sctrlResumeThreads
#endif
#ifdef F_SystemCtrlForUser_0041
    IMPORT_FUNC "SystemCtrlForUser",0x287C1E4F,sctrlPatchExitGame
#endif
#ifdef F_SystemCtrlForUser_0042
    IMPORT_FUNC "SystemCtrlForUser",0x79553CCE,sctrlExitGameMenu
#endif
#ifdef F_SystemCtrlForUser_0043
    IMPORT_FUNC "SystemCtrlForUser",0x99C088A5,sctrlExitToRecovery
#endif
#ifdef F_SystemCtrlForUser_0044
    IMPORT_FUNC "SystemCtrlForUser",0x162DC9FC,sctrlExitToLauncher
#endif
#ifdef F_SystemCtrlForUser_0045
    IMPORT_FUNC "SystemCtrlForUser",0xD339E2E9,sctrlHENIsSE
#endif
#ifdef F_SystemCtrlForUser_0046
    IMPORT_FUNC "SystemCtrlForUser",0x2E2935EF,sctrlHENIsDevhook
#endif
#ifdef F_SystemCtrlForUser_0047
	IMPORT_FUNC  "SystemCtrlForUser",0xCE0A654E,sctrlHENLoadModuleOnReboot
#endif
#ifdef F_SystemCtrlForUser_0048
	IMPORT_FUNC  "SystemCtrlForUser",0x5D665044,sctrlGzipDecompress
#endif
#ifdef F_SystemCtrlForUser_0049
	IMPORT_FUNC  "SystemCtrlForUser",0x8B591EB8,lzo1x_decompress
#endif
