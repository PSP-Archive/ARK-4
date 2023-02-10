/* Ultimate VSH Menu Revised (2009) */
/* by Total_Noob */

#include <systemctrl.h>
#include <pspctrl.h>
#include <pspreg.h>
#include <stdio.h>

PSP_MODULE_INFO("UVMRevised", 0x1007, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define WHITE colconf.fgcol
#define RED colconf.bgcol
#define BLUE colconf.cursorcol
#define MAX_THREAD 64
#define MAKE_CALL(a, f) _sw(0x0C000000 | (((u32)(f) >> 2) & 0x03FFFFFF), a);
#define PatchSyscall sctrlHENPatchSyscall
#define isvalidchar(c)(validchar(c) || c == '_' || c == '=')

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

typedef struct
{
    DWORD bfSize;
    DWORD bfReserved;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
BITMAPFILEHEADER h1;

typedef struct
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;
BITMAPINFOHEADER h2;

typedef struct
{
    int titlefgcol;
    int titlebgcol;
    int fgcol;
    int bgcol;
    int cursorcol;
    int messagefgcol;
    int messagebgcol;
} UVMColor;
UVMColor colconf;

typedef struct
{
    int magic;
    int slimcolors;
    int hidemacaddr;
    int hideumdupd;
    int screenshot;
    int selectbut;
    int randomcolor;
    int reserved[2];
} UVMConfig;
UVMConfig uvmconf;

typedef struct
{
    int magic;
    int hidecorrupt;
    int skiplogo;
    int gamemodules;
    int gamekernel;
    int executebootbin;
    int autorun;
    int umdmode;
    int res1;
    int vshcpuspeed;
    int vshbusspeed;
    int gamecpuspeed;
    int gamebusspeed;
    int fakeregion;
    int res2;
    int res3; 
    int usbdevice;
    int novshmenu;
    int usbcharge;
    int notusedaxupd;
    int hideiconpic;
    int xmbplugins;
    int gameplugins;
    int popsplugins;
    int useversion;
    int unk1;
    int speedupms;
    int reserved[2];
} M33Config;
M33Config m33conf;

struct isovideos 
{
    char video[128];
};

struct isovideos getvideo[128];

extern u8 msx[];

STMOD_HANDLER previous = NULL;
SceCtrlData pad;

SceUID vshthread = -1, usbdevice = -1;
u32 new_pad, old_pad, now_pad;
u64 firsttick, curtick;
int button = 0, total = 0, counter = 0, configchanged = 0, inrecovery = 0;

int thread_count_start, thread_count_now;
SceUID pauseuid = -1, thread_buf_start[MAX_THREAD], thread_buf_now[MAX_THREAD], thid1 = -1;

int(*umdIoOpen)(PspIoDrvFileArg* arg, const char* file, int flags, SceMode mode);
int(*SetSpeed)(int cpu, int bus);
int(*vshCtrlReadBufferPositive)(SceCtrlData* pad_data, int count);
int(*createThread)(const char* name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam* option);
int(*vshKernelExitVSH)(void* unk);
int(*Shutdown)(), (*Suspend)(), (*ColdReset)();

char* umdmode[] = { "Normal", "OE isofs", "M33 driver", "Sony NP9660" };
char* screenshotbutton[] = { "Disabled", "NOTE", "R + NOTE", "L + NOTE", "R + SCREEN", "L + SCREEN", "NOTE + SCREEN" };
char* selectbutton[] = { "VshMenu", "Recovery", "Disabled" };
char* battery[] = { "Pandora", "Autoboot", "Normal", "Unsupported" };
char* disenabled[] = { "Disabled", "Enabled" };
char* eboot = "ms0:/PSP/GAME/VSHMENU/EBOOT.PBP";

char* modulelist[] =
{
    "flash0:/kd/semawm.prx",
    "flash0:/kd/usbstor.prx",
    "flash0:/kd/usbstormgr.prx",
    "flash0:/kd/usbstorms.prx",
    "flash0:/kd/usbstorboot.prx",
    "flash0:/kd/usbdevice.prx",
    "ms0:/seplugins/vshrecovery.prx",
    "flash0:/vsh/module/recovery.prx"
};

wchar_t mac_info[] = L"[UVM Revised]";

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    sceKernelIcacheClearAll();
}

int pauseGame(SceUID thid)
{
    if(pauseuid >= 0) return -1;
    pauseuid = thid;

    sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_now, MAX_THREAD, &thread_count_now);

    int x, y, match;
    for(x = 0; x < thread_count_now; x++)
    {
        match = 0;
        SceUID tmp_thid = thread_buf_now[x];

        for(y = 0; y < thread_count_start; y++)
        {
            if(tmp_thid == thread_buf_start[y] || tmp_thid == thid1)
            {
                match = 1;
                break;
            }
        }
        if(!match) sceKernelSuspendThread(tmp_thid);
    }
    return 0;
}

int resumeGame(SceUID thid)
{
    if(pauseuid != thid) return -1;
    pauseuid = -1;

    int x, y, match;
    for(x = 0; x < thread_count_now; x++)
    {
        match = 0;
        SceUID tmp_thid = thread_buf_now[x];

        for(y = 0; y < thread_count_start; y++)
        {
            if(tmp_thid == thread_buf_start[y] || tmp_thid == thid1)
            {
                match = 1;
                break;
            }
        }
        if(!match) sceKernelResumeThread(tmp_thid);
    }
    return 0;
}

void setReg(char* dir, char* name, u32 val)
{
    struct RegParam reg;
    REGHANDLE h;

    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "/system");
    if(sceRegOpenRegistry(&reg, 2, &h) == 0)
    {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, dir, 2, &hd))
        {
            sceRegSetKeyValue(hd, name, &val, 4);
            sceRegFlushCategory(hd);
            sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }
}

int Rand(int min, int max)
{
    u64 tick;
    SceKernelUtilsMt19937Context ctx;
    sceRtcGetCurrentTick(&tick);
    sceKernelUtilsMt19937Init(&ctx, (u32)tick);

    return min + (sceKernelUtilsMt19937UInt(&ctx) % (max + 1));
}

