#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>
#include <psploadcore.h>
#include <pspiofilemgr_kernel.h>
#include <pspsyscon.h>
#include <pspnand_driver.h>
#include <pspidstorage.h>
#include <pspdisplay_kernel.h>
#include <systemctrl.h>
#include <pspidstorage.h>
#include <pspnand_driver.h>
#include <pspsysreg.h>
#include <pspidstorage.h>
#include <pspthreadman_kernel.h>
#include <pspcrypt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcman.h"

PSP_MODULE_INFO("DesCemManager", 0x1007, 1, 0);

#define JAL_OPCODE    0x0C000000
#define J_OPCODE    0x08000000
#define SC_OPCODE    0x0000000C
#define JR_RA        0x03e00008

#define NOP    0x00000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 
#define MAKE_SYSCALL(a, n) _sw(SC_OPCODE | (n << 6), a);
#define JUMP_TARGET(x) (0x80000000 | ((x & 0x03FFFFFF) << 2))

#define REDIRECT_FUNCTION(a, f) _sw(J_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a);  _sw(NOP, a+4);
#define MAKE_DUMMY_FUNCTION0(a) _sw(0x03e00008, a); _sw(0x00001021, a+4);
#define MAKE_DUMMY_FUNCTION1(a) _sw(0x03e00008, a); _sw(0x24020001, a+4);

u32 tachyon, baryon, pommel, mb, fuseconfig, nandsize;
u64 fuseid;
int totalblocks;
SceUID phformat_cb = -1;

u8 mac[6];
int mac_obtained;

int cancel_mode;
STMOD_HANDLER previous;

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    sceKernelIcacheClearAll();
}

static void GetHardwareInfo()
{
    tachyon = 0;
    baryon = 0;
    pommel = 0;
    mb = UNKNOWN;

    u32 (*SysregGetTachyonVersion)() = sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0xE2A5D1EE);
    u64 (*SysregGetFuseId)() = sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0x4F46EEDE);
    u32 (*SysregGetFuseConfig)() = sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0x8F4F4E96);
    u32 (*SysconGetBaryonVersion)(u32*) = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
    u32 (*SysconGetPommelVersion)(u32*) = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0xE7E87741);

    tachyon = SysregGetTachyonVersion();
    
    if (SysconGetBaryonVersion(&baryon) < 0)
        baryon = 0xDADADADA;

    if (SysconGetPommelVersion(&pommel) < 0)
        pommel = 0xDADADADA;
    
    switch (tachyon)
    {
        case 0x00140000:
        	switch(baryon)
        	{
        		case 0x00010600: mb = TA_079v1; break;
        		case 0x00010601: mb = TMU_001v1; break;
        		case 0x00020600: mb = TA_079v2; break;
        		case 0x00020601: mb = TMU_001v2; break;
        		case 0x00030600: mb = TA_079v3; break;
        		case 0x00030601: mb = TMU_002; break;
        	}
        	break;

        case 0x00200000:
        	switch(baryon)
        	{
        		case 0x00030600: mb = TA_079v4; break;
        		case 0x00040600: mb = TA_079v5; break;
        	}
        	break;

        case 0x00300000:
        	switch(baryon)
        	{
        		case 0x00040600:
        			switch(pommel)
        			{
        				case 0x00000103: mb = TA_081v1; break;
        				case 0x00000104: mb = TA_081v2; break;
        			}
        			break;
        	}
        	break;

        case 0x00400000:
        	switch(baryon)
        	{
        		case 0x00114000: mb = TA_082; break;
        		case 0x00121000: mb = TA_086; break;
        	}
        	break;

        case 0x00500000:
        	switch(baryon)
        	{
        		case 0x0022B200: mb = TA_085v1; break;
        		case 0x00234000: mb = TA_085v2; break;
        		case 0x00243000:
        			switch(pommel)
        			{
        				case 0x00000123: mb = TA_088v1_TA_088v2; break;
        				case 0x00000132: mb = TA_090v1; break;
        			}
        			break;
        	}
        	break;

        case 0x00600000:
        	switch(baryon)
        	{
        		case 0x00243000: mb = TA_088v3; break;
        		case 0x00263100:
        			switch(pommel)
        			{
        				case 0x00000132: mb = TA_090v2; break;
        				case 0x00000133: mb = TA_090v3; break;
        			}
        			break;
        		case 0x00285000: mb = TA_092; break;
        	}
        	break;

        case 0x00720000: mb = TA_091; break;

        case 0x00800000: mb = TA_094; break;

        case 0x00810000:
        	switch(baryon)
        	{
        		case 0x002C4000:
        			switch(pommel)
        			{
        				case 0x00000141: mb = TA_093v1; break;
        				case 0x00000143: mb = TA_093v2; break;
        			}
        			break;
        		case 0x002E4000: mb = TA_095v1; break;
        		case 0x012E4000: mb = TA_095v3; break;
        	}
        	break;


        case 0x00820000: 
        	switch(baryon)
        	{
        		case 0x012E4000: mb = TA_095v4; break;
        		case 0x002E4000: mb = TA_095v2; break;
        	}
        	break;

        case 0x00900000: 
        	mb = TA_096_TA_097; break;
    }

    fuseid = SysregGetFuseId();
    fuseconfig = SysregGetFuseConfig();
    totalblocks = sceNandGetTotalBlocks();
    nandsize = totalblocks * sceNandGetPagesPerBlock() * sceNandGetPageSize();
}

