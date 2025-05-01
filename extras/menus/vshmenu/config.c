#include "config.h"

#include <pspiofilemgr.h>

#include <string.h>

#include "common.h"
#include "module2.h"
#include "kubridge.h"
#include <systemctrl.h>

#include "scepaf.h"
#include "battery.h"
#include "color.h"
#include "registry.h"
#include "vpl.h"
#include "advanced.h"

int button_accept(u32 button){
    vsh_Menu* vsh = vsh_menu_pointer();
    return (vsh->status.swap_xo && (button & PSP_CTRL_CROSS)) || (!vsh->status.swap_xo && (button & PSP_CTRL_CIRCLE));
}

int button_decline(u32 button){
    vsh_Menu* vsh = vsh_menu_pointer();
    return (vsh->status.swap_xo && (button & PSP_CTRL_CIRCLE)) || (!vsh->status.swap_xo && (button & PSP_CTRL_CROSS));
}

void config_reset(vsh_Menu *vsh){
    memset(&vsh->config.ark_menu, 0, sizeof(vsh->config.ark_menu));
    vsh->config.ark_menu.font = 1;
    vsh->config.ark_menu.sort_entries = 1;
    vsh->config.ark_menu.show_recovery = 1;
    vsh->config.ark_menu.text_glow = 3;
    vsh->config.ark_menu.screensaver = 2;
    vsh->config.ark_menu.redirect_ms0 = 1;
    vsh->config.ark_menu.menusize = 2;
    vsh->config.ark_menu.browser_icon0 = 1;
}

void config_load(vsh_Menu *vsh) {
    int is_pandora;
    char path[ARK_PATH_SIZE];
    scePaf_strcpy(path, vsh->config.ark.arkpath);
    strcat(path, MENU_SETTINGS);

    config_reset(vsh);

    int fp = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fp >= 0){
        sceIoRead(fp, &vsh->config.ark_menu, sizeof(vsh->config.ark_menu));
        sceIoClose(fp);
    }

    vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &vsh->status.swap_xo);
    is_pandora = battery_check();

    if (IS_VITA_ADR(vsh->config.p_ark) || is_pandora < 0){
        vsh->battery = 2;
    }
    else{
        vsh->battery = is_pandora;
    }

    if (IS_VITA_ADR(vsh->config.p_ark)){
        vsh->config.se.usbdevice_rdonly = 2;
    }
    
    vsh->codecs = codecs_activated();

    color_check_random(vsh);
}

void config_save(vsh_Menu *vsh){
    int fp;
    char path[ARK_PATH_SIZE];
    scePaf_strcpy(path, vsh->config.ark.arkpath);
    strcat(path, MENU_SETTINGS);

    fp = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fp >= 0){
        sceIoWrite(fp, &vsh->config.ark_menu, sizeof(vsh->config.ark_menu));
        sceIoClose(fp);
    }

}

void config_check(vsh_Menu *vsh) {
    color_check_random(vsh);
    if (scePaf_memcmp(&vsh->config.old_se, &vsh->config.se, sizeof(vsh->config.se)) || scePaf_memcmp(&vsh->config.old_ark_menu, &vsh->config.ark_menu, sizeof(vsh->config.ark_menu))){
        vctrlVSHUpdateConfig(&vsh->config.se);
        config_save(vsh);	
    }
}

