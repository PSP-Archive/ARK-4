#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <pspreg.h>
#include <psprtc.h>
#include <psputils.h>
#include <pspwlan.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <pspipl_update.h>
#include <vlf.h>

#include "dcman.h"
#include "main.h"
#include "nandoperations.h"
#include "idstools.h"
#include "idsregeneration.h"



extern u8 *big_buffer;
extern int status, progress_text, progress_bar;

int lastids_sel;
int selectreg_text;
int macskip_text;
int mactext;
int regiontext;
int macrandom;
int selectedregion;

SceUID mac_thid = -1;
u8 macaddress[6];
void *waiticon;
char error_msg[256];


void SelectRegionPage(int nextpage);
void MacPage();
void PreCreateIdsPage();

static int LoadRegeneration()
{    
    SceUID mod = sceKernelLoadModule("flash0:/kd/idsregeneration.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return mod;

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return mod;
    }
    
    return 0;
}

static int OnNextPage(int page)
{
    if (page == 1)
    {
        selectedregion = vlfGuiCentralMenuSelection();
        vlfGuiRemoveText(selectreg_text);
        vlfGuiCancelBottomDialog();
        vlfGuiCancelCentralMenu();
        MacPage();
    }
    else if (page == 2)
    {
        if (mac_thid < 0)
        {		
        	vlfGuiRemoveText(macskip_text);
        	vlfGuiRemoveText(mactext);
        	vlfGuiCancelBottomDialog();
        	vlfGuiCancelCentralMenu();
        	vlfGuiCancelNextPageControl();
        	PreCreateIdsPage();
        }
    }
    
    return VLF_EV_RET_NOTHING;
}

static int OnPreviousPage(int page)
{
    if (page == 0)
    {
        if (mac_thid < 0)
        {
        	vlfGuiRemoveText(mactext);
        	vlfGuiRemoveText(macskip_text);
        	vlfGuiCancelBottomDialog();
        	vlfGuiCancelCentralMenu();
        	vlfGuiCancelPreviousPageControl();
        	SelectRegionPage(1);
        }
    }

    else if (page == 1)
    {
        vlfGuiRemoveText(regiontext);
        vlfGuiRemoveText(mactext);
        vlfGuiCancelCentralMenu();
        vlfGuiCancelBottomDialog();
        MacPage();
    }

    return VLF_EV_RET_NOTHING;
}

int AddWaitIcon(void *param)
{
    vlfGuiAddWaitIcon(&waiticon);
    
    return VLF_EV_RET_REMOVE_HANDLERS;
}

