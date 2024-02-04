/*
    6.39 TN-A, XmbControl
    Copyright (C) 2011, Total_Noob
    Copyright (C) 2011, Frostegater
    Copyright (C) 2011, codestation

    main.c: XmbControl main code
    
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

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <stddef.h>

#include "globals.h"
#include "macros.h"

#include "include/main.h"
#include "include/utils.h"
#include "include/settings.h"

#include "list.h"
#include "settings.h"
#include "plugins.h"

PSP_MODULE_INFO("XmbControl", 0x0007, 1, 5);

ARKConfig _arkconf;
ARKConfig* ark_config = &_arkconf;
extern List plugins;

static char custom_app_path[] = "ms0:/PSP/APP/EBOOT.PBP";

static char buf[64];

typedef struct
{
    int mode;
    int negative;
    char *item;
} GetItem;

GetItem GetItemes[] =
{
    { 2, 0, "USB Charge" },
    { 3, 0, "Overclock" },
    { 4, 0, "PowerSave" },
    { 5, 0, "Balanced Energy Mode" },
    { 6, 0, "Autoboot Launcher" },
    { 7, 0, "Disable Pause on PSP Go" },
    { 8, 0, "Force Extra Memory" },
    { 9, 0, "Memory Stick Speedup" },
    { 10, 0, "Inferno Cache" },
    { 11, 0, "Old Plugin Support on PSP Go" },
    { 12, 0, "Skip Sony Logos" },
    { 13, 0, "Hide PIC0 and PIC1" },
    { 14, 0, "Prevent hibernation deletion on PSP Go" },
    { 15, 0, "Hide MAC Address" },
    { 16, 0, "Hide DLC" },
    { 17, 0, "Turn off LEDs" },
    { 18, 0, "Disable UMD Drive" },
    { 19, 0, "Disable Analog Stick" },
    { 20, 0, "QA Flags" },
};

#define PLUGINS_CONTEXT 1

char* ark_settings_options[] = {
    (char*)"Disabled",
    (char*)"Always",
    (char*)"Game",
    (char*)"UMD/ISO",
    (char*)"Homebrew",
    (char*)"PS1",
    (char*)"XMB",
    (char*)"Launcher"
};

#define N_OPTS sizeof(ark_settings_options)/sizeof(ark_settings_options[0])

char* ark_settings_boolean[] = {
    (char*)"Off",
    (char*)"On"
};

char* ark_settings_infernocache[] = {
    (char*)"Off",
    (char*)"LRU",
    (char*)"RR"
};

char* ark_plugins_options[] = {
    (char*)"Off",
    (char*)"On",
    (char*)"Remove",
};

struct {
    int n;
    char** c;
} item_opts[] = {
    {0, NULL}, // None
    {3, ark_plugins_options}, // Plugins
    {N_OPTS, ark_settings_options}, // USB Charge
    {N_OPTS, ark_settings_options}, // Overclock
    {N_OPTS, ark_settings_options}, // PowerSave
    {N_OPTS, ark_settings_options}, // Balanced Energy
    {2, ark_settings_boolean}, // Autoboot Launcher
    {2, ark_settings_boolean}, // Disable Go Pause
    {N_OPTS, ark_settings_options}, // Extra RAM
    {N_OPTS, ark_settings_options}, // MS Speedup
    {3, ark_settings_infernocache}, // Inferno Cache
    {N_OPTS, ark_settings_options}, // Go Old Plugins 
    {2, ark_settings_boolean}, // Skip Sony logos
    {2, ark_settings_boolean}, // Hide PIC0 and PIC1
    {2, ark_settings_boolean}, // Prevent hib delete
    {2, ark_settings_boolean}, // Hide MAC
    {2, ark_settings_boolean}, // Hide DLC
    {N_OPTS, ark_settings_options}, // Turn off LEDs
    {2, ark_settings_boolean}, // Disable UMD Drive
    {2, ark_settings_boolean}, // Disable Analog Stick 
    {2, ark_settings_boolean}, // QA Flags
};

#define N_ITEMS (sizeof(GetItemes) / sizeof(GetItem))

typedef struct
{
    char *items[4];
    char *options[N_ITEMS];
} StringContainer;

StringContainer string;

#define N_STRINGS ((sizeof(string) / sizeof(char **)))

int count = 0;

int (* AddVshItem)(void *a0, int topitem, SceVshItem *item);
SceSysconfItem *(*GetSysconfItem)(void *a0, void *a1);
int (* ExecuteAction)(int action, int action_arg);
int (* UnloadModule)(int skip);
int (* OnXmbPush)(void *arg0, void *arg1);
int (* OnXmbContextMenu)(void *arg0, void *arg1);

void (* LoadStartAuth)();
int (* auth_handler)(int a0);
void (* OnRetry)();

void (* AddSysconfItem)(u32 *option, SceSysconfItem **item);
void (* OnInitMenuPspConfig)();

extern int GetPlugin(char *buf, int size, char *str, int *activated);
extern int ReadLine(SceUID fd, char *str);
extern int utf8_to_unicode(wchar_t *dest, char *src);


u32 sysconf_unk, sysconf_option;

int is_cfw_config = 0;
int unload = 0;

u32 backup[4];
int context_mode = 0;

char user_buffer[2*LINE_BUFFER_SIZE];

STMOD_HANDLER previous = NULL;
CFWConfig config;

int psp_model;
ARKConfig ark_conf;
SEConfig se_config;

int startup = 1;

SceContextItem *context;
SceVshItem *new_item;
SceVshItem *new_item2;
SceVshItem *new_item3;
SceVshItem *new_item4;
char image[4];
void *xmb_arg0, *xmb_arg1;

static unsigned char signup_item[] __attribute__((aligned(16))) = {
	0x2a, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x46, 
	0x55, 0x00, 0x00, 0x46, 0x59, 0x00, 0x00, 0x47, 0x43, 0x00, 0x00, 0x6d, 0x73, 0x67, 0x5f, 0x73, 
	0x69, 0x67, 0x6e, 0x75, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};


static unsigned char ps_store_item[] __attribute__((aligned(16))) = {
	0x2c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x46, 
	0x56, 0x00, 0x00, 0x46, 0x5a, 0x00, 0x00, 0x47, 0x44, 0x00, 0x00, 0x6d, 0x73, 0x67, 0x5f, 0x70, 
	0x73, 0x5f, 0x73, 0x74, 0x6f, 0x72, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

static unsigned char information_board_item[] __attribute__((aligned(16))) = {
	0x2e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x08, 0xdf, 0x09, 0x0a, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x46, 
	0x58, 0x00, 0x00, 0x47, 0x42, 0x00, 0x00, 0x47, 0x46, 0x00, 0x00, 0x6d, 0x73, 0x67, 0x5f, 0x69, 
	0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x5f, 0x62, 0x6f, 0x61, 0x72, 0x64, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    kuKernelIcacheInvalidateAll();
}

void exec_custom_launcher() {
	char menupath[ARK_PATH_SIZE];
	sce_paf_private_strcpy(menupath, ark_config->arkpath);
	strcat(menupath, VBOOT_PBP);

	SceIoStat stat; int res = sceIoGetstat(menupath, &stat);

	if (res >= 0){
		struct SceKernelLoadExecVSHParam param;
		sce_paf_private_memset(&param, 0, sizeof(param));
		param.size = sizeof(param);
		param.args = sce_paf_private_strlen(menupath) + 1;
		param.argp = menupath;
		param.key = "game";
		sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
	}
	else{
		// reboot system in proshell mode
		ark_config->recovery = 0;
		strcpy(ark_config->launcher, "PROSHELL"); // reboot in proshell mode

		struct KernelCallArg args;
		args.arg1 = ark_config;
		u32 setArkConfig = sctrlHENFindFunction("SystemControl", "SystemCtrlPrivate", 0x6EAFC03D);    
		kuKernelCall((void*)setArkConfig, &args);

		sctrlSESetUmdFile("");
	    sctrlSESetBootConfFileIndex(MODE_UMD);
        sctrlKernelExitVSH(NULL);
	}
}

void exec_custom_app(char *path) {

	struct SceKernelLoadExecVSHParam param;
	sce_paf_private_memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = sce_paf_private_strlen(path) + 1;
	param.argp = path;
	param.key = "game";
	sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
}


SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath){

    int pkg = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);
    if (pkg < 0)
        return 0;
     
    unsigned pkgsize = sceIoLseek32(pkg, 0, PSP_SEEK_END);
    unsigned size2 = 0;
     
    sceIoLseek32(pkg, 0, PSP_SEEK_SET);

    if (size != NULL)
        *size = 0;

    unsigned offset = 0;
    char name[64];
           
    while (offset != 0xFFFFFFFF){
        sceIoRead(pkg, &offset, 4);
        if (offset == 0xFFFFFFFF){
            sceIoClose(pkg);
            return 0;
        }
        unsigned namelength;
        sceIoRead(pkg, &namelength, 4);
        sceIoRead(pkg, name, namelength+1);
                   
        if (!strncmp(name, filename, namelength)){
            sceIoRead(pkg, &size2, 4);
    
            if (size2 == 0xFFFFFFFF)
                size2 = pkgsize;

            if (size != NULL)
                *size = size2 - offset;
     
            sceIoClose(pkg);
            return offset;
        }
    }
    return 0;
}

int LoadTextLanguage(int new_id)
{
    static char *languages[] = { "JP", "EN", "FR", "ES", "DE", "IT", "NL", "PT", "RU", "KO", "CHT", "CHS" };

    int id;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id);

    if(new_id >= 0)
    {
        if(new_id == id) return 0;
        id = new_id;
    }

    SceUID fd = -1;
    SceOff offset = 0;
    if (id < NELEMS(languages)){
        unsigned size = 0;
        char file[64];
        char pkgpath[ARK_PATH_SIZE];
        strcpy(pkgpath, ark_config->arkpath);
        strcat(pkgpath, "XMB_LANG.TXT");

        SceIoStat stat;

        if (sceIoGetstat(pkgpath, &stat) < 0){
            strcpy(pkgpath, ark_config->arkpath);
            strcat(pkgpath, "LANG.ARK");
            sce_paf_private_sprintf(file, "XMB_%s.TXT", languages[id]);
            offset = findPkgOffset(file, &size, pkgpath);
            if (!offset || !size)
                pkgpath[0] = 0;
        }
        fd = sceIoOpen(pkgpath, PSP_O_RDONLY, 0);
    }

    if(fd >= 0)
    {
        /* Skip UTF8 magic */
        u32 magic;
        sceIoLseek32(fd, offset, PSP_SEEK_SET);
        sceIoRead(fd, &magic, sizeof(magic));
        sceIoLseek(fd, (magic & 0xFFFFFF) == 0xBFBBEF ? offset+3 : offset, PSP_SEEK_SET);
    }

    char line[128];

    int i;
    int j = 0;
    for(i = 0; i < N_STRINGS; i++)
    {
        sce_paf_private_memset(line, 0, sizeof(line));

        if(fd >= 0)
        {
            ReadLine(fd, line);
        }
        else
        {
            sce_paf_private_strcpy(line, settings[j]); //to use the settings.h and its text entries.
            j++;
        }
        

        if(((char **)&string)[i]) sce_paf_private_free(((char **)&string)[i]);

        ((char **)&string)[i] = sce_paf_private_malloc(sce_paf_private_strlen(line) + 1);

        sce_paf_private_strcpy(((char **)&string)[i], line);
    }

    if(fd >= 0) sceIoClose(fd);

    if (IS_VITA(ark_config)){
        sce_paf_private_free(string.options[1]);
        string.options[1] = settings[24]; // replace "Overclock" with "PSP Overclock" on Vita
    }

    return 1;
}

