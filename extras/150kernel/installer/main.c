// Based on 3.90 M33 1.50 Addon Installer: https://github.com/mathieulh/3.90-M33/

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

#include <ark.h>
#include <libpsardumper.h>
#include <pspdecrypt.h>
#include <kubridge.h>

#include "pspbtcnf_game.h"
#include "reboot150.h"
#include "systemctrl150.h"
#include "tmctrl150.h"

#include "../common/include/rebootbin.h"

PSP_MODULE_INFO("legacy150_installer", 0x0800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define printf    pspDebugScreenPrintf

#define PSAR_SIZE_150		10149440

#define N_FILES	4

#define LOADEXEC_661_SIZE 0xBA00

typedef struct
{
    char *path;
    u8 *buf;
    int size;
} ARKFile;

ARKFile arkfiles[N_FILES] =
{
    { ARK_DC_PATH "/150/reboot150.prx", reboot150, sizeof(reboot150) },
    { ARK_DC_PATH "/150/kd/ark_systemctrl150.prx", systemctrl150, sizeof(systemctrl150) },
    { ARK_DC_PATH "/150/tmctrl150.prx", tmctrl150, sizeof(tmctrl150) },
    { ARK_DC_PATH "/150/kd/pspbtcnf_game.txt", pspbtcnf_game, sizeof(pspbtcnf_game) },
};

////////////////////////////////////////////////////////////////////
// big buffers for data. Some system calls require 64 byte alignment

// big enough for the full PSAR file
static u8 g_dataPSAR[13000000] __attribute__((aligned(0x40)));; 

// big enough for the largest (multiple uses)
static u8 g_dataOut[2000000] __attribute__((aligned(0x40)));
   
// for deflate output
//u8 g_dataOut2[3000000] __attribute__((aligned(0x40)));
static u8 *g_dataOut2;

static char com_table[0x4000];
static int comtable_size;

static char _1g_table[0x4000];
static int _1gtable_size;

static char _2g_table[0x4000];
static int _2gtable_size;

u8 psar_150_md5[16] = 
{
    0xF5, 0x65, 0x54, 0xE2, 0xBE, 0x35, 0x64, 0x5A, 
    0x76, 0x0B, 0x18, 0xED, 0x65, 0x05, 0xC4, 0xCC
};

u8 reboot150_header[REBOOT_HEADER_SIZE] = 
{
    REBOOT150_HEADER
};

u8 reboot661_header[REBOOT_HEADER_SIZE] = 
{
    REBOOT661_HEADER
};

#define N_DELETE 13
#define N_150 109

char *todelete[N_DELETE] =
{
    "flash0:/vsh/module/lftv_main_plugin.prx",
    "flash0:/vsh/module/lftv_middleware.prx",
    "flash0:/vsh/module/lftv_plugin.prx",
    "flash0:/vsh/resource/lftv_main_plugin.rco",
    "flash0:/vsh/resource/lftv_rmc_univer3in1.rco",
    "flash0:/vsh/resource/lftv_rmc_univer3in1_jp.rco",
    "flash0:/vsh/resource/lftv_rmc_univerpanel.rco",
    "flash0:/vsh/resource/lftv_rmc_univerpanel_jp.rco",
    "flash0:/vsh/resource/lftv_rmc_univertuner.rco",
    "flash0:/vsh/resource/lftv_rmc_univertuner_jp.rco",
    "flash0:/vsh/resource/lftv_tuner_jp_jp.rco",
    "flash0:/vsh/resource/lftv_tuner_us_en.rco",
    "flash0:/font/kr0.pgf"
};

char *subset150[N_150] =
{	
    "flash0:/kd/ata.prx",
    "flash0:/kd/audio.prx",
    "flash0:/kd/audiocodec.prx",
    "flash0:/kd/blkdev.prx",
    "flash0:/kd/chkreg.prx",
    "flash0:/kd/clockgen.prx",
    "flash0:/kd/codec.prx",
    "flash0:/kd/ctrl.prx",
    "flash0:/kd/display.prx",
    "flash0:/kd/dmacman.prx",
    "flash0:/kd/dmacplus.prx",
    "flash0:/kd/emc_ddr.prx",
    "flash0:/kd/emc_sm.prx",
    "flash0:/kd/exceptionman.prx",
    "flash0:/kd/fatmsmod.prx",
    "flash0:/kd/ge.prx",
    "flash0:/kd/gpio.prx",
    "flash0:/kd/hpremote.prx",
    "flash0:/kd/i2c.prx",
    "flash0:/kd/idstorage.prx",
    "flash0:/kd/ifhandle.prx",
    "flash0:/kd/impose.prx",
    "flash0:/kd/init.prx",
    "flash0:/kd/interruptman.prx",
    "flash0:/kd/iofilemgr.prx",
    "flash0:/kd/isofs.prx",
    "flash0:/kd/lcdc.prx",
    "flash0:/kd/led.prx",
    "flash0:/kd/lfatfs.prx",
    "flash0:/kd/lflash_fatfmt.prx",
    "flash0:/kd/libatrac3plus.prx",
    "flash0:/kd/libhttp.prx",
    "flash0:/kd/libparse_http.prx",
    "flash0:/kd/libparse_uri.prx",
    "flash0:/kd/libupdown.prx",
    "flash0:/kd/loadcore.prx",
    "flash0:/kd/loadexec.prx",
    "flash0:/kd/me_for_vsh.prx",
    "flash0:/kd/me_wrapper.prx",
    "flash0:/kd/mebooter.prx",
    "flash0:/kd/mediaman.prx",
    "flash0:/kd/mediasync.prx",
    "flash0:/kd/memab.prx",
    "flash0:/kd/memlmd.prx",
    "flash0:/kd/mesg_led.prx",
    "flash0:/kd/mgr.prx",
    "flash0:/kd/modulemgr.prx",
    "flash0:/kd/mpeg_vsh.prx",
    "flash0:/kd/mpegbase.prx",
    "flash0:/kd/msaudio.prx",
    "flash0:/kd/mscm.prx",
    "flash0:/kd/msstor.prx",
    "flash0:/kd/openpsid.prx",
    "flash0:/kd/peq.prx",
    "flash0:/kd/power.prx",
    //"flash0:/kd/pspbtcnf.txt",
    //"flash0:/kd/pspbtcnf_game.txt",
    //"flash0:/kd/pspbtcnf_updater.txt",
    "flash0:/kd/pspcnf_tbl.txt",
    "flash0:/kd/pspnet.prx",
    "flash0:/kd/pspnet_adhoc.prx",
    "flash0:/kd/pspnet_adhoc_auth.prx",
    "flash0:/kd/pspnet_adhoc_download.prx",
    "flash0:/kd/pspnet_adhoc_matching.prx",
    "flash0:/kd/pspnet_adhocctl.prx",
    "flash0:/kd/pspnet_ap_dialog_dummy.prx",
    "flash0:/kd/pspnet_apctl.prx",
    "flash0:/kd/pspnet_inet.prx",
    "flash0:/kd/pspnet_resolver.prx",
    "flash0:/kd/pwm.prx",
    "flash0:/kd/registry.prx",
    "flash0:/kd/resource/impose.rsc",
    "flash0:/kd/rtc.prx",
    "flash0:/kd/semawm.prx",
    "flash0:/kd/sircs.prx",
    "flash0:/kd/stdio.prx",
    "flash0:/kd/sysclib.prx",
    "flash0:/kd/syscon.prx",
    "flash0:/kd/sysmem.prx",
    "flash0:/kd/sysreg.prx",
    "flash0:/kd/systimer.prx",
    "flash0:/kd/threadman.prx",
    "flash0:/kd/uart4.prx",
    "flash0:/kd/umd9660.prx",
    "flash0:/kd/umdman.prx",
    "flash0:/kd/usb.prx",
    "flash0:/kd/usbstor.prx",
    "flash0:/kd/usbstorboot.prx",
    "flash0:/kd/usbstormgr.prx",
    "flash0:/kd/usbstorms.prx",
    "flash0:/kd/usersystemlib.prx",
    "flash0:/kd/utility.prx",
    "flash0:/kd/utils.prx",
    "flash0:/kd/vaudio.prx",
    "flash0:/kd/vaudio_game.prx",
    "flash0:/kd/videocodec.prx",
    "flash0:/kd/vshbridge.prx",
    "flash0:/kd/wlan.prx",
    "flash0:/vsh/module/chnnlsv.prx",
    "flash0:/vsh/module/common_gui.prx",
    "flash0:/vsh/module/common_util.prx",
    "flash0:/vsh/module/dialogmain.prx",
    "flash0:/vsh/module/heaparea1.prx",
    "flash0:/vsh/module/heaparea2.prx",
    "flash0:/vsh/module/netconf_plugin.prx",
    "flash0:/vsh/module/netplay_client_plugin.prx",
    "flash0:/vsh/module/netplay_server_utility.prx",
    "flash0:/vsh/module/osk_plugin.prx",
    //"flash0:/vsh/module/paf.prx",
    "flash0:/vsh/module/pafmini.prx",
    "flash0:/vsh/module/savedata_auto_dialog.prx",
    "flash0:/vsh/module/savedata_plugin.prx",
    "flash0:/vsh/module/savedata_utility.prx",
    //"flash0:/vsh/module/vshmain.prx"	
};

void ErrorExit(int milisecs, char *fmt, ...)
{
    va_list list;
    char msg[256];	

    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    printf(msg);
    
    sceKernelDelayThread(milisecs*1000);
    sceKernelExitGame();
}

////////////////////////////////////////////////////////////////////
// File helpers

int ReadFile(char *file, int seek, void *buf, int size)
{
    SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
    if (fd < 0)
        return fd;

    if (seek > 0)
    {
        if (sceIoLseek(fd, seek, PSP_SEEK_SET) != seek)
        {
            sceIoClose(fd);
            return -1;
        }
    }

    int read = sceIoRead(fd, buf, size);
    
    sceIoClose(fd);
    return read;
}

int WriteFile(char *file, void *buf, int size)
{
    SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    
    if (fd < 0)
    {
        return fd;
    }

    int written = sceIoWrite(fd, buf, size);

    sceIoClose(fd);
    return written;
}

static int FindTablePath(char *table, int table_size, char *number, char *szOut)
{
    int i, j, k;

    for (i = 0; i < table_size-5; i++)
    {
        if (strncmp(number, table+i, 5) == 0)
        {
            for (j = 0, k = 0; ; j++, k++)
            {
                if (table[i+j+6] < 0x20)
                {
                    szOut[k] = 0;
                    break;
                }

                if (!strncmp(table+i+6, "flash", 5) &&
                    j == 6)
                {
                    szOut[6] = ':';
                    szOut[7] = '/';
                    k++;
                }
                else if (!strncmp(table+i+6, "ipl", 3) &&
                    j == 3)
                {
                    szOut[3] = ':';
                    szOut[4] = '/';
                    k++;
                }
                else
                {				
                    szOut[k] = table[i+j+6];
                }
            }

            return 1;
        }
    }

    return 0;
}

static int WritePrx(char *name)
{
    int i;

    for (i = 0; i < N_150; i++)
    {
        if (strcmp(name, subset150[i]) == 0)
        {
            return 1;
        }
    }
    
    return 0;
}

static void CorrectPath(char *path)
{
    int len = strlen(path);
    int base_len = strlen("flash0:/");
    int new_base_len = strlen(ARK_DC_PATH "/150/");

    for (int i = len; i >= base_len; i--) {
        path[i + new_base_len - base_len] = path[i];
    }

    memcpy(path, ARK_DC_PATH "/150/", new_base_len);
}

int LoadStartModule(char *module, int partition)
{
    SceUID mod = kuKernelLoadModule(module, 0, NULL);

    if (mod < 0)
        return mod;

    return sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}

u8 *FindRebootBinBuf(u8 *buf, u8 *header, unsigned int size)
{
    for (int i = 0; i < size - REBOOT_HEADER_SIZE; i++)
    {
        if (memcmp((void *)&buf[i], (void *)header, REBOOT_HEADER_SIZE) == 0)
            return &buf[i + REBOOT_HEADER_SIZE];
    }

    return 0;
}

int GetReboot(u8 *dataOut, u8 *dataOut2, int cbExpanded, int decompress)
{
    cbExpanded = pspDecryptPRX(dataOut, dataOut2, cbExpanded);
    if (cbExpanded <= 0)
    {
        ErrorExit(5000, "Cannot decrypt loadexec.\n");
    }

    cbExpanded = pspDecompress(dataOut2, dataOut, sizeof(g_dataOut));
    if (cbExpanded <= 0)
    {
        ErrorExit(5000, "Cannot decompress loadexec.\n");
    }

    int i;

    for (i = 0; i < cbExpanded-20; i++)
    {
        if (memcmp(dataOut+i, "~PSP", 4) == 0)
        {
            break;
        }
    }

    if (i == (cbExpanded-20))
    {
        ErrorExit(5000, "Cannot find reboot.bin inside loadexec.\n");
    }

    cbExpanded = pspDecryptPRX(dataOut+i, dataOut2, *(u32 *)&dataOut[i+0x2C]);
    if (cbExpanded <= 0)
    {
        ErrorExit(5000, "Cannot decrypt reboot.bin.\n");
    }

    if (decompress)
    {
        cbExpanded = pspDecompress(dataOut2, dataOut, sizeof(g_dataOut));
        if (cbExpanded <= 0)
        {
            ErrorExit(5000, "Cannot decompress reboot.bin (0x%08X)\n", cbExpanded);
        }	
    }
    
    return cbExpanded;
}

static int GetReboot661()
{
    int size;
    
    size = ReadFile(ARK_DC_PATH "/kd/loadexec_01g.prx", 0, g_dataOut, sizeof(g_dataOut));

    if (size < 0) {
        if (sceKernelDevkitVersion() == FW_661)
        {
            ReadFile("flash0:/kd/loadexec_01g.prx", 0, g_dataOut, sizeof(g_dataOut));
            pspUnsignCheck(g_dataOut);
        }
        else
        {
            ErrorExit(5000, "Unable to find 6.61 loadexec_01g.prx in TM/DCARK or flash0.\n");
        }
    }

    if (size != LOADEXEC_661_SIZE)
    {
        ErrorExit(5000, "Unable to find 6.61 loadexec_01g.prx in TM/DCARK or flash0.\n");
    }

    size = GetReboot(g_dataOut, g_dataOut2, size, 1);

    u8 *rebootBuf = FindRebootBinBuf(systemctrl150, reboot661_header, size_systemctrl150);
    if (!rebootBuf)
    {
        ErrorExit(5000, "Unable to insert 661 reboot.bin into systemctrl150.prx\n");
    }
    memcpy(rebootBuf, g_dataOut, size);
}

void ExtractPrxs(int cbFile)
{
    int initres = pspPSARInit(g_dataPSAR, g_dataOut, g_dataOut2);

    if (initres < 0)
    {
        printf("pspPSARInit res: %x\n", initres);
        ErrorExit(5000, "pspPSARInit failed!. %d\n", initres);
    }   

    while (1)
    {
        char name[128];
        int cbExpanded;
        int pos;
        int signcheck;

        int res = pspPSARGetNextFile(g_dataPSAR, cbFile, g_dataOut, g_dataOut2, name, &cbExpanded, &pos, &signcheck);

        if (res < 0)
        {
            ErrorExit(5000, "PSAR decode error, pos=0x%08X.\n", pos);
        }
        else if (res == 0) /* no more files */
        {
            break;
        }
        
        if (!strncmp(name, "com:", 4) && comtable_size > 0)
        {
            if (!FindTablePath(com_table, comtable_size, name+4, name))
            {
                ErrorExit(5000, "Error: cannot find path of %s.\n", name);
            }
        }

        else if (!strncmp(name, "01g:", 4) && _1gtable_size > 0)
        {
            if (!FindTablePath(_1g_table, _1gtable_size, name+4, name))
            {
                ErrorExit(5000, "Error: cannot find path of %s.\n", name);
            }
        }

        else if (!strncmp(name, "02g:", 4) && _2gtable_size > 0)
        {
            if (!FindTablePath(_2g_table, _2gtable_size, name+4, name))
            {
                ErrorExit(5000, "Error: cannot find path of %s.\n", name);
            }
        }

           char* szFileBase = strrchr(name, '/');
        
        if (szFileBase != NULL)
            szFileBase++;  // after slash
        else
            szFileBase = "err.err";

        if (cbExpanded > 0)
        {
            if (WritePrx(name))
            {				
                CorrectPath(name);

                printf("Writing %s (%d)... ", name, cbExpanded);
                if (WriteFile(name, g_dataOut2, cbExpanded) != cbExpanded)
                {
                    ErrorExit(5000, "Error\n");
                }

                printf("OK\n");

                if (strcmp(name, ARK_DC_PATH "/150/kd/loadexec.prx") == 0)
                {
                    cbExpanded = GetReboot(g_dataOut2, g_dataOut, cbExpanded, 0);
                    u8 *rebootBuf = FindRebootBinBuf(reboot150, reboot150_header, size_reboot150);
                    if (!rebootBuf)
                    {
                        ErrorExit(5000, "Unable to insert 150 reboot.bin into reboot150.prx\n");
                    }
                    memcpy(rebootBuf, g_dataOut, cbExpanded);

                    rebootBuf = FindRebootBinBuf(systemctrl150, reboot150_header, size_systemctrl150);
                    if (!rebootBuf)
                    {
                        ErrorExit(5000, "Unable to insert 150 reboot.bin into systemctrl150.prx\n");
                    }
                    memcpy(rebootBuf, g_dataOut, cbExpanded);
                }
            }

            else if (!strcmp(name, "com:00000"))
            {
                comtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded, 0);
                            
                if (comtable_size <= 0)
                {
                    ErrorExit(5000, "Cannot decrypt common table.\n");
                }

                if (comtable_size > sizeof(com_table))
                {
                    ErrorExit(5000, "Com table buffer too small. Recompile with bigger buffer.\n");
                }

                memcpy(com_table, g_dataOut2, comtable_size);						
            }
                    
            else if (!strcmp(name, "01g:00000"))
            {
                _1gtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded, 0);
                            
                if (_1gtable_size <= 0)
                {
                    ErrorExit(5000, "Cannot decrypt 1g table.\n");
                }

                if (_1gtable_size > sizeof(_1g_table))
                {
                    ErrorExit(5000, "1g table buffer too small. Recompile with bigger buffer.\n");
                }

                memcpy(_1g_table, g_dataOut2, _1gtable_size);						
            }
                    
            else if (!strcmp(name, "02g:00000"))
            {
                _2gtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded, 0);
                            
                if (_2gtable_size <= 0)
                {
                    ErrorExit(5000, "Cannot decrypt 2g table %08X.\n", _2gtable_size);
                }

                if (_2gtable_size > sizeof(_2g_table))
                {
                    ErrorExit(5000, "2g table buffer too small. Recompile with bigger buffer.\n");
                }

                memcpy(_2g_table, g_dataOut2, _2gtable_size);						
            }			
        }

        scePowerTick(0);
    }
}