int sceKernelCreateThreadPatched(const char* name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam* option)
{
    int res = createThread(name, entry, initPriority, stackSize, attr, option);

    char buf[16];
    buf[0] = 'S';
    buf[1] = 'C';
    buf[2] = 'E';
    buf[3] = '_';
    buf[4] = 'V';
    buf[5] = 'S';
    buf[6] = 'H';
    buf[7] = '_';
    buf[8] = 'G';
    buf[9] = 'R';
    buf[10] = 'A';
    buf[11] = 'P';
    buf[12] = 'H';
    buf[13] = 'I';
    buf[14] = 'C';
    buf[15] = 'S';
    if(strcmp(name, buf) == 0) vshthread = res;

    return res; 
}

int vshCtrlReadBufferPositivePatched(SceCtrlData* pad_data, int count) 
{
    int res = vshCtrlReadBufferPositive(pad_data, count);

    int k1 = pspSdkSetK1(0);

    if(!configchanged)
    {
        if(m33conf.vshcpuspeed != 0)
        {
            sceRtcGetCurrentTick(&curtick);
            curtick -= firsttick;
            u32 t = (u32)curtick;

            if(t >= 10000000) SetSpeed(m33conf.vshcpuspeed, m33conf.vshbusspeed);
        }
        else SetSpeed(222, 111);
    }

    if(!sceKernelFindModuleByName("htmlviewer_plugin_module"))
    {
        if(!sceKernelFindModuleByName("sceVshOSK_Module"))
        {
            if(!sceKernelFindModuleByName("camera_plugin_module"))
            {
                if(uvmconf.selectbut != 2)
                {
                    int i;
                    for(i = 0; i < count; i++)
                    {
                        if(pad_data[i].Buttons & PSP_CTRL_SELECT) button = 1;
                        if(button) pad_data[i].Buttons &= ~0xF0FFFF;
                    }
                }
            }
        }
    }

    pspSdkSetK1(k1);

    return res;
}

void returnToVsh()
{
    button = 0;
    inrecovery = 0;

    sceKernelResumeThread(vshthread);

    sctrlSEGetConfig(&m33conf);

    char string[128];
    sprintf(string, "ms0:/ISO/VIDEO/%s", getvideo[counter].video);
    vctrlVSHExitVSHMenu(&m33conf, !counter ? NULL : string, !counter ? NULL : 0x20);

    if(usbdevice >= 0)
    {
        sceKernelStopModule(usbdevice, 0, NULL, NULL, NULL);
        sceKernelUnloadModule(usbdevice);
    }

    sceKernelExitDeleteThread(0);    
}

int vshKernelExitVSHPatched(void* unk)
{
    int res;

    int k1 = pspSdkSetK1(0);
    
    if(inrecovery)
    {
        vshCtrlReadBufferPositive(&pad, 1);

        if(pad.Buttons & PSP_CTRL_RTRIGGER) res = vshKernelExitVSH(unk);
        else returnToVsh();
    }
    else res = vshKernelExitVSH(unk);

    pspSdkSetK1(k1);

    return res;
}

int ShutdownPatched()
{
    int res = Shutdown();
    int k1 = pspSdkSetK1(0);
    if(inrecovery) returnToVsh();
    pspSdkSetK1(k1);
    return res;
}

int SuspendPatched()
{
    int res = Suspend();
    int k1 = pspSdkSetK1(0);
    if(inrecovery) returnToVsh();
    pspSdkSetK1(k1);
    return res;
}

int ColdResetPatched()
{
    int res = ColdReset();
    int k1 = pspSdkSetK1(0);
    if(inrecovery) returnToVsh();
    pspSdkSetK1(k1);
    return res;
}

int umdIoOpenPatched(PspIoDrvFileArg* arg, const char* file, int flags, SceMode mode)
{
    int res;

    res = umdIoOpen(arg, file, flags, mode);

    if(strcmp(file, "/PSP_GAME/SYSDIR/UPDATE/PARAM.SFO") == 0)
    {
        if(uvmconf.hideumdupd) res = -1;
    }

    return res;
}

int FindPower(int nid)
{
    return FindProc("scePower_Service", "scePower_driver", nid);
}

int OnModuleStart(SceModule2* mod)
{
    if(strcmp(mod->modname, "vsh_module") == 0)
    {
        if(uvmconf.randomcolor)
        {
            SceUID fd = sceIoOpen("flash0:/vsh/resource/13-27.bmp", PSP_O_RDONLY, 0777);

            int randomcolor = Rand(0, fd > 0 ? 29 : 11);

            setReg("/CONFIG/SYSTEM/XMB/THEME", "color_mode", 1);
            setReg("/CONFIG/SYSTEM/XMB/THEME", "system_color", randomcolor);
        }

        PspIoDrv* umddrv = sctrlHENFindDriver("isofs");
        umdIoOpen = umddrv->funcs->IoOpen;
        umddrv->funcs->IoOpen = umdIoOpenPatched;

        SetSpeed = (void*)FindProc("SystemControl", "SystemCtrlForKernel", 0x98012538);
        vshCtrlReadBufferPositive = (void*)FindProc("sceController_Service", "sceCtrl_driver", 0x919215D7);
        createThread = (void*)FindProc("sceThreadManager", "ThreadManForUser", 0x446D8DE6);
        vshKernelExitVSH = (void*)FindProc("sceVshBridge_Driver", "sceVshBridge", 0x44DBAED5);
        Shutdown = (void*)FindPower(0x2B7C7CF4);
        Suspend = (void*)FindPower(0xAC32C9CC);
        ColdReset = (void*)FindPower(0x0442D852);

        SceModule2* module = sceKernelFindModuleByName("sceVshBridge_Driver");
        MAKE_CALL(module->text_addr + 0x264, vshCtrlReadBufferPositivePatched);
        
        PatchSyscall(createThread, sceKernelCreateThreadPatched);
        PatchSyscall(vshKernelExitVSH, vshKernelExitVSHPatched);
        PatchSyscall(Shutdown, ShutdownPatched);
        PatchSyscall(Suspend, SuspendPatched);
        PatchSyscall(ColdReset, ColdResetPatched);

        ClearCaches();
    }
    else if(strcmp(mod->modname, "sysconf_plugin_module") == 0)
    {
        u32 slimcol = -1, macaddr = -1;

        u32 text_end = mod->text_addr + mod->text_size;
        u32 text_addr = mod->text_addr;

        for(; text_addr < text_end; text_addr += 4)
        {
            if(_lw(text_addr) == 0x24040018 && _lw(text_addr + 4) == 0x26530008)
            {
                slimcol = text_addr - 0x18;
            }
            else if(_lw(text_addr) == 0x0025000A && _lw(text_addr + 4) == 0x00000031)
            {
                macaddr = text_addr + 8;
            }
        }
        
        if(uvmconf.slimcolors)
        {
            if(sceKernelGetModel() == 0)
            {
                SceUID fd = sceIoOpen("flash0:/vsh/resource/13-27.bmp", PSP_O_RDONLY, 0777);

                if(fd > 0)
                {
                    _sw(_lw(slimcol + 4), slimcol);
                    _sw(0x24020001, slimcol + 4);
                }
            }
        }
        if(uvmconf.hidemacaddr) memcpy(macaddr, mac_info, sizeof(mac_info));

        ClearCaches();
    }
    else if(strcmp(mod->modname, "Recovery mode") == 0)
    {
        u32 velf = -1;
            
        u32 text_end = mod->text_addr + mod->text_size;
        u32 text_addr = mod->text_addr;

        for(; text_addr < text_end; text_addr += 4)
        {
            if(strcmp((char*)text_addr, "flash0:/vsh/module/velf.prx") == 0) velf = text_addr;
        }

        if(velf != -1) _sw(0, velf);

        ClearCaches();
    }
    
    if(!previous) return 0;

    return previous(mod);
}