int sysconf_action = 0;

void* addCustomVshItem(int id, char* text, int action_arg, SceVshItem* orig){
    SceVshItem* item = (SceVshItem *)sce_paf_private_malloc(sizeof(SceVshItem));
    sce_paf_private_memcpy(item, orig, sizeof(SceVshItem));

    item->id = id; // custom id
    item->action = sysconf_action;
    item->action_arg = action_arg;
    item->play_sound = 1;
    item->context = NULL;
    sce_paf_private_strcpy(item->text, text);

    return item;
}

int AddVshItemPatched(void *a0, int topitem, SceVshItem *item)
{

    #if 0
    sceIoMkdir("ms0:/vshitems", 0777);
    static char path[256];
    sprintf(path, "ms0:/vshitems/%s", item->text);
    int fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_TRUNC|PSP_O_CREAT, 0777);
    sceIoWrite(fd, item, sizeof(SceVshItem));
    sceIoClose(fd);
    #endif

    static int items_added = 0;

    if (sce_paf_private_strcmp(item->text, "msgtop_sysconf_console")==0){
        sysconf_action = item->action;
        LoadTextLanguage(-1);
    }

    if ( !items_added && // prevent adding more than once
        // Game Items
        (sce_paf_private_strcmp(item->text, "msgtop_game_gamedl")==0 ||
        sce_paf_private_strcmp(item->text, "msgtop_game_savedata")==0 ||
        // Extras Items
        sce_paf_private_strcmp(item->text, "msg_digitalcomics")==0 ||
        sce_paf_private_strcmp(item->text, "msg_bookreader")==0 ||
        sce_paf_private_strcmp(item->text, "msg_1seg")==0 ||
        sce_paf_private_strcmp(item->text, "msg_xradar_portable")==0 ||
        sce_paf_private_strcmp(item->text, "msg_tdmb")==0)
        )
    {
        items_added = 1;
        startup = 0;

        int cur_icon = 0;

        if (psp_model == PSP_11000){
            u32 value = 0;
            vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &value);
            cur_icon = !value;
        }

        // Add CFW Settings
        new_item = addCustomVshItem(81, "msgtop_sysconf_configuration", sysconf_tnconfig_action_arg, (cur_icon)?item:signup_item);
        AddVshItem(a0, topitem, new_item);

        // Add Plugins Manager
        new_item2 = addCustomVshItem(82, "msgtop_sysconf_plugins", sysconf_plugins_action_arg, (cur_icon)?item:ps_store_item);
        AddVshItem(a0, topitem, new_item2);

        // Add Custom Launcher
        new_item3 = addCustomVshItem(83, "msgtop_custom_launcher", sysconf_custom_launcher_arg, (cur_icon)?item:information_board_item);
        AddVshItem(a0, topitem, new_item3);
		if (se_config.magic != ARK_CONFIG_MAGIC) sctrlSEGetConfig(&se_config);
		
		SceIoStat stat; 
		int ebootFound;
		if(psp_model == PSP_GO) {
			custom_app_path[0] = 'e';
			custom_app_path[1] = 'f';
			ebootFound = sceIoGetstat(custom_app_path, &stat);
			if(ebootFound < 0) {
				custom_app_path[0] = 'm'; 
				custom_app_path[1] = 's';
				ebootFound = sceIoGetstat(custom_app_path, &stat);
			}
		}
		else {
			ebootFound = sceIoGetstat(custom_app_path, &stat);
		}

		if(ebootFound >= 0) {
        	new_item4 = addCustomVshItem(84, "msgtop_custom_app", sysconf_custom_app_arg, information_board_item);
        	AddVshItem(a0, topitem, new_item4);
		}
    }
	
	return AddVshItem(a0, topitem, item);

}

