/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspsdk.h>
#include <macros.h>
#include "nidresolver.h"

static NidResolverEntry SysMemForKernel_nid[] = {
	{ 0xC7E57B9C, 0xFC639A2B, }, // sceKernelSetQTGP3
	{ 0xCF4DE78C, 0xC90B0992, }, // sceKernelGetUIDcontrolBlock
	{ 0xFC114573, 0xB4F00CB5, }, // sceKernelGetCompiledSdkVersion
	{ 0x3FC9AE6A, 0xC886B169, }, // sceKernelDevkitVersion
	{ 0x536AD5E1, 0xAC12F678, }, // SysMemForKernel_536AD5E1
	{ 0x636C953B, 0x23D81675, }, // sceKernelAllocHeapMemory
	{ 0xC9805775, 0xDD6512D0, }, // sceKernelDeleteHeap
	{ 0x7B749390, 0x87C2AB85, }, // sceKernelFreeHeapMemory
	{ 0x1C1FBFE7, 0x58148F07, }, // sceKernelCreateHeap
	{ 0x55A40B2C, 0xC4EEAF20, }, // sceKernelQueryMemoryPartitionInfo
	{ 0xE6581468, 0x13F4A0DE, }, // sceKernelPartitionMaxFreeMemSize
	{ 0x9697CD32, 0x0115B0F8, }, // sceKernelPartitionTotalFreeMemSize
	{ 0x237DBD4F, 0x7158CE7E, }, // sceKernelAllocPartitionMemory
	{ 0xB6D61D02, 0xC1A26C6F, }, // sceKernelFreePartitionMemory
	{ 0x9D9A5BA1, 0xF12A62F7, }, // sceKernelGetBlockHeadAddr
	{ 0xB2C7AA36, 0x83B5226D, }, // sceKernelSetDdrMemoryProtection
	{ 0x6373995D, 0x07C586A1, }, // sceKernelGetModel
	{ 0xEB7A74DB, 0xF2284ECC, }, // sceKernelAllocHeapMemoryWithOption
	{ 0xA823047E, 0x01810023, }, // sceKernelHeapTotalFreeSize
	{ 0xA2A65F0E, 0xEA1CABF1, }, // sceKernelFillFreeBlock
	{ 0xEE867074, 0x8E742787, }, // sceKernelSizeLockMemoryBlock
	{ 0xCE5544F4, 0xFB5BEB66, }, // sceKernelResizeMemoryBlock
	{ 0x915EF4AC, 0xF7E78B33, }, // SysMemForKernel_915EF4AC
	{ 0x5EBE73DE, 0x1AB50974, }, // sceKernelJointMemoryBlock
	{ 0x2A3E5280, 0xFAF29F34, }, // sceKernelQueryMemoryInfo
	{ 0x82CCA14F, 0xE860BE8F, }, // sceKernelQueryMemoryBlockInfo
	{ 0x7B3E7441, 0x1E6BB8E8, }, // sceKernelMemoryExtendSize
	{ 0x6C1DCD41, 0x757EAC7D, }, // sceKernelCallUIDFunction
	{ 0x5367923C, 0x7D50CE82, }, // sceKernelCallUIDObjFunction
	{ 0xCE05CCB7, 0x235C2646, }, // SysMemForKernel_CE05CCB7
	{ 0x6CD838EC, 0x21487E5B, }, // sceKernelLookupUIDFunction
	{ 0xAD09C397, 0xD222DAA7, }, // sceKernelCreateUIDtypeInherit
	{ 0xFEFC8666, 0x034129FB, }, // sceKernelCreateUIDtype
	{ 0xD1BAB054, 0x1ECAB5C4, }, // sceKernelDeleteUIDtype
	{ 0x1C221A08, 0x3EC451EB, }, // sceKernelGetUIDname
	{ 0x2E3402CC, 0xA7622297, }, // sceKernelRenameUID
	{ 0x39357F07, 0x9F205D3E, }, // sceKernelGetUIDtype
	{ 0x89A74008, 0x0A34C078, }, // sceKernelCreateUID
	{ 0x8F20C4C0, 0x361F0F88, }, // sceKernelDeleteUID
	{ 0x55BFD686, 0xE3F9C38E, }, // sceKernelSearchUIDbyName
	{ 0x41FFC7F9, 0x44BDF332, }, // sceKernelGetUIDcontrolBlockWithType
	{ 0x82D3CEE3, 0xFFC63884, }, // SysMemForKernel_82D3CEE3
	{ 0xFC207849, 0x3E384043, }, // SysMemForKernel_FC207849
	{ 0xDFAEBD5B, 0xE13DDC87, }, // sceKernelIsHold
	{ 0x7BE95FA0, 0x7DDA0E01, }, // sceKernelHoldUID
	{ 0xFE8DEBE0, 0xB0CA8A4E, }, // sceKernelReleaseUID
	{ 0xA089ECA4, 0x8AE776AF, }, // sceKernelMemset
	{ 0x2F808748, 0x22A114DC, }, // sceKernelMemset32
	{ 0x1C4B1713, 0x9BAC123D, }, // sceKernelMemmove
	{ 0x1890BE9C, 0xFDBDEB5C, }, // sceKernelSysMemMemSize
	{ 0x03072750, 0x00154B6D, }, // sceKernelSysMemMaxFreeMemSize
	{ 0x811BED79, 0x45332D44, }, // sceKernelSysMemTotalFreeMemSize
	{ 0xF6C10E27, 0x57FA5AF4, }, // sceKernelGetSysMemoryInfo
	{ 0xCDA3A2F7, 0xFDC97D28, }, // SysMemForKernel_CDA3A2F7
	{ 0x960B888C, 0xE0EB8CBE, }, // SysMemForKernel_960B888C
	{ 0x452E3696, 0x36C503A9, }, // sceKernelGetSystemStatus
	{ 0x95F5E8DA, 0x521AC5A4, }, // sceKernelSetSystemStatus
	{ 0x0D547E7F, 0xEF29061C, }, // sceKernelGetUMDData
	{ 0xAB5E85E5, 0xEF29061C, }, // sceKernelGetUMDData (620 to 639)
	{ 0x419DB8F4, 0x310802A9, }, // sceKernelRegisterGetIdFunc
	{ 0x002BA296, 0x79BD1975, }, // sceKernelQueryHeapInfo
	{ 0x03808C51, 0x476B244F, }, // SysMemForKernel_03808C51
	{ 0xBB90D841, 0x53D50AC2, }, // SysMemForKernel_BB90D841
	{ 0x0D1D8AC8, 0x9B20ACEF, }, // sceKernelMemmoveWithFill
	{ 0x97B18FA8, 0xB9B7281A, }, // sceKernelCopyGameInfo
	{ 0xE40D3A1D, 0xF3BDB718, }, // sceKernelSetUmdData
	{ 0xCE8D3DB3, 0x474CA24F, }, // sceKernelGetQTGP2
	{ 0x55E4719C, 0x9E5B1ACB, }, // sceKernelSetQTGP2
	{ 0x6D8E0CDF, 0x43E0A861, }, // sceKernelGetQTGP3
	{ 0xA262FEF0, 0x4972F9D1, }, // sceKernelGetAllowReplaceUmd
	{ 0xF91FE6AA, 0x807179E7, }, // sceKernelSetParamSfo
	{ 0xDEC0A0D8, 0x6E4F9F62, }, // sceKernelGetCompilerVersion
	{ 0x7ECBDBD9, 0xBFD53FB7, }, // sceKernelGetDNAS
	{ 0x9C304ED7, 0x982A4779, }, // sceKernelSetDNAS
	{ 0xF4390489, 0xE73FBC0B, }, // sceKernelRebootKernel
	{ 0xA1ACEA31, 0xD0C1460D, }, // sceKernelGetId
	{ 0xE0058030, 0x7A7CD7BC, }, // sceKernelMemoryShrinkSize
	{ 0x719792F1, 0x35170484, }, // sceKernelMemoryOpenSize
	{ 0x101C3E59, 0xE5FB2EE5, }, // sceKernelMemoryCloseSize
	{ 0xE0645895, 0xF5228C40, }, // sceKernelGetSysmemIdList
	{ 0xCBB05241, 0xF19BA38D, }, // sceKernelSetAllowReplaceUmd
	{ 0x2269BFA2, 0x1404C1AA, }, // SysMemForKernel_2269BFA2
	{ 0xCD617A94, 0xEF29061C, }, // sceKernelGetGameInfo
	{ 0xF77D77CB, 0x476F687A, }, // sceKernelSetCompilerVersion
	{ 0x38495D84, 0x4A325AA0, }, // sceKernelGetInitialRandomValue
	{ 0x29A5899B, 0x96A3CE2C, }, // sceKernelSetRebootKernel
	{ 0x071D9804, 0xC5EC364C, }, // sceKernelApiEvaluationInit
	{ 0x02786087, 0xF5153060, }, // sceKernelRegisterApiEvaluation
	{ 0x1F7F7F40, 0xF8F54A95, }, // SysMemForKernel_1F7F7F40
	{ 0x049CC735, 0x845310CF, }, // sceKernelApiEvaluationReport
	{ 0x39351245, 0xFD15D334, }, // SysMemForKernel_39351245
	{ 0xD2E3A399, 0x5FE712E9, }, // SysMemForKernel_D2E3A399
	{ 0x4852F8DD, 0x1A726C89, }, // SysMemForKernel_4852F8DD
	{ 0x4EC43DC4, 0xE7A3F4D5, }, // SysMemForKernel_4EC43DC4
	{ 0x1F01A9E2, 0x6D9E2DD6, }, // SysMemForKernel_1F01A9E2
	{ 0xB9F8561C, 0xD3CA555C, }, // SysMemForKernel_B9F8561C
	{ 0x9452B542, 0xC4D3A378, }, // SysMemForKernel_9452B542
	{ 0xEF2EE8C1, 0xFE3CF2BC, }, // SysMemForKernel_EF2EE8C1
};

static NidResolverEntry KDebugForKernel_nid[] = {
	{ 0xA2983280, 0x2C7D85FA, }, // sceKernelCheckDebugHandler
	{ 0x2FF4E9F9, 0xAA9B2F07, }, // sceKernelAssert
	{ 0x5282DD5E, 0xB20CABDB, }, // sceKernelDipswSet
	{ 0xEE75658D, 0xEFF672D1, }, // sceKernelDipswClear
	{ 0x428A8DA3, 0x47570AC5, }, // sceKernelIsToolMode
	{ 0x72E6B3B8, 0x27B23800, }, // sceKernelIsUMDMode
	{ 0xA216AE06, 0xB41E2430, }, // sceKernelIsDVDMode
	{ 0x9B868276, 0x276C02E3, }, // sceKernelGetDebugPutchar
	{ 0xE7A3874D, 0x0C56F87A, }, // sceKernelRegisterAssertHandler
	{ 0xAFB8FC80, 0x43F0F8AB, }, // sceKernelDipswLow32
	{ 0x23906FB2, 0x6CB0BDA4, }, // sceKernelDipswHigh32
	{ 0xE146606D, 0x01FFE328, }, // sceKernelRegisterDebugPutchar
	{ 0x7CEB2C09, 0xE201A0D8, }, // sceKernelRegisterKprintfHandler
	{ 0x5CE9838B, 0x2214799B, }, // sceKernelDebugWrite
	{ 0x66253C4E, 0x4A74A9D4, }, // sceKernelRegisterDebugWrite
	{ 0xDBB5597F, 0xD4EC38C1, }, // sceKernelDebugRead
	{ 0xE6554FDA, 0x48BE5AA9, }, // sceKernelRegisterDebugRead
	{ 0xB9C643C9, 0xE8FE3EE3, }, // sceKernelDebugEcho
	{ 0x7D1C74F0, 0xA1C5B42F, }, // sceKernelDebugEchoSet
	{ 0x24C32559, 0x86010FCB, }, // sceKernelDipsw

	{ 0x333DCEC7, 0x02668C61, },
	{ 0x8B041DFB, 0x8A0F4E94, },
	{ 0x9F8703E4, 0x568DCD25, },
	{ 0xFFD2F2B9, 0xACF427DC, },
	{ 0xA126F497, 0x604276EB, },
	{ 0xB7251823, 0xF339073C, },
};

