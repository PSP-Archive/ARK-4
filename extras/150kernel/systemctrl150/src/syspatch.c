#include <pspkernel.h>
#include <pspreg.h>
#include <pspsysevent.h>
#include <systemctrl.h>
#include <macros.h>

int sceDisplaySetBrightnessPatched(int level, int unk1);
int SysEventHandler(int eventId, char *eventName, void *param, int *result);

int last_br;
int last_unk;
int (* GetMsSize)(void);

extern void* custom_rebootex;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

PspSysEventHandler sysEventHandler =
{
    .size = sizeof(PspSysEventHandler),
    .name = "",
    .type_mask = 0x0000FF00,
    .handler = SysEventHandler
};

int ValidateSeekPatched(u32 *drv_str, SceOff ofs)
{
    if ((ofs & 0x1FF))
        return 0;

    SceOff max_size = GetMsSize(); // size of partition in sectors
    max_size *= 512; // size in bytes

    if (ofs >= max_size)
        return 0;

    return 1;
}

int ValidateSeekP1Patched(u32 *drv_str, SceOff ofs)
{
    if ((ofs & 0x1FF))
        return 0;

    u32 *p = (u32 *)drv_str[0x10/4];
    u32 *q = (u32 *)p[8/4];
    SceOff max_size = (SceOff)q[4/4]; // partition_start

    max_size = GetMsSize() - max_size; // size of partition in sectors
    max_size *= 512; // size in bytes

    if (ofs >= max_size)
        return 0;

    return 1;
}

REGHANDLE lang_hk = -1;

int sceRegGetKeyValuePatched(REGHANDLE hd, REGHANDLE hk, void *buffer, SceSize size)
{
    int res = sceRegGetKeyValue(hd, hk, buffer, size);
    if (res >= 0 && hk == lang_hk)
    {
    	if (*(u32 *)buffer > 8)
    		*(u32 *)buffer = 1;
    	
    	lang_hk = -1;
    }
    return res;
}

int sceRegGetKeyInfoPatched(REGHANDLE hd, const char *name, REGHANDLE *hk, unsigned int *type, SceSize *size)
{
    int res = sceRegGetKeyInfo(hd, name, hk, type, size);

    if (res >= 0 && strcmp(name, "language") == 0)
    {
    	if (hk)
    		lang_hk = *hk;
    }

    return res;
}

int sceDisplaySetBrightnessPatched(int level, int unk1)
{
    last_br = level;
    last_unk = unk1;

    if (level < 100)
    {
    	if (level >= 70)
    		level = 100 - level;
    	else if (level >= 35)
    		level = 100 - level - 5;
    	else
    		level = 100 - level - 10;
    }
    else if (level == 100)
    {
    	level = 1;
    }

    return sceDisplaySetBrightness(level, unk1);
}

// Module Start Handler
static void ARKSyspatchOnModuleStart(SceModule2 * mod)
{
    char *moduleName = mod->modname;
    u32 text_addr = mod->text_addr;

    if (strcmp(moduleName, "sceRegistry_Service") == 0)
    {
    	_sw((u32)sceRegGetKeyInfoPatched, text_addr+0x76DC);
    	_sw((u32)sceRegGetKeyValuePatched, text_addr+0x76E0);

    	flushCache();
    }
    else if (strcmp(moduleName, "scePower_Service") == 0)
    {
    	_sw(0x00e02021, text_addr+0x558); //ADDU $a0 $a3 $zero
    	flushCache();
    }
    else if (strcmp(moduleName, "sceDisplay_Service") == 0)
    {
    	int lcd;
    	if (sceIdStorageLookup(8, 0, &lcd, 4) >= 0 && lcd == 0x4C434470)
    	{
    		_sw((u32)sceDisplaySetBrightnessPatched, text_addr + 0x2858);
    		flushCache();
    	}
    }
    else if (strcmp(moduleName, "sceMSstor_Driver") == 0)
    {
    	REDIRECT_FUNCTION(text_addr + 0x5138, ValidateSeekPatched);
    	REDIRECT_FUNCTION(text_addr + 0x51bc, ValidateSeekP1Patched);
    	GetMsSize = (void *)(text_addr + 0x0288);

    	flushCache();
    }
    else if (strcmp(moduleName, "sceLoadExec") == 0)
    {
    	patchLoadExec();
    	flushCache();
    }
}

// Add Module Start Patcher
void syspatchInit(void)
{
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(ARKSyspatchOnModuleStart);
    sceKernelRegisterSysEventHandler(&sysEventHandler);
}

int SysEventHandler(int eventId, char *eventName, void *param, int *result)
{
    if (eventId == 0x1000B)
    {
    	if (last_br == 100)
    		sceDisplaySetBrightnessPatched(last_br, last_unk);
    }

    return 0;
}