int OnXmbPushPatched(void *arg0, void *arg1)
{
    xmb_arg0 = arg0;
    xmb_arg1 = arg1;
    return OnXmbPush(arg0, arg1);
}

int OnXmbContextMenuPatched(void *arg0, void *arg1)
{
    new_item->context = NULL;
    new_item2->context = NULL;
    return OnXmbContextMenu(arg0, arg1);
}

int ExecuteActionPatched(int action, int action_arg)
{
    int old_is_cfw_config = is_cfw_config;

    if(action == sysconf_console_action)
    {
        if(action_arg == sysconf_tnconfig_action_arg)
        {
            is_cfw_config = 1;
            action = sysconf_console_action;
            action_arg = sysconf_console_action_arg;
        }
        else if (action_arg == sysconf_plugins_action_arg)
        {
            is_cfw_config = 2;
            action = sysconf_console_action;
            action_arg = sysconf_console_action_arg;
        }
        else if (action_arg == sysconf_custom_launcher_arg){
            exec_custom_launcher();
        }
        else if (action_arg == sysconf_custom_app_arg){
            exec_custom_app(custom_app_path);
        }
        else is_cfw_config = 0;
    }
    if(old_is_cfw_config != is_cfw_config)
    {
        sce_paf_private_memset(backup, 0, sizeof(backup));
        context_mode = 0;

        unload = 1;
    }

    return ExecuteAction(action, action_arg);
}