static NidResolverEntry LoadCoreForKernel_nid[] = {
	{ 0xCF8A41B1, 0xF6B1BF0F, }, // sceKernelFindModuleByName
	{ 0xCCE4A157, 0x40972E6E, }, // sceKernelFindModuleByUID
	{ 0xDD303D79, 0xBC99C625, }, // sceKernelFindModuleByAddress (5.XX NID)
	{ 0xFB8AE27D, 0xBC99C625, }, // sceKernelFindModuleByAddress (3.xx NID)
	{ 0xACE23476, 0xFC47F93A, }, // sceKernelCheckPspConfig
	{ 0x7BE1421C, 0xD3353EC4, }, // sceKernelCheckExecFile
	{ 0xBF983EF2, 0x41D10899, }, // sceKernelProbeExecutableObject
	{ 0x7068E6BA, 0x1C394885, }, // sceKernelLoadExecutableObject
	{ 0x99A695F0, 0x48AF96A9, }, // sceKernelRegisterLibrary
	{ 0x5873A31F, 0x2C60CCB8, }, // sceKernelRegisterLibraryForUser
	{ 0x0B464512, 0xCB636A90, }, // sceKernelReleaseLibrary
	{ 0x9BAF90F6, 0x538129F8, }, // sceKernelCanReleaseLibrary
	{ 0x0E760DBA, 0x8EAE9534, }, // sceKernelLinkLibraryEntries
	{ 0x0DE1F600, 0x6ECFFFBA, }, // sceKernelLinkLibraryEntriesForUser
	{ 0xC99DD47A, 0x696594C8, }, // sceKernelQueryLoadCoreCB
	{ 0x616FCCCD, 0xF976EF41, }, // sceKernelSetBootCallbackLevel
	{ 0x6B2371C2, 0x001B57BB, }, // sceKernelDeleteModule
	{ 0xAE7C6E76, 0xBF2E388C, }, // sceKernelRegisterModule
	{ 0x5248A98F, 0x493EE781, }, // sceKernelLoadModuleBootLoadCore
	{ 0xC0913394, 0xA481E30E, }, // sceKernelLinkLibraryEntriesWithModule
	{ 0xEC20C661, 0xB27CC244, }, // LoadCoreForKernel_EC20C661
	{ 0xCDCAA8B2, 0x5FDDB07A, }, // LoadCoreForKernel_CDCAA8B2
	{ 0xCD0F3BAC, 0x2C44F793, }, // sceKernelCreateModule
	{ 0x8D8A8ACE, 0xF3DD4808, }, // sceKernelAssignModule
	{ 0x74CF001A, 0xB17F5075, }, // sceKernelReleaseModule
	{ 0x05D915DB, 0x37E6F41B, }, // sceKernelGetModuleIdListForKernel
	{ 0x52A86C21, 0xCD26E0CA, }, // sceKernelGetModuleFromUID
	{ 0xAFF947D4, 0x84D5C971, }, // sceKernelCreateAssignModule
	{ 0xA6D40F56, 0x410084F9, }, // LoadCoreForKernel_A6D40F56
	{ 0x929B5C69, 0x3FE631F0, }, // sceKernelGetModuleListWithAlloc
	{ 0x4440853B, 0x1999032F, }, // LoadCoreForKernel_4440853B
	{ 0xFA3101A4, 0xB6C037EA, }, // LoadCoreForKernel_FA3101A4
	{ 0x0C0D8B7F, 0x1915737F, }, // LoadCoreForKernel_0C0D8B7F
	{ 0x3BB7AC18, 0xC8FF5EE5, }, // sceKernelLoadCoreMode
};

static NidResolverEntry LoadExecForKernel_nid[] = {
	{ 0x6D302D3D, 0xC3474C2A, }, // sceKernelExitVSHKernel
	{ 0x28D0D249, 0xD940C83C, }, // sceKernelLoadExecVSHMs2
	{ 0xA3D5E142, 0x08F7166C, }, // sceKernelExitVSHVSH
	{ 0x1B97BDB3, 0xD8320A28, }, // sceKernelLoadExecVSHDisc(3.xx)
	{ 0x94A1C627, 0x08F7166C, }, // sceKernelExitVSHVSH
	{ 0x821BE114, 0xD4B49C4B, }, // sceKernelLoadExecVSHDiscUpdater
	{ 0x015DA036, 0xBEF585EC, }, // sceKernelLoadExecBufferVSHUsbWlan
	{ 0x4F41E75E, 0x2B8813AF, }, // sceKernelLoadExecBufferVSHUsbWlanDebug
	{ 0x31DF42BF, 0x4FB44D27, }, // sceKernelLoadExecVSHMs1
	{ 0x70901231, 0xCC6A47D2, }, // sceKernelLoadExecVSHMs3
	{ 0x4AC57943, 0x1F88A490, }, // sceKernelRegisterExitCallback
	{ 0xD9739B89, 0x24114598, }, // sceKernelUnregisterExitCallback
	{ 0x659188E1, 0xB57D0DEC, }, // sceKernelCheckExitCallback
	{ 0x62A27008, 0x1F08547A, }, // sceKernelInvokeExitCallback
	{ 0x7B7C47EF, 0x1B305B09, }, // sceKernelLoadExecVSHDiscDebug
	{ 0x061D9514, 0x00745486, }, // sceKernelLoadExecVSHMs4
	{ 0xB7AB08DA, 0x7CABED9B, }, // sceKernelLoadExecVSHMs5
	{ 0x11412288, 0xA5ECA6E3, }, // LoadExecForKernel_11412288
	{ 0x6D8D3A3A, 0xF9CFCF2F, }, // LoadExecForKernel_6D8D3A3A

	{ 0xD3D13800, 0xDBD0CF1B, },
	{ 0x15EB8575, 0xBC26BEEF, },
};

static NidResolverEntry ModuleMgrForKernel_nid[] = {
	{ 0x977DE386, 0x939E4270, }, // sceKernelLoadModule
	{ 0x50F0C1EC, 0x3FF74DF1, }, // sceKernelStartModule
	{ 0xD1FF982A, 0xE5D6087B, }, // sceKernelStopModule
	{ 0x2E0911AA, 0x387E3CA9, }, // sceKernelUnloadModule
	{ 0x644395E2, 0x303FAB7F, }, // sceKernelGetModuleIdList
	{ 0xA1A78C58, 0xCE0A74A5, }, // sceKernelLoadModuleDisc
	{ 0x748CBED9, 0x22BDBEFF, }, // sceKernelQueryModuleInfo
	{ 0xD675EBB8, 0x5805C1CA, }, // sceKernelSelfStopUnloadModule
	{ 0xBA889C07, 0x4E62C48A, }, // sceKernelLoadModuleBuffer
	{ 0xB7F46618, 0xEEC2A745, }, // sceKernelLoadModuleByID
	{ 0x91B87FAE, 0xCBA02988, }, // sceKernelLoadModuleVSHByID
	{ 0xA4370E7C, 0xD5DDAB1F, }, // sceKernelLoadModuleVSH
	{ 0xF0CAC59E, 0xC6DE0B9C, }, // sceKernelLoadModuleBufferVSH
	{ 0xCC1D3699, 0xE97E0DB7, }, // sceKernelStopUnloadSelfModule
	{ 0x04B7BD22, 0xD86DD11B, }, // sceKernelSearchModuleByName
	{ 0x54D9E02E, 0x12F99392, }, // sceKernelSearchModuleByAddress
	{ 0x5F0CC575, 0xCC873DFA, }, // sceKernelRebootBeforeForUser
	{ 0xB49FFB9E, 0x5FC3B3DA, }, // sceKernelRebootBeforeForKernel
	{ 0xF0A26395, 0xCAB06D30, }, // sceKernelGetModuleId
	{ 0xD8B73127, 0x433D5287, }, // sceKernelGetModuleIdByAddress
	{ 0x6DE9FF11, 0x2B7FC10D, }, // sceKernelLoadModuleWithApitype2
	{ 0x896C15B6, 0xCAE8E169, }, // sceKernelLoadModuleDiscUpdater
	{ 0x92EDEBD0, 0x2C4F270D, }, // sceKernelLoadModuleDiscDebug
	{ 0x6723BBFF, 0xFE61F16D, }, // sceKernelLoadModuleMs1
	{ 0x49C5B9E1, 0x7BD53193, }, // sceKernelLoadModuleMs2/sceKernelLoadModuleForLoadExecVSHMs2
	{ 0xECC2EAA9, 0xD60AB6CC, }, // sceKernelLoadModuleMs3
	{ 0xF07E1A2F, 0x76F0E956, }, // sceKernelLoadModuleMs4
	{ 0x0CE03872, 0x4E8A2C9D, }, // sceKernelLoadModuleForLoadExecVSHMs5
	{ 0x9F3AD1E9, 0x61E3EC69, }, // sceKernelLoadModuleBufferForExitGame
	{ 0x667C1174, 0x9236B422, }, // sceKernelLoadModuleBufferForExitVSHVSH
	{ 0x5799EE32, 0x253AA17C, }, // sceKernelLoadModuleBufferForExitVSHKernel
	{ 0x53CA70EE, 0x4E38EA1D, }, // sceKernelLoadModuleBufferForRebootKernel
	{ 0x2BE4807D, 0xEE6E8F49, }, // sceKernelStopUnloadSelfModuleWithStatusKernel
	{ 0xA6E8C1F5, 0x9B7102E2, }, // sceKernelRebootPhaseForKernel
	{ 0x74690AEB, 0x5FC32087, }, // sceKernelLoadModuleByIDBootInitConfig
	{ 0xF80F3C96, 0xE8B9D19D, }, // sceKernelLoadModuleBufferBootInitConfig
	{ 0x61EC6AB0, 0x1CFFC5DE, }, // sceKernelModuleMgrMode
	{ 0x3E4FEDDC, 0xD4EE2D26, }, // sceKernelLoadModuleToBlock
	{ 0xAB77DC5A, 0xF7C7FEBC, }, // sceKernelLoadModuleBootInitConfig
	{ 0x5C88479B, 0x4493E013, }, // sceKernelLoadModuleDeci
	{ 0xCE70664B, 0x2F3F9B6A, }, // sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlan
	{ 0xB5DDABD3, 0xC13E2DE5, }, // sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlanDebug

	{ 0xEF7A7F02, 0x1CF0B794, },
	{ 0x2BE2B17E, 0xC3DDABEF, },
	{ 0x25E1F458, 0x955D6CB2, },
	{ 0xF7E46A72, 0xA40EC254, },
	{ 0x1B91F6EC, 0x853A6C16, },
	{ 0x245B698D, 0xE8422026, },
};

static NidResolverEntry ExceptionManagerForKernel_nid[] = {
	{ 0x565C0B0E, 0x0A998599, }, // sceKernelRegisterDefaultExceptionHandler
	{ 0x3FB264FC, 0xFBC2353A, }, // sceKernelRegisterExceptionHandler
	{ 0x5A837AD4, 0x7C11759D, }, // sceKernelRegisterPriorityExceptionHandler
	{ 0x1AA6CFFA, 0x54B0CAD2, }, // sceKernelReleaseExceptionHandler
	{ 0xDF83875E, 0x27E6A663, }, // sceKernelGetActiveDefaultExceptionHandler
	{ 0x291FF031, 0x409DBB53, }, // sceKernelReleaseDefaultExceptionHandler
	{ 0xFC26C354, 0x79454858, },
};