int dcGetHardwareInfo(u32 *ptachyon, u32 *pbaryon, u32 *ppommel, u32 *pmb, u64 *pfuseid, u32 *pfuseconfig, u32 *pnandsize)
{
    int k1 = pspSdkSetK1(0);

    if (ptachyon)    	
        *ptachyon = tachyon;

    if (pbaryon)
        *pbaryon = baryon;

    if (ppommel)
        *ppommel = pommel;

    if (pmb)
        *pmb = mb;
    
    if (pfuseid)
        *pfuseid = fuseid;
    
    if (pfuseconfig)
        *pfuseconfig = fuseconfig;

    if (pnandsize)
        *pnandsize = nandsize;

    pspSdkSetK1(k1);
    return 0;
}

int dcPatchModule(char *modname, int type, u32 addr, u32 word)
{
    int k1 = pspSdkSetK1(0);

    SceModule2 *mod = sceKernelFindModuleByName(modname);
    if (!mod)
    {
        pspSdkSetK1(k1);
        return -1;
    }

    if (type == 0)
        _sw(word, mod->text_addr+addr);
    else if (type == 1)
        _sh(word, mod->text_addr+addr);
    else if (type == 2)
        _sb(word, mod->text_addr+addr);

    sceKernelDcacheWritebackAll();
    sceKernelIcacheClearAll();

    pspSdkSetK1(k1);
    return 0;
}

int dcPatchModuleString(char *modname, char *string, char *replace)
{
    int k1 = pspSdkSetK1(0);
    int len = strlen(string);
    int i;
    int count = 0;

    SceModule2 *mod = sceKernelFindModuleByName(modname);
    if (!mod)
    {
        pspSdkSetK1(k1);
        return -1;
    }

    int totalsize = mod->text_size+mod->data_size+mod->bss_size;
    
    for (i = 0; i < totalsize-len; i++)
    {
        if (memcmp((void *)mod->text_addr+i, string, len) == 0)
        {
        	memcpy((void *)mod->text_addr+i, replace, len);
        	count++;
        }
    }

    if (count > 0)
    {
        sceKernelDcacheWritebackAll();
        sceKernelIcacheClearAll();
    }

    pspSdkSetK1(k1);
    return count;
}

int dcGetCancelMode()
{
    return cancel_mode;
}

int dcSetCancelMode(int mode)
{
    cancel_mode = mode;
    return 0;
}

