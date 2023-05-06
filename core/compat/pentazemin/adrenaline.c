/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include "binary.h"

#define ClearCaches flushCache
#define FindProc sctrlHENFindFunction

typedef struct {
	void *sasCore;
	int grainSamples;
	int maxVoices;
	int outMode;
	int sampleRate;
} SasInitArguments;

SasInitArguments sas_args;
int sas_inited = 0;

SceAdrenaline *adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;

SceUID adrenaline_semaid = -1;

int (* _scePowerSuspendOperation)();

int (* SetFlag1)();
int (* SetFlag2)();
int (* sceKermitSyncDisplay)();

int (* uiResumePoint)(u32 *data);
void (* VitaSync)();

int (* sceSasCoreInit)();
int (* sceSasCoreExit)();

int (* __sceSasInit)(void *sasCore, int grainSamples, int maxVoices, int outMode, int sampleRate);

int SendAdrenalineCmd(int cmd) {
	int k1 = pspSdkSetK1(0);

	char buf[sizeof(SceKermitRequest) + 0x40];
	SceKermitRequest *request_aligned = (SceKermitRequest *)ALIGN((u32)buf, 0x40);
	SceKermitRequest *request_uncached = (SceKermitRequest *)((u32)request_aligned | 0x20000000);
	sceKernelDcacheInvalidateRange(request_aligned, sizeof(SceKermitRequest));

	u64 resp;
	sceKermitSendRequest661(request_uncached, KERMIT_MODE_EXTRA_2, cmd, 0, 0, &resp);

	pspSdkSetK1(k1);
	return resp;
}

int getSfoTitle(char *title, int n) {
	return sctrlGetInitPARAM("TITLE", NULL, &n, title);
}

void initAdrenalineInfo() {
	memset(adrenaline, 0, sizeof(SceAdrenaline));

	int keyconfig = sceKernelInitKeyConfig();
	if (keyconfig == PSP_INIT_KEYCONFIG_GAME) {
		getSfoTitle(adrenaline->title, 128);
	} else if (keyconfig == PSP_INIT_KEYCONFIG_POPS) {
		getSfoTitle(adrenaline->title, 128);
		adrenaline->pops_mode = 1;
	} else if (keyconfig == PSP_INIT_KEYCONFIG_VSH) {
		strcpy(adrenaline->title, "XMB\xE2\x84\xA2");
	} else {
		strcpy(adrenaline->title, "Unknown");
	}

	void *game_info = sceKernelGetGameInfo661();
	if (game_info)
		strcpy(adrenaline->titleid, game_info + 0x44);

	char *filename = sceKernelInitFileName();
	if (filename)
		strcpy(adrenaline->filename, filename);
}

int adrenaline_interrupt() {
	// Signal adrenaline semaphore
	sceKernelSignalSema(adrenaline_semaid, 1);
	return 0;
}

int adrenaline_thread(SceSize args, void *argp) {
	while (1) {
		// Wait for semaphore signal
		sceKernelWaitSema(adrenaline_semaid, 1, NULL);

		switch (adrenaline->psp_cmd) {
			case ADRENALINE_PSP_CMD_REINSERT_MS:
				sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0);
				break;
				
			case ADRENALINE_PSP_CMD_SAVESTATE:
				adrenaline->savestate_mode = SAVESTATE_MODE_SAVE;
				_scePowerSuspendOperation(0x202);
				break;
				
			case ADRENALINE_PSP_CMD_LOADSTATE:
				adrenaline->savestate_mode = SAVESTATE_MODE_LOAD;
				_scePowerSuspendOperation(0x202);
				break;
		}
	}

	return 0;
}

int __sceSasInitPatched(void *sasCore, int grainSamples, int maxVoices, int outMode, int sampleRate) {
	sas_args.sasCore = sasCore;
	sas_args.grainSamples = grainSamples;
	sas_args.maxVoices = maxVoices;
	sas_args.outMode = outMode;
	sas_args.sampleRate = sampleRate;

	sas_inited = 1;

	return __sceSasInit(sasCore, grainSamples, maxVoices, outMode, sampleRate);
}