int UnloadModulePatched(int skip)
{
    if(unload)
    {
        skip = -1;
        unload = 0;
    }
    return UnloadModule(skip);
}

void AddSysconfContextItem(char *text, char *subtitle, char *regkey)
{
    SceSysconfItem *item = (SceSysconfItem *)sce_paf_private_malloc(sizeof(SceSysconfItem));

    item->id = 5;
    item->unk = (u32 *)sysconf_unk;
    item->regkey = regkey;
    item->text = text;
    item->subtitle = subtitle;
    item->page = "page_psp_config_umd_autoboot";

    ((u32 *)sysconf_option)[2] = 1;

    AddSysconfItem((u32 *)sysconf_option, &item);
}

int skipSetting(int i){
    if (IS_VITA_ADR((&ark_conf))) return  ( i==0 || i==5 || i==9 || i==12 || i==14 || i == 15 || i==16);
    else if (psp_model == PSP_1000) return ( i == 0 || i == 5 || i == 6 || i == 9 || i == 12);
    else if (psp_model == PSP_11000) return ( i == 5 || i == 9 || i == 12 || i == 13);
    else if (psp_model != PSP_GO) return ( i == 5 || i == 9 || i == 12);
	else if (psp_model == PSP_GO) return (i == 16);
    return 0;
}

