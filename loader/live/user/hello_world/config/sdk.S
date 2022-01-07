/*  Generated with SDKCreator 1.0 by GUIDOBOT  */

.macro AddNID funcname, nid
	.globl  \funcname
	.ent    \funcname
\funcname = \nid
	.end    \funcname

.endm

	.file	1 "sdk.c"
	.section .mdebug.eabi32
	.section .gcc_compiled_long32
	.previous
	.text
	.align	2


	AddNID module_start, 0x08897C6C
	AddNID module_stop, 0x08898074
	AddNID module_info, 0x08925E98
	AddNID syslib_11B97506, 0x08926310
	AddNID module_start_thread_parameter, 0x0892E878
	AddNID module_stop_thread_parameter, 0x0892E888
	AddNID sceUtilityLoadModule, 0x08925C74
	AddNID sceUtilitySavedataInitStart, 0x08925C7C
	AddNID sceUtilitySavedataGetStatus, 0x08925C84
	AddNID sceUtilitySavedataShutdownStart, 0x08925C8C
	AddNID sceUtilityGetSystemParamInt, 0x08925C94
	AddNID sceUtilitySavedataUpdate, 0x08925C9C
	AddNID sceUtilityUnloadModule, 0x08925CA4
	AddNID sceCtrlSetSamplingMode, 0x08925C54
	AddNID sceCtrlReadBufferPositive, 0x08925C5C
	AddNID sceCtrlSetSamplingCycle, 0x08925C64
	AddNID sceCtrlSetIdleCancelThreshold, 0x08925C6C
	AddNID sceDisplaySetMode, 0x08925C34
	AddNID sceDisplaySetFrameBuf, 0x08925C3C
	AddNID sceDisplayWaitVblankStart, 0x08925C44
	AddNID sceDisplayGetVcount, 0x08925C4C
	AddNID sceGeListSync, 0x08925BCC
	AddNID sceGeUnsetCallback, 0x08925BD4
	AddNID sceGeRestoreContext, 0x08925BDC
	AddNID sceGeListEnQueueHead, 0x08925BE4
	AddNID sceGeSaveContext, 0x08925BEC
	AddNID sceGeContinue, 0x08925BF4
	AddNID sceGeSetCallback, 0x08925BFC
	AddNID sceGeListEnQueue, 0x08925C04
	AddNID sceGeDrawSync, 0x08925C0C
	AddNID sceGeEdramSetAddrTranslation, 0x08925C14
	AddNID sceGeGetCmd, 0x08925C1C
	AddNID sceGeListUpdateStallAddr, 0x08925C24
	AddNID sceGeEdramGetAddr, 0x08925C2C
	AddNID scePowerRegisterCallback, 0x08925BBC
	AddNID scePower_469989AD, 0x08925BC4
	AddNID sceIoWrite, 0x08925B74
	AddNID sceIoRead, 0x08925B7C
	AddNID sceIoRename, 0x08925B84
	AddNID sceIoClose, 0x08925B8C
	AddNID sceIoReadAsync, 0x08925B94
	AddNID sceIoWaitAsync, 0x08925B9C
	AddNID sceIoOpen, 0x08925BA4
	AddNID sceIoLseek, 0x08925BAC
	AddNID sceIoPollAsync, 0x08925BB4
	AddNID sceKernelCpuSuspendIntr, 0x08925B4C
	AddNID Kernel_Library_293B45B8, 0x08925B54
	AddNID sceKernelCpuResumeIntr, 0x08925B5C
	AddNID Kernel_Library_A089ECA4, 0x08925B64
	AddNID Kernel_Library_1839852A, 0x08925B6C
	AddNID sceKernelExitGame, 0x08925B3C
	AddNID sceKernelRegisterExitCallback, 0x08925B44
	AddNID sceKernelUnloadModule, 0x08925B04
	AddNID sceKernelStopModule, 0x08925B0C
	AddNID sceKernelGetModuleIdByAddress, 0x08925B14
	AddNID sceKernelGetModuleId, 0x08925B1C
	AddNID sceKernelStartModule, 0x08925B24
	AddNID ModuleMgrForUser_8F2DF740, 0x08925B2C
	AddNID sceKernelLoadModule, 0x08925B34
	AddNID sceKernelStdin, 0x08925AEC
	AddNID sceKernelStdout, 0x08925AF4
	AddNID sceKernelStderr, 0x08925AFC
	AddNID sceKernelPrintf, 0x08925ABC
	AddNID sceKernelSetCompilerVersion, 0x08925AC4
	AddNID SysMemUserForUser_1B4217BC, 0x08925ACC
	AddNID sceKernelAllocPartitionMemory, 0x08925AD4
	AddNID sceKernelGetBlockHeadAddr, 0x08925ADC
	AddNID sceKernelFreePartitionMemory, 0x08925AE4
	AddNID sceKernelDelayThread, 0x08925A0C
	AddNID sceKernelCreateSema, 0x08925A14
	AddNID sceKernelCreateCallback, 0x08925A1C
	AddNID sceKernelChangeCurrentThreadAttr, 0x08925A24
	AddNID sceKernelDeleteCallback, 0x08925A2C
	AddNID sceKernelStartThread, 0x08925A34
	AddNID sceKernelDeleteSema, 0x08925A3C
	AddNID sceKernelCheckCallback, 0x08925A44
	AddNID sceKernelGetSystemTimeLow, 0x08925A4C
	AddNID sceKernelSignalSema, 0x08925A54
	AddNID sceKernelCreateThread, 0x08925A5C
	AddNID sceKernelWaitSema, 0x08925A64
	AddNID sceKernelTerminateThread, 0x08925A6C
	AddNID sceKernelDelayThreadCB, 0x08925A74
	AddNID sceKernelExitDeleteThread, 0x08925A7C
	AddNID sceKernelDeleteThread, 0x08925A84
	AddNID sceKernelExitThread, 0x08925A8C
	AddNID sceKernelSetEventFlag, 0x08925A94
	AddNID sceKernelDeleteEventFlag, 0x08925A9C
	AddNID sceKernelWaitThreadEnd, 0x08925AA4
	AddNID sceKernelWaitEventFlag, 0x08925AAC
	AddNID sceKernelCreateEventFlag, 0x08925AB4
	AddNID sceKernelLibcGettimeofday, 0x089259EC
	AddNID sceKernelDcacheWritebackAll, 0x089259F4
	AddNID sceKernelLibcClock, 0x089259FC
	AddNID sceKernelLibcTime, 0x08925A04
	AddNID sceAudioOutputBlocking, 0x089259A4
	AddNID sceAudioOutputPannedBlocking, 0x089259AC
	AddNID sceAudioChReserve, 0x089259B4
	AddNID sceAudioChRelease, 0x089259BC
	AddNID sceAudioChangeChannelConfig, 0x089259C4
	AddNID sceAudioGetChannelRestLength, 0x089259CC
	AddNID sceAudioChangeChannelVolume, 0x089259D4
	AddNID sceAudioSetChannelDataLen, 0x089259DC
	AddNID sceAudioOutputPanned, 0x089259E4
	AddNID sceAtracGetChannel, 0x08925964
	AddNID sceAtracGetStreamDataInfo, 0x0892596C
	AddNID sceAtracReleaseAtracID, 0x08925974
	AddNID sceAtracDecodeData, 0x0892597C
	AddNID sceAtracSetDataAndGetID, 0x08925984
	AddNID sceAtracAddStreamData, 0x0892598C
	AddNID sceAtracSetLoopNum, 0x08925994
	AddNID sceAtracGetMaxSample, 0x0892599C
	AddNID __sceSasSetADSR, 0x0892588C
	AddNID __sceSasGetAllEnvelopeHeights, 0x08925894
	AddNID __sceSasRevParam, 0x0892589C
	AddNID __sceSasGetPauseFlag, 0x089258A4
	AddNID __sceSasRevType, 0x089258AC
	AddNID __sceSasInit, 0x089258B4
	AddNID __sceSasSetVolume, 0x089258BC
	AddNID __sceSasCoreWithMix, 0x089258C4
	AddNID __sceSasSetSL, 0x089258CC
	AddNID __sceSasGetEndFlag, 0x089258D4
	AddNID __sceSasGetEnvelopeHeight, 0x089258DC
	AddNID __sceSasSetKeyOn, 0x089258E4
	AddNID __sceSasSetPause, 0x089258EC
	AddNID __sceSasSetVoice, 0x089258F4
	AddNID __sceSasSetADSRmode, 0x089258FC
	AddNID __sceSasSetKeyOff, 0x08925904
	AddNID __sceSasCore, 0x0892590C
	AddNID __sceSasSetPitch, 0x08925914
	AddNID __sceSasSetNoise, 0x0892591C
	AddNID __sceSasGetGrain, 0x08925924
	AddNID __sceSasSetSimpleADSR, 0x0892592C
	AddNID __sceSasSetGrain, 0x08925934
	AddNID __sceSasRevEVOL, 0x0892593C
	AddNID __sceSasGetOutputmode, 0x08925944
	AddNID __sceSasSetVoicePCM, 0x0892594C
	AddNID __sceSasSetOutputmode, 0x08925954
	AddNID __sceSasRevVON, 0x0892595C
	AddNID sceUmdCheckMedium, 0x08925864
	AddNID sceUmdGetDriveStat, 0x0892586C
	AddNID sceUmdWaitDriveStat, 0x08925874
	AddNID sceUmdRegisterUMDCallBack, 0x0892587C
	AddNID sceUmdActivate, 0x08925884
	AddNID sceRtcGetCurrentTick, 0x08925854
	AddNID sceRtcTickAddSeconds, 0x0892585C
	AddNID sceWlanGetEtherAddr, 0x0892584C
	AddNID sceNetGetLocalEtherAddr, 0x0892582C
	AddNID sceNetTerm, 0x08925834
	AddNID sceNetInit, 0x0892583C
	AddNID sceNetEtherNtostr, 0x08925844
	AddNID sceNetAdhocPdpCreate, 0x089257FC
	AddNID sceNetAdhocPdpDelete, 0x08925804
	AddNID sceNetAdhocTerm, 0x0892580C
	AddNID sceNetAdhocPdpSend, 0x08925814
	AddNID sceNetAdhocPdpRecv, 0x0892581C
	AddNID sceNetAdhocInit, 0x08925824
	AddNID sceNetAdhocctlConnect, 0x089257CC
	AddNID sceNetAdhocctlAddHandler, 0x089257D4
	AddNID sceNetAdhocctlDisconnect, 0x089257DC
	AddNID sceNetAdhocctlDelHandler, 0x089257E4
	AddNID sceNetAdhocctlTerm, 0x089257EC
	AddNID sceNetAdhocctlInit, 0x089257F4
	AddNID sceImposeSetLanguageMode, 0x089257C4


	.ident	"VAL-SDK"