static NidResolverEntry IoFileMgrForKernel_nid[] = {
	{ 0x3C54E908, 0x947D7A06, }, // sceIoReopen
	{ 0x411106BA, 0x1FC0620B, }, // sceIoGetThreadCwd
	{ 0xA905B705, 0xC658603A, }, // sceIoCloseAll
	{ 0xCB0A151F, 0xC30581F4, }, // sceIoChangeThreadCwd
	{ 0x73B7671F, 0x18881E58, }, // sceIoGetFdDebugInfo(371)
	{ 0x9B86630B, 0xDCCD6185, }, // sceIoGetUID
	{ 0x30E8ABB3, 0x2B6A9B21, }, // IoFileMgrForKernel_30E8ABB3
	{ 0x42F954D4, 0x22F15793, }, // sceIoAddHook
	{ 0xBD17474F, 0x49356C12, }, // IoFileMgrForKernel_BD17474F
};

static NidResolverEntry scePower_driver_nid[] = {
	{ 0xEFD3C963, 0x0EFEE60E, }, // scePowerTick
	{ 0x87440F5E, 0x872F4ECE, }, // scePowerIsPowerOnline
	{ 0x0AFD0D8B, 0x8C873AA7, }, // scePowerIsBatteryExist
	{ 0x1E490401, 0x7A9EA6DE, }, // scePowerIsBatteryCharging
	{ 0xD3075926, 0xFA651CE1, }, // scePowerIsLowBattery
	{ 0x2085D15D, 0x31AEA94C, }, // scePowerGetBatteryLifePercent
	{ 0x8EFB3FA2, 0xC79F9157, }, // scePowerGetBatteryLifeTime
	{ 0x28E12023, 0x40870DAC, }, // scePowerGetBatteryTemp
	{ 0x483CE86B, 0xF7DE0E81, }, // scePowerGetBatteryVolt
	{ 0x2B7C7CF4, 0x9B44CFD9, }, // scePowerRequestStandby
	{ 0xFEE03A2F, 0xFDB5BFE9, }, // scePowerGetCpuClockFrequency
	{ 0x478FE6F5, 0x04711DFB, }, // scePowerGetBusClockFrequency
	{ 0x737486F2, 0xEBD177D6, }, // scePowerSetClockFrequency
	{ 0xD6E50D7B, 0x766CD857, }, // scePowerRegisterCallback(3.71)
	{ 0x04B7766E, 0x766CD857, }, // scePowerRegisterCallback(1.50)
	{ 0x1688935C, 0x04711DFB, }, // scePowerGetBusClockFrequencyInt
	{ 0x6D2CA84B, 0x2638EF48, }, // scePowerWlanActivate
	{ 0x23BB0A60, 0x8C6BEFD9, }, // scePowerWlanDeactivate
	{ 0x2B51FE2F, 0xCE2032CD, }, // scePower_driver_2B51FE2F
	{ 0x442BFBAC, 0x2509FF3B, }, // scePowerGetBacklightMaximum
	{ 0xEDC13FE5, 0xDF336CDE, }, // scePowerGetIdleTimer
	{ 0x7F30B3B1, 0x1E3B1FAE, }, // scePowerIdleTimerEnable
	{ 0x972CE941, 0x961A06A5, }, // scePowerIdleTimerDisable
	{ 0x27F3292C, 0x0DA940D2, }, // scePowerBatteryUpdateInfo
	{ 0xE8E4E204, 0xA641CF3F, }, // scePower_driver_E8E4E204
	{ 0xB999184C, 0x7B908CAA, }, // scePowerGetLowBatteryCapacity
	{ 0x166922EC, 0x10CE273F, }, // scePowerBatteryForbidCharging
	{ 0xDD3D4DAC, 0xEF751B4A, }, // scePowerBatteryPermitCharging
	{ 0xB4432BC8, 0x67492C52, }, // scePowerGetBatteryChargingStatus
	{ 0x78A1A796, 0x88C79735, }, // scePowerIsSuspendRequired
	{ 0x94F5A53F, 0x41ADFF48, }, // scePowerGetBatteryRemainCapacity
	{ 0xFD18A0FF, 0x003B1E03, }, // scePowerGetBatteryFullCapacity
	{ 0x862AE1A6, 0x993B8C4A, }, // scePowerGetBatteryElec
	{ 0x23436A4A, 0xC730F432, }, // scePowerGetInnerTemp
	{ 0x0CD21B1F, 0x73785D34, }, // scePowerSetPowerSwMode
	{ 0x165CE085, 0xE11CDFFA, }, // scePowerGetPowerSwMode
	{ 0xD6D016EF, 0x6CF50928, }, // scePowerLock
	{ 0xCA3D34C1, 0xC3024FE6, }, // scePowerUnlock
	{ 0x79DB9421, 0x1EC2D4E4, }, // scePowerRebootStart
	{ 0x7FA406DD, 0x566B8353, }, // scePowerIsRequest
	{ 0xAC32C9CC, 0x5C1333B7, }, // scePowerRequestSuspend
	{ 0x2875994B, 0xD1FFF513, }, // scePower_driver_2875994B
	{ 0x3951AF53, 0x3300D85A, }, // scePowerEncodeUBattery
	{ 0x0074EF9B, 0xB45C9066, }, // scePowerGetResumeCount
	{ 0xF535D928, 0xBA566CD0, }, // scePowerSetWakeupCondition
	{ 0xDFA8BAF8, 0x315B8CB6, }, // scePowerUnregisterCallback
	{ 0xDB9D28DD, 0x315B8CB6, }, // scePowerUnregitserCallback
	{ 0xD24E6BEB, 0xDF904CDE, }, // scePowerLimitScCpuClock
	{ 0x35B7662E, 0xEEFB2ACF, }, // scePowerGetSectionDescriptionEntry
	{ 0xF9B4DEA1, 0xB7000C75, }, // scePowerLimitPllClock
	{ 0x843FBF43, 0x53808CBB, }, // scePowerSetCpuClockFrequency
	{ 0xB8D7B3FB, 0xB71A8B2F, }, // scePowerSetBusClockFrequency
	{ 0x478FE6F5, 0x04711DFB, }, // scePowerGetBusClockFrequency
	{ 0xBD681969, 0x04711DFB, }, // scePowerGetBusClockFrequencyInt
	{ 0xB1A52C83, 0xDC4395E2, }, // scePowerGetCpuClockFrequencyFloat
	{ 0x9BADB3EB, 0x1FF8DA3B, }, // scePowerGetBusClockFrequencyFloat
	{ 0xE0B7A95D, 0x100A77A1, }, // scePowerSetGeEdramRefreshMode
	{ 0xC23AC778, 0xC520F5DC, }, // scePowerGetGeEdramRefreshMode
	{ 0x23C31FFE, 0x70F42744, }, // scePowerVolatileMemLock
	{ 0xFA97A599, 0xA882AEB7, }, // scePowerVolatileMemTryLock
	{ 0xB3EDD801, 0x5978B1C2, }, // scePowerVolatileMemUnlock
	{ 0xA9D22232, 0x29E23416, }, // scePowerSetCallbackMode
	{ 0xBAFA3DF0, 0x17EEA285, }, // scePowerGetCallbackMode
	{ 0x57A098B4, 0xD7DD9D38, }, // scePowerGetCurrentTachyonVoltage(2.80)
	{ 0x55D2D789, 0xBADA8332, }, // scePowerGetTachyonVoltage
	{ 0xDD27F119, 0x12F8302D, }, // scePowerGetTachyonVoltage
	{ 0x75F91FE4, 0x9127E5B2, }, // scePowerGetCurrentDdrVoltage
	{ 0x06C83A4F, 0x75906F9A, }, // scePowerGetDdrVoltage
	{ 0x4E759486, 0x018AB235, }, // scePowerSetDdrVoltage
	{ 0x53C1255D, 0x0655D7C3, }, // scePower_driver_53C1255D
	{ 0x8762BCE6, 0x16F965C9, }, // scePower_driver_8762BCE6
	{ 0xCAE2A4CA, 0xD13377F7, }, // scePower_driver_CAE2A4CA
	{ 0xF06B96D4, 0x13D7CCE4, }, // scePower_driver_F06B96D4
	{ 0x34F9C463, 0x67BD889B, }, // scePowerGetPllClockFrequencyInt
	{ 0xEA382A27, 0xBA8CBCBF, }, // scePowerGetPllClockFrequencyFloat
	{ 0x3234844A, 0x5F5006D2, }, // scePower_driver_3234844A
	{ 0xA4D02055, 0xC463E7F2, }, // scePower_driver_A4D02055
	{ 0x9C40E184, 0x0EFEE60E, }, // scePowerTick(371)
	{ 0xCB49F5CE, 0x8432901E, }, // scePowerGetBatteryChargeCycle
	{ 0x0442D852, 0x9DAF25A0, }, // scePowerRequestColdReset
	{ 0x733F973B, 0x72D1B53A, }, // scePowerBatteryEnableUsbCharging
	{ 0x90285886, 0x7EAA4247, }, // scePowerBatteryDisableUsbCharging
	{ 0xD66EF08D, 0xE52B4362, }, // scePowerCheckWlanCondition

	{ 0x9B1A9C5F, 0x9B44CFD9, },
	{ 0xE65F00BD, 0x9DAF25A0, },
	{ 0xD7B9C925, 0x4E32E9B8, },
	{ 0xC71EE866, 0x114B75AB, },
	{ 0xC3C33306, 0x071160B1, },
};

static NidResolverEntry InterruptManagerForKernel_nid[] = {
	{ 0x8A389411, 0x4023E1A7, }, // sceKernelDisableSubIntr
	{ 0xD2E8363F, 0x68B5CA51, }, // QueryIntrHandlerInfo
	{ 0x8B61808B, 0xF153B371, }, // sceKernelQuerySystemCall
	{ 0x85F7766D, 0x6042AEE0, }, // SaveThreadContext
	{ 0x00B6B0F3, UNKNOWNNID, }, // QueryInterruptManCB removed in 6.60
	{ 0x15894D0B, 0xA1B88367, }, // InterruptManagerForKernel_15894D0B
	{ 0xB5A15B30, 0xB941600E, }, // sceKernelSetIntrLevel
	{ 0x43A7BBDC, 0xAB1FC793, }, // InterruptManagerForKernel_43A7BBDC
	{ 0x750E2507, 0x2412F096, }, // sceKernelSuspendIntr
	{ 0x494D6D2B, 0xDB14CBE0, }, // sceKernelResumeIntr
	{ 0x55242A8B, 0xDB9A5496, }, // ReleaseContextHooks
	{ 0x27BC9A45, 0x8357E7FA, }, // UnSupportIntr
	{ 0x0E224D66, UNKNOWNNID, }, // SupportIntr removed in 6.60
	{ 0xCDC86B64, 0x0C5F7AE3, }, // sceKernelCallSubIntrHandler
	{ 0xD6878EB6, 0xCD36EB65, }, // sceKernelGetUserIntrStack
	{ 0xF4454E44, 0xAF799F96, }, // sceKernelCallUserIntrHandler
	{ 0xCA04A2B9, 0xFFA8B183, }, // sceKernelRegisterSubIntrHandler
	{ 0x5CB5A78B, 0xC495F536, }, // sceKernelSuspendSubIntr
	{ 0x7860E0DC, 0x2980AE03, }, // sceKernelResumeSubIntr
	{ 0xFC4374B8, 0x4351DD4E, }, // sceKernelIsSubInterruptOccurred
	{ 0x35634A64, 0xF9E06DF1, }, // sceKernelGetCpuClockCounterWide
	{ 0x2DC9709B, 0x6DDA4D7B, }, // _sceKernelGetCpuClockCounterLow
	{ 0xE9E652A9, 0xE6FB16E3, }, // _sceKernelGetCpuClockCounterHigh
	{ 0x0FC68A56, 0x55D18836, }, // sceKernelSetPrimarySyscallHandler
	{ 0xF4D443F3, 0x14D4C61A, }, // sceKernelRegisterSystemCallTable
	{ 0x36B1EF81, 0x68B5CA51, }, // sceKernelQueryIntrHandlerInfo
	{ 0xEB988556, 0xF153B371, }, // sceKernelQuerySystemCall(5xx)

	{ 0x07E138EE, 0xA4C1C627, },
	{ 0xD8410DE6, 0xF2F1E983, },
	{ 0x8DC2BAEF, 0x1373F20E, },
	{ 0x0B907DBC, 0x6FCBA912, },
	{ 0x7A05D976, 0xDBD52A5D, },
	{ 0x7EF7500F, 0x19596CD3, },
	{ 0x05E9D6C4, 0xA6978B70, },
	{ 0xD63F3B31, 0x2F3C4DCD, },
	{ 0x72CE22E8, 0x02AC50D5, },
};