void OnInitMenuPspConfigPatched()
{
    if(is_cfw_config == 1)
    {
        if(((u32 *)sysconf_option)[2] == 0)
        {
            loadSettings();
            int i;
            for(i = 0; i < N_ITEMS; i++)
            {
                if ( skipSetting(i) ){
                    continue;
                }
                else{
                    AddSysconfContextItem(GetItemes[i].item, NULL, GetItemes[i].item);
                }
            }
        }
    }
    else if (is_cfw_config == 2){
        if(((u32 *)sysconf_option)[2] == 0)
        {
            loadPlugins();
            for (int i=0; i<plugins.count; i++){
                Plugin* plugin = (Plugin*)(plugins.table[i]);
                if (plugin->name != NULL){
                    AddSysconfContextItem(plugin->name, plugin->surname, plugin->name);
                }
            }
        }
    }
    else
    {
        OnInitMenuPspConfig();
    }
}

SceSysconfItem *GetSysconfItemPatched(void *a0, void *a1)
{
    SceSysconfItem *item = GetSysconfItem(a0, a1);

    if(is_cfw_config == 1)
    {
        int i;
        for(i = 0; i < N_ITEMS; i++)
        {
            if(sce_paf_private_strcmp(item->text, GetItemes[i].item) == 0)
            {
                context_mode = GetItemes[i].mode;
            }
        }
    }
    else if (is_cfw_config == 2){
        context_mode = PLUGINS_CONTEXT;
    }
    return item;
}

wchar_t *scePafGetTextPatched(void *a0, char *name)
{
    if(name)
    {
        if(is_cfw_config == 1)
        {
            int i;
            for(i = 0; i < N_ITEMS; i++)
            {
                if(sce_paf_private_strcmp(name, GetItemes[i].item) == 0)
                {
                    utf8_to_unicode((wchar_t *)user_buffer, string.options[i]);
                    return (wchar_t *)user_buffer;
                }
            }
        }
        else if (is_cfw_config == 2){
            if(sce_paf_private_strncmp(name, "plugin_", 7) == 0){
                u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
				static char file[128];
				sce_paf_private_strcpy(file, plugin->path);

				char *p = sce_paf_private_strrchr(plugin->path, '/');
				if(p)
				{
					char *p2 = sce_paf_private_strchr(p + 1, '.');
					if(p2)
					{
						int len = (int)(p2 - (p + 1));
						sce_paf_private_strncpy(file, p + 1, len);
						file[len] = '\0';
					}
				}

				utf8_to_unicode((wchar_t *)user_buffer, file);
				return (wchar_t *)user_buffer;
            }
            else if (sce_paf_private_strncmp(name, "plugins", 7) == 0){
                u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
                utf8_to_unicode((wchar_t *)user_buffer, plugin->path);
				return (wchar_t *)user_buffer;
            }
        }
        if(sce_paf_private_strcmp(name, "msgtop_sysconf_configuration") == 0)
        {
			sce_paf_private_sprintf(buf, "%s %s", STAR, string.items[1]);
            utf8_to_unicode((wchar_t *)user_buffer, buf);
            return (wchar_t *)user_buffer;
        }
        else if(sce_paf_private_strcmp(name, "msgtop_sysconf_plugins") == 0)
        {
			sce_paf_private_sprintf(buf, "%s %s", STAR, string.items[2]);
			utf8_to_unicode((wchar_t *)user_buffer, buf);
			return (wchar_t *)user_buffer;
        }
        else if(sce_paf_private_strcmp(name, "msgtop_custom_launcher") == 0)
        {
			if(string.items[3]) {
				sce_paf_private_sprintf(buf, "%s %s", STAR, string.items[3]);
            	utf8_to_unicode((wchar_t *)user_buffer, buf);
            	return (wchar_t *)user_buffer;
			}
        }
		else if(sce_paf_private_strcmp(name, "msgtop_custom_app") == 0)
        {
			sce_paf_private_sprintf(buf, "%s %s", STAR, settings[23]);
            utf8_to_unicode((wchar_t *)user_buffer, buf);
            return (wchar_t *)user_buffer;
        }
		else if(sce_paf_private_strcmp(name, "msg_system_update") == 0) 
		{
            if (se_config.magic != ARK_CONFIG_MAGIC) sctrlSEGetConfig(&se_config);
            if (se_config.custom_update && string.items[0]) {
                utf8_to_unicode((wchar_t *)user_buffer, string.items[0]);
                return (wchar_t *)user_buffer;
            }
		}
		
    }

    wchar_t *res = scePafGetText(a0, name);

    return res;
}