int RemoveWaitIcon(void *param)
{
    vlfGuiRemoveShadowedPicture(waiticon);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int OnIdsCreateError(void *param)
{
    vlfGuiMessageDialog(error_msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);

    vlfGuiRemoveText(status);
    vlfGuiRemoveText(progress_text);
    vlfGuiRemoveProgressBar(progress_bar);

    progress_bar = -1;
    progress_text = -1;
    status = -1;

    dcSetCancelMode(0);
    IdStorageMenu(0);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

void IdsCreateError(char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(error_msg, fmt, list);
    va_end(list);

    dcIdStorageFlush();
    vlfGuiAddEventHandler(0, -1, OnIdsCreateError, NULL);
    sceKernelExitDeleteThread(0);
}

int OnBackFromCreateIDS(int enter)
{
    if (!enter)
    {
        vlfGuiRemoveText(status);
        status = -1;
        vlfGuiCancelBottomDialog();
        IdStorageMenu(0);
    }
    
    return VLF_EV_RET_NOTHING;
}

int OnCreateIdsComplete(void *param)
{
    vlfGuiRemoveProgressBar(progress_bar);
    vlfGuiRemoveText(progress_text);

    progress_bar = -1;
    progress_text = -1;

    vlfGuiBottomDialog(VLF_DI_BACK, -1, 1, 0, VLF_DEFAULT, OnBackFromCreateIDS);

    SetStatus("IDStorage succesfully created.");
    vlfGuiSetTextXY(status, 240, 110);    
    vlfGuiSetTextAlignment(status, VLF_ALIGNMENT_CENTER);

    dcSetCancelMode(0);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int VerifyCertificates(u8 *buf)
{
    int i;
    int res;

    for (i = 0; i < 3; i++)
    {
        res = dcIdStorageReadLeaf(0x100+i, buf+(i*0x200));
        if (res < 0)
        {
        	res = dcIdStorageReadLeaf(0x120+i, buf+(i*0x200));
        	if (res < 0)
        		return res;
        }
    }

    u8 *certStart = &buf[0x38];
    
    for(i = 0; i < 6; i++)
    {
        res = dcKirkCmd(NULL, 0, &certStart[0xB8*i], 0xB8, 0x12);
        
        if (res	!= 0)
        {
        	return (0xC0000000 | (i << 24) | res);
        }		
    }

    return 0;
}

int idscreate_thread(SceSize args, void *argp)
{
    u8 setparam[8];
    u32 tachyon, baryon, pommel, mb, region;
    u64 fuseid;
    int i;
    IdsIndex *index;
    int n, res;

    if (LoadRegeneration() < 0)
    {
        IdsCreateError("Error loading module.");
    }
        
    for(i = 0; i < 5; i++)
    {
        if (dcSysconReceiveSetParam(1, setparam) == 0)
        {
        	break;
        }       
    }

    dcGetHardwareInfo(&tachyon, &baryon, &pommel, &mb, &fuseid, NULL, NULL);
    
    if (selectedregion == 0)
    {
        region = 5;
    }
    else if (selectedregion == 1)
    {
        region = 3;
    }
    else
    {
        region = 4;
    }

    dcSetCancelMode(1);    

    if (idsRegenerationSetup(tachyon, baryon, pommel, mb, fuseid, region, setparam) < 0)
    {
        IdsCreateError("Error in idsRegenerationSetup.");
    }

    sceKernelDelayThread(2300000);
    SetProgress(5, 1);

    SetStatus("Formatting idstorage...");
    sceKernelDelayThread(100000);

    dcIdStorageUnformat();
    res = dcIdStorageFormat();
    dcIdStorageFlush();

    if (res < 0)
    {
        IdsCreateError("Error 0x%08X formatting idstorage.", res);
    }

    sceKernelDelayThread(2300000);
    SetProgress(11, 1);

    SetStatus("Creating idstorage index...");
    sceKernelDelayThread(500000);
    
    index = (IdsIndex *)big_buffer;

    if (idsRegenerationGetIndex(index, &n) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetIndex.");
    }

    for (i = 0; i < n; i++)
    {
        if (index[i].keyfirst == index[i].keyend)
        {
        	res = dcIdStorageCreateLeaf(index[i].keyfirst);
        	if (res < 0)
        	{
        		IdsCreateError("Error 0x%08X creating key 0x%04X\n", res, index[i].keyfirst);
        	}
        }
        else
        {
        	int j;
        	int m = index[i].keyend - index[i].keyfirst + 1;
        	u16 leaves[0x50];

        	if (m <= 1 || m > 0x50)
        		IdsCreateError("Ugly bug in idsRegenerationGetIndex code :P"); 

        	for (j = 0; j < m; j++)
        	{
        		leaves[j] = index[i].keyfirst + j;
        	}

        	res = dcIdStorageCreateAtomicLeaves(leaves, m);
        	if (res < 0)
        	{
        		IdsCreateError("Error 0x%08X creating keys 0x%04X-0x%04X\n", res, index[i].keyfirst, index[i].keyend);
        	}
        }
    }

    sceKernelDelayThread(2500000);
    SetProgress(17, 1);

    SetStatus("Generating certificates and UMD keys...");
    sceKernelDelayThread(300000);

    res = idsRegenerationCreateCertificatesAndUMDKeys(big_buffer);
    if (res < 0)
    {
        IdsCreateError("Error 0x%08X creating certificate&umd keys.\n", res);
    }

    for (i = 0; i < 0x20; i++)
    {
        res = dcIdStorageWriteLeaf(0x100+i, big_buffer+(0x200*i));
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x%04X.\n", res, 0x100+i);
        }

        res = dcIdStorageWriteLeaf(0x120+i, big_buffer+(0x200*i));
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x%04X.\n", res, 0x120+i);
        }
    }

    res = dcIdStorageFlush();
    if (res < 0)
    {
        IdsCreateError("sceIdStorageFlush failed: 0x%08X.\n", res);
    }

    sceKernelDelayThread(2500000);
    SetProgress(23, 1);

    SetStatus("Verifying certificates...");
    sceKernelDelayThread(400000);

    res = VerifyCertificates(big_buffer);
    if (res < 0)
    {
        IdsCreateError("Certificates verification failed (0x%08X).", res);
    }

    sceKernelDelayThread(2500000);
    SetProgress(28, 1);

    SetStatus("Creating other keys...");
    sceKernelDelayThread(800000);

    if (idsRegenerationGetHwConfigKeys(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetHwConfigKeys.");
    }

    for (i = 0; i < 3; i++)
    {
        res = dcIdStorageWriteLeaf(0x0004+i, big_buffer+(0x200*i));
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x%04X.\n", res, 0x0004+i);
        }
    }

    sceKernelDelayThread(800000);
    SetProgress(33, 1);

    if (idsRegenerationGetMGKeys(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetMGKeys.");
    }

    for (i = 0; i < 0x20; i++)
    {
        res = dcIdStorageWriteLeaf(0x0010+i, big_buffer+(0x200*i));
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x%04X.\n", res, 0x0010+i);
        }
    }

    sceKernelDelayThread(800000);
    SetProgress(39, 1);

    if (idsRegenerationGetFactoryBadBlocksKey(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetFactoryBadBlocksKey.");
    }

    res = dcIdStorageWriteLeaf(0x000F, big_buffer);
    if (res < 0)
    {
        IdsCreateError("Error 0x%08X writing key 0x000F.", res);
    }

    sceKernelDelayThread(700000);
    SetProgress(44, 1);

    if (idsRegenerationGetSerialKey(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetSerialKey.");
    }

    res = dcIdStorageWriteLeaf(0x0050, big_buffer);
    if (res < 0)
    {
        IdsCreateError("Error 0x%08X writing key 0x0050.", res);
    }

    sceKernelDelayThread(700000);
    SetProgress(49, 1);

    memset(big_buffer, 0, 0x200);
    memcpy(big_buffer, macaddress, 6);

    res = dcIdStorageWriteLeaf(0x0044, big_buffer);
    if (res < 0)
    {
        IdsCreateError("Error 0x%08X writing key 0x0044.", res);
    }

    sceKernelDelayThread(700000);
    SetProgress(54, 1);

    if (idsRegenerationGetWlanKey(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetWlanKey.");
    }

    res = dcIdStorageWriteLeaf(0x0045, big_buffer);
    if (res < 0)
    {
        IdsCreateError("Error 0x%08X writing key 0x0045.", res);
    }

    sceKernelDelayThread(700000);
    SetProgress(59, 1);

    if (idsRegenerationGetUsbKeys(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetUsbKeys.");
    }

    for (i = 0; i < 3; i++)
    {
        res = dcIdStorageWriteLeaf(0x0041+i, big_buffer+(0x200*i));
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x%04X.", res, 0x0041+i);
        }
    }

    sceKernelDelayThread(800000);
    SetProgress(64, 1);

    if (idsRegenerationGetUnkKey140(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetUnkKey140.");
    }

    res = dcIdStorageWriteLeaf(0x0140, big_buffer);
    if (res < 0)
    {
        IdsCreateError("Error 0x%08X writing key 0x0140.", res);
    }

    sceKernelDelayThread(700000);
    SetProgress(69, 1);

    if (idsRegenerationGetMGKey40(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetMGKey40.");
    }

    res = dcIdStorageWriteLeaf(0x0040, big_buffer);
    if (res < 0)
    {
        IdsCreateError("Error 0x%08X writing key 0x0040.", res);
    }

    sceKernelDelayThread(700000);
    SetProgress(74, 1);

    if (idsRegenerationGetUnkKeys3X(big_buffer) < 0)
    {
        IdsCreateError("Error in idsRegenerationGetUnkKeys3X.");
    }

    for (i = 0; i < 0x10; i++)
    {
        res = dcIdStorageWriteLeaf(0x0030+i, big_buffer+(0x200*i));
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x%04X.", res, 0x0030+i);
        }
    }

    sceKernelDelayThread(800000);
    SetProgress(80, 1);

    res = idsRegenerationGetParentalLockKey(big_buffer);
    if (res > 0)
    {
        res = dcIdStorageWriteLeaf(0x0047, big_buffer);
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x0047.", res);
        }
    }
    else if (res == 0)
    {
        /* Doesn't apply to this psp */
    }
    else
    {
        IdsCreateError("Error in idsRegenerationGetParentalLockKey.");
    }

    sceKernelDelayThread(800000);
    SetProgress(85, 1);

    res = idsRegenerationGenerateFactoryFirmwareKey(big_buffer);
    if (res > 0)
    {
        res = dcIdStorageWriteLeaf(0x0051, big_buffer);
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x0051.", res);
        }
    }
    else if (res == 0)
    {
        /* Doesn't apply to this psp */
    }
    else
    {
        IdsCreateError("Error in idsRegenerationGenerateFactoryFirmwareKey.");
    }

    sceKernelDelayThread(800000);
    SetProgress(89, 1);

    res = idsRegenerationGetLCDKey(big_buffer);
    if (res > 0)
    {
        res = dcIdStorageWriteLeaf(0x0008, big_buffer);
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x0008.", res);
        }
    }
    else if (res == 0)
    {
        /* Doesn't apply to this psp */
    }
    else
    {
        IdsCreateError("Error in idsRegenerationGetLCDKey.");
    }

    sceKernelDelayThread(800000);
    SetProgress(93, 1);

    res = idsRegenerationGenerateCallibrationKey(big_buffer);
    if (res > 0)
    {
        res = dcIdStorageWriteLeaf(0x0007, big_buffer);
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x0007.", res);
        }
    }
    else if (res == 0)
    {
        /* Doesn't apply to this psp */
    }
    else
    {
        IdsCreateError("Error in idsRegenerationGenerateCallibrationKey.");
    }

    sceKernelDelayThread(800000);
    SetProgress(98, 1);

    res = idsRegenerationGetUnkKeys5253(big_buffer);
    if (res > 0)
    {
        for (i = 0; i < 2; i++)
        {
        	res = dcIdStorageWriteLeaf(0x0052+i, big_buffer+(0x200*i));
        	if (res < 0)
        	{
        		IdsCreateError("Error 0x%08X writing key 0x%04X.", res, 0x0052+i);
        	}
        }
    }
    else if (res == 0)
    {
        /* Doesn't apply to this psp */
    }
    else
    {
        IdsCreateError("Error in idsRegenerationGetUnkKeys5253.");
    }

    res = idsRegenerationGetDefaultXMBColorKey(big_buffer);
    if (res > 0)
    {
        res = dcIdStorageWriteLeaf(0x0054, big_buffer);
        if (res < 0)
        {
        	IdsCreateError("Error 0x%08X writing key 0x0054.", res);
        }
    }
    else if (res == 0)
    {
        /* Doesn't apply to this psp */
    }
    else
    {
        IdsCreateError("Error in idsRegenerationGetDefaultXMBColorKey.");
    }

    res = dcIdStorageFlush();
    if (res < 0)
    {
        IdsCreateError("sceIdStorageFlush failed: 0x%08X.\n", res);
    }

    sceKernelDelayThread(800000);
    SetProgress(100, 1);

    vlfGuiAddEventHandler(0, 1200000, OnCreateIdsComplete, NULL);

    return sceKernelExitDeleteThread(0);
}

int OnBeginCreateIDS(int enter)
{
    if (enter)
    {
        if (vlfGuiMessageDialog("This operation will delete your current idstorage and create a new one.\n"
        	                    "This process should only be done when a part of the psp is malfunctioning (umd, wlan, etc).\n\n"
        						"Do you want to continue?", VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_YESNO | VLF_MD_INITIAL_CURSOR_NO) != VLF_MD_YES)
        {
        	return VLF_EV_RET_NOTHING;
        }
        
        vlfGuiCancelPreviousPageControl();
        vlfGuiCancelCentralMenu();
        vlfGuiRemoveText(regiontext);
        vlfGuiRemoveText(mactext);
        
        ClearProgress();
        status = vlfGuiAddText(80, 100, "Loading modules...");

        progress_bar = vlfGuiAddProgressBar(136);	
        progress_text = vlfGuiAddText(240, 148, "0%");
        vlfGuiSetTextAlignment(progress_text, VLF_ALIGNMENT_CENTER);

        SceUID idscreate_thid = sceKernelCreateThread("idscreate_thread", idscreate_thread, 0x18, 0x10000, 0, NULL);
        if (idscreate_thid >= 0)
        {
        	sceKernelStartThread(idscreate_thid, 0, NULL);
        }	

        return VLF_EV_RET_REMOVE_HANDLERS | VLF_EV_RET_REMOVE_OBJECTS;
    }
    
    return VLF_EV_RET_NOTHING;
}

void PreCreateIdsPage()
{
    char *regions[] =
    {
        "Europe",
        "Japan",
        "America"		
    };

    char *items[] =
    {
        "Create"
    };
    
    if (macrandom)
    {
        u32 md5[4];
        u64 tick;
        SceKernelUtilsMt19937Context ctx;
        
        if (sceRtcGetCurrentTick(&tick) < 0)
        	tick = sceKernelGetSystemTimeWide();

        sceKernelUtilsMd5Digest((u8 *)&tick, sizeof(u64), (u8 *)md5);
        sceKernelUtilsMt19937Init(&ctx, md5[0] ^ md5[1] ^ md5[2] ^ md5[3]);

        macaddress[0] = 0x00;
        
        if (kuKernelGetModel() == 0)
        {
        	macaddress[1] = 0x16;
        	macaddress[2] = 0xFE;
        }
        else
        {
        	macaddress[1] = 0x1D;
        	macaddress[2] = 0xD9;
        }

        macaddress[3] = sceKernelUtilsMt19937UInt(&ctx);
        macaddress[4] = sceKernelUtilsMt19937UInt(&ctx);
        macaddress[5] = sceKernelUtilsMt19937UInt(&ctx);
    }

    regiontext = vlfGuiAddTextF(100, 87, "Region: %s", regions[selectedregion]);
    mactext = vlfGuiAddTextF(100, 110, "MAC address: %02X:%02X:%02X:%02X:%02X:%02X %s", macaddress[0], macaddress[1], macaddress[2], macaddress[3], macaddress[4], macaddress[5], (macrandom) ? "(random)" : "");
    
    vlfGuiCentralMenu(1, items, 0, NULL, 0, 24);
    vlfGuiBottomDialog(-1, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, OnBeginCreateIDS);
}

int OnMacErrorSwitch(void *param)
{
    vlfGuiMessageDialog("The WLAN switch is off.", VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);    

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int OnMacErrorOther(void *param)
{
    vlfGuiMessageDialog("Cannot get real mac address.", VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int OnUpdateMacAddress(void *param)
{
    macrandom = 0;
    vlfGuiSetTextF(mactext, "MAC address: %02X:%02X:%02X:%02X:%02X:%02X", macaddress[0], macaddress[1], macaddress[2], macaddress[3], macaddress[4], macaddress[5]);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int getmac_thread(SceSize args, void *argp)
{
    int res, i;
    
    res = dcQueryRealMacAddress(macaddress);
    if (res < 0)
    {
        //switch was off during all DC execution, and has been just turned on
        // Wait some seconds for wlanchipinit thread initialization
        
        vlfGuiAddEventHandler(0, -2, AddWaitIcon, NULL);
        
        for (i = 0; i < 7; i++)
        {
        	vlfGuiDelayAllEvents(1000000);
        	sceKernelDelayThread(1000000);

        	res = dcQueryRealMacAddress(macaddress);

        	if (res >= 0 || (res < 0 && sceWlanGetSwitchState() == 0))
        		break;
        }

        vlfGuiAddEventHandler(0, -2, RemoveWaitIcon, NULL);
        sceKernelDelayThread(100000);

        if (res < 0)
        {
        	if (sceWlanGetSwitchState() == 0)
        	{
        		vlfGuiAddEventHandler(0, -1, OnMacErrorSwitch, NULL);
        	}
        	else
        	{
        		vlfGuiAddEventHandler(0, -1, OnMacErrorOther, NULL);
        	}
        }
    }

    if (res >= 0)
    {
        vlfGuiAddEventHandler(0, -2, OnUpdateMacAddress, NULL);
    }
    
    mac_thid = -1;
    return sceKernelExitDeleteThread(0);
}

int OnGetMac(int enter)
{
    if (enter)
    {
        if (mac_thid >= 0)
        	return VLF_EV_RET_NOTHING;
        
        if (sceWlanGetSwitchState() == 0)
        {
        	vlfGuiMessageDialog("The WLAN switch is off.", VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);	
        }
        else
        {
        	mac_thid = sceKernelCreateThread("getmac_thread", getmac_thread, 0x18, 0x10000, 0, NULL);
        	if (mac_thid >= 0)
        	{
        		sceKernelStartThread(mac_thid, 0, NULL);
        	}
        }
    }
    
    return VLF_EV_RET_NOTHING;
}

void MacPage()
{
    char *items[] =
    {
        "Get Real MAC"
    };

    macskip_text = vlfGuiAddText(240, 80, "Skip this step to generate a random MAC");
    vlfGuiSetTextAlignment(macskip_text, VLF_ALIGNMENT_CENTER);

    if (macrandom)
    {
        mactext = vlfGuiAddText(74, 180, "MAC address: (random)");
    }
    else
    {
        mactext = vlfGuiAddTextF(74, 180, "MAC address: %02X:%02X:%02X:%02X:%02X:%02X", macaddress[0], macaddress[1], macaddress[2], macaddress[3], macaddress[4], macaddress[5]);
    }

    vlfGuiCentralMenu(1, items, 0, NULL, 0, -8);
    vlfGuiBottomDialog(-1, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, OnGetMac);
    vlfGuiNextPageControl(OnNextPage);
    vlfGuiPreviousPageControl(OnPreviousPage);
}

int OnChangeRegionExitMessage(void *param)
{
    int error = (int)param;

    dcSetCancelMode(0);

    vlfGuiMessageDialog(error_msg, (error) ? VLF_MD_TYPE_ERROR : VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_NONE);

    vlfGuiRemoveText(selectreg_text);
    vlfGuiCancelCentralMenu();
    vlfGuiCancelBottomDialog();
    IdStorageMenu(1);
    
    return VLF_EV_RET_REMOVE_HANDLERS;
}

int ChangeRegionExitMessage(int error, char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(error_msg, fmt, list);
    va_end(list);

    vlfGuiAddEventHandler(0, -1, OnChangeRegionExitMessage, (void *)error);
    
    return sceKernelExitDeleteThread(0);
}

int changereg_thread(SceSize args, void *argp)
{
    u32 tachyon, baryon, pommel, mb, region;
    u64 fuseid;
    int i, res;
    
    if (LoadRegeneration() < 0)
    {
        ChangeRegionExitMessage(1, "Error loading module.");
    }

    dcGetHardwareInfo(&tachyon, &baryon, &pommel, &mb, &fuseid, NULL, NULL);
    
    if (selectedregion == 0)
    {
        region = 5;
    }
    else if (selectedregion == 1)
    {
        region = 3;
    }
    else
    {
        region = 4;
    }

    dcSetCancelMode(1);    

    if (idsRegenerationSetup(tachyon, baryon, pommel, mb, fuseid, region, NULL) < 0)
    {
        ChangeRegionExitMessage(1, "Error in idsRegenerationSetup.");
    }

    res = idsRegenerationCreateCertificatesAndUMDKeys(big_buffer);
    if (res < 0)
    {
        ChangeRegionExitMessage(1, "Error 0x%08X creating certificate&umd keys.", res);
    }

    for (i = 0; i < 0x20; i++)
    {
        int res2;
        
        res = dcIdStorageWriteLeaf(0x100+i, big_buffer+(0x200*i));
        res2 = dcIdStorageWriteLeaf(0x120+i, big_buffer+(0x200*i));
        
        if (res == 0x80000025)
        	res = res2;
        else if (res2 == 0x80000025)
        	res2 = res;
        
        if (res < 0 || res2 < 0)
        {
        	ChangeRegionExitMessage(1, "Error 0x%08X writing keys 0x%04X/0x%04X.\n", res, 0x100+i, 0x120+i);
        }
    }

    res = dcIdStorageFlush();
    if (res < 0)
    {
        ChangeRegionExitMessage(1, "sceIdStorageFlush failed: 0x%08X.\n", res);
    }

    res = VerifyCertificates(big_buffer);
    if (res < 0)
    {
        ChangeRegionExitMessage(1, "Certificates verification failed (0x%08X).", res);
    }

    ChangeRegionExitMessage(0, "Region changed succesfully.");
    
    return sceKernelExitDeleteThread(0);
}

int SelectRegionBD(int enter)
{
    if (enter)
    {
        selectedregion = vlfGuiCentralMenuSelection();
        
        if (vlfGuiMessageDialog("After this operation, your current PSN original games will stop working until you download/buy them again, and your PSP will be identified by PS3 and PC as a new device.\n\n"
        						"Do you want to continue?", VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_YESNO | VLF_MD_INITIAL_CURSOR_NO) != VLF_MD_YES)
        {
        	return VLF_EV_RET_NOTHING;
        }

        SceUID thid = sceKernelCreateThread("changereg_thread", changereg_thread, 0x18, 0x10000, 0, NULL);
        if (thid >= 0)
        {
        	sceKernelStartThread(thid, 0, NULL);
        }

        return VLF_EV_RET_REMOVE_HANDLERS;
    }
    else
    {
        vlfGuiRemoveText(selectreg_text);
        vlfGuiCancelCentralMenu();
        vlfGuiCancelBottomDialog();

        if (lastids_sel == 0)
        	vlfGuiCancelNextPageControl();

        IdStorageMenu(lastids_sel);
    }

    return VLF_EV_RET_NOTHING;
}

void SelectRegionPage(int nextpage)
{
    char *items[] =
    {
        "Europe",
        "Japan",
        "America"		
    };

    selectreg_text = vlfGuiAddText(240, 60, "Select a region.");
    vlfGuiSetTextAlignment(selectreg_text, VLF_ALIGNMENT_CENTER);

    vlfGuiCentralMenu(3, items, selectedregion, NULL, 0, -6);
    vlfGuiBottomDialog(VLF_DI_BACK, (nextpage) ? -1 : VLF_DI_ENTER, 1, 0, VLF_DEFAULT, SelectRegionBD);

    if (nextpage)
    {
        vlfGuiNextPageControl(OnNextPage);
    }
}

int OnFixMacExitMessage(void *param)
{
    int error = (int)param;

    vlfGuiMessageDialog(error_msg, (error) ? VLF_MD_TYPE_ERROR : VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_NONE);
    return VLF_EV_RET_REMOVE_HANDLERS;
}

void FixMacMessage(int error, char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(error_msg, fmt, list);
    va_end(list);

    vlfGuiAddEventHandler(0, -1, OnFixMacExitMessage, (void *)error);
}

int fixmac_thread(SceSize args, void *argp)
{
    int res, i;
    u8 macaddress[6];
    
    res = dcQueryRealMacAddress(macaddress);
    if (res < 0)
    {
        //switch was off during all DC execution, and has been just turned on
        // Wait some seconds for wlanchipinit thread initialization
        
        vlfGuiAddEventHandler(0, -2, AddWaitIcon, NULL);
        
        for (i = 0; i < 7; i++)
        {
        	vlfGuiDelayAllEvents(1000000);
        	sceKernelDelayThread(1000000);

        	res = dcQueryRealMacAddress(macaddress);

        	if (res >= 0 || (res < 0 && sceWlanGetSwitchState() == 0))
        		break;
        }

        vlfGuiAddEventHandler(0, -2, RemoveWaitIcon, NULL);
        sceKernelDelayThread(100000);

        if (res < 0)
        {
        	if (sceWlanGetSwitchState() == 0)
        	{
        		vlfGuiAddEventHandler(0, -1, OnMacErrorSwitch, NULL);
        	}
        	else
        	{
        		vlfGuiAddEventHandler(0, -1, OnMacErrorOther, NULL);
        	}
        }
    }

    if (res >= 0)
    {
        memset(big_buffer, 0, 0x200);
        memcpy(big_buffer, macaddress, 6);

        res = dcIdStorageWriteLeaf(0x0044, big_buffer);
        if (res < 0)
        {
        	FixMacMessage(1, "Error 0x%08X writing to idstorage.", res);
        }
        else
        {
        	res = dcIdStorageFlush();
        	
        	if (res >= 0)
        	{
        		FixMacMessage(0, "Original MAC written succesfully.");
        	}
        	else
        	{
        		FixMacMessage(1, "Error 0x%08X in sceIdStorageFlush.", res);
        	}
        }
    }
    
    mac_thid = -1;
    return sceKernelExitDeleteThread(0);
}

void FixMac()
{
    if (mac_thid >= 0)
        return;
    
    if (sceWlanGetSwitchState() == 0)
    {
        vlfGuiMessageDialog("The WLAN switch is off.", VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);	
    }
    else
    {
        mac_thid = sceKernelCreateThread("fixmac_thread", fixmac_thread, 0x18, 0x10000, 0, NULL);
        if (mac_thid >= 0)
        {
        	sceKernelStartThread(mac_thid, 0, NULL);
        }
    }    
}

int OnIdStorageToolsSelect(int sel)
{
    lastids_sel = sel;
    
    switch (sel)
    {
        case 0:
        	
        	macrandom = 1;
        	selectedregion = 0;
        	vlfGuiCancelCentralMenu();
        	vlfGuiCancelBottomDialog();
        	SelectRegionPage(1);
        	
        	return VLF_EV_RET_NOTHING;
        break;

        case 1:
        	
        	selectedregion = 0;
        	vlfGuiCancelCentralMenu();
        	vlfGuiCancelBottomDialog();
        	SelectRegionPage(0);
        	
        	return VLF_EV_RET_NOTHING;
        break;

        case 2:

        	FixMac();
        	return VLF_EV_RET_NOTHING;

        break;
    }    
    
    return VLF_EV_RET_REMOVE_OBJECTS | VLF_EV_RET_REMOVE_HANDLERS;
}

int OnBackToNOFromIT(int enter)
{
    if (!enter)
    {
        vlfGuiCancelCentralMenu();
        vlfGuiCancelBottomDialog();
        NandOperationsMenu(3);		
    }

    return VLF_EV_RET_NOTHING;
}

void IdStorageMenu(int sel)
{
    char *items[] =
    {
        "Create New IdStorage",
        "Change region",
        "Fix MAC address"		
    };

    vlfGuiCentralMenu(3, items, sel, OnIdStorageToolsSelect, 0, -6);
    vlfGuiBottomDialog(VLF_DI_BACK, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, OnBackToNOFromIT);
}