static NidResolverEntry sceCtrl_driver_nid[] = {
	{ 0x454455AC, 0xBE30CED0, }, // sceCtrlReadBufferPositive
	{ 0xC4AAD55F, 0x2BA616AF, }, // sceCtrlPeekBufferPositive
	{ 0x28E71A16, 0xF6E94EA3, }, // sceCtrlSetSamplingMode
	{ 0x3E65A0EA, 0x121097D5, }, // sceCtrlInit
	{ 0xE03956E9, 0x1A1A7D40, }, // sceCtrlEnd
	{ 0xC3F607F3, 0x55497589, }, // sceCtrlSuspend
	{ 0xC245B57B, 0x33D03FD5, }, // sceCtrlResume
	{ 0xA88E8D22, 0x6A1DF4CB, }, // sceCtrlSetIdleCancelKey
	{ 0xB7CEAED4, 0x7511CCFE, }, // sceCtrlGetIdleCancelKey
	{ 0x7CA723DC, 0xF8346777, }, // sceCtrl_driver_7CA723DC
	{ 0x5E77BC8A, 0x1809B9FC, }, // sceCtrl_driver_5E77BC8A
	{ 0x5C56C779, 0xDF53E160, }, // sceCtrl_driver_5C56C779
	{ 0x1F4011E6, 0xF6E94EA3, }, // sceCtrlSetSamplingMode(150)
	{ 0x3A622550, 0x2BA616AF, }, // sceCtrlPeekBufferPositive(150)
	{ 0xC152080A, 0xE6085C33, }, // sceCtrlPeekBufferNegative(150)
	{ 0x1F803938, 0xBE30CED0, }, // sceCtrlReadBufferPositive(150)
	{ 0x60B81F86, 0x3A6A612A, }, // sceCtrlReadBufferNegative(150)
	{ 0xDA6B76A1, 0xF8EC18BD, }, // sceCtrlGetSamplingMode
	{ 0x6A2774F3, 0x83B15A81, }, // sceCtrlSetSamplingCycle
	{ 0xB1D0E5CD, 0x637CB76C, }, // sceCtrlPeekLatch
	{ 0x0B588501, 0x7F7C4E0A, }, // sceCtrlReadLatch
	{ 0xA7144800, 0x37533267, }, // sceCtrlSetIdleCancelThreshold
	{ 0x687660FA, 0xE54253E7, }, // sceCtrlGetIdleCancelThreshold
	{ 0x6841BE1A, 0x89438C13, }, // sceCtrlSetRapidFire
	{ 0x02BAAD91, 0x501E0C70, }, // sceCtrlGetSamplingCycle
	{ 0xA68FD260, 0x994488EC, }, // sceCtrlClearRapidFire
	{ 0x252D3A8D, 0xF0074903, }, // sceCtrlSetPollingMode

	{ 0x6D74BF08, 0x83B15A81, },
	{ 0xFF847C31, 0x3A6A612A, },
	{ 0x348D99D4, 0x547F89D3, },
	{ 0xAF5960F3, 0xBC8D1A3B, },
};

static NidResolverEntry sceDisplay_driver_nid[] = {
	{ 0x4AB7497F, 0xA38B3F89, }, // sceDisplaySetFrameBuf
	{ 0xB685BA36, 0xB0942511, }, // sceDisplayWaitVblankStart
	{ 0xE56B11BA, 0xFBB369FD, }, // sceDisplayGetFrameBuf
	{ 0x776ADFDB, 0x60112E07, }, // sceDisplaySetBacklightInternal
	{ 0x31C4BAA8, 0x0043973F, }, // sceDisplayGetBrightness
	{ 0x5B5AEFAD, 0xF338AAE0, }, // from 1.50
	{ 0xDBA6C4C4, 0x1EAA0BDC, }, // sceDisplayGetFramePerSec
	{ 0x432D133F, 0x117C3E2C, }, // sceDisplayEnable
	{ 0x681EE6A7, 0x33B620AF, }, // sceDisplayDisable
	{ 0x7ED59BC4, 0x3552AB11, }, // sceDisplaySetHoldMode
	{ 0xA544C486, 0x03F16FD4, }, // sceDisplaySetResumeMode
	{ 0x63E22A26, 0x3E17FE8D, }, // sceDisplay_driver_63E22A26
	{ 0x289D82FE, 0xA38B3F89, }, // sceDisplaySetFrameBuf
	{ 0xEEDA2E54, 0xFBB369FD, }, // sceDisplayGetFrameBuf
	{ 0xB4F378FA, 0x99E358F1, }, // sceDisplayIsForeground
	{ 0xAC14F1FF, 0x8059136A, }, // sceDisplayGetForegroundLevel
	{ 0x9E3C6DC6, 0x60112E07, }, // sceDisplaySetBrightness
	{ 0x9C6EAAD7, 0x1FBE8856, }, // sceDisplayGetVcount
	{ 0x4D4E10EC, 0x572D7804, }, // sceDisplayIsVblank
	{ 0x69B53541, 0xE7397E7A, }, // sceDisplayGetVblankRest
	{ 0x36CDFADE, 0xFE5884EF, }, // sceDisplayWaitVblank
	{ 0x8EB9EC49, 0x58E8680E, }, // sceDisplayWaitVblankCB
	{ 0x984C27E7, 0xB0942511, }, // sceDisplayWaitVblankStart
	{ 0x46F186C3, 0xE38CA615, }, // sceDisplayWaitVblankStartCB
	{ 0x773DD3A3, 0xEC80D435, }, // sceDisplayGetCurrentHcount
	{ 0x210EAB3A, 0xF84D16CC, }, // sceDisplayGetAccumulatedHcount
	{ 0xBF79F646, 0x46CC2336, }, // sceDisplayGetResumeMode
	{ 0xA83EF139, 0xD81A8FF9, }, // sceDisplayAdjustAccumulatedHcount
	{ 0x21038913, 0x71695015, }, // sceDisplayIsVsync
	{ 0x92C8F8B7, 0x34516323, }, // sceDisplayIsPseudoField
	{ 0xAE0E8972, 0xE97E9718, }, // sceDisplaySetPseudoVsync

	{ 0x643F3841, 0x3E17FE8D, },
	{ 0x820C6038, 0xF338AAE0, },
	{ 0x1CB8CB47, 0x0043973F, },
	{ 0x13AA96B7, 0x1EAA0BDC, },
	{ 0x66961521, 0x8475E040, },
	{ 0xF5EEEFEF, 0x996881D2, },
	{ 0xE8466BC2, 0x1FBE8856, },
	{ 0x946155FD, 0x117C3E2C, },
	{ 0x32B67781, 0x33B620AF, },
};

static NidResolverEntry sceAudio_driver_nid[] = {
	{ 0xB282F4B2, 0x9D77949E, }, // sceAudioGetChannelRestLength
	{ 0x669D93E4, 0x837701CC, }, // sceAudioSRCChReserve
	{ 0x138A70F1, 0xB7F5A1B2, }, // sceAudioSRCChRelease
	{ 0x43645E69, 0xAC81DE4F, }, // sceAudioSRCOutputBlocking
	{ 0x80F1F7E0, 0x22E4FB37, }, // sceAudioInit
	{ 0x210567F7, 0x4A7F9E9D, }, // sceAudioEnd
	{ 0xA2BEAA6C, 0x4A0FE97D, }, // sceAudioSetFrequency
	{ 0xB61595C0, 0x53A4FE20, }, // sceAudioLoopbackTest
	{ 0x927AC32B, 0xF86DFDD6, }, // sceAudioSetVolumeOffset
	{ 0x8C1009B2, 0x671E97E8, }, // sceAudioOutput
	{ 0x136CAF51, 0x5CDEF9A4, }, // sceAudioOutputBlocking
	{ 0xE2D56B2D, 0x17D856B9, }, // sceAudioOutputPanned
	{ 0x13F592BC, 0xB7CCF1D7, }, // sceAudioOutputPannedBlocking
	{ 0x5EC81C55, 0x4F959094, }, // sceAudioChReserve
	{ 0x41EFADE7, 0xE440DF7D, }, // sceAudioOneshotOutput
	{ 0x6FC46853, 0xDA16FF61, }, // sceAudioChRelease
	{ 0xB011922F, 0x9D77949E, }, // sceAudioGetChannelRestLength
	{ 0xCB2E439E, 0x093C7A46, }, // sceAudioSetChannelDataLen
	{ 0x95FD0C2D, 0x63B5A460, }, // sceAudioChangeChannelConfig
	{ 0xB7E1D8E7, 0xFAD7B114, }, // sceAudioChangeChannelVolume
	{ 0x38553111, 0x837701CC, }, // sceAudioSRCChReserve
	{ 0x5C37C0AE, 0xB7F5A1B2, }, // sceAudioSRCChRelease
	{ 0xE0727056, 0xAC81DE4F, }, // sceAudioSRCOutputBlocking
	{ 0x086E5895, 0x42CC2D83, }, // sceAudioInputBlocking
	{ 0x6D4BEC68, 0x76BBA42E, }, // sceAudioInput
	{ 0xA708C6A6, 0xFA05EF51, }, // sceAudioGetInputLength
	{ 0x87B2E651, 0xD2F5A41A, }, // sceAudioWaitInputEnd
	{ 0x7DE61688, 0x10900742, }, // sceAudioInputInit
	{ 0xE926D3FB, 0x4AFA04D9, }, // sceAudioInputInitEx
	{ 0xA633048E, 0x60B750B2, }, // sceAudioPollInputEnd
	{ 0xE9D97901, 0xCF62029E, }, // sceAudioGetChannelRestLen
	{ 0x647CEF33, 0x8A7CD9C6, }, // sceAudioOutput2GetRestSample
};

static NidResolverEntry sceReg_driver_nid[] = {
	{ 0x98279CF1, 0x30E567D5, }, // sceRegInit
	{ 0x9B25EDF1, 0xBDE681CF, }, // sceRegExit
	{ 0x92E41280, 0xDBA46704, }, // sceRegOpenRegistry
	{ 0xFA8A5739, 0x49D77D65, }, // sceRegCloseRegistry
	{ 0xDEDA92BF, 0xC3DEEBC6, }, // sceRegRemoveRegistry
	{ 0x1D8A762E, 0x4F471457, }, // sceRegOpenCategory
	{ 0x0CAE832B, 0xFC742751, }, // sceRegCloseCategory
	{ 0x39461B4D, 0x5FD4764A, }, // sceRegFlushRegistry
	{ 0x0D69BF40, 0xD743A608, }, // sceRegFlushCategory
	{ 0x57641A81, 0x3B6CA1E6, }, // sceRegCreateKey
	{ 0x17768E14, 0x49C70163, }, // sceRegSetKeyValue
	{ 0xD4475AA8, 0x9980519F, }, // sceRegGetKeyInfo
	{ 0x28A8E98A, 0xF4A3E396, }, // sceRegGetKeyValue
	{ 0x2C0DB9DD, 0xD323FC94, }, // sceRegGetKeysNum
	{ 0x2D211135, 0xBAE3371A, }, // sceRegGetKeys
	{ 0x4CA16893, 0x61DB9D06, }, // sceRegRemoveCategory
	{ 0x3615BC87, 0x405D36C3, }, // sceRegRemoveKey
	{ 0xC5768D02, 0xF2619407, }, // sceRegGetKeyInfoByName
	{ 0x30BE0259, 0x38415B9F, }, // sceRegGetKeyValueByName
	{ 0xBE8C1263, 0x90AA3904, }, // sceReg_BE8C1263
	{ 0x835ECE6F, 0x6328E4C6, }, // sceReg_835ECE6F
};