void ReInitSasCore() {
	if (__sceSasInit && sas_inited) {
		sceSasCoreExit();
		sceSasCoreInit();
		__sceSasInit(sas_args.sasCore, sas_args.grainSamples, sas_args.maxVoices, sas_args.outMode, sas_args.sampleRate);
	}
}

int SysEventHandler(int ev_id, char *ev_name, void *param, int *result) {
	// Resume completed
	if (ev_id == 0x400000) {
		if (adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
			adrenaline->savestate_mode = SAVESTATE_MODE_NONE;
			ReInitSasCore();
			
			if (adrenaline->pops_mode) {
				int (* sceKermitPeripheralInitPops)() = (void *)FindProc("sceKermitPeripheral_Driver", "sceKermitPeripheral", 0xC0EBC631);
				if (sceKermitPeripheralInitPops)
					sceKermitPeripheralInitPops();
			}
		}
	}

	return 0;
}

void VitaSyncPatched() {
	if (adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		void (* SaveStateBinary)() = (void *)0x00010000;
		memcpy((void *)SaveStateBinary, binary, size_binary);
		ClearCaches();

		SaveStateBinary();

		// Param for uiResumePoint
		u32 data[53];
		memset(data, 0, sizeof(data));
		data[0] = sizeof(data);
		data[8] = 0xFFFF;
		data[9] = 0x2;
		data[12] = 0x4B0;
		uiResumePoint(data);

		while(1);
	}

	VitaSync();
}

int SetFlag1Patched() {
	if (adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		return 0;
	}
	
	return SetFlag1();
}

int SetFlag2Patched() {
	if (adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		return 0;
	}
	
	return SetFlag2();
}

int sceKermitSyncDisplayPatched() {
	if (adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		return 0;
	}
	
	return sceKermitSyncDisplay();
}

void PatchSasCore() {
	sceSasCoreInit = (void *)FindProc("sceSAScore", "sceSasCore_driver", 0xB0F9F98F);
	sceSasCoreExit = (void *)FindProc("sceSAScore", "sceSasCore_driver", 0xE143A1EA);

	HIJACK_FUNCTION(FindProc("sceSAScore", "sceSasCore", 0x42778A9F), __sceSasInitPatched, __sceSasInit);

	ClearCaches();
}

void PatchLowIODriver2(u32 text_addr) {
	HIJACK_FUNCTION(text_addr + 0x880, SetFlag1Patched, SetFlag1);
	HIJACK_FUNCTION(text_addr + 0xCD8, SetFlag2Patched, SetFlag2);
	HIJACK_FUNCTION(FindProc("sceKermit_Driver", "sceKermit_driver", 0xD69C50BB), sceKermitSyncDisplayPatched, sceKermitSyncDisplay);
	ClearCaches();
}

void PatchPowerService2(u32 text_addr) {
	// Patch to inject binary and to call uiResumePoint
	uiResumePoint = (void *)text_addr + 0x24C0;
	K_HIJACK_CALL(text_addr + 0x22FC, VitaSyncPatched, VitaSync);

	_scePowerSuspendOperation = (void *)text_addr + 0x1710;
}

int initAdrenaline() {
	// Register sysevent handler
	static PspSysEventHandler event_handler = {
		sizeof(PspSysEventHandler),
		"",
		0x00FFFF00,
		SysEventHandler
	};

	sceKernelRegisterSysEventHandler(&event_handler);

	// Register adrenaline interrupt
	sceKermitRegisterVirtualIntrHandler661(KERMIT_VIRTUAL_INTR_IMPOSE_CH1, adrenaline_interrupt);

	// Create adrenaline semaphore
	adrenaline_semaid = sceKernelCreateSema("", 0, 0, 1, NULL);
	if (adrenaline_semaid < 0)
		return adrenaline_semaid;

	// Create and start adrenaline thread
	SceUID thid = sceKernelCreateThread("adrenaline_thread", adrenaline_thread, 0x10, 0x4000, 0, NULL);
	if (thid < 0)
		return thid;

	sceKernelStartThread(thid, 0, NULL);

	*(u32 *)DRAW_NATIVE = 0;

	return 0;
}