int dcGetNandInfo(u32 *pagesize, u32 *ppb, u32 *totalblocks)
{
    int k1 = pspSdkSetK1(0);

    if (pagesize)
        *pagesize = sceNandGetPageSize();
    
    if (ppb)
        *ppb = sceNandGetPagesPerBlock();
    
    if (totalblocks)
        *totalblocks = sceNandGetTotalBlocks();

    pspSdkSetK1(k1);
    return 0;
}

static void SetScrambleZero()
{
    int (* sceNandSetScramble)(int) = NULL;

    sceNandSetScramble = (void *)sctrlHENFindFunction("sceNAND_Updater_Driver", "sceNand_updater_driver", 0x0BEE8F36);
    if (!sceNandSetScramble)
    {
        sceNandSetScramble = (void *)sctrlHENFindFunction("sceLowIO_Driver", "sceNand_driver", 0x0BEE8F36);
    }

    if (sceNandSetScramble)
        sceNandSetScramble(0);
}

int dcLockNand(int flag)
{
    int k1 = pspSdkSetK1(0);
    int res = sceNandLock(flag);
    pspSdkSetK1(k1);
    return res;
}

int dcUnlockNand()
{
    int k1 = pspSdkSetK1(0);
    sceNandUnlock();
    pspSdkSetK1(k1);
    return 0;
}

int dcReadNandBlock(u32 page, u8 *block)
{
    u32 i, j;
    int k1 = pspSdkSetK1(0);
    u32 ppb = sceNandGetPagesPerBlock();

    SetScrambleZero();

    if (sceNandIsBadBlock(page))
    {
        pspSdkSetK1(k1);
        return -1;
    }
    
    for (i = 0; i < ppb; i++)
    {
        for (j = 0; j < 4; j++)
        {
        	sceNandReadPagesRawAll(page, block, NULL, 1);
        	sceNandReadExtraOnly(page, block+512, 1);
        }

        page++;
        block += 528;
    }

    pspSdkSetK1(k1);
    return 0;
}

int dcWriteNandBlock(u32 page, u8 *user, u8 *spare)
{
    int i;
    int k1 = pspSdkSetK1(0);
    u32 ppb = sceNandGetPagesPerBlock();

    SetScrambleZero();

    for (i = 0; i < ppb; i++)
    {
        sceNandWriteAccess(page, user, spare, 1, 0x31);
        page++;
        user += 512;
        spare += 16;
    }

    pspSdkSetK1(k1);
    return 0;
}

int dcEraseNandBlock(u32 page)
{
    int k1 = pspSdkSetK1(0);
    int res = sceNandEraseBlock(page);
    pspSdkSetK1(k1);
    return res;
}

int    sceLflashFatfmtStartFatfmt(int argc, char *argv[]);
int pspLflashFdiskStartFdisk(int argc, char *argv[]);

int dcLflashStartFatfmt(int argc, char *argv[])
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(4);

    int res = sceLflashFatfmtStartFatfmt(argc, argv);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int dcLflashStartFDisk(int argc, char *argv[])
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = pspLflashFdiskStartFdisk(argc, argv);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int dcRegisterPhysicalFormatCallback(SceUID cbid)
{
    int k1 = pspSdkSetK1(0);

    phformat_cb = cbid;

    pspSdkSetK1(k1);
    return 0;
}

int dcUnregisterPhysicalFormatCallback()
{
    int k1 = pspSdkSetK1(0);

    phformat_cb = -1;

    pspSdkSetK1(k1);
    return 0;
}

int dcQueryRealMacAddress(u8 *macbuf)
{
    int k1 = pspSdkSetK1(0);
    int res = 0;

    if (!mac_obtained)
        res = -1;
    else
    {
        memcpy(macbuf, mac, 6);
    }

    pspSdkSetK1(k1);
    return res;
}