static NidResolverEntry SysTimerForKernel_nid[] = {
	{ 0xC99073E3, 0x893DEC1A, }, // sceSTimerAlloc
	{ 0xC105CF38, 0x93B952D1, }, // sceSTimerFree
	{ 0xB53534B4, 0xCAC43FEB, }, // SysTimerForKernel_B53534B4
	{ 0x975D8E84, 0xEEE11CA5, }, // sceSTimerSetHandler
	{ 0xA95143E2, 0x4BC8C61C, }, // sceSTimerStartCount
	{ 0x4A01F9D3, 0x973104D0, }, // sceSTimerStopCount
	{ 0x54BB5DB4, 0x6AC97B04, }, // sceSTimerResetCount
	{ 0x228EDAE4, 0xDDA5D3EA, }, // sceSTimerGetCount
	{ 0x53231A15, 0xE265669A, }, // SysTimerForKernel_53231A15
};

static NidResolverEntry sceImpose_driver_nid[] = {
	{ 0x0F341BE4, 0xE9A42056, }, // sceImposeGetHomePopup
	{ 0x116CFF64, 0xE9B0E008, }, // sceImposeCheckVideoOut
	{ 0x116DDED6, 0x28B97035, }, // sceImposeSetVideoOutMode
	{ 0x1AEED8FE, 0x686B040E, }, // sceImposeSuspend
	{ 0x1B6E3400, 0xB3EAE8F0, }, // sceImposeGetStatus
	{ 0x24FD7BCF, 0x842309DD, }, // sceImposeGetLanguageMode
	{ 0x36AA6E91, 0xB376EF7F, }, // sceImposeSetLanguageMode
	{ 0x381BD9E7, 0xEFB54A28, }, // sceImposeHomeButton
	{ 0x531C9778, 0xDC3BECFF, }, // sceImposeGetParam
	{ 0x5595A71A, 0xC08C41EF, }, // sceImposeSetHomePopup
	{ 0x7084E72C, 0x0D5F811D, }, // sceImpose_driver_7084E72C
	{ 0x72189C48, 0x9FF64142, }, // sceImposeSetUMDPopup
	{ 0x810FB7FB, 0x3C318569, }, // sceImposeSetParam
	{ 0x86924032, 0x9E921836, }, // sceImposeResume
	{ 0x8C943191, 0x5557F4E2, }, // sceImposeGetBatteryIconStatus
	{ 0x8F6E3518, 0xE1A76241, }, // sceImposeGetBacklightOffTime
	{ 0x967F6D4A, 0x37F3F1D2, }, // sceImposeSetBacklightOffTime
	{ 0x9C8C6C81, 0xBB12F974, }, // sceImposeSetStatus
	{ 0x9DBCE0C4, 0xBCF1D254, }, // sceImpose_driver_9DBCE0C4
	{ 0xB415FC59, 0x0F067E16, }, // sceImposeChanges
	{ 0xBDBC42A6, 0xD067235E, }, // sceImposeInit
	{ 0xC7E36CC7, 0xAABEE36A, }, // sceImposeEnd
	{ 0xE0887BC8, 0xEF4D1BC5, }, // sceImposeGetUMDPopup

	{ 0xFF1A2F07, 0xCE16DF6D, },
};

static NidResolverEntry sceGe_driver_nid[] = {
	{ 0x71FCD1D6, 0x410EE294, }, // sceGeInit
	{ 0x9F2C2948, 0xEF76EDF5, }, // sceGeEnd
	{ 0x8F185DF7, 0x0F204A96, }, // sceGeEdramInit
	{ 0x1F6752AD, 0x5D28A52B, }, // sceGeEdramGetSize
	{ 0xE47E40E4, 0x7D662DF2, }, // sceGeEdramGetAddr
	{ 0xB77905EA, 0x2DFAAF99, }, // sceGeEdramSetAddrTranslation
	{ 0xB415364D, 0x1FDB4C62, }, // sceGeGetReg
	{ 0xDC93CFEF, 0xE6EE2394, }, // sceGeGetCmd
	{ 0x57C8945B, 0x95D0CDC2, }, // sceGeGetMtx
	{ 0x438A385A, 0x0FFB3AFD, }, // sceGeSaveContext
	{ 0x0BF608FB, 0x9B893EFC, }, // sceGeRestoreContext
	{ 0xAB49E76A, 0xAFE6C9EF, }, // sceGeListEnQueue
	{ 0x1C0D95A6, 0x762FA6D0, }, // sceGeListEnQueueHead
	{ 0x5FB86AB0, 0xA741076A, }, // sceGeListDeQueue
	{ 0xE0D68148, 0x7CFD2733, }, // sceGeListUpdateStallAddr
	{ 0x03444EB4, 0x4B9554EB, }, // sceGeListSync
	{ 0xB287BD61, 0x1F6B3C34, }, // sceGeDrawSync
	{ 0xB448EC0D, 0x71F8DB0E, }, // sceGeBreak
	{ 0x4C06E472, 0x816D56EE, }, // sceGeContinue
	{ 0xA4FC06A4, 0x6A600875, }, // sceGeSetCallback
	{ 0x05DB22CE, 0x0B986A8D, }, // sceGeUnsetCallback
	{ 0x9DA4A75F, 0x9691676D, }, // sceGe_driver_9DA4A75F
	{ 0x114E1745, 0x2539D845, }, // sceGe_driver_114E1745
	{ 0x51C8BB60, 0x1B01E589, }, // sceGeSetReg
	{ 0x51D44C58, 0x4F66BD43, }, // sceGeSetCmd
	{ 0x5A0103E6, 0x5E7DE870, }, // sceGeSetMtx
	{ 0xAEC21518, 0x05238809, }, // sceGe_driver_AEC21518
	{ 0x7B481502, 0x6E9C829D, }, // sceGe_driver_7B481502
	{ 0x9ACFF59D, 0x82F1049F, }, // sceGeGetListIdList
	{ 0x67B01D8E, 0x7AF3DE00, }, // sceGeGetList
	{ 0xE66CB92E, 0xA8A9CE31, }, // sceGeGetStack
	{ 0x5BAA5439, 0xD8633888, }, // sceGeEdramSetSize

	{ 0xBAD6E1CA, 0x17133A54, },
	{ 0xC576E897, 0x547EC5F0, },
};

static NidResolverEntry sceRtc_driver_nid[] = {
	{ 0xE7C27D1B, 0x9012B140, }, // sceRtcGetCurrentClockLocalTime
	{ 0x34885E0D, 0x4E267E02, }, // sceRtcConvertUtcToLocalTime
	{ 0x779242A2, 0x3E66CB7E, }, // sceRtcConvertLocalTimeToUTC
	{ 0x42307A17, 0x00F66D06, }, // sceRtcIsLeapYear
	{ 0x05EF322C, 0x1DAB3CF3, }, // sceRtcGetDaysInMonth
	{ 0x57726BC1, 0x321A839A, }, // sceRtcGetDayOfWeek
	{ 0x3A807CC8, 0x40B07E72, }, // sceRtcSetTime_t
	{ 0x27C4594C, 0xE86D8FC0, }, // sceRtcGetTime_t
	{ 0xF006F264, 0x74772CCC, }, // sceRtcSetDosTime
	{ 0x36075567, 0xA4A5BF1B, }, // sceRtcGetDosTime
	{ 0x7ACE4C04, 0xCEF8FE8E, }, // sceRtcSetWin32FileTime
	{ 0xCF561893, 0xB2B6578C, }, // sceRtcGetWin32FileTime
	{ 0x9ED0AE87, 0x281144FE, }, // sceRtcCompareTick
	{ 0x26D25A5D, 0xB84AC7D7, }, // sceRtcTickAddMicroseconds
	{ 0xE6605BCA, 0x77138347, }, // sceRtcTickAddMinutes
	{ 0x26D7A24A, 0x8413CADC, }, // sceRtcTickAddHours
	{ 0xE51B4B7A, 0xCB0538FD, }, // sceRtcTickAddDays
	{ 0xCF3A2CA8, 0x80F21937, }, // sceRtcTickAddWeeks
	{ 0xDBF74F1B, 0xE45726F6, }, // sceRtcTickAddMonths
	{ 0x42842C77, 0xAAAE90FF, }, // sceRtcTickAddYears
	{ 0xC663B3B9, 0x1A86F5FD, }, // sceRtcFormatRFC2822
	{ 0x7DE6711B, 0x27FAEC90, }, // sceRtcFormatRFC2822LocalTime
	{ 0x0498FB3C, 0x1FCE9E23, }, // sceRtcFormatRFC3339
	{ 0x27F98543, 0x8DED141A, }, // sceRtcFormatRFC3339LocalTime
	{ 0xDFBC5F16, 0xC3A806EE, }, // sceRtcParseDateTime
	{ 0x28E1E988, 0xBDA60897, }, // sceRtcParseRFC3339
	{ 0x029CA3B3, 0x011F03C1, }, // sceRtcGetAccumulativeTime
	{ 0x17C26C00, 0x66054C2A, }, // sceRtcSetCurrentSecureTick
	{ 0x1909C99B, 0xCF4E0EE0, }, // sceRtcSetTime64_t
	{ 0x203CEB0D, 0x7C6E9610, }, // sceRtcGetLastReincarnatedTime
	{ 0x62685E98, 0xE98FEC46, }, // sceRtcGetLastAdjustedTime
	{ 0x6A676D2D, 0x7D8E37E1, }, // sceRtcUnregisterCallback
	{ 0x759937C5, 0xDFF30673, }, // sceRtcSetConf
	{ 0x7D1FBED3, 0xE09880CF, }, // sceRtcSetAlarmTick
	{ 0x81FCDA34, 0xCF76CFE5, }, // sceRtcIsAlarmed
	{ 0xC2DDBEB5, 0x366669D6, }, // sceRtcGetAlarmTick
	{ 0xC499AF2F, 0x1C1859DF, }, // sceRtcReset
	{ 0xE1C93E47, 0x94225550, }, // sceRtcGetTime64_t
	{ 0xF0B5571C, 0x852255B8, }, // sceRtcSynchronize
	{ 0xFB3B18CD, 0x530A903E, }, // sceRtcRegisterCallback
	{ 0x9763C138, 0x929620CE, }, // sceRtcSetCurrentTick
	{ 0xF2A4AFE5, 0x89FA4262, }, // sceRtcTickAddSeconds

	{ 0xD24DF719, 0xED15334F, },
	{ 0xF5FCC995, 0xED15334F, },
};

