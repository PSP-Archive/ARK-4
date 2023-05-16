#include "rebootex.h"
#include "pspbtcnf.h"

#ifdef MS_IPL
#include <fat.h>
#endif

extern int UnpackBootConfigPatched(char **p_buffer, int length);

int loadcoreModuleStartPSP(void * arg1, void * arg2, void * arg3, int (* start)(void *, void *, void *)){
    u32 text_addr = loadCoreModuleStartCommon(start);
#ifdef MS_IPL
    // TODO make these patches dynamic
	// disable unsign check
	_sw(0x1021, text_addr + 0x5994);
	_sw(0x1021, text_addr + 0x59C4);
	_sw(0x1021, text_addr + 0x5A5C);
#endif
    flushCache();
    return start(arg1, arg2, arg3);
}

// patch reboot on psp
void patchRebootBuffer(){

    _sw(0x27A40004, UnpackBootConfigArg); // addiu $a0, $sp, 4
    _sw(JAL(UnpackBootConfigPatched), UnpackBootConfigCall); // Hook UnpackBootConfig
    // make sure we read as little ram as possible
#ifdef MS_IPL
    int patches = 6;
#else
    int patches = 5;
#endif
    for (u32 addr = reboot_start; addr<reboot_end && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x02A0E821 || data == 0x0280E821){ // found loadcore jump on PSP
            _sw(0x3821 | ((_lw(addr-4) & 0x3E00000) >> 5), addr-4); // ADDU $a3 $zero <reg>
            _sw(JUMP(loadcoreModuleStartPSP), addr);
            _sw(data, addr + 4);
            patches--;
            addr += 4;
        }
        else if (data == 0x2C860040 || data == 0x2C850040){ // kdebug patch
            _sw(0x03E00008, addr-4); // make it return 1
            _sw(0x24020001, addr); // rebootexcheck1
            patches--;
        }
        else if (data == 0x24D90001 || data == 0x256A0001){  // rebootexcheck5
            u32 a = addr;
            u32 insMask;
            do {
                a-=4;
                insMask = _lw(a) & 0xFFFF0000;
            } while (insMask != 0x04400000 && insMask != 0x04420000);
            _sw(NOP, a); // Killing Branch Check bltz/bltzl ...
        }
#ifdef REBOOTEX
        else if (data == 0x34650001){ // rebootexcheck2
            _sw(NOP, addr-4); // Killing Branch Check bltz ...
            patches--;
        }
        else if (data == 0x00903021 && _lw(addr+4) == 0x00D6282B){ // rebootexcheck3 and rebootexcheck4
            u32 a = addr;
            do {a-=4;} while (_lw(a) != NOP);
            _sw(NOP, a-4); // Killing Branch Check beqz
            _sw(NOP, addr+8); // Killing Branch Check bltz ...
            patches--;
        }
#else
        else if (data == 0x25AC003F){ // payloadexcheck2
            _sw(NOP, addr-44); // Killing Branch Check bltz ...
            patches--;
        }
        else if (data == 0x01F7702B){ // rebootexcheck3 and rebootexcheck4
            _sw(NOP, addr-12); // Killing Branch Check bltz
            _sw(NOP, addr+4); // Killing Branch Check beqz ...
            patches--;
        }
#endif
#ifdef MS_IPL
        else if (data == 0x27BDFFE0 && _lw(addr+4) == 0x3C028861) { // nand enc
            MAKE_DUMMY_FUNCTION_RETURN_0(addr);
            patches--;
        }
#endif
    }

    patchRebootIoPSP();

    // Flush Cache
    flushCache();
}

// pspbtcnf patches

int patch_bootconf_vsh(char *buffer, int length)
{

    int newsize, result;

    result = length;

    newsize = AddPRX(buffer, "/kd/vshbridge.prx", PATH_VSHCTRL+sizeof(PATH_FLASH0)-2, VSH_RUNLEVEL );
    if (newsize > 0) result = newsize;

    newsize = AddPRX(buffer, "/kd/vshbridge_tool.prx", PATH_VSHCTRL+sizeof(PATH_FLASH0)-2, VSH_RUNLEVEL );
    if (newsize > 0) result = newsize;

    return result;
}