u32 write_eeprom(u8 addr, u16 data)
{
    int res;
    u8 param[0x60];

    if(addr > 0x7F) return 0x80000102;

    param[0x0C] = 0x73;
    param[0x0D] = 5;
    param[0x0E] = addr;
    param[0x0F] = data;
    param[0x10] = data >> 8;

    res = sceSysconCmdExec(param, 0);

    if(res < 0) return res;

    return 0;
}

u32 read_eeprom(u8 addr)
{
    int res;
    u8 param[0x60];

    if(addr > 0x7F) return 0x80000102;

    param[0x0C] = 0x74;
    param[0x0D] = 3;
    param[0x0E] = addr;

    res = sceSysconCmdExec(param, 0);

    if(res < 0) return res;

    return((param[0x21] << 8) | param[0x20]);
}

int errCheck(u32 data)
{
    if((data & 0x80250000) == 0x80250000) return -1;
    else if(data & 0xFFFF0000) return((data & 0xFFFF0000) >> 16);
    return 0;
}

int ReadSerial(u16* pdata)
{
    int err = 0;
    u32 data;

    data = read_eeprom(0x07);
    err = errCheck(data);
    if(!(err < 0))
    {
        pdata[0] = (data & 0xFFFF);
        data = read_eeprom(0x09);
        err = errCheck(data);
        if(!(err < 0)) pdata[1] = (data & 0xFFFF);
        else err = data;
    }
    else err = data;

    return err;
}

int WriteSerial(u16* pdata)
{
    int err = 0;

    err = write_eeprom(0x07, pdata[0]);
    if(!err) err = write_eeprom(0x09, pdata[1]);

    return err;
}

int validchar(char* ch)
{
    int i;
    if(ch >= 'A' && ch <= 'Z') i = 1;
    else if(ch >= 'a' && ch <= 'z') i = 1;
    else if(ch >= '0' && ch <= '9') i = 1;
    else i = 0;
    return i;
}

int readline(int fd, char* line)
{
    int read, i = 0;
    char ch;

    do
    {
        read = sceIoRead(fd, &ch, 1);
        if(read && ch != '\n' && ch != '\r') line[i++] = ch;
    }
    while(ch != '\n' && read == 1 && i < 128);

    line[i] = 0;

    return !read;
}

int gettokens(char tokens[][128], char* line)
{
    int iline = 0, itoken = 0, jtoken = 0, intoken = 0, instring = 0;

    while(itoken < 2)
    {
        char ch = line[iline++];

        if(ch == 0)
        {
            if(instring) return 0;
            if(intoken) tokens[itoken++][jtoken] = 0;
            break;
        }
        else if(ch == '=')
        {
            if(intoken)
            {
                if(!instring)
                {
                    intoken = 0;
                    tokens[itoken++][jtoken] = 0;
                    jtoken = 0;
                }
                else tokens[itoken][jtoken++] = ch;
            }
        }
        else if(ch == '"')
        {
            if(intoken)
            {
                if(!instring && jtoken != 0 || instring && isvalidchar(line[iline])) return 0;
                tokens[itoken][jtoken++] = ch;
                instring = !instring;
            }
            else
            {
                intoken = 1;
                instring = 1;
                tokens[itoken][jtoken++] = ch;
            }
        }
        else if(isvalidchar(ch))
        {
            if(!intoken) intoken = 1;
            tokens[itoken][jtoken++] = ch;
        }
        else if(instring) tokens[itoken][jtoken++] = ch;
    }

    return itoken;
}

int getint(char* str)
{
    return strtol(str, NULL, 0);
}

int ReadConf()
{
    SceUID fd = sceIoOpen("ms0:/seplugins/vshmenucolor.txt", PSP_O_RDONLY, 0777);
    
    if(fd < 0)
    {
        colconf.titlefgcol = 0x00FFFFFF;
        colconf.titlebgcol = 0x8000FF00;
        colconf.fgcol = 0x00FFFFFF;
        colconf.bgcol = 0xC00000FF;
        colconf.cursorcol = 0x00FF8080;
        colconf.messagefgcol = 0x0000FF00;
        colconf.messagebgcol = 0xFFFFFFFF;
        return -1;
    }

    memset(&colconf, 0, sizeof(colconf));

    int eof = 0;
    char line[128];
    char tokens[2][128];

    while(!eof)
    {
        eof = readline(fd, line);
        int ntokens = gettokens(tokens, line);

        if(ntokens == 2)
        {
            if(strcmp(tokens[0], "TITLE_FG_COLOR") == 0) colconf.titlefgcol = getint(tokens[1]);
            else if(strcmp(tokens[0], "TITLE_BG_COLOR") == 0) colconf.titlebgcol = getint(tokens[1]);
            else if(strcmp(tokens[0], "FG_COLOR") == 0) colconf.fgcol = getint(tokens[1]);
            else if(strcmp(tokens[0], "BG_COLOR") == 0) colconf.bgcol = getint(tokens[1]);
            else if(strcmp(tokens[0], "CURSOR_COLOR") == 0) colconf.cursorcol = getint(tokens[1]);
            else if(strcmp(tokens[0], "MESSAGE_FG_COLOR") == 0) colconf.messagefgcol = getint(tokens[1]);
            else if(strcmp(tokens[0], "MESSAGE_BG_COLOR") == 0) colconf.messagebgcol = getint(tokens[1]);
        }
    }

    sceIoClose(fd);
    return 0;
}