static void CreateDirs()
{
    printf("Creating directories... ");
    sceIoMkdir("ms0:/TM", 0777);
    sceIoMkdir(ARK_DC_PATH, 0777);
    sceIoMkdir(ARK_DC_PATH "/150", 0777);
    sceIoMkdir(ARK_DC_PATH "/150/kd", 0777);
    sceIoMkdir(ARK_DC_PATH "/150/kd/resource", 0777);
    sceIoMkdir(ARK_DC_PATH "/150/registry", 0777);
    sceIoMkdir(ARK_DC_PATH "/150/vsh", 0777);
    sceIoMkdir(ARK_DC_PATH "/150/vsh/module", 0777);
    printf("OK\n");
}

static void CopyRegistry()
{
    int size;

    printf("Copying registry... ");
    size = ReadFile("flash1:/registry/system.dreg", 0, g_dataOut, sizeof(g_dataOut));
    if (WriteFile(ARK_DC_PATH "/150/registry/system.dreg", g_dataOut, size) != size)
    {
        ErrorExit(5000, "Error writing registry file\n");
    }
    size = ReadFile("flash1:/registry/system.ireg", 0, g_dataOut, sizeof(g_dataOut));
    if (WriteFile(ARK_DC_PATH "/150/registry/system.ireg", g_dataOut, size) != size)
    {
        ErrorExit(5000, "Error writing registry file\n");
    }
    printf("OK\n");
}