int patch_bootconf_pops(char *buffer, int length)
{
    int newsize, result;

    result = length;
    newsize = AddPRX(buffer, "/kd/usersystemlib.prx", PATH_POPCORN+sizeof(PATH_FLASH0)-2, POPS_RUNLEVEL);

    if (newsize > 0) result = newsize;

    return result;
}

struct add_module {
    char *prxname;
    char *insertbefore;
    u32 flags;
};

struct del_module {
    char *prxname;
    u32 flags;
};

static struct add_module inferno_add_mods[] = {
    { "/kd/mgr.prx", "/kd/amctrl.prx", GAME_RUNLEVEL },
    { PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/utility.prx", GAME_RUNLEVEL },
    { PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/isofs.prx", UMDEMU_RUNLEVEL },
    { "/kd/isofs.prx", "/kd/utility.prx", GAME_RUNLEVEL },
};

static struct del_module inferno_del_mods[] = {
    { "/kd/mediaman.prx", GAME_RUNLEVEL },
    { "/kd/ata.prx", GAME_RUNLEVEL },
    { "/kd/umdman.prx", GAME_RUNLEVEL },
    { "/kd/umdcache.prx", GAME_RUNLEVEL },
    { "/kd/umd9660.prx", GAME_RUNLEVEL },
    { "/kd/np9660.prx", UMDEMU_RUNLEVEL },
};

int patch_bootconf_inferno(char *buffer, int length)
{
    int newsize, result, ret;

    result = length;

    int i; for(i=0; i<NELEMS(inferno_del_mods); ++i) {
        RemovePrx(buffer, inferno_del_mods[i].prxname, inferno_del_mods[i].flags);
    }

    for(i=0; i<NELEMS(inferno_add_mods); ++i) {
        newsize = MovePrx(buffer, inferno_add_mods[i].insertbefore, inferno_add_mods[i].prxname, inferno_add_mods[i].flags);

        if (newsize > 0) result = newsize;
    }

    return result;
}

static struct add_module vshumd_add_mods[] = {
    { "/kd/isofs.prx", "/kd/utility.prx", VSH_RUNLEVEL },
    { PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/chnnlsv.prx", VSH_RUNLEVEL },
};

static struct del_module vshumd_del_mods[] = {
    { "/kd/mediaman.prx", VSH_RUNLEVEL },
    { "/kd/ata.prx", VSH_RUNLEVEL },
    { "/kd/umdman.prx", VSH_RUNLEVEL },
    { "/kd/umd9660.prx", VSH_RUNLEVEL },
};

int patch_bootconf_vshumd(char *buffer, int length)
{
    int newsize, result, ret;

    result = length;

    int i; for(i=0; i<NELEMS(vshumd_del_mods); ++i) {
        RemovePrx(buffer, vshumd_del_mods[i].prxname, vshumd_del_mods[i].flags);
    }
    
    for(i=0; i<NELEMS(vshumd_add_mods); ++i) {
        newsize = MovePrx(buffer, vshumd_add_mods[i].insertbefore, vshumd_add_mods[i].prxname, vshumd_add_mods[i].flags);

        if (newsize > 0) result = newsize;
    }

    return result;
}

static struct add_module updaterumd_add_mods[] = {
    { "/kd/isofs.prx", "/kd/utility.prx", UPDATER_RUNLEVEL },
    { PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/chnnlsv.prx", UPDATER_RUNLEVEL },
};

static struct del_module updaterumd_del_mods[] = {
    { "/kd/mediaman.prx", UPDATER_RUNLEVEL },
    { "/kd/ata.prx", UPDATER_RUNLEVEL },
    { "/kd/umdman.prx", UPDATER_RUNLEVEL },
    { "/kd/umd9660.prx", UPDATER_RUNLEVEL },
};

int patch_bootconf_updaterumd(char *buffer, int length)
{
    int newsize, result, ret;

    result = length;

    int i; for(i=0; i<NELEMS(updaterumd_del_mods); ++i) {
        RemovePrx(buffer, updaterumd_del_mods[i].prxname, updaterumd_del_mods[i].flags);
    }
    
    for(i=0; i<NELEMS(updaterumd_add_mods); ++i) {
        newsize = MovePrx(buffer, updaterumd_add_mods[i].insertbefore, updaterumd_add_mods[i].prxname, updaterumd_add_mods[i].flags);

        if (newsize > 0) result = newsize;
    }

    return result;
}

int UnpackBootConfigPatched(char **p_buffer, int length)
{
    int result;
    int newsize;
    char *buffer;

    result = (*UnpackBootConfig)(*p_buffer, length);
    buffer = (void*)BOOTCONFIG_TEMP_BUFFER;
    memcpy(buffer, *p_buffer, length);
    *p_buffer = buffer;

    // Insert SystemControl
    newsize = AddPRX(buffer, "/kd/init.prx", PATH_SYSTEMCTRL+sizeof(PATH_FLASH0)-2, 0x000000EF);
    if (newsize > 0) result = newsize;
    
    // Insert compat layer
    newsize = AddPRX(buffer, "/kd/init.prx", PATH_PSPCOMPAT+sizeof(PATH_FLASH0)-2, 0x000000EF);
    if (newsize > 0) result = newsize;
    
    // Insert Stargate No-DRM Engine
    newsize = AddPRX(buffer, "/kd/me_wrapper.prx", PATH_STARGATE+sizeof(PATH_FLASH0)-2, GAME_RUNLEVEL | UMDEMU_RUNLEVEL);
    if (newsize > 0) result = newsize;
    
    // Insert VSHControl
    if (SearchPrx(buffer, "/vsh/module/vshmain.prx") >= 0) {
        newsize = patch_bootconf_vsh(buffer, length);
        if (newsize > 0) result = newsize;
    }

    // Insert Popcorn
    newsize = patch_bootconf_pops(buffer, length);
    if (newsize > 0) result = newsize;

    // Insert Inferno and RTM
    if (IS_ARK_CONFIG(reboot_conf)){
        switch(reboot_conf->iso_mode) {
            case MODE_VSHUMD:
                newsize = patch_bootconf_vshumd(buffer, length);
                if (newsize > 0) result = newsize;
                break;
            case MODE_UPDATERUMD:
                newsize = patch_bootconf_updaterumd(buffer, length);
                if (newsize > 0) result = newsize;
                break;
            case MODE_NP9660:
            case MODE_MARCH33:
            case MODE_INFERNO:
                reboot_conf->iso_mode = MODE_INFERNO;
                newsize = patch_bootconf_inferno(buffer, length);
                if (newsize > 0) result = newsize;
                break;
            default:
                break;
        }
        //reboot variable set
        if(reboot_conf->rtm_mod.before && reboot_conf->rtm_mod.buffer && reboot_conf->rtm_mod.size)
        {
            //add reboot prx entry
            newsize = AddPRX(buffer, reboot_conf->rtm_mod.before, REBOOT_MODULE, reboot_conf->rtm_mod.flags);
            if(newsize > 0){
                result = newsize;
                setRebootModule();
            }
        }
    }

#ifdef MS_IPL
    // Insert tmctrl
    newsize = AddPRX(buffer, "/kd/lfatfs.prx", PATH_TMCTRL+sizeof(PATH_FLASH0)-2, 0x000000EF);
    if (newsize > 0) result = newsize;

    // Remove lfatfs
    newsize = RemovePrx(buffer, "/kd/lfatfs.prx", 0x000000EF);
    if (newsize > 0) result = newsize;
#endif
    
    return result;
}

// IO Patches

//io flags
extern volatile int rebootmodule_set;
extern volatile int rebootmodule_open;
extern volatile char *p_rmod;
extern volatile int size_rmod;
extern void* rtm_buf;
extern int rtm_size;

//io functions
extern int (* sceBootLfatOpen)(char * filename);
extern int (* sceBootLfatRead)(char * buffer, int length);
extern int (* sceBootLfatClose)(void);

#ifdef MS_IPL
char path[128];

int _sceBootLfatMount()
{
    return MsFatMount();
}

#endif

int _sceBootLfatRead(char * buffer, int length)
{
    //load on reboot module
    if(rebootmodule_open && p_rmod != NULL && size_rmod > 0)
    {
        int min;

        //copy load on reboot module
        min = size_rmod < length ? size_rmod : length;
        if (min > 0){
            memcpy(buffer, p_rmod, min);
            p_rmod += min;
            size_rmod -= min;
        }

        //set filesize
        return min;
    }

#ifdef MS_IPL
    return MsFatRead(buffer, length);
#else
    //forward to original function
    return sceBootLfatRead(buffer, length);
#endif
}

int _sceBootLfatOpen(char * filename)
{
    //load on reboot module open
    if(rebootmodule_set && strcmp(filename, REBOOT_MODULE) == 0)
    {
        //mark for read
        rebootmodule_open = 1;
        p_rmod = rtm_buf;
        size_rmod = rtm_size;

        //return success
        return 0;
    }

#ifdef MS_IPL
    strcpy(path, "/TM/DCARK");
	strcat(path, filename);

#ifdef PAYLOADEX
    if (memcmp(filename+4, "pspbtcnf", 8) == 0)
        memcpy(&path[strlen(path) - 4], "_dc.bin", 8);
#endif

	return MsFatOpen(path);
#else
    //forward to original function
    return sceBootLfatOpen(filename);
#endif
}

int _sceBootLfatClose(void)
{
    //reboot module close
    if(rebootmodule_open && p_rmod != NULL && size_rmod == 0)
    {
        //mark as closed
        rebootmodule_open = 0;
        p_rmod = NULL;
        size_rmod = 0;
        reboot_conf->rtm_mod.buffer = NULL;
        reboot_conf->rtm_mod.size = 0;

        //return success
        return 0;
    }
    
#ifdef MS_IPL
    return MsFatClose();
#else
    //forward to original function
    return sceBootLfatClose();
#endif
}

void setRebootModule(){
    rebootmodule_set = 1;
    rtm_buf = reboot_conf->rtm_mod.buffer;
    rtm_size = reboot_conf->rtm_mod.size;
}

void patchRebootIoPSP(){
    int patches = 3;
    for (u32 addr = reboot_start; addr<reboot_end && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x8E840000 || data == 0x8EA40000){
#ifdef MS_IPL
            int found = 0;
            for (int i=8; !found; i+=4) {
                if (IS_JAL(_lw(addr-i))) {
                    _sw(JAL(_sceBootLfatMount), addr-i);
                    found = 1;
                }
            }
#endif
            sceBootLfatOpen = K_EXTRACT_CALL(addr-4);
            _sw(JAL(_sceBootLfatOpen), addr-4);
            patches--;
        }
        else if (data == 0xAE840004 || data == 0xAEA30004){
            addr += 4;
            while (!IS_JAL(_lw(addr))) { addr += 4; }
            sceBootLfatRead = K_EXTRACT_CALL(addr);
            _sw(JAL(_sceBootLfatRead), addr);
            patches--;
        }
        else if (data == 0xAE930008 || data == 0xAEB40008){
            sceBootLfatClose = K_EXTRACT_CALL(addr-4);
            _sw(JAL(_sceBootLfatClose), addr-4);
            patches--;
        }
    }
    // Flush Cache
    flushCache();
}