int uvmGetConfig(UVMConfig* config)
{
    memset(config, 0, sizeof(UVMConfig));
    SceUID fd = sceIoOpen("flash1:/config.tn", PSP_O_RDONLY, 0777);

    if(fd < 0) return -1;

    if(sceIoRead(fd, config, sizeof(UVMConfig)) < sizeof(UVMConfig))
    {
        sceIoClose(fd);
        return -1;
    }

    sceIoClose(fd);
    return 0;
}

int uvmSetConfig(UVMConfig* config)
{
    sceIoRemove("flash1:/config.tn");
    SceUID fd = sceIoOpen("flash1:/config.tn", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    if(fd < 0) return -1;

    config->magic = 0x47434E54;

    if(sceIoWrite(fd, config, sizeof(UVMConfig)) < sizeof(UVMConfig))
    {
        sceIoClose(fd);
        return -1;
    }

    sceIoClose(fd);
    return 0;
}

int getISOVideo() 
{
    SceIoDirent dir;
    char string[128];
    int count = 1;

    SceUID fd = sceIoDopen("ms0:/ISO/VIDEO");

    if(fd < 0) return -1;

    memset(&dir, 0, sizeof(dir));
    while(sceIoDread(fd, &dir) > 0)
    {
        sprintf(string, "%s", dir.d_name);
        char* suffix = strrchr(string, '.');
        if(suffix != NULL)
        {
            if(strcmp(suffix, ".ISO") == 0 || strcmp(suffix, ".iso") == 0)
            {
                sprintf(getvideo[count].video, "%s", string);
                total = count;
                count++;
            }
        }
    }

    sceIoDclose(fd);
    return 0;
}

int loadmodule(char* file)
{
    SceUID mod = sceKernelLoadModule(file, 0, NULL);

    if(mod < 0) return mod;

    return sceKernelStartModule(mod, strlen(file) + 1, file, NULL, NULL);
}

int recoverymenu()
{
    if(vshthread >= 0) sceKernelSuspendThread(vshthread);
    else
    {
        button = 0;
        return -1;
    }

    if(loadmodule(modulelist[6]) < 0)
    {
        if(loadmodule(modulelist[7]) < 0)
        {
            button = 0;
            sceKernelResumeThread(vshthread);
            return -1;
        }
    }

    inrecovery = 1;

    int i;
    for(i = 0; i <= 4; i++) loadmodule(modulelist[i]);
    usbdevice = loadmodule(modulelist[5]);

    return 0;
}

int screenshot(char* filename)
{
    SceUID fd = sceIoOpen(filename, PSP_O_CREAT | PSP_O_WRONLY | PSP_O_TRUNC, 0777);

    if(fd < 0) return -1;

    int x, y, pwidth, pheight, bufferwidth, pixelformat, unk;
    u32* vram32;
    u16* vram16;

    sceDisplayGetMode(&unk, &pwidth, &pheight);
    sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, unk);
    vram16 = (u16*)vram32;
    vram32 = (u32*)vram32;

    char bm[2];
    bm[0] = 'B';
    bm[1] = 'M';
    sceIoWrite(fd, bm, 2);

    int pw = pwidth, ph = pheight;

    char padding = 3 * pw / 2 % 4;

    h1.bfSize = (24 * pw + padding) * ph / 8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 2;
    h1.bfReserved = 0;
    h1.bfOffBits = 2 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    h2.biSize = sizeof(BITMAPINFOHEADER);
    h2.biPlanes = 1;
    h2.biBitCount = 24;
    h2.biCompression = 0;
    h2.biWidth = pw;
    h2.biHeight = ph;
    h2.biSizeImage = (24 * pw + padding) * ph / 8;
    h2.biXPelsPerMeter = 0xEC4;
    h2.biYPelsPerMeter = 0xEC4;
    h2.biClrUsed = 0;
    h2.biClrImportant = 0;

    sceIoWrite(fd, &h1, sizeof(BITMAPFILEHEADER));
    sceIoWrite(fd, &h2, sizeof(BITMAPINFOHEADER));

    SceUID mem = sceKernelAllocPartitionMemory(1, "", 0, (3 * pw + padding) * sizeof(u8), NULL);
    u8* buf = (u8*)sceKernelGetBlockHeadAddr(mem);

    for(x = 0; x < padding; x++) buf[3 * pw + x] = 0;

    for(y = (ph - 1); y >= 0; y--)
    {
        int i;
        for(i = 0, x = 0; x < pw; x++)
        {
            u32 color, offset = x + y * bufferwidth;
            u8 r = 0, g = 0, b = 0;

            switch(pixelformat)
            {
                case 0:
                color = vram16[offset];
                vram16[offset] ^= 0xFFFF;
                r = (color & 0x1F) << 3;
                g = ((color >> 5) & 0x3F) << 2;
                b = ((color >> 11) & 0x1F) << 3;
                break;

                case 1:
                color = vram16[offset];
                vram16[offset] ^= 0x7FFF;
                r = (color & 0x1F) << 3; 
                g = ((color >> 5) & 0x1F) << 3;
                b = ((color >> 10) & 0x1F) << 3;
                break;

                case 2:
                color = vram16[offset];
                vram16[offset] ^= 0x0FFF;
                r = (color & 0xF) << 4;
                g = ((color >> 4) & 0xF) << 4;
                b = ((color >> 8) & 0xF) << 4;
                break;

                case 3:
                color = vram32[offset];
                vram32[offset] ^= 0x00FFFFFF;
                r = color & 0xFF; 
                g = (color >> 8) & 0xFF;
                b = (color >> 16) & 0xFF;
                break;
            }
            buf[i++] = b;
            buf[i++] = g;
            buf[i++] = r;
        }
        sceIoWrite(fd, buf, 3 * pw + padding);
    }

    for(y = 0; y < ph; y++)
    {
        for(x = 0; x < pw; x++)
        {
            u32 offset = x + y * bufferwidth;

            switch(pixelformat)
            {
                case 0:
                vram16[offset] ^= 0xFFFF;
                break;

                case 1:
                vram16[offset] ^= 0x7FFF;
                break;

                case 2:
                vram16[offset] ^= 0x0FFF;
                break;

                case 3:
                vram32[offset] ^= 0x00FFFFFF;
                break;
            }
        }
    }
    
    sceKernelFreePartitionMemory(mem);
    sceIoClose(fd);

    return 0;
}