int vshGetRegistryValuePatched(u32 *option, char *name, void *arg2, int size, int *value)
{
    if(name)
    {

        if(is_cfw_config == 1)
        {
            int configs[] =
            {
                config.usbcharge,		// 0
                config.overclock,		// 1
                config.powersave,		// 2
                config.defaultclock,	// 3
                config.launcher,		// 4
                config.disablepause,	// 5
                config.highmem,			// 6
                config.mscache,			// 7
                config.infernocache,	// 8
                config.oldplugin,		// 9
                config.skiplogos,		// 10
                config.hidepics,		// 11
                config.hibblock, 		// 12
                config.hidemac, 		// 13
                config.hidedlc,			// 14
                config.noled,			// 15
                config.noumd,			// 16
                config.noanalog,		// 17
                config.qaflags,		    // 18
            };
            
            int i;
            for(i = 0; i < N_ITEMS; i++)
            {
                if(sce_paf_private_strcmp(name, GetItemes[i].item) == 0)
                {
                    context_mode = GetItemes[i].mode;
                    *value = configs[i];
                    return 0;
                }
            }

        }
        else if (is_cfw_config == 2){
            if(sce_paf_private_strncmp(name, "plugin_", 7) == 0)
			{
				u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
				context_mode = PLUGINS_CONTEXT;
				*value = plugin->active;
				return 0;
			}
        }
    }

    int res = vshGetRegistryValue(option, name, arg2, size, value);

    #if 0
    char tmp[512];
    snprintf(tmp, 512, "%s, %p, %p, %d, %d\n", name, option, arg2, size, *value);
    int fd = sceIoOpen("ms0:/regvals.txt", PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
    sceIoWrite(fd, tmp, strlen(tmp));
    sceIoWrite(fd, "\n", 1);
    sceIoClose(fd);
    #endif

    return res;
}

int vshSetRegistryValuePatched(u32 *option, char *name, int size, int *value)
{
    if(name)
    {
        if(is_cfw_config == 1)
        {
            static int *configs[] =
            {
                &config.usbcharge,
                &config.overclock,
                &config.powersave,
                &config.defaultclock,
                &config.launcher,
                &config.disablepause,
                &config.highmem,
                &config.mscache,
                &config.infernocache,
                &config.oldplugin,
                &config.skiplogos,
                &config.hidepics,
                &config.hibblock,
                &config.hidemac,
                &config.hidedlc,
                &config.noled,
                &config.noumd,
                &config.noanalog,
                &config.qaflags,
            };
            
            int i;
            for(i = 0; i < N_ITEMS; i++)
            {
                if(sce_paf_private_strcmp(name, GetItemes[i].item) == 0)
                {
                    *configs[i] = GetItemes[i].negative ? !(*value) : *value;
                    saveSettings();
                    vctrlVSHExitVSHMenu(&config, NULL, 0);
                    return 0;
                }
            }
        }
        else if (is_cfw_config == 2){
            if(sce_paf_private_strncmp(name, "plugin_", 7) == 0)
			{
				u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
				context_mode = PLUGINS_CONTEXT;
				plugin->active = *value;
                savePlugins();
                if (*value == PLUGIN_REMOVED){
                    sctrlKernelExitVSH(NULL);
                }
            	return 0;
			}
        }
        if(sce_paf_private_strcmp(name, "/CONFIG/SYSTEM/XMB/language") == 0)
        {
            LoadTextLanguage(*value);
        }
    }

    return vshSetRegistryValue(option, name, size, value);
}

void HijackContext(SceRcoEntry *src, char **options, int n)
{
    SceRcoEntry *plane = (SceRcoEntry *)((u32)src + src->first_child);
    SceRcoEntry *mlist = (SceRcoEntry *)((u32)plane + plane->first_child);
    u32 *mlist_param = (u32 *)((u32)mlist + mlist->param);

    /* Backup */
    if(backup[0] == 0 && backup[1] == 0 && backup[2] == 0 && backup[3] == 0)
    {
        backup[0] = mlist->first_child;
        backup[1] = mlist->child_count;
        backup[2] = mlist_param[16];
        backup[3] = mlist_param[18];
    }

    if(context_mode)
    {
        SceRcoEntry *base = (SceRcoEntry *)((u32)mlist + mlist->first_child);

        SceRcoEntry *item = (SceRcoEntry *)sce_paf_private_malloc(base->next_entry * n);
        u32 *item_param = (u32 *)((u32)item + base->param);

        mlist->first_child = (u32)item - (u32)mlist;
        mlist->child_count = n;
        mlist_param[16] = 13;
        mlist_param[18] = 6;

        int i;
        for(i = 0; i < n; i++)
        {
            sce_paf_private_memcpy(item, base, base->next_entry);

            item_param[0] = 0xDEAD;
            item_param[1] = (u32)options[i];

            if(i != 0) item->prev_entry = item->next_entry;
            if(i == n - 1) item->next_entry = 0;

            item = (SceRcoEntry *)((u32)item + base->next_entry);
            item_param = (u32 *)((u32)item + base->param);
        }
    }
    else
    {
        // Restore
        mlist->first_child = backup[0];
        mlist->child_count = backup[1];
        mlist_param[16] = backup[2];
        mlist_param[18] = backup[3];
    }

    sceKernelDcacheWritebackAll();
}

int PAF_Resource_GetPageNodeByID_Patched(void *resource, char *name, SceRcoEntry **child)
{
    int res = PAF_Resource_GetPageNodeByID(resource, name, child);

	if(name)
    {
        if(is_cfw_config == 1 || is_cfw_config == 2)
        {
            if(sce_paf_private_strcmp(name, "page_psp_config_umd_autoboot") == 0)
            {
                HijackContext(*child, item_opts[context_mode].c, item_opts[context_mode].n);
            }
        }
    }

    return res;
}

int PAF_Resource_ResolveRefWString_Patched(void *resource, u32 *data, int *a2, char **string, int *t0)
{
    if(data[0] == 0xDEAD)
    {
        utf8_to_unicode((wchar_t *)user_buffer, (char *)data[1]);
        *(wchar_t **)string = (wchar_t *)user_buffer;
        return 0;
    }

    return PAF_Resource_ResolveRefWString(resource, data, a2, string, t0);
}

int auth_handler_new(int a0)
{
    startup = a0;
    return auth_handler(a0);
}

int OnInitAuthPatched(void *a0, int (* handler)(), void *a2, void *a3, int (* OnInitAuth)())
{
    return OnInitAuth(a0, startup ? auth_handler_new : handler, a2, a3);
}

int sceVshCommonGuiBottomDialogPatched(void *a0, void *a1, void *a2, int (* cancel_handler)(), void *t0, void *t1, int (* handler)(), void *t3)
{
    return sceVshCommonGuiBottomDialog(a0, a1, a2, startup ? OnRetry : (void *)cancel_handler, t0, t1, handler, t3);
}

void PatchVshMain(u32 text_addr, u32 text_size)
{
    int patches = 13;
    u32 scePafGetText_call = _lw(&scePafGetText);

    for (u32 addr=text_addr; addr<text_addr+text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x00063100){
            AddVshItem = U_EXTRACT_CALL(addr+12);
            MAKE_CALL(addr + 12, AddVshItemPatched);
            patches--;
        }
        else if (data == 0x3A14000F){
            ExecuteAction = (void*)addr-72;
            MAKE_CALL(addr - 72 - 36, ExecuteActionPatched);
            patches--;
        }
        else if (data == 0xA0C3019C){
            UnloadModule = (void*)addr-52;
            patches--;
        }
        else if (data == 0x9042001C){
            OnXmbPush = (void*)addr-124;
            patches--;
        }
        else if (data == 0x00021202 && OnXmbContextMenu==NULL){
            OnXmbContextMenu = (void*)addr-24;
            patches--;
        }
        else if (data == 0x34420080 && LoadStartAuth==NULL){
            LoadStartAuth = (void*)addr-208;
            patches--;
        }
        else if (data == 0xA040014D){
            auth_handler = (void*)addr-32;
            patches--;
        }
        else if (data == 0x8E050038){
            MAKE_CALL(addr + 4, ExecuteActionPatched);
            patches--;
        }
        else if (data == 0xAC520124){
            MAKE_CALL(addr + 4, UnloadModulePatched);
            patches--;
        }
        else if (data == 0x24040010 && _lw(addr+20) == 0x0040F809){
            _sw(0x8C48000C, addr + 16); //lw $t0, 12($v0)
            MAKE_CALL(addr + 20, OnInitAuthPatched);
            patches--;
        }
        else if (data == scePafGetText_call){
            REDIRECT_FUNCTION(addr, scePafGetTextPatched);
            patches--;
        }
        else if (data == OnXmbPush && OnXmbPush != NULL && addr > text_addr+0x50000){
            _sw((u32)OnXmbPushPatched, addr);
            patches--;
        }
        else if (data == OnXmbContextMenu && OnXmbContextMenu != NULL && addr > text_addr+0x50000){
            _sw((u32)OnXmbContextMenuPatched, addr);
            patches--;
        }
    }
    ClearCaches();
}