int dcKirkCmd(u8 *in, u32 insize, u8 *out, u32 outsize, int cmd)
{
    int k1 = pspSdkSetK1(0);

    int res = sceUtilsBufferCopyWithRange(in, insize, out, outsize, cmd);

    pspSdkSetK1(k1);
    return res;
}

int _dcIdStorageUnformat()
{
    return sceIdStorageUnformat();
}

int dcIdStorageUnformat()
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceKernelExtendKernelStack(0x4000, (void *)_dcIdStorageUnformat, NULL);


    sctrlKernelSetUserLevel(level);    
    pspSdkSetK1(k1);

    return res;
}

SceUID format_thid = -1;
int format_res;

int idsformat_thread(SceSize args, void *argp)
{
    format_res = sceIdStorageFormat();
        
    return sceKernelExitDeleteThread(0);
}

int dcIdStorageFormat()
{
    int k1 = pspSdkSetK1(0);

    if (format_thid >= 0)
    {
        pspSdkSetK1(k1);
        return -1;
    }

    format_thid = sceKernelCreateThread("idsformat_thread", idsformat_thread, 0x6C, 0x010000, 0, NULL);
    sceKernelStartThread(format_thid, 0, NULL);
    sceKernelWaitThreadEnd(format_thid, NULL);
    format_thid = -1;

    pspSdkSetK1(k1);
    return format_res;
}

int dcIdStorageCreateLeaf(u16 leafid)
{    
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIdStorageCreateLeaf(leafid);

    sctrlKernelSetUserLevel(level);    
    pspSdkSetK1(k1);

    return res;
}

int dcIdStorageCreateAtomicLeaves(u16 *leaves, int n)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIdStorage_driver_99ACCB71(leaves, n); //sceIdStorageCreateAtomicLeaves(leaves, n);

    sctrlKernelSetUserLevel(level);    
    pspSdkSetK1(k1);

    return res;
}

int dcIdStorageReadLeaf(u16 leafid, u8 *buf)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIdStorageReadLeaf(leafid, buf);

    sctrlKernelSetUserLevel(level);    
    pspSdkSetK1(k1);

    return res;
}

int dcIdStorageWriteLeaf(u16 leafid, u8 *buf)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIdStorageWriteLeaf(leafid, buf);

    sctrlKernelSetUserLevel(level);    
    pspSdkSetK1(k1);

    return res;
}

int _dcIdStorageFlush()
{
    return sceIdStorageFlush();
}

int dcIdStorageFlush()
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceKernelExtendKernelStack(0x4000, (void *)_dcIdStorageFlush, NULL);

    sctrlKernelSetUserLevel(level);    
    pspSdkSetK1(k1);

    return res;
}

int dcSysconReceiveSetParam(int n, u8 *buf)
{
    int k1 = pspSdkSetK1(0);

    int res = sceSysconReceiveSetParam(n, buf);

    pspSdkSetK1(k1);
    return res;
}

void SW(u32 word, u32 address)
{
    int k1 = pspSdkSetK1(0);
    _sw(word, address);
    pspSdkSetK1(k1);
}

u32 LW(u32 address)
{
    int k1 = pspSdkSetK1(0);
    u32 word = _lw(address);
    pspSdkSetK1(k1);
    return word;
}

int (* WriteBlock)(void *, int, void *);
static int WriteBlockPatched(void *a, int block, void *b)
{
    int res = WriteBlock(a, block, b);

    if (phformat_cb >= 0)
    {
        sceKernelNotifyCallback(phformat_cb, (block << 16) | (totalblocks-64));
    }

    return res;
}

int (* WlanFunc)(void *, int, void *);
static int WlanFuncPatched(void *a, int cmd, void *out)
{
    int res = WlanFunc(a, cmd, out);

    memcpy(mac, out+6, 6);
    mac_obtained = 1;
    
    return res;
}

static int EventHandler(int ev_id, char* ev_name, void* param, int* result)
{
    if (ev_id == 0x100 && cancel_mode != 0)
        return -1;

    return 0;
}