void readpad()
{
    static int n = 0;

    vshCtrlReadBufferPositive(&pad, 1);

    now_pad = pad.Buttons;
    new_pad = now_pad & ~old_pad;

    if(old_pad == now_pad)
    {
        n++;
        if(n >= 15)
        {
            new_pad = now_pad;
            n = 10;
        }
    }
    else
    {
        n = 0;
        old_pad = now_pad;
    }
}

void wait_release(u32 buttons)
{
    vshCtrlReadBufferPositive(&pad, 1);
    while(pad.Buttons & buttons)
    {
        vshCtrlReadBufferPositive(&pad, 1);
        buttons = 0xFFFF;
    }     
}

int adjust_alpha(u32 col)
{
    u8 mul;
    u32 c1, c2;
    u32 alpha = col >> 24;

    if(alpha == 0) return col;
    if(alpha == 0xFF) return col;

    c1 = col & 0x00FF00FF;
    c2 = col & 0x0000FF00;
    mul = (u8)(255 - alpha);
    c1 = (c1 * mul >> 8) & 0x00FF00FF;
    c2 = (c2 * mul >> 8) & 0x0000FF00;

    return(alpha << 24) | c1 | c2;
}

int blitPrint(int sx, int sy, char* msg, u32 fg_col, u32 bg_col)
{
    int x, y, p, pwidth, pheight, bufferwidth, pixelformat, unk;
    u32 c1, c2;
    u32* vram32;

    u32 fg = adjust_alpha(fg_col);
    u32 bg = adjust_alpha(bg_col);

    sceDisplayGetMode(&unk, &pwidth, &pheight);
    sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, unk);
    vram32 = (u32*)vram32;

    if(bufferwidth == 0 || pixelformat != 3) return -1;

    for(x = 0; msg[x] && x < pwidth / 8; x++)
    {
        char code = msg[x] & 0x7F;

        for(y = 0; y < 8; y++)
        {
            u32 offset = (sy * 8 + y) * bufferwidth + sx * 8 + x * 8;
            u8 font = msx[code * 8 + y];

            for(p = 0; p < 8; p++)
            {
                u32 col = (font & 0x80) ? fg : bg;
                u32 alpha = col >> 24;
                if(alpha == 0) vram32[offset] = col;
                else if(alpha != 0xFF)
                {
                    c2 = vram32[offset];
                    c1 = c2 & 0x00FF00FF;
                    c2 = c2 & 0x0000FF00;
                    c1 = (c1 * alpha) >> 8 & 0x00FF00FF;
                    c2 = (c2 * alpha) >> 8 & 0x0000FF00;
                    vram32[offset] = (col & 0xFFFFFF) + c1 + c2;
                }
                font <<= 1;
                offset++;
            }
        }
    }
    return 0;
}

