#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pspkernel.h>
#include <pspsysevent.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>

#include <systemctrl.h>
#include <macros.h>

#include "flashemu.h"
#include "rebootex/payload.h"

PSP_MODULE_INFO("TimeMachine_Control", PSP_MODULE_KERNEL | PSP_MODULE_SINGLE_START | PSP_MODULE_SINGLE_LOAD | PSP_MODULE_NO_STOP, 1, 0);

int SysEventHandler(int eventId, char *eventName, void *param, int *result);

PspSysEventHandler sysEventHandler =
	{
		.size = sizeof(PspSysEventHandler),
		.name = "",
		.type_mask = 0x00FFFF00,
		.handler = SysEventHandler};

extern SceUID flashemu_sema;
extern int msNotReady;
extern FileHandler file_handler[MAX_FILES];

extern int df_dopenPatched(int type, void * cb, void *arg);
extern int df_openPatched(int type, void * cb, void *arg);
extern int df_devctlPatched(int type, void *cb, void *arg);

STMOD_HANDLER previous;

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheInvalidateAll();
}

int codePagesceIoOpenPatched(const char *file, int flags, SceMode mode)
{
	SceModule2 *mod = (SceModule2 *)sceKernelFindModuleByName("vsh_module");

	if (!mod)
		return 0x80010018;

	return sceIoOpen(file, flags, mode);
}

void OnModuleStart(SceModule2 *mod)
{
	char *moduleName = mod->modname;

	if (strcmp(moduleName, "sceUtility_Driver") == 0)
	{
		SceModule2 *mod2 = (SceModule2 *)sceKernelFindModuleByName("sceMSFAT_Driver");

		MAKE_CALL(mod2->text_addr + 0x30fc, df_openPatched);
		MAKE_CALL(mod2->text_addr + 0x3ba4, df_dopenPatched);
		MAKE_CALL(mod2->text_addr + 0x44cc, df_devctlPatched);

		ClearCaches();
	}
	else if (strcmp(moduleName, "sceLflashFatfmt") == 0)
	{
		u32 funcAddr = sctrlHENFindFunction("sceLflashFatfmt", "LflashFatfmt", 0xb7a424a4); // sceLflashFatfmtStartFatfmt
		if (funcAddr)
		{
			MAKE_DUMMY_FUNCTION_RETURN_0(funcAddr);
			ClearCaches();
		}
	}
	else if (strcmp(moduleName, "sceCodepage_Service") == 0)
	{
		hookImportByNID(mod, "IoFileMgrForKernel", 0x109f50bc, codePagesceIoOpenPatched);

		ClearCaches();
	}
	else if (strcmp(moduleName, "sceMediaSync") == 0)
	{
		//TODO make this patch dynamic
		MAKE_DUMMY_FUNCTION_RETURN_0(mod->text_addr + 0x135c);

		ClearCaches();
	}

	if (previous) previous(mod);
}

int module_start(SceSize args, void *argp)
{
	InstallFlashEmu();
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	sctrlHENSetRebootexOverride(rebootbuffer_ms_psp);
	ClearCaches();

	return 0;
}

int module_reboot_before(SceSize args, void *argp)
{
	SceUInt timeout = 500000;
	sceKernelWaitSema(flashemu_sema, 1, &timeout);
	sceKernelDeleteSema(flashemu_sema);
	sceIoUnassign("flash0:");
	sceIoUnassign("flash1:");
	sceIoUnassign("flash2:");
	sceIoUnassign("flash3:");
	sceKernelUnregisterSysEventHandler(&sysEventHandler);

	return 0;
}

int SysEventHandler(int eventId, char *eventName, void *param, int *result)
{
	if (eventId == 0x4000) //suspend
	{
		int i;
		for(i = 0; i < MAX_FILES; i++)
		{
			if(file_handler[i].opened && file_handler[i].unk_8 == 0 && file_handler[i].flags != DIR_FLAG)
			{
				file_handler[i].offset = sceIoLseek(file_handler[i].fd, 0, PSP_SEEK_CUR);
				file_handler[i].unk_8 = 1;
				sceIoClose(file_handler[i].fd);
			}
		}
	}
	else if (eventId == 0x10009) // resume
		msNotReady = 1;
	return 0;
}