#define NAND_NIDS 41

u32 nandnids[NAND_NIDS] =
{
    0x01F09203,
    0x0ADC8686,
    0x0BEE8F36,    
    0x0F9BBBBD,
    0x18B78661,
    0x2674CFFE,
    0x2FF6081B,
    0x3F76BC21,
    0x41FFA822,
    0x5182C394, // 10
    0x5AC02755,
    0x716CD2B2,
    0x73A68408,
    0x766756EF,
    0x7AF7B77A,
    0x84EE5D76,
    0x88CC9F72,
    0x8932166A,
    0x8933B2E0,
    0x89BDCA08, // 20
    0x8AF0AB9F,
    0x9B2AC433,
    0xA513BB12,
    0xAE4438C7,
    0xB07C41D4,
    0xB2B021E5,
    0xB795D2ED,
    0xBADD5D46,
    0xC1376222,
    0xC29DA136, // 30
    0xC32EA051,
    0xC478C1DE,
    0xCE9843E6,
    0xD305870E,
    0xD897C343,
    0xE05AE88D,
    0xE41A11DE,
    0xEB0A0022,
    0xEBA0E6C6,
    0xEF55F193, // 40
    0xFCDF7610
};

u32 nand_offsets[NAND_NIDS] =
{
    0x18C8,
    0x0918,
    0x0D70,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0x05E8,
    0x0BE0, // 10
    0x1704,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0x0734,
    0x0654,
    0x04E8,
    0x1E54,
    0x0E08,
    0x1854,
    0x0D80, // 20
    0x0DC4,
    0x1A60,
    0x015C,
    0x0568,
    0x1C10,
    0x15CC,
    0x1E54,
    0xFFFFFFFF,
    0x1C1C,
    0x1954, // 30
    0x1680,
    0x0E2C,
    0xFFFFFFFF,
    0xCE8,
    0x14A8,
    0x0DEC,
    0x0D3C,
    0x0ADC,
    0xFFFFFFFF,
    0x1C28, // 40
    0x06DC
};

void RedirectNandFunc(u32 nid, void *addr)
{
    u32 orig = sctrlHENFindFunction("sceLowIO_Driver", "sceNand_driver", nid);

    if (!orig)
        return;

    REDIRECT_FUNCTION(orig, addr);
}

void OnModuleStart(SceModule2 *mod)
{
    if (strcmp(mod->modname, "sceLflashFatfmt") == 0)
    {
        return; // avoid tmctrl patch
    }
    else if (strcmp(mod->modname, "sceLFatFs_Updater_Driver") == 0)
    {
        MAKE_CALL(mod->text_addr+0x89B4, WriteBlockPatched);
        WriteBlock = (void *)(mod->text_addr+0x8EFC);
        ClearCaches();
    }
    else if (strcmp(mod->modname, "sceWlan_Driver") == 0)
    {
        MAKE_CALL(mod->text_addr+0x4F00, WlanFuncPatched);
        WlanFunc = (void *)(mod->text_addr+0xCD5C);
        ClearCaches();
    }
    
    else if (strcmp(mod->modname, "sceNAND_Updater_Driver") == 0)
    {
        int i;

        for (i = 0; i < NAND_NIDS; i++)
        {
        	if (nand_offsets[i] != 0xFFFFFFFF)
        	{			
        		RedirectNandFunc(nandnids[i], (void *)(mod->text_addr+nand_offsets[i]));
        	}
        }

        ClearCaches();
    }
    
    if (previous)
        previous(mod);
}

static PspSysEventHandler ev_handler =
{
    sizeof(PspSysEventHandler), 
    "SuspendCanceler",
    0x00ffff00, 
    EventHandler,
};

int module_start(SceSize args, void *argp)
{
    GetHardwareInfo();    
    sceKernelRegisterSysEventHandler(&ev_handler);

    previous = sctrlHENSetStartModuleHandler(OnModuleStart);

    return 0;
}