void vshmenu()
{
    int sel = 0, loadeboot = 0;
    int batsel;

    char string[128];

    u32 baryon;
    sceSyscon_driver_7EC5A957(&baryon);

    if(sceSysreg_driver_E2A5D1EE() >= 0x00500000 && baryon >= 0x00234000) batsel = 3;
    else
    {
        u16 serial[2];
        ReadSerial(serial);
    
        if(serial[0] == 0xFFFF && serial[1] == 0xFFFF) batsel = 0;
        else if(serial[0] == 0x0000 && serial[1] == 0x0000) batsel = 1;
        else batsel = 2;
    }

    getISOVideo();

    int batchanged = batsel;
    configchanged = 1;

    while(1)
    {
        readpad();

        blitPrint(20, 6, "ULTIMATE VSH MENU", colconf.titlefgcol, colconf.titlebgcol);
        
        blitPrint(16, 8, "CPU CLOCK XMB   ", WHITE, sel == 0 ? BLUE : RED);
        sprintf(string, m33conf.vshcpuspeed == 0 ? "Default" : "%d/%d", m33conf.vshcpuspeed, m33conf.vshbusspeed);
        blitPrint(33, 8, string, WHITE, sel == 0 ? BLUE : RED);
        
        blitPrint(16, 9, "CPU CLOCK GAME  ", WHITE, sel == 1 ? BLUE : RED);
        sprintf(string, m33conf.gamecpuspeed == 0 ? "Default" : "%d/%d", m33conf.gamecpuspeed, m33conf.gamebusspeed);
        blitPrint(33, 9, string, WHITE, sel == 1 ? BLUE : RED);

        blitPrint(16, 10, "USB DEVICE      ", WHITE, sel == 2 ? BLUE : RED);
        sprintf(string, m33conf.usbdevice == 0 ? "Memory Stick" : m33conf.usbdevice == 5 ? "UMD Disc" : "Flash %d", m33conf.usbdevice - 1);
        blitPrint(33, 10, string, WHITE, sel == 2 ? BLUE : RED);

        blitPrint(16, 11, "UMD ISO MODE    ", WHITE, sel == 3 ? BLUE : RED);
        blitPrint(33, 11, umdmode[m33conf.umdmode], WHITE, sel == 3 ? BLUE : RED);

        blitPrint(16, 12, sceKernelGetModel() == 0 ? "SLIM COLORS     " : "USB CHARGE      ", WHITE, sel == 4 ? BLUE : RED);
        blitPrint(33, 12, sceKernelGetModel() == 0 ? disenabled[uvmconf.slimcolors] : disenabled[m33conf.usbcharge], WHITE, sel == 4 ? BLUE : RED);

        blitPrint(16, 13, "HIDE MAC ADDRESS", WHITE, sel == 5 ? BLUE : RED);
        blitPrint(33, 13, disenabled[uvmconf.hidemacaddr], WHITE, sel == 5 ? BLUE : RED);

        blitPrint(16, 14, "HIDE UMD UPDATE ", WHITE, sel == 6 ? BLUE : RED);
        blitPrint(33, 14, disenabled[uvmconf.hideumdupd], WHITE, sel == 6 ? BLUE : RED);

        blitPrint(16, 15, "SCREENSHOT      ", WHITE, sel == 7 ? BLUE : RED);
        blitPrint(33, 15, screenshotbutton[uvmconf.screenshot], WHITE, sel == 7 ? BLUE : RED);

        blitPrint(16, 16, "SELECT BUTTON   ", WHITE, sel == 8 ? BLUE : RED);
        blitPrint(33, 16, selectbutton[uvmconf.selectbut], WHITE, sel == 8 ? BLUE : RED);

        blitPrint(16, 17, "RANDOM COLOR    ", WHITE, sel == 9 ? BLUE : RED);
        blitPrint(33, 17, disenabled[uvmconf.randomcolor], WHITE, sel == 9 ? BLUE : RED);

        blitPrint(16, 18, "CONVERT BATTERY ", WHITE, sel == 10 ? BLUE : RED);
        blitPrint(33, 18, battery[batsel], WHITE, sel == 10 ? BLUE : RED);

        blitPrint(16, 19, "ISO VIDEO MOUNT ", WHITE, sel == 11 ? BLUE : RED);
        blitPrint(33, 19, !counter ? "NONE" : getvideo[counter].video, WHITE, sel == 11 ? BLUE : RED);

        blitPrint(21, 20, "SHUTDOWN DEVICE", WHITE, sel == 12 ? BLUE : RED);
        blitPrint(21, 21, "SUSPEND DEVICE", WHITE, sel == 13 ? BLUE : RED);
        blitPrint(22, 22, "REBOOT DEVICE", WHITE, sel == 14 ? BLUE : RED);
        blitPrint(22, 23, "RECOVERY MENU", WHITE, sel == 15 ? BLUE : RED);
        blitPrint(23, 24, "LOAD EBOOT", WHITE, sel == 16 ? BLUE : RED);
        blitPrint(26, 25, "EXIT", WHITE, sel == 17 ? BLUE : RED);

        if(new_pad & PSP_CTRL_DOWN)
        {
            if(sel >= 17) sel = 0;
            else sel++;
        }
        if(new_pad & PSP_CTRL_UP)
        {
            if(sel <= 0) sel = 17;
            else sel--;
        }

        if(sel == 0)
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(m33conf.vshcpuspeed == 0)
                {
                    m33conf.vshcpuspeed = 20;
                    m33conf.vshbusspeed = 10;
                }
                else if(m33conf.vshcpuspeed == 20)
                {
                    m33conf.vshcpuspeed = 75;
                    m33conf.vshbusspeed = 37;    
                }
                else if(m33conf.vshcpuspeed == 75)
                {
                    m33conf.vshcpuspeed = 100;
                    m33conf.vshbusspeed = 50;    
                }
                else if(m33conf.vshcpuspeed == 100)
                {
                    m33conf.vshcpuspeed = 133;
                    m33conf.vshbusspeed = 66;    
                }
                else if(m33conf.vshcpuspeed == 133)
                {
                    m33conf.vshcpuspeed = 222;
                    m33conf.vshbusspeed = 111;
                }
                else if(m33conf.vshcpuspeed == 222)
                {
                    m33conf.vshcpuspeed = 266;
                    m33conf.vshbusspeed = 133;    
                }
                else if(m33conf.vshcpuspeed == 266)
                {
                    m33conf.vshcpuspeed = 300;
                    m33conf.vshbusspeed = 150;    
                }
                else if(m33conf.vshcpuspeed == 300)
                {
                    m33conf.vshcpuspeed = 333;
                    m33conf.vshbusspeed = 166;    
                }
                else if(m33conf.vshcpuspeed == 333)
                {
                    m33conf.vshcpuspeed = 0;
                    m33conf.vshbusspeed = 0;    
                }                    
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(m33conf.vshcpuspeed == 0)
                {
                    m33conf.vshcpuspeed = 333;
                    m33conf.vshbusspeed = 166;
                }
                else if(m33conf.vshcpuspeed == 20)
                {
                    m33conf.vshcpuspeed = 0;
                    m33conf.vshbusspeed = 0;
                }
                else if(m33conf.vshcpuspeed == 75)
                {
                    m33conf.vshcpuspeed = 20;
                    m33conf.vshbusspeed = 10;    
                }
                else if(m33conf.vshcpuspeed == 100)
                {
                    m33conf.vshcpuspeed = 75;
                    m33conf.vshbusspeed = 37;    
                }
                else if(m33conf.vshcpuspeed == 133)
                {
                    m33conf.vshcpuspeed = 100;
                    m33conf.vshbusspeed = 50;
                }
                else if(m33conf.vshcpuspeed == 222)
                {
                    m33conf.vshcpuspeed = 133;
                    m33conf.vshbusspeed = 66;    
                }
                else if(m33conf.vshcpuspeed == 266)
                {
                    m33conf.vshcpuspeed = 222;
                    m33conf.vshbusspeed = 111;    
                }
                else if(m33conf.vshcpuspeed == 300)
                {
                    m33conf.vshcpuspeed = 266;
                    m33conf.vshbusspeed = 133;    
                }
                else if(m33conf.vshcpuspeed == 333)
                {
                    m33conf.vshcpuspeed = 300;
                    m33conf.vshbusspeed = 150;
                }
            }
        }
        if(sel == 1)
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(m33conf.gamecpuspeed == 0)
                {
                    m33conf.gamecpuspeed = 20;
                    m33conf.gamebusspeed = 10;
                }
                else if(m33conf.gamecpuspeed == 20)
                {
                    m33conf.gamecpuspeed = 75;
                    m33conf.gamebusspeed = 37;    
                }
                else if(m33conf.gamecpuspeed == 75)
                {
                    m33conf.gamecpuspeed = 100;
                    m33conf.gamebusspeed = 50;    
                }
                else if(m33conf.gamecpuspeed == 100)
                {
                    m33conf.gamecpuspeed = 133;
                    m33conf.gamebusspeed = 66;    
                }
                else if(m33conf.gamecpuspeed == 133)
                {
                    m33conf.gamecpuspeed = 222;
                    m33conf.gamebusspeed = 111;
                }
                else if(m33conf.gamecpuspeed == 222)
                {
                    m33conf.gamecpuspeed = 266;
                    m33conf.gamebusspeed = 133;    
                }
                else if(m33conf.gamecpuspeed == 266)
                {
                    m33conf.gamecpuspeed = 300;
                    m33conf.gamebusspeed = 150;    
                }
                else if(m33conf.gamecpuspeed == 300)
                {
                    m33conf.gamecpuspeed = 333;
                    m33conf.gamebusspeed = 166;    
                }
                else if(m33conf.gamecpuspeed == 333)
                {
                    m33conf.gamecpuspeed = 0;
                    m33conf.gamebusspeed = 0;    
                }                    
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(m33conf.gamecpuspeed == 0)
                {
                    m33conf.gamecpuspeed = 333;
                    m33conf.gamebusspeed = 166;
                }
                else if(m33conf.gamecpuspeed == 20)
                {
                    m33conf.gamecpuspeed = 0;
                    m33conf.gamebusspeed = 0;
                }
                else if(m33conf.gamecpuspeed == 75)
                {
                    m33conf.gamecpuspeed = 20;
                    m33conf.gamebusspeed = 10;    
                }
                else if(m33conf.gamecpuspeed == 100)
                {
                    m33conf.gamecpuspeed = 75;
                    m33conf.gamebusspeed = 37;    
                }
                else if(m33conf.gamecpuspeed == 133)
                {
                    m33conf.gamecpuspeed = 100;
                    m33conf.gamebusspeed = 50;
                }
                else if(m33conf.gamecpuspeed == 222)
                {
                    m33conf.gamecpuspeed = 133;
                    m33conf.gamebusspeed = 66;    
                }
                else if(m33conf.gamecpuspeed == 266)
                {
                    m33conf.gamecpuspeed = 222;
                    m33conf.gamebusspeed = 111;    
                }
                else if(m33conf.gamecpuspeed == 300)
                {
                    m33conf.gamecpuspeed = 266;
                    m33conf.gamebusspeed = 133;    
                }
                else if(m33conf.gamecpuspeed == 333)
                {
                    m33conf.gamecpuspeed = 300;
                    m33conf.gamebusspeed = 150;
                }
            }
        }
        if(sel == 2)
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(m33conf.usbdevice >= 5) m33conf.usbdevice = 0;
                else m33conf.usbdevice++;
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(m33conf.usbdevice <= 0) m33conf.usbdevice = 5;
                else m33conf.usbdevice--;
            }
        }
        if(sel == 3)
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(m33conf.umdmode >= 3) m33conf.umdmode = 0;
                else m33conf.umdmode++;
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(m33conf.umdmode <= 0) m33conf.umdmode = 3;
                else m33conf.umdmode--;
            }
        }
        if((new_pad & PSP_CTRL_RIGHT || new_pad & PSP_CTRL_LEFT) && sel == 4)
        {
            if(sceKernelGetModel() == 0)
            {
                uvmconf.slimcolors = !uvmconf.slimcolors;
            }
            else
            {
                m33conf.usbcharge = !m33conf.usbcharge;   
            }
        }
        if((new_pad & PSP_CTRL_RIGHT || new_pad & PSP_CTRL_LEFT) && sel == 5)
        {
            uvmconf.hidemacaddr = !uvmconf.hidemacaddr;
        }
        if((new_pad & PSP_CTRL_RIGHT || new_pad & PSP_CTRL_LEFT) && sel == 6)
        {
            uvmconf.hideumdupd = !uvmconf.hideumdupd;
        }
        if(sel == 7)
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(uvmconf.screenshot >= 6) uvmconf.screenshot = 0;
                else uvmconf.screenshot++;
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(uvmconf.screenshot <= 0) uvmconf.screenshot = 6;
                else uvmconf.screenshot--;
            }
        }
        if(sel == 8)
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(uvmconf.selectbut >= 2) uvmconf.selectbut = 0;
                else uvmconf.selectbut++;
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(uvmconf.selectbut <= 0) uvmconf.selectbut = 2;
                else uvmconf.selectbut--;
            }
        }
        if((new_pad & PSP_CTRL_RIGHT || new_pad & PSP_CTRL_LEFT) && sel == 9)
        {
            uvmconf.randomcolor = !uvmconf.randomcolor;
        }
        if(sel == 10 && batsel != 3)
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(batsel >= 2) batsel = 0;
                else batsel++;
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(batsel <= 0) batsel = 2;
                else batsel--;
            }
        }
        if(sel == 11 && !(!total))
        {
            if(new_pad & PSP_CTRL_RIGHT)
            {
                if(counter >= total) counter = 0;
                else counter++;
            }
            if(new_pad & PSP_CTRL_LEFT)
            {
                if(counter <= 0) counter = total;
                else counter--;
            }
        }
        if(new_pad & PSP_CTRL_CROSS || new_pad & PSP_CTRL_LEFT || new_pad & PSP_CTRL_RIGHT)
        {
            if(sel >= 12 && sel <= 16)
            {
                wait_release(PSP_CTRL_CROSS | PSP_CTRL_LEFT | PSP_CTRL_RIGHT);
            }
            if(sel == 12)
            {
                Shutdown();
                break;
            }
            if(sel == 13)
            {
                Suspend();
                break;
            }
            if(sel == 14)
            {
                ColdReset();
                break;
            }
            if(sel == 15)
            {
                recoverymenu();
                break;
            }
            if(sel == 16)
            {
                loadeboot = 1;
                break;
            }
        }
        if((new_pad & PSP_CTRL_CROSS) && sel == 17)
        {
            wait_release(PSP_CTRL_CROSS);
            break;
        }
        if(new_pad & PSP_CTRL_SELECT || new_pad & PSP_CTRL_HOME)
        {
            wait_release(PSP_CTRL_SELECT | PSP_CTRL_HOME);
            break;
        }

        sceKernelDelayThreadCB(1);
    }

    button = 0;

    uvmSetConfig(&uvmconf);
    sctrlSESetConfig(&m33conf);

    if(batsel != 3 && batsel != batchanged)
    {
        u16 buffer[0x80];
        if(batsel == 0)
        {
            buffer[0] = 0xFFFF;
            buffer[1] = 0xFFFF;
        }
        else if(batsel == 1)
        {
            buffer[0] = 0x0000;
            buffer[1] = 0x0000;
        }
        else if(batsel == 2)
        {
            buffer[0] = 0x1234;
            buffer[1] = 0x5678;
        }
        WriteSerial(buffer);
    }

    if(loadeboot)
    {
        u8 vshmain_args[0x400];
        struct SceKernelLoadExecVSHParam param;

        memset(vshmain_args, 0, sizeof(vshmain_args));
        vshmain_args[0x40] = 1;
        vshmain_args[0x280] = 1;
        vshmain_args[0x284] = 3;
        vshmain_args[0x286] = 5;

        memset(&param, 0, sizeof(param));
        param.size = sizeof(param);
        param.args = strlen(eboot) + 1;
        param.argp = eboot;
        param.key = "game";
        param.vshmain_args_size = sizeof(vshmain_args);
        param.vshmain_args = vshmain_args;

        sctrlKernelLoadExecVSHMs2(eboot, &param);
    }
    
    configchanged = 0;

    sprintf(string, "ms0:/ISO/VIDEO/%s", getvideo[counter].video);
    vctrlVSHExitVSHMenu(&m33conf, !counter ? NULL : string, !counter ? NULL : 0x20);
}