static NidResolverEntry sceSyscon_driver_nid[] = {
	{ 0x58531E69, 0xF8EC18BD, }, // sceSysconSetGSensorCallback
	{ 0xAD555CE5, 0x599EB8A0, }, // sceSysconSetLowBatteryCallback
	{ 0xF281805D, 0xD76A105E, }, // sceSysconSetPowerSwitchCallback
	{ 0xA068B3D0, 0x2B7A0D32, }, // sceSysconSetAlarmCallback
	{ 0xE540E532, 0x657DCEF7, }, // sceSysconSetAcSupplyCallback
	{ 0xBBFB70C0, 0x672B79E8, }, // sceSysconSetHPConnectCallback
	{ 0x805180D1, 0x21AC8621, }, // sceSysconSetHRPowerCallback
	{ 0xA8E34316, 0x39456DE1, }, // sceSysconSetHRWakeupCallback
	{ 0x53072985, 0x50446BE5, }, // sceSysconSetWlanSwitchCallback
	{ 0xF9193EC3, 0xD2C053E7, }, // sceSysconSetWlanPowerCallback
	{ 0x88FDB279, 0xE19BC2DF, }, // sceSysconSetBtSwitchCallback
	{ 0xBAAECDF8, 0x5C4C1130, }, // sceSysconSetBtPowerCallback
	{ 0x7479DB05, 0x399708EB, }, // sceSysconSetHoldSwitchCallback
	{ 0x6848D817, 0xD6C2FD5F, }, // sceSysconSetUmdSwitchCallback
	{ 0xFC32141A, 0x22240B41, }, // sceSysconGetPowerSupplyStatus
	{ 0xF775BC34, 0xF33E1B14, }, // sceSysconGetWakeUpFactor
	{ 0xA9AEF39F, 0x9D88A8DE, }, // sceSysconGetWakeUpReq
	{ 0x25F47F96, 0x9FB6B045, }, // sceSysconGetVideoCable
	{ 0xC4D66C1D, 0xF436BB12, }, // sceSysconReadClock
	{ 0xC7634A7A, 0x3FB3FD08, }, // sceSysconWriteClock
	{ 0x7A805EE4, 0xF2AE6D5E, }, // sceSysconReadAlarm
	{ 0x6C911742, 0x80711575, }, // sceSysconWriteAlarm
	{ 0xE9FF8226, 0x2BE8EBC2, }, // sceSysconSetUSBStatus
	{ 0x882F0AAB, 0x38CBE06E, }, // sceSysconGetTachyonWDTStatus
	{ 0x00E7B6C2, 0x327A82F2, }, // sceSysconCtrlAnalogXYPolling
	{ 0x44439604, 0xBB7260C8, }, // sceSysconCtrlHRPower
	{ 0x91E183CB, 0x17E7753D, }, // sceSysconPowerSuspend
	{ 0xBE27FE66, 0xEDD3AB8B, }, // sceSysconCtrlPower
	{ 0x1A08CD66, 0x88EAAB07, }, // sceSysconCtrlDvePower
	{ 0x19D5A97A, 0x89A2024D, }, // sceSysconCtrlCharge
	{ 0xE5E35721, 0x9BC5E33B, }, // sceSysconCtrlTachyonAvcPower
	{ 0x01677F91, 0xF7BCD2A6, }, // sceSysconCtrlVoltage
	{ 0x09721F7F, 0x28363C97, }, // sceSysconGetPowerStatus
	{ 0x99BBB24C, 0x1088ABA8, }, // sceSysconCtrlMsPower
	{ 0xF0ED3255, 0x48448373, }, // sceSysconCtrlWlanPower
	{ 0xA975F224, 0xA0FA8CF7, }, // sceSysconCtrlHddPower
	{ 0x9C4266FC, 0x7E3A82AF, }, // sceSysconCtrlBtPower
	{ 0x3987FEA3, 0x47378317, }, // sceSysconCtrlUsbPower
	{ 0xEAB13FBE, 0x5EDEDED6, }, // sceSysconPermitChargeBattery
	{ 0xC5075828, 0x27F94EE6, }, // sceSysconForbidChargeBattery
	{ 0x3C6DB1C5, 0x1C5D4229, }, // sceSysconCtrlTachyonVmePower
	{ 0xB2558E37, 0x9F39BDC8, }, // sceSysconCtrlTachyonAwPower
	{ 0x9478F399, 0x457D8D7C, }, // sceSysconCtrlLcdPower
	{ 0x6A53F3F8, 0x85F5F601, }, // sceSysconBatteryGetStatusCap
	{ 0xFA4C4518, 0x7686E7A7, }, // sceSysconBatteryGetInfo
	{ 0xE448FD3F, 0x8F4AD2CA, }, // sceSysconBatteryNop
	{ 0x70C10E61, 0xCE8B6633, }, // sceSysconBatteryGetTemp
	{ 0x8BDEBB1E, 0xA7DB34BB, }, // sceSysconBatteryGetVolt
	{ 0x373EC933, 0x483088B0, }, // sceSysconBatteryGetElec
	{ 0x82861DE2, 0x8E54A128, }, // sceSysconBatteryGetRCap
	{ 0x876CA580, 0x3E16A759, }, // sceSysconBatteryGetCap
	{ 0x71135D7D, 0x4C871BEA, }, // sceSysconBatteryGetFullCap
	{ 0x7CBD4522, 0x8DD190A1, }, // sceSysconBatteryGetIFC
	{ 0x284FE366, 0x4D5A19BB, }, // sceSysconBatteryGetLimitTime
	{ 0x75025299, 0x3FA9F842, }, // sceSysconBatteryGetStatus
	{ 0xB5105D51, 0x68AF19F1, }, // sceSysconBatteryGetCycle
	{ 0xD5340103, 0x031CCDD7, }, // sceSysconBatteryGetSerial
	{ 0xB71B98A8, 0xD06FA2C6, }, // sceSysconBatteryGetTempAD
	{ 0x87671B18, 0x2B6BA4AB, }, // sceSysconBatteryGetVoltAD
	{ 0x75D22BF8, 0x011AC062, }, // sceSysconBatteryGetElecAD
	{ 0x4C539345, 0xF8F6E1F4, }, // sceSysconBatteryGetTotalElec
	{ 0x4C0EE2FA, 0xF87A1D11, }, // sceSysconBatteryGetChargeTime
	{ 0x08DA3752, 0x82D9F1BB, }, // sceSysconCtrlTachyonVoltage
	{ 0x34C36FF9, 0x3E4BD909, }, // sceSysconGetDigitalKey
	{ 0x565EF519, 0x0E5FA7EA, }, // sceSysconGetFallingDetectTime
	{ 0x2B9E6A06, 0xF0D1443F, }, // sceSysconGetPowerError
	{ 0x5FF1D610, 0x0E4FC766, }, // sceSysconSetPollingMode
	{ 0x3357EE5C, 0x05734D21, }, // sceSysconIsFalling
	{ 0x32CFD20F, 0x1605847F, }, // sceSysconIsLowBattery
	{ 0xEC0DE439, 0x47C04A04, }, // sceSysconGetPowerSwitch
	{ 0xEA5B9823, 0xC8DB7B74, }, // sceSysconIsAlarmed
	{ 0xE0DDFE18, 0x2D6F2728, }, // sceSysconGetHPConnect
	{ 0xBDA16E46, 0x2D510164, }, // sceSysconGetWlanSwitch
	{ 0xA6776FB4, 0x579A30EA, }, // sceSysconGetBtSwitch
	{ 0xF6BB4649, 0x01792223, }, // sceSysconGetHoldSwitch
	{ 0x71AE1BCE, 0xE58B9388, }, // sceSysconGetHRPowerStatus
	{ 0xF953EF93, 0xEF31EF4E, }, // sceSysconGetHRWakeupStatus
	{ 0x7016161C, 0x7216917F, }, // sceSysconGetWlanPowerStatus
	{ 0x56A73EDD, 0x67B66898, }, // sceSysconGetBtPowerStatus
	{ 0x48AB0E44, 0x577C5771, }, // sceSysconGetLeptonPowerCtrl
	{ 0x628F2351, 0x7672103B, }, // sceSysconGetMsPowerCtrl
	{ 0xEC37C549, 0xB5B06B81, }, // sceSysconGetWlanPowerCtrl
	{ 0x051186F3, 0x64257B5C, }, // sceSysconGetHddPowerCtrl
	{ 0xCE76F93E, 0x833017E5, }, // sceSysconGetDvePowerCtrl
	{ 0x8EDF1AD7, 0x3032943A, }, // sceSysconGetBtPowerCtrl
	{ 0xEE81C5B1, 0xEB11E9DE, }, // sceSysconGetUsbPowerCtrl
	{ 0x8DDA4CA6, 0x9E82A08C, }, // sceSysconGetTachyonVmePowerCtrl
	{ 0x52B74B6C, 0xDEB91FF2, }, // sceSysconGetTachyonAwPowerCtrl
	{ 0x1B17D3E3, 0x3932315D, }, // sceSysconGetTachyonAvcPowerCtrl
	{ 0x5F19C00F, 0x9497E906, }, // sceSysconGetLcdPowerCtrl
	{ 0x3C739F57, 0xD38A3708, }, // sceSysconGetHRPowerCtrl
	{ 0x72EDA9AF, 0xB5FA7580, }, // sceSysconGetGSensorVersion
	{ 0xC68F1573, 0xC56D0B5A, }, // sceSysconCtrlGSensor
	{ 0x7C8A5503, 0x93A3B23E, }, // sceSysconGetScvCode
//	{ 0x2EE82492, 0x2EE82492, }, // sceSysconCtrlTachyonWDT

	{ 0x64FA0B22, 0x28F7032E, },
	{ 0x4AB44BFC, 0xAA1B32D4, },
	{ 0xD653E02E, 0xE8C20DB5, },
	{ 0x9E6E060F, 0xB72DDFD2, },
	{ 0xE00BFC9E, 0xFD5C58CB, },
	{ 0xC8D97773, 0x862A93DE, },
	{ 0xE20D08FE, 0xA3406117, },
	{ 0x14909C63, 0x1083C71D, },
	{ 0x05727A0B, 0x294A7ED9, },
	{ 0x2C03E53E, 0xA2DAACB4, },
	{ 0xBF458F40, 0xD1216838, },
};

static NidResolverEntry sceUmd_nid[] = {
	{ 0xAE53DC2D, 0x6EDF57F1, }, // sceUmdClearDriveStatus
	{ 0x230666E3, 0x982272FE, }, // sceUmdSetDriveStatus
	{ 0x319ED97C, 0x72B1C5B5, }, // sceUmdUnRegisterActivateCallBack
	{ 0x086DDC0D, 0x9B0F59CE, }, // sceUmdRegisterActivateCallBack
	{ 0x4832ABF3, 0x3748C4DB, }, // sceUmdRegisterReplaceCallBack
	{ 0x4BA25F4A, 0x0E3F8ED9, }, // sceUmdUnRegisterReplaceCallBack
	{ 0xBD2BDE07, 0x598EC4DC, }, // sceUmdUnRegisterUMDCallBack
	{ 0xAEE7404D, 0x5EBB491F, }, // sceUmdRegisterUMDCallBack
	{ 0x46EBB729, 0xA9B5B972, }, // sceUmdCheckMedium
	{ 0x6AF9B50A, 0x18E225C8, }, // sceUmdCancelWaitDriveStat
	{ 0x87533940, 0x4F017CDE, }, // sceUmdReplaceProhibit
	{ 0xCBE9F02A, 0x0B14CE61, }, // sceUmdReplacePermit
	{ 0x3D0DECD5, 0xD2214D75, }, // sceUmdGetUserEventFlagId
	{ 0xD45D1FE6, 0xB7BF4C31, }, // sceUmdGetDriveStatus
	{ 0xD01B2DC6, 0x4A908DDE, }, // sceUmdGetAssignedFlag
	{ 0x3925CBD8, 0xCB297D67, }, // sceUmdSetAssignedFlag
	{ 0x9B22AED7, 0x07E98AF8, }, // sceUmdSetErrorStatus
	{ 0x71F81482, 0x666580EA, }, // sceUmdGetErrorStatus
	{ 0x7850F057, 0x48EF868C, }, // sceUmdRegisterGetUMDInfoCallBack
	{ 0x27A764A1, 0xA0E8E513, }, // sceUmdUnRegisterGetUMDInfoCallBack
	{ 0x2D81508D, 0xD1C80E51, }, // sceUmdRegisterDeactivateCallBack
	{ 0x56202973, 0x8DCFBA06, }, // sceUmdWaitDriveStatWithTimer
	{ 0x4A9E5E29, 0x98AFBD10, }, // sceUmdWaitDriveStatCB
	{ 0x075F1E0B, 0x060E934D, }, // sceUmdSetDetectUMDCallBackId
	{ 0x340B7686, 0xBA3D2A5F, }, // sceUmdGetDiscInfo
	{ 0xEB56097E, 0xA55109DD, }, // sceUmdGetDetectUMDCallBackId
	{ 0x6A41ED25, 0x899B5C41, }, // sceUmdGetSuspendResumeMode
	{ 0x4C952ACF, 0x816E656B, }, // sceUmdSetSuspendResumeMode
	{ 0xF2D0EDFB, 0xA9B5B972, }, // sceUmdCheckMedium

	{ 0x659587F7, 0x040A7090, },
	{ 0x38503380, 0x99CA645A, },
	{ 0xF8352373, 0x63517CBA, },
	{ 0x5469DC37, 0x1471F63D, },
};