void PatchAuthPlugin(u32 text_addr, u32 text_size)
{
    for (u32 addr=text_addr; addr<text_addr+text_size; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x27BE0040){
            u32 a = addr-4;
            do {a-=4;} while (_lw(a) != 0x27BDFFF0);
            OnRetry = (void*)a;
        }
        else if (data == 0x44816000 && _lw(addr-4) == 0x3C0141F0){
            MAKE_CALL(addr+4, sceVshCommonGuiBottomDialogPatched);
            break;
        }
    }
    ClearCaches();
}

void PatchSysconfPlugin(u32 text_addr, u32 text_size)
{
    u32 PAF_Resource_GetPageNodeByID_call = _lw(&PAF_Resource_GetPageNodeByID);
    u32 PAF_Resource_ResolveRefWString_call = _lw(&PAF_Resource_ResolveRefWString);
    u32 scePafGetText_call = _lw(&scePafGetText);
    int patches = 10;
    for (u32 addr=text_addr; addr<text_addr+text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x24420008 && _lw(addr-4) == 0x00402821){
            AddSysconfItem = (void*)addr-36;
            patches--;
        }
        else if (data == 0x8C840008 && _lw(addr+4) == 0x27BDFFD0){
            GetSysconfItem = (void*)addr;
            patches--;
        }
        else if (data == 0xAFBF0060 && _lw(addr+4) == 0xAFB3005C && _lw(addr-12) == 0xAFB00050){
            OnInitMenuPspConfig = (void*)addr-20;
            patches--;
        }
        else if (data == 0x2C420012){
            // Allows more than 18 items
            _sh(0xFF, addr);
            patches--;
        }
        else if (data == 0x01202821){
            MAKE_CALL(addr + 8, vshGetRegistryValuePatched);
            MAKE_CALL(addr + 44, vshSetRegistryValuePatched);
            patches--;
        }
        else if (data == 0x2C620012 && _lw(addr-4) == 0x00408821){
            MAKE_CALL(addr - 16, GetSysconfItemPatched);
            patches--;
        }
        else if (data == OnInitMenuPspConfig && OnInitMenuPspConfig != NULL){
            _sw((u32)OnInitMenuPspConfigPatched, addr);
            patches--;
        }
        else if (data == PAF_Resource_GetPageNodeByID_call){
            REDIRECT_FUNCTION(addr, PAF_Resource_GetPageNodeByID_Patched);
            patches--;
        }
        else if (data == PAF_Resource_ResolveRefWString_call){
            REDIRECT_FUNCTION(addr, PAF_Resource_ResolveRefWString_Patched);
            patches--;
        }
        else if (data == scePafGetText_call){
            REDIRECT_FUNCTION(addr, scePafGetTextPatched);
            patches--;
        }
    }

    for (u32 addr=text_addr+0x33000; addr<text_addr+0x40000; addr++){
        if (strcmp((char*)addr, "fiji") == 0){
            sysconf_unk = addr+216;
            if (_lw(sysconf_unk+4) == 0) sysconf_unk -= 4; // adjust on TT/DT firmware
            sysconf_option = sysconf_unk + 0x4cc; //CHECK
            break;
        }
    }

    ClearCaches();
}

void OnModuleStart(SceModule2 *mod)
{
    char *modname = mod->modname;
    u32 text_addr = mod->text_addr;
    u32 text_size = mod->text_size;

    if(strcmp(modname, "vsh_module") == 0)
        PatchVshMain(text_addr, text_size);
    else if(strcmp(modname, "sceVshAuthPlugin_Module") == 0)
        PatchAuthPlugin(text_addr, text_size);
    else if(strcmp(modname, "sysconf_plugin_module") == 0)
        PatchSysconfPlugin(text_addr, text_size);

    if (previous) previous(mod);
}

int module_start(SceSize args, void *argp)
{        
    psp_model = kuKernelGetModel();

    sctrlHENGetArkConfig(&ark_conf);
    
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);

    sctrlHENGetArkConfig(ark_config);
    

    return 0;
}
