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
#include <systemctrl.h>
#include <kubridge.h>
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

typedef struct
{
    int mode;
    int negative;
    char *item;
} GetItem;

GetItem GetItemes[] =
{
    { 1, 0, "USB Charge" },
    { 1, 0, "Overclock" },
    { 1, 0, "PowerSave" },
    { 1, 0, "Autoboot Launcher" },
    { 1, 0, "Disable Pause on PSP Go" },
    { 1, 0, "Force Extra Memory" },
    { 1, 0, "Memory Stick Speedup" },
    { 1, 0, "Inferno Cache" },
    { 1, 0, "Old Plugin Support on PSP Go" },
    { 1, 0, "Skip Sony Logos" },
    { 1, 0, "Hide PIC0 and PIC1" },
    { 1, 0, "Prevent hibernation deletion on PSP Go" },
    { 1, 0, "Hide MAC Address" },
    { 1, 0, "Hide DLC" },
    { 1, 0, "Turn off LEDs" },
};

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

char* ark_plugins_options[] = {
    (char*)"Off",
    (char*)"On",
    (char*)"Remove",
};

#define N_ITEMS (sizeof(GetItemes) / sizeof(GetItem))

typedef struct
{
    char *items[2];
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

STMOD_HANDLER previous;
CFWConfig config;

int psp_model;
ARKConfig ark_conf;

int startup = 1;

SceContextItem *context;
SceVshItem *new_item;
SceVshItem *new_item2;
void *xmb_arg0, *xmb_arg1;

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    kuKernelIcacheInvalidateAll();
}

int LoadTextLanguage(int new_id)
{
    static char *language[] = { "JA", "EN", "FR", "ES", "DE", "IT", "NL", "PT", "RU", "KO", "CHT", "CHS" };

    int id;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id);

    if(new_id >= 0)
    {
        if(new_id == id) return 0;
        id = new_id;
    }

    SceUID fd = -1;
    if (id < NELEMS(language)){
        char file[64];
        sce_paf_private_sprintf(file, "%sXMB_%s.TXT", ark_config->arkpath, language[id]);
        fd = sceIoOpen(file, PSP_O_RDONLY, 0);
    }

    if(fd >= 0)
    {
        /* Skip UTF8 magic */
        u32 magic;
        sceIoRead(fd, &magic, sizeof(magic));
        sceIoLseek(fd, (magic & 0xFFFFFF) == 0xBFBBEF ? 3 : 0, PSP_SEEK_SET);
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

    return 1;
}

void* addCustomVshItem(int id, char* text, int action_arg, SceVshItem* orig){
    SceVshItem* item = (SceVshItem *)sce_paf_private_malloc(sizeof(SceVshItem));
    sce_paf_private_memcpy(item, orig, sizeof(SceVshItem));

    item->id = id; //information board id
    item->action_arg = action_arg;
    item->play_sound = 1;
    sce_paf_private_strcpy(item->text, text);

    return item;
}

int AddVshItemPatched(void *a0, int topitem, SceVshItem *item)
{
    if(sce_paf_private_strcmp(item->text, "msgtop_sysconf_console") == 0)
    {
        startup = 0;
        
        LoadTextLanguage(-1);

        context = (SceContextItem *)sce_paf_private_malloc((4 * sizeof(SceContextItem)) + 1);

        new_item = addCustomVshItem(46, "msgtop_sysconf_configuration", sysconf_tnconfig_action_arg, item);
        AddVshItem(a0, topitem, new_item);

        new_item2 = addCustomVshItem(47, "msgtop_sysconf_plugins", sysconf_plugins_action_arg, item);
        AddVshItem(a0, topitem, new_item2);
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
    if (IS_VITA_ADR((&ark_conf))) return  ( i==0 || i==1 || i==2 || i==4 || i==8 || i==11 || i==13 || i == 14);
    else if (psp_model == PSP_1000) return ( i == 0 || i == 4 || i == 5 || i == 8 || i == 11);
    else if (psp_model == PSP_11000) return ( i == 4 || i == 8 || i == 11 || i == 12 );
    else if (psp_model != PSP_GO) return ( i == 4 || i == 8 || i == 11);
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
        context_mode = 11;
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
            utf8_to_unicode((wchar_t *)user_buffer, string.items[0]);
            return (wchar_t *)user_buffer;
        }
        else if(sce_paf_private_strcmp(name, "msgtop_sysconf_plugins") == 0)
        {
            utf8_to_unicode((wchar_t *)user_buffer, string.items[1]);
            return (wchar_t *)user_buffer;
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
                config.usbcharge,
                config.overclock,
                config.powersave,
                config.launcher,
                config.disablepause,
                config.highmem,
                config.mscache,
                config.infernocache,
                config.oldplugin,
                config.skiplogos,
                config.hidepics,
                config.hibblock,
                config.hidemac,
                config.hidedlc,
                config.noled,
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
				context_mode = 11;
				*value = plugin->active;
				return 0;
			}
        }
    }

    return vshGetRegistryValue(option, name, arg2, size, value);
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
				context_mode = 11;
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
        /* Restore */
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
                switch(context_mode)
                {
                    case 0:
                        HijackContext(*child, NULL, 0);
                        break;
                    case 11:
                        HijackContext(*child, ark_plugins_options, sizeof(ark_plugins_options) / sizeof(char *));
                        break;
                    default:
                        HijackContext(*child, ark_settings_options, sizeof(ark_settings_options) / sizeof(char *));
                        break;
                }
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

int OnModuleStart(SceModule2 *mod)
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

    return previous ? previous(mod) : 0;
}

int module_start(SceSize args, void *argp)
{        
    psp_model = kuKernelGetModel();

    sctrlHENGetArkConfig(&ark_conf);
    
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    
    sctrlHENGetArkConfig(ark_config);

    return 0;
}