void message(char* msg)
{
    int i;
    for(i = 0; i < 2000; i++)
    {
        blitPrint(0, 33, msg, colconf.messagefgcol, colconf.messagebgcol);
        sceKernelDelayThreadCB(1);
    }
}

int main_thread(SceSize args, void* argp) 
{
    sceKernelDelayThread(100000);

    while(1)
    {
        sceCtrlPeekBufferPositive(&pad, 1);

        if(!inrecovery)
        {
            if(button)
            {
                while(pad.Buttons & PSP_CTRL_SELECT)
                {
                    vshCtrlReadBufferPositive(&pad, 1);
                }

                if(!uvmconf.selectbut) vshmenu();
                else if(uvmconf.selectbut) recoverymenu();
            }
            if(pad.Buttons & PSP_CTRL_LTRIGGER && pad.Buttons & PSP_CTRL_RTRIGGER && pad.Buttons & PSP_CTRL_SQUARE)
            {
                if(uvmconf.selectbut >= 2) uvmconf.selectbut = 0;
                else uvmconf.selectbut++;

                message(selectbutton[uvmconf.selectbut]);

                uvmSetConfig(&uvmconf);

                while(pad.Buttons & PSP_CTRL_LTRIGGER && pad.Buttons & PSP_CTRL_RTRIGGER && pad.Buttons & PSP_CTRL_SQUARE)
                {
                    vshCtrlReadBufferPositive(&pad, 1);
                }
            }
            if(uvmconf.screenshot == 1 ? pad.Buttons & PSP_CTRL_NOTE :
                uvmconf.screenshot == 2 ? pad.Buttons & PSP_CTRL_RTRIGGER && pad.Buttons & PSP_CTRL_NOTE :
                uvmconf.screenshot == 3 ? pad.Buttons & PSP_CTRL_LTRIGGER && pad.Buttons & PSP_CTRL_NOTE :
                uvmconf.screenshot == 4 ? pad.Buttons & PSP_CTRL_RTRIGGER && pad.Buttons & PSP_CTRL_SCREEN :
                uvmconf.screenshot == 5 ? pad.Buttons & PSP_CTRL_LTRIGGER && pad.Buttons & PSP_CTRL_SCREEN :
                uvmconf.screenshot == 6 ? pad.Buttons & PSP_CTRL_NOTE && pad.Buttons & PSP_CTRL_SCREEN : NULL)
            {
                char string[128];
                int count = 0;

                SceUID screenshotthid = sceKernelGetThreadId();
                pauseGame(screenshotthid);

                sceIoMkdir("ms0:/PICTURE", 0777);
                sceIoMkdir("ms0:/PICTURE/UVMRevised", 0777);

                while(1)
                {
                    sprintf(string, "ms0:/PICTURE/UVMRevised/screenshot%03d.bmp", count);
                    SceUID fd = sceIoOpen(string, PSP_O_RDONLY, 0777);
                    if(fd < 0)
                    {
                        sceIoClose(fd);
                        break;
                    }
                    sceIoClose(fd);
                    count++;
                }
                screenshot(string);
                resumeGame(screenshotthid);

                while(uvmconf.screenshot == 1 ? pad.Buttons & PSP_CTRL_NOTE :
                      uvmconf.screenshot == 2 ? pad.Buttons & PSP_CTRL_RTRIGGER && pad.Buttons & PSP_CTRL_NOTE :
                      uvmconf.screenshot == 3 ? pad.Buttons & PSP_CTRL_LTRIGGER && pad.Buttons & PSP_CTRL_NOTE :
                      uvmconf.screenshot == 4 ? pad.Buttons & PSP_CTRL_RTRIGGER && pad.Buttons & PSP_CTRL_SCREEN :
                      uvmconf.screenshot == 5 ? pad.Buttons & PSP_CTRL_LTRIGGER && pad.Buttons & PSP_CTRL_SCREEN :
                      uvmconf.screenshot == 6 ? pad.Buttons & PSP_CTRL_NOTE && pad.Buttons & PSP_CTRL_SCREEN : NULL)
                {
                    vshCtrlReadBufferPositive(&pad, 1);                        
                }
            }
        }

        sceKernelDelayThread(100);
        sceDisplayWaitVblankStart();
    }

    return 0;
}

int module_start(SceSize args, void* argp)
{
    if(sceKernelInitKeyConfig() != PSP_INIT_KEYCONFIG_VSH) return -1;
    if(sceKernelDevkitVersion() < 0x05000010 || sceKernelDevkitVersion() > 0x05050010) return -1;

    uvmGetConfig(&uvmconf);
    sctrlSEGetConfig(&m33conf);
    ReadConf();

    sceRtcGetCurrentTick(&firsttick);

    previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    
    sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_start, MAX_THREAD, &thread_count_start);

    SceUID mainthid = sceKernelCreateThread("UVMRevised_Thread", main_thread, 0x10, 0x1000, 0, NULL);
    if(mainthid >= 0) sceKernelStartThread(mainthid, 0, 0);

    return 0;
}