void reset_ark_settings(vsh_Menu *vsh){
    const char settings[] =
        "always, usbcharge, on\n"
        "always, overclock, on\n"
        "always, wpa2, on\n"
        "always, launcher, off\n"
        "always, highmem, off\n"
        "always, mscache, on\n"
        "always, infernocache, on\n"
        "always, disablepause, off\n"
        "always, oldplugin, on\n"
        "always, skiplogos, off\n"
        "always, hidepics, off\n"
        "always, hibblock, on\n"
        "always, hidemac, on\n"
        "always, hidedlc, on\n"
        "always, noled, off\n"
        "always, noumd, off\n"
        "always, noanalog, off\n"
        "always, qaflags, on\n"
        "\n"
        "# The following games don't like Inferno Cache\n"
        "# Luxor - The Wrath of Set (the other Luxor game works fine)\n"
        "ULUS10201, infernocache, off\n"
        "# Flat-Out Head On (both US and EU)\n"
        "ULUS10328 ULES00968, infernocache, off\n"
        "\n"
        "# Enable Extra RAM on GTA LCS and VCS for CheatDeviceRemastered\n"
        "ULUS10041 ULUS10160 ULES00151 ULES00502, highmem, on\n"
    ;

    char arkMenuPath[ARK_PATH_SIZE];
    char arkSettingsPath[ARK_PATH_SIZE];
    scePaf_strcpy(arkMenuPath, vsh->config.ark.arkpath);
    scePaf_strcpy(arkSettingsPath, vsh->config.ark.arkpath);
    strcat(arkMenuPath, MENU_SETTINGS);
    strcat(arkSettingsPath, ARK_SETTINGS);
    int fd = sceIoOpen(arkMenuPath, PSP_O_RDONLY, 0);
    if(fd) {
        sceIoClose(fd);
        sceIoRemove(arkMenuPath);
        sceIoRemove(arkSettingsPath);
        sceKernelDelayThread(8000);
        int settings_file = sceIoOpen(arkSettingsPath, PSP_O_CREAT | PSP_O_WRONLY, 0777);
        sceIoWrite(settings_file, settings, sizeof(settings));
        sceKernelDelayThread(8000);
        sceIoClose(settings_file);

    }
    vsh->status.reset_vsh = 1; 

}

void import_classic_plugins(vsh_Menu *vsh, int devpath) {
    SceUID game, vsh_id, pops, plugins;
    int i = 0;
    int chunksize = 512;
    int bytesRead;
    char *buf = vpl_alloc(chunksize);
    char *gameChar = "game, ";
    int gameCharLength = scePaf_strlen(gameChar);
    char *vshChar = "vsh, ";
    int vshCharLength = scePaf_strlen(vshChar);
    char *popsChar = "pops, ";
    int popsCharLength = scePaf_strlen(popsChar);
    
    char* filename = (devpath)? "ef0:/SEPLUGINS/PLUGINS.TXT" : "ms0:/SEPLUGINS/PLUGINS.TXT";
    char* gamepath = (devpath)? "ef0:/SEPLUGINS/GAME.TXT" : "ms0:/SEPLUGINS/GAME.TXT";
    char* vshpath = (devpath)? "ef0:/SEPLUGINS/VSH.TXT" : "ms0:/SEPLUGINS/VSH.TXT";
    char* popspath = (devpath)? "ef0:/SEPLUGINS/POPS.TXT" : "ms0:/SEPLUGINS/POPS.TXT";

    game = sceIoOpen(gamepath, PSP_O_RDONLY, 0777);
    vsh_id = sceIoOpen(vshpath, PSP_O_RDONLY, 0777);
    pops = sceIoOpen(popspath, PSP_O_RDONLY, 0777);
    plugins = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    // GAME.txt
    scePaf_memset(buf, 0, chunksize);
    while ((bytesRead = sceIoRead(game, buf, chunksize)) > 0) {
        for(i = 0; i < bytesRead; i++) {
        	if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
        		sceIoWrite(plugins, gameChar, gameCharLength);
        	}
        	if (buf[i] == ' ' && i != 0)
        		sceIoWrite(plugins, ",", 1);
        	sceIoWrite(plugins, &buf[i], 1);
        }
    }
    
    sceIoClose(game);


    scePaf_memset(buf, 0, chunksize);

    // VSH.txt
    while ((bytesRead = sceIoRead(vsh_id, buf, chunksize)) > 0) {
        for(i = 0; i < bytesRead; i++) {
        	if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
        		sceIoWrite(plugins, vshChar, vshCharLength);
        	}
        	if (buf[i] == ' ' && i != 0)
        		sceIoWrite(plugins, ",", 1);
        	sceIoWrite(plugins, &buf[i], 1);
        }
    }

    sceIoClose(vsh_id);

    scePaf_memset(buf, 0, chunksize);


    // POP.txt
    while ((bytesRead = sceIoRead(pops, buf, chunksize)) > 0) {
        for(i = 0; i < bytesRead; i++) {
        	if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
        		sceIoWrite(plugins, popsChar, popsCharLength);
        	}
        	if (buf[i] == ' ' && i != 0)
        		sceIoWrite(plugins, ",", 1);
        	sceIoWrite(plugins, &buf[i], 1);
        }
    }

    sceIoClose(pops);

    sceIoClose(plugins);
    vpl_free(buf);

    vsh->status.reset_vsh = 1;
}
