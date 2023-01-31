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

#include "include/main.h"
#include "include/utils.h"
#include "include/settings.h"

PSP_MODULE_INFO("XmbControl", 0x0007, 1, 0);

ARKConfig _arkconf;
ARKConfig* ark_config = &_arkconf;

typedef struct
{
    char *name;
    char *items[1];
    char *options[11];
} StringContainer;

StringContainer string;

#define N_STRINGS ((sizeof(string) / sizeof(char **)))

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
};

char* ark_settings_options[] = {
    (char*)"Disabled",
    (char*)"Always",
    (char*)"Game",
    (char*)"UMD",
    (char*)"Homebrew",
    (char*)"Pops",
    (char*)"VSH"
};

#define N_ITEMS (sizeof(GetItemes) / sizeof(GetItem))

char *items[] =
{
    "msgtop_sysconf_configuration",
};

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

char user_buffer[128];

STMOD_HANDLER previous;
CFWConfig config;

int psp_model;

int startup = 1;

SceContextItem *context;
SceVshItem *new_item;
void *xmb_arg0, *xmb_arg1;

void* my_malloc(size_t size){
    SceUID uid = sceKernelAllocPartitionMemory(2, "", PSP_SMEM_High, size+sizeof(u32), NULL);
    int* ptr = sceKernelGetBlockHeadAddr(uid);
    ptr[0] = uid;
    return &(ptr[1]);
}

void my_free(int* ptr){
    int uid = ptr[-1];
    sceKernelFreePartitionMemory(uid);
}

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    kuKernelIcacheInvalidateAll();
}

int LoadTextLanguage(int new_id)
{

    int id;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id);

    if(new_id >= 0)
    {
        if(new_id == id) return 0;
        id = new_id;
    }

    char file[64];

    SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);

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