static void Update()
{
    u8 md5[16];
    int size;

    printf("Reading 1.50 psar... ");
    
    if (ReadFile("ms0:/150.PBP", 0x393B81, g_dataPSAR, PSAR_SIZE_150) != PSAR_SIZE_150)
    {
        ErrorExit(5000, "Incorrect or inexistant 150.PBP at root.\n");
    }   
    
    sceKernelUtilsMd5Digest(g_dataPSAR, PSAR_SIZE_150, md5);

    if (memcmp(md5, psar_150_md5, 16) != 0)
    {
        ErrorExit(5000, "Incorrect or corrupted 150.PBP.\n");
    }

    printf("OK\n");

    CreateDirs();

    printf("\nExtracting and writing 150 prx's to ms...\n");
    ExtractPrxs(PSAR_SIZE_150);

    GetReboot661();
}

int main(void)
{
    int i;
    
    pspDebugScreenInit();

    if (kuKernelGetModel() != PSP_1000)
    {
        ErrorExit(5000, "The 150 kernel addon only works on the PSP 1000\n");
    }

    SceUID mod = LoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL);
    if (mod < 0)
    {
        ErrorExit(5000, "Error 0x%08X loading/starting libpsardumper.prx.\n", mod);
    }

    mod = LoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
    if (mod < 0)
    {
        ErrorExit(5000, "Error 0x%08X loading/starting pspdecrypt.prx.\n", mod);
    }

    printf("Press cross to begin the installation of 1.50 subset, or R to exit.\n");

    while (1)
    {
        SceCtrlData pad;

        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_CROSS)
        {
            break;
        }
        else if (pad.Buttons & PSP_CTRL_RTRIGGER)
        {
            ErrorExit(5000, "Cancelled by user.\n");
        }
        sceKernelDelayThread(10000);
    }

    sceKernelVolatileMemLock(0, (void *)&g_dataOut2, &i);

    Update();	

    printf("Writing custom modules...\n");

    for (i = 0; i < N_FILES; i++)
    {
        printf("Flashing %s... ", arkfiles[i].path);
        
        if (WriteFile(arkfiles[i].path, arkfiles[i].buf, arkfiles[i].size) != arkfiles[i].size)
        {
            ErrorExit(5000, "Error.\n");
        }

        printf("OK\n");
    }

    CopyRegistry();

    GetReboot661();

    ErrorExit(7000, "\n\nDone.\nAuto-exiting in 7 seconds.\n");

    return 0;
}