static NidResolverEntry memlmd_nid[] = {
	{ 0x323366CA, 0x6192F715, },
	{ 0x1570BAB4, 0xF26A33C3, },
	{ 0x7CF1CD3E, 0xEF73E85B, },
};

static NidResolverEntry sceDdr_driver_nid[] = {
	{ 0x286E1372, 0xE0A39D3E, }, // sceDdrSetPowerDownCounter
	{ 0xB5694ECD, 0x77CD1FB3, }, // sceDdrExecSeqCmd
	{ 0x655A9AB0, 0x4F30BFE8, }, // sceDdrWriteMaxAllocate
	{ 0x0DC43DE4, 0x00E36648, }, // sceDdrGetPowerDownCounter
	{ 0xD7C6C313, 0x9F882141, }, // sceDdrSetup
	{ 0x91CD8F94, 0x397756C0, }, // sceDdrResetDevice
	{ 0x4CE55E76, 0xCF07B9E2, }, // sceDdrChangePllClock

	{ 0x6078F911, 0x95B9A692, },
	{ 0x5B5831E5, 0x3D50DEC9, },
	{ 0xF23B7983, 0x2AA39A80, },
	{ 0x6955346C, 0xF1005384, },
	{ 0x888129E8, 0x0BAAE4C5, },
	{ 0x7251F5AB, 0x6DE74D80, },
};

static NidResolverEntry sceDmacplus_driver_nid[] = {
	{ 0x29B50A82, 0xA3AA8D00, }, // sceDmacplusLcdcSetBaseAddr
	{ 0x8AE579CD, 0x88ACB6F1, }, // sceDmacplusLcdcSetFormat
	{ 0x241601AE, 0x33CD36DD, }, // sceDmacplusLcdcGetFormat
	{ 0xF8C7C180, 0xED849260, }, // sceDmacplusLcdcEnable
	{ 0x576522BC, 0xE9B746F9, }, // sceDmacplusLcdcDisable
	{ 0x3A98EE05, 0xBF0DB45E, }, // sceDmacplusAvcSync
	{ 0x4B980588, 0x9EBF7D3E, }, // sceDmacplusSc2MeNormal16
	{ 0x3438DA0B, 0x282CA0D7, }, // sceDmacplusSc2MeLLI
	{ 0x58DE4914, 0x47D400CB, }, // sceDmacplusSc2MeSync
	{ 0xBE693828, 0xDBD8526F, }, // sceDmacplusMe2ScNormal16
	{ 0xAB49D2CB, 0x5FCF43BD, }, // sceDmacplusMe2ScSync
	{ 0x2D5940FF, 0x6945F1D3, }, // sceDmacplusMe2ScLLI
	{ 0x28558DBF, 0xD57D5BC2, }, // sceDmacTryMemcpy
	{ 0xFD183BCF, 0xB0EC610D, }, // sceDmacplusSc128LLI
	{ 0x58C380BB, 0xCCCF6069, }, // sceDmacplusLcdcGetBaseAddr
	{ 0xC7D33466, 0x2C5FC1E3, }, // sceDmacMemcpy

	{ 0x1D73DDA8, 0xD126494B, },
	{ 0x74323807, 0x9C492B9B, },
	{ 0xB2EFA681, 0xB269EAC9, },
};

static NidResolverEntry sceGpio_driver_nid[] = {
	{ 0xCA8BE2EA, 0x2B280C43, }, // sceGpioGetPortMode
	{ 0x37C8DADC, 0xFBC85E74, }, // sceGpioSetIntrMode
	{ 0xF856CE46, 0xA21E44CB, }, // sceGpioGetIntrMode
	{ 0x95135905, 0x2FFF2F8B, }, // sceGpioPortInvert
	{ 0x785206CD, 0xEF1F40F5, }, // sceGpioEnableIntr
	{ 0x31F34AE6, 0x1BB49B65, }, // sceGpioQueryIntr
	{ 0xBE77D1D0, 0x1A730F20, }, // sceGpioAcquireIntr
	{ 0xC6928224, 0x4DE658CC, }, // sceGpioGetCapturePort
	{ 0x6B38B826, 0x9C4CFE04, }, // sceGpioSetCapturePort
	{ 0x5691CEFA, 0x2605F404, }, // sceGpioEnableTimerCapture
	{ 0x2CDC8EDC, 0x0C5B2EAF, }, // sceGpioDisableTimerCapture
};

static NidResolverEntry sceSysreg_driver_nid[] = {
	{ 0xC29D614E, 0x47C971B2, }, // sceSysregTopResetEnable
	{ 0xDC6139A4, 0x655C9CFC, }, // sceSysregScResetEnable
	{ 0xDE59DACB, 0x457FEBA9, }, // sceSysregMeResetEnable
	{ 0x2DB0EB28, 0x48F1C4AD, }, // sceSysregMeResetDisable
	{ 0x26283A6F, 0x66899952, }, // sceSysregAwResetEnable
	{ 0xA374195E, 0xAEB8DBD1, }, // sceSysregAwResetDisable
	{ 0xD20581EA, 0x1A27B224, }, // sceSysregVmeResetEnable
	{ 0x7558064A, 0xB73D3619, }, // sceSysregVmeResetDisable
	{ 0x9BB70D34, 0x0AE8E549, }, // sceSysregAvcResetEnable
	{ 0xFD6C562B, 0x55FF02E9, }, // sceSysregAvcResetDisable
	{ 0xCD4FB614, 0x30C0A141, }, // sceSysregUsbResetEnable
	{ 0x69EECBE5, 0x9306F27B, }, // sceSysregUsbResetDisable
	{ 0xF5B80837, 0x64C8E8DD, }, // sceSysregAtaResetEnable
	{ 0x8075303F, 0x8CFD0DCA, }, // sceSysregAtaResetDisable
	{ 0x00C2628E, 0x370419AD, }, // sceSysregMsifResetEnable
	{ 0xEC4BF81F, 0x7DD0CBEE, }, // sceSysregMsifResetDisable
	{ 0x8A7F9EB4, 0xC1A37B37, }, // sceSysregKirkResetEnable
	{ 0xC32F2491, 0x2F9B03E0, }, // sceSysregKirkResetDisable
	{ 0xA23BC2C4, 0x866EEB74, }, // sceSysregAtahddResetEnable
	{ 0xF5EA8570, 0x9EB8C49E, }, // sceSysregAtahddResetDisable
	{ 0x3E961C02, 0x4A433DC3, }, // sceSysregUsbhostResetEnable
	{ 0xACFA3764, 0x9B710D3C, }, // sceSysregUsbhostResetDisable
	{ 0x44F6CDA7, 0x38527743, }, // sceSysregMeBusClockEnable
	{ 0x158AD4FC, 0xC4C21CAB, }, // sceSysregMeBusClockDisable
	{ 0x4D4CE2B8, 0x51571E8F, }, // sceSysregAwRegABusClockEnable
	{ 0x789BD609, 0x52B74976, }, // sceSysregAwRegABusClockDisable
	{ 0x391CE1C0, 0x44277D0D, }, // sceSysregAwRegBBusClockEnable
	{ 0x82D35024, 0x7E1B1F28, }, // sceSysregAwRegBBusClockDisable
	{ 0xAF904657, 0xC2E0E869, }, // sceSysregAwEdramBusClockEnable
	{ 0x438AECE9, 0x258782A3, }, // sceSysregAwEdramBusClockDisable
	{ 0x4F5AFBBE, 0xBBC721EA, }, // sceSysregKirkBusClockEnable
	{ 0x845DD1A6, 0x867BD103, }, // sceSysregKirkBusClockDisable
	{ 0x3E216017, 0xD7AD9705, }, // sceSysregUsbBusClockEnable
	{ 0xBFBABB63, 0x1D233EF9, }, // sceSysregUsbBusClockDisable
	{ 0x4716E71E, 0x93F96D8F, }, // sceSysregMsifBusClockEnable
	{ 0x826430C0, 0xF288E58E, }, // sceSysregMsifBusClockDisable
	{ 0x7CC6CBFD, 0x0BBD4ED6, }, // sceSysregEmcddrBusClockEnable
	{ 0xEE6B9411, 0xF1924607, }, // sceSysregEmcddrBusClockDisable
	{ 0x63B9EB65, 0x191D7461, }, // sceSysregApbBusClockEnable
	{ 0xE1AA9788, 0x3D2CE374, }, // sceSysregApbBusClockDisable
	{ 0xAA63C8BD, 0x8B1DD83A, }, // sceSysregAudioBusClockEnable
	{ 0x054AC8C6, 0x85BA0C0B, }, // sceSysregAudioBusClockDisable
	{ 0xE45BED6A, 0x94B89638, }, // sceSysregAtahddBusClockEnable
	{ 0x681B35C4, 0x5F179286, }, // sceSysregAtahddBusClockDisable
	{ 0xE321F41A, 0x35C23493, }, // sceSysregUsbhostBusClockEnable
	{ 0x4D2FFC60, 0x4479C9BD, }, // sceSysregUsbhostBusClockDisable
	{ 0x6B01D71B, 0x6A9B0426, }, // sceSysregAtaClkEnable
	{ 0xFC186A83, 0xFC5CDD48, }, // sceSysregAtaClkDisable
	{ 0xB59DB832, 0x018F913A, }, // sceSysregAtahddClkEnable
	{ 0x9155812C, 0x55373EE4, }, // sceSysregAtahddClkDisable
	{ 0xDA4FCA1D, 0x08FE40F5, }, // sceSysregUsbhostClkEnable
	{ 0x228A73E1, 0xA781B599, }, // sceSysregUsbhostClkDisable
	{ 0x31154490, 0x4DB0C55D, }, // sceSysregMsifClkEnable
	{ 0x8E2D835D, 0x1E111B75, }, // sceSysregMsifClkDisable
	{ 0xE8533DCA, 0x74C6F776, }, // sceSysregApbTimerClkEnable
	{ 0xF6D83AD0, 0xDED12806, }, // sceSysregApbTimerClkDisable
	{ 0xA9CD1C1F, 0xB1751B06, }, // sceSysregAudioClkEnable
	{ 0x2F216F38, 0x092AF6A9, }, // sceSysregAudioClkDisable
	{ 0xE3AECFFA, 0x283D7A95, }, // sceSysregGpioClkEnable
	{ 0x3BBD0C0C, 0xA9EE3124, }, // sceSysregGpioClkDisable
	{ 0xC1DA05D2, 0xBE03D832, }, // sceSysregAudioClkoutClkEnable
	{ 0xDE170397, 0xD507A82D, }, // sceSysregAudioClkoutClkDisable
	{ 0x7234EA80, 0x84A279A4, }, // sceSysregUsbClkEnable
	{ 0x38EC3281, 0x6C0EE043, }, // sceSysregUsbClkDisable
	{ 0xBC68D9B6, 0x95CA8AA1, }, // sceSysregUsbIoEnable
	{ 0xA3C8E075, 0xC11B5C0D, }, // sceSysregUsbIoDisable
	{ 0x79338EA3, 0x3E95AB4D, }, // sceSysregAtaIoEnable
	{ 0xCADB92AA, 0xEB5C723A, }, // sceSysregAtaIoDisable
	{ 0xD74F1D48, 0x518E3F29, }, // sceSysregMsifIoEnable
	{ 0x18172C6A, 0x176E590A, }, // sceSysregMsifIoDisable
	{ 0xBB26CF1F, 0x4634C9DC, }, // sceSysregAudioIoEnable
	{ 0x8E2FB536, 0xC60FAFB4, }, // sceSysregAudioIoDisable
	{ 0xF844DDF3, 0x4B4CCE80, }, // sceSysregAudioClkoutIoEnable
	{ 0x29A119A1, 0xE5764EAC, }, // sceSysregAudioClkoutIoDisable
	{ 0x8B95C17F, 0x5D5118CD, }, // sceSysregAtahddIoEnable
	{ 0xCCF911C0, 0xF4A3C03A, }, // sceSysregAtahddIoDisable
	{ 0xB627582E, 0x37FBACA5, }, // sceSysregGpioIoEnable
	{ 0x1E9C3607, 0x15DC34BC, }, // sceSysregGpioIoDisable
	{ 0x633595F2, 0x72C1CA96, }, // sceSysregUsbGetConnectStatus
	{ 0x32471457, 0x9A6E7BB8, }, // sceSysregUsbQueryIntr
	{ 0x692F31FF, 0xEC03F6E2, }, // sceSysregUsbAcquireIntr
	{ 0xFFEB6E00, 0xD3E23912, }, // sceSysregUsbhostQueryIntr
	{ 0x87C2BA20, 0x915B3772, }, // sceSysregUsbhostAcquireIntr
	{ 0xD43E98F6, 0x20DF8278, }, // sceSysregMsifGetConnectStatus
	{ 0xBF91FBDA, 0xBB6BAA00, }, // sceSysregMsifQueryConnectIntr
	{ 0x36A75390, 0x56E95BB6, }, // sceSysregMsifAcquireConnectIntr
	{ 0x61BFF85F, 0x61FAE917, }, // sceSysregInterruptToOther
	{ 0x9FC87ED4, 0x0143E8A8, }, // sceSysregSemaTryLock
	{ 0x8BE2D520, 0xDB97C70E, }, // sceSysregSemaUnlock
	{ 0xB21B6CBF, 0xA5CC6025, }, // sceSysregPllGetFrequency
	{ 0xB4560C45, 0x187F651D, }, // sceSysregPllGetOutSelect
	{ 0xDCA57573, 0x19A6E54B, }, // sceSysregPllSetOutSelect
	{ 0x1969E840, 0x48124AFE, }, // sceSysregMsifClkSelect
	{ 0x1D382514, 0xD3E8F2AF, }, // sceSysregMsifDelaySelect
	{ 0x03340297, 0xA4706857, }, // sceSysregApbTimerClkSelect
	{ 0x9100B4E5, 0xC36775AD, }, // sceSysregAudioClkSelect
	{ 0x833E6FB1, 0x9057C9E2, }, // sceSysregAtaClkSelect
	{ 0x8CE2F97A, 0x48CF8E69, }, // sceSysregAtahddClkSelect
	{ 0x0A83FC7B, 0x41B0337B, }, // sceSysregAudioClkoutClkSelect
	{ 0x3EA188AD, 0xD8E6CAE0, }, // sceSysregRequestIntr
	{ 0xCD0F6715, 0xAFE47914, }, // sceSysregDoTimerEvent
	{ 0xF3443B9A, 0xAEC87DFD, }, // sceSysregSetAwEdramSize
	{ 0x53A6838B, 0x6B3A3417, }, // sceSysregPllGetBaseFrequency
	{ 0xBB3623DF, 0x231EE757, }, // sceSysregPllUpdateFrequency
	{ 0x25673620, 0x1E881843, }, // sceSysregIntrInit
	{ 0x4EE8E2C8, 0x7AA8A8BE, }, // sceSysregIntrEnd
	{ 0x083F56E2, 0xBE1FF8BD, }, // sceSysregEnableIntr
	{ 0x7C5B543C, 0xBD7B035B, }, // sceSysregDisableIntr
	{ 0xA9997109, 0x457FEBA9, }, // sceSysregMeResetEnable(371)
	{ 0x76220E94, 0x48F1C4AD, }, // sceSysregMeResetDisable(371)
	{ 0x3199CF1C, 0x38527743, }, // sceSysregMeBusClockEnable(371)
	{ 0x07881A0B, 0xC4C21CAB, }, // sceSysregMeBusClockDisable(371)

	{ 0x55B18B84, 0x33EE43F0, },
	{ 0x2112E686, 0x6DA9347D, },
	{ 0x7B9E9A53, 0x9E2F8FD5, },
	{ 0x7BDF0556, 0x4C0BED71, },
	{ 0x8D0FED1E, 0x1561BCD2, },
	{ 0xA46E9CA8, 0x9275DD37, },
	{ 0xB08A4E85, 0x1E62714E, },
	{ 0xA41929DA, 0x554E97F7, },
	{ 0xE88B77ED, 0x4841B2D2, },
	{ 0x312F9419, 0x4E5C86AA, },
	{ 0x844AF6BD, 0x9CD29D6C, },
	{ 0xEE8760C6, 0x20B1D0A9, },
	{ 0x5664F8B5, 0x3F6F2CC7, },
	{ 0x44704E1D, 0x2458B6AC, },
	{ 0x584AD989, 0x63E1EE9C, },
	{ 0x377F035F, 0x7CF05E81, },
	{ 0xAB3185FD, 0xB413B041, },
	{ 0x0EA487FA, 0x73EBC752, },
	{ 0x136E8F5A, 0x85B74FDA, },
	{ 0xF4811E00, 0x96D74557, },
};