int AddVshItemPatched(void *a0, int topitem, SceVshItem *item)
{
    if(sce_paf_private_strcmp(item->text, "msgtop_sysconf_console") == 0)
    {
        startup = 0;
        
        LoadTextLanguage(-1);

        new_item = (SceVshItem *)sce_paf_private_malloc(sizeof(SceVshItem));
        sce_paf_private_memcpy(new_item, item, sizeof(SceVshItem));

        new_item->id = 46; //information board id
        new_item->action_arg = sysconf_tnconfig_action_arg;
        new_item->play_sound = 0;
        sce_paf_private_strcpy(new_item->text, "msgtop_sysconf_tnconfig");

        context = (SceContextItem *)sce_paf_private_malloc((4 * sizeof(SceContextItem)) + 1);

        AddVshItem(a0, topitem, new_item);
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
    return OnXmbContextMenu(arg0, arg1);
}

int ExecuteActionPatched(int action, int action_arg)
{
    int old_is_cfw_config = is_cfw_config;

    if(action == sysconf_console_action)
    {
        if(action_arg == sysconf_tnconfig_action_arg)
        {
            loadSettings();
            int n = 1;

            sce_paf_private_memset(context, 0, (4 * sizeof(SceContextItem)) + 1);

            int i;
            for(i = 0; i < n; i++)
            {
                sce_paf_private_strcpy(context[i].text, items[i]);
                context[i].play_sound = 1;
                context[i].action = 0x80000;
                context[i].action_arg = i + 1;
            }

            new_item->context = context;

            OnXmbContextMenu(xmb_arg0, xmb_arg1);
            return 0;
        }

        is_cfw_config = 0;
    }
    else if(action == 0x80000)
    {
        is_cfw_config = action_arg;
        action = sysconf_console_action;
        action_arg = sysconf_console_action_arg;
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

void OnInitMenuPspConfigPatched()
{
    if(is_cfw_config == 1)
    {
        if(((u32 *)sysconf_option)[2] == 0)
        {
            int i;
            for(i = 0; i < N_ITEMS; i++)
            {
                AddSysconfContextItem(GetItemes[i].item, NULL, GetItemes[i].item);
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
        if(sce_paf_private_strcmp(name, "msgtop_sysconf_tnconfig") == 0)
        {
            utf8_to_unicode((wchar_t *)user_buffer, string.name);
            return (wchar_t *)user_buffer;
        }
        else if(sce_paf_private_strcmp(name, "msgtop_sysconf_configuration") == 0)
        {
            utf8_to_unicode((wchar_t *)user_buffer, string.items[0]);
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
        if(is_cfw_config == 1)
        {
            if(sce_paf_private_strcmp(name, "page_psp_config_umd_autoboot") == 0)
            {
                switch(context_mode)
                {
                    case 0:
                        HijackContext(*child, NULL, 0);
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

void PatchVshMain(u32 text_addr)
{
    AddVshItem = (void *)text_addr + 0x22648;
    ExecuteAction = (void *)text_addr + 0x16A70;
    UnloadModule = (void *)text_addr + 0x16E64;
    OnXmbPush = (void *)text_addr + 0x169B4;
    OnXmbContextMenu = (void *)text_addr + 0x16468;
    LoadStartAuth = (void *)text_addr + 0x5DA0;
    auth_handler = (void *)text_addr + 0x1A2D0;

    MAKE_CALL(text_addr + 0x20EFC, AddVshItemPatched);
    MAKE_CALL(text_addr + 0x16A4C, ExecuteActionPatched);
    MAKE_CALL(text_addr + 0x308B4, ExecuteActionPatched);
    MAKE_CALL(text_addr + 0x16C44, UnloadModulePatched);

    _sw(0x8C48000C, text_addr + 0x5ED4); //lw $t0, 12($v0)
    MAKE_CALL(text_addr + 0x5ED8, OnInitAuthPatched);

    REDIRECT_FUNCTION(text_addr + 0x3F1B0, scePafGetTextPatched);

    _sw((u32)OnXmbPushPatched, text_addr + 0x530A8);
    _sw((u32)OnXmbContextMenuPatched, text_addr + 0x530B4);

    ClearCaches();
}

void PatchAuthPlugin(u32 text_addr)
{
    OnRetry = (void *)text_addr + 0x5C8;
    MAKE_CALL(text_addr + 0x13A0, sceVshCommonGuiBottomDialogPatched);
    ClearCaches();
}

void PatchSysconfPlugin(u32 text_addr)
{
    AddSysconfItem = (void *)text_addr + 0x286AC;
    GetSysconfItem = (void *)text_addr + 0x23C74;
    OnInitMenuPspConfig = (void *)text_addr + 0x1D054;

    sysconf_unk = text_addr + 0x33600;
    sysconf_option = text_addr + 0x33ACC; //CHECK

    /* Allows more than 18 items */
    _sh(0xFF, text_addr + 0x29AC);

    MAKE_CALL(text_addr + 0x1714, vshGetRegistryValuePatched);
    MAKE_CALL(text_addr + 0x1738, vshSetRegistryValuePatched);

    MAKE_CALL(text_addr + 0x2A28, GetSysconfItemPatched);

    REDIRECT_FUNCTION(text_addr + 0x29A90, PAF_Resource_GetPageNodeByID_Patched);
    REDIRECT_FUNCTION(text_addr + 0x29B18, PAF_Resource_ResolveRefWString_Patched);
    REDIRECT_FUNCTION(text_addr + 0x299E0, scePafGetTextPatched);

    _sw((u32)OnInitMenuPspConfigPatched, text_addr + 0x30908);

    ClearCaches();
}

int OnModuleStart(SceModule2 *mod)
{
    char *modname = mod->modname;
    u32 text_addr = mod->text_addr;

    if(sce_paf_private_strcmp(modname, "vsh_module") == 0)
        PatchVshMain(text_addr);
    else if(sce_paf_private_strcmp(modname, "sceVshAuthPlugin_Module") == 0)
        PatchAuthPlugin(text_addr);
    else if(sce_paf_private_strcmp(modname, "sysconf_plugin_module") == 0)
        PatchSysconfPlugin(text_addr);

    return previous ? previous(mod) : 0;
}

int module_start(SceSize args, void *argp)
{        
    psp_model = kuKernelGetModel();
    
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    
    sctrlHENGetArkConfig(ark_config);

    loadSettings();

    return 0;
}