static NidResolverEntry sceDve_driver_nid[] = {
	{ 0x0836B2B5, 0x94EF6EEE, }, // sceDvePowerOn
	{ 0x17BC42E4, 0x7FE2E002, }, // sceDvePowerOff
	{ 0xADE8C471, 0x3BC74BDB, }, // sceDveResetEnable
	{ 0xD986F31B, 0xC43BA75E, }, // sceDveResetDisable
	{ 0xB7827F16, 0xECA94FBE, }, // sceDveUnk1
	{ 0x34E9274D, 0x303E8022, }, // sceDveUnk2
	{ 0xE84E975E, 0x73E8E25F, }, // sceDveUnk3
	{ 0x0B85524C, 0xED631526, }, // sceDveUnk4
	{ 0xA265B504, 0x253B69B6, }, // sceDveUnk5
	{ 0xAE2A6570, 0x9A284796, }, // sceDveDisplayOn
	{ 0xD8863512, 0xF43F6F6A, }, // sceDveUnk10
	{ 0xA2D53F99, 0x6B2F9C64, }, // sceDveDisplayOff
	{ 0xF0CF5A67, 0xF00F2E74, }, // sceDveUnk12
	{ 0xBF910381, 0xDB309DD7, }, // sceDveUnk11
	{ 0x36970008, 0xA0129831, }, // sceDveUnk7
	{ 0x59E1B113, 0xDCCB9456, }, // sceDveUnk8
	{ 0xC9362C91, 0x68CB1669, }, // sceDveGetDisplayMode
	{ 0x22D50DA7, 0xD29DD200, }, // sceDveInit
	{ 0x292F15DA, 0x403B0135, }, // sceDveUnk9

	{ 0xB2E4E437, 0xCC146A6F, },
	{ 0xDEB2F80C, 0x48005974, },
	{ 0x93828323, 0x77832653, },
};

static NidResolverEntry sceHprm_driver_nid[] = {
	{ 0xC7154136, 0xDFC57C88, }, // sceHprmRegisterCallback
	{ 0xFD7DE6CD, 0xEB0CFCCC, }, // sceHprmUnregitserCallback
	{ 0x8EC787E0, 0xA0B1A19B, }, // sceHprmUpdateCableType
	{ 0xBAD0828E, 0x0B83352B, }, // sceHprmGetModel
	{ 0x4D1E622C, 0xC00DE13F, }, // sceHprmReset
	{ 0x2BCEC83E, 0x1F64B227, }, // sceHprmPeekLatch
	{ 0x40D2F9F0, 0xE9B776BE, }, // sceHprmReadLatch
	{ 0x7B038374, 0xB6895C1C, }, // sceHprmGetInternalState
	{ 0xF04591FA, 0x2F17A548, }, // sceHprmSetIdleCancelKey
	{ 0x971AE8FB, 0xD3F17366, }, // sceHprmGetIdleCancelKey
	{ 0x71B5FB67, 0x370DDC12, }, // sceHprmGetHpDetect
	{ 0x7E69EDA4, 0xFA4A25A7, }, // sceHprmIsHeadphoneExist
	{ 0x208DB1BD, 0xEFCFD0C5, }, // sceHprmIsRemoteExist
	{ 0x219C58F1, 0xAD158331, }, // sceHprmIsMicrophoneExist
	{ 0xC1777D84, 0xB2BEADB8, }, // sceHprmGetCableType
	{ 0xD22913DB, 0x8AEC8EE9, }, // sceHprmSetConnectCallback

	{ 0x9C1D9ADB, 0x301063CC, },
	{ 0xB6B6786B, 0x06C478D4, },
	{ 0x21E8AFD5, 0x602D5955, },
	{ 0x6BC7BEE9, 0x2903130A, },
	{ 0xC0FE0FAD, 0xDC895B2B, },
	{ 0x7B1A14B8, 0x8C728BB4, },
	{ 0x1528D408, 0xB2BEADB8, },
	{ 0x133DE8FA, 0x2E1A7B7C, },
};

static NidResolverEntry sceMesgLed_driver_nid[] = {
	{ 0xDFF0F308, 0x2CB700EC, },
};

static NidResolverEntry sceClockgen_driver_nid[] = {
	{ 0x4EB657D5, 0xDAB6E612, }, // sceClockgenAudioClkSetFreq
};

static NidResolverEntry sceCodec_driver_nid[] = {
	{ 0x20C61103, 0xE4456BC3, }, // sceCodecSelectVolumeTable
	{ 0xEEB91526, 0xD27707A8, }, // sceCodecSetVolumeOffset
	{ 0x6D945509, 0x49C13ACF, }, // sceCodecSetHeadphoneVolume
	{ 0x40D5C897, 0xEACF7284, }, // sceCodecSetSpeakerVolume
};

static NidResolverEntry scePaf_nid[] = {
	{ 0x2BE8DDBB, 0xEC805E95, },
	{ 0xE8CCC611, 0x95692855, },
	{ 0xCDDCFFB3, 0xC5C17E46, },
	{ 0x48BB05D5, 0x5E909060, },
	{ 0x22FB4177, 0x412B2F09, },
	{ 0xBC8DC92B, 0xD590412B, },
	{ 0xE3D530AE, 0x4CF09BA2, },
};

NidResolverLib nidTable660[] = {
	NID_ENTRY(SysMemForKernel),
	NID_ENTRY(KDebugForKernel),
	NID_ENTRY(LoadCoreForKernel),
	NID_ENTRY(LoadExecForKernel),
	NID_ENTRY(ModuleMgrForKernel),
	NID_ENTRY(ExceptionManagerForKernel),
	NID_ENTRY(IoFileMgrForKernel),
	NID_ENTRY(scePower_driver),
	NID_ENTRY(InterruptManagerForKernel),
	NID_ENTRY(sceReg_driver),
	NID_ENTRY(sceCtrl_driver),
	NID_ENTRY(sceDisplay_driver),
	NID_ENTRY(sceAudio_driver),
	NID_ENTRY(SysTimerForKernel),
	NID_ENTRY(sceImpose_driver),
	NID_ENTRY(sceGe_driver),
	NID_ENTRY(sceRtc_driver),
	NID_ENTRY(memlmd),
	NID_ENTRY(sceSyscon_driver),
	NID_ENTRY(sceUmd),
	NID_ENTRY(sceDdr_driver),
	NID_ENTRY(sceDmacplus_driver),
	NID_ENTRY(sceGpio_driver),
	NID_ENTRY(sceSysreg_driver),
	NID_ENTRY(sceDve_driver),
	NID_ENTRY(sceHprm_driver),
	NID_ENTRY(sceMesgLed_driver),
	NID_ENTRY(sceClockgen_driver),
	NID_ENTRY(sceCodec_driver),
	NID_ENTRY(scePaf),
};

unsigned int nidTableSize660 = NELEMS(nidTable660);

