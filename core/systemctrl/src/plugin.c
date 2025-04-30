/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <string.h>
#include <pspinit.h>
#include <pspmodulemgr.h>
#include <pspiofilemgr.h>
#include <ark.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include "rebootex.h"
#include "plugin.h"
#include "libs/graphics/graphics.h"

#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

extern ARKConfig* ark_config;
extern SEConfig se_config;

#define MAX_PLUGINS 32
#define MAX_PLUGIN_PATH 64

typedef struct{
    int count;
    char paths[MAX_PLUGINS][MAX_PLUGIN_PATH];
}Plugins;

Plugins* plugins = NULL;

static int ef0PluginHandler(const char* path, int modid);
int (*plugin_handler)(const char* path, int modid) = &ef0PluginHandler;

enum {
    RUNLEVEL_UNKNOWN,
    RUNLEVEL_VSH,
    RUNLEVEL_UMD,
    RUNLEVEL_POPS,
    RUNLEVEL_HOMEBREW,
};
static int cur_runlevel = RUNLEVEL_UNKNOWN;

int disable_plugins = 0;
int disable_settings = 0;
int is_plugins_loading = 0;

int isLoadingPlugins(){
    return is_plugins_loading;
}

static addPlugin(char* path){
    for (int i=0; i<plugins->count; i++){
        if (stricmp(plugins->paths[i], path) == 0)
            return; // plugin already added
    }
    if (plugins->count < MAX_PLUGINS)
        strcpy(plugins->paths[plugins->count++], path);
}

static removePlugin(char* path){
    for (int i=0; i<plugins->count; i++){
        if (stricmp(plugins->paths[i], path) == 0){
            if (--plugins->count > i){
                strcpy(plugins->paths[i], plugins->paths[plugins->count]);
            }
            break;
        }
    }
}

// Load and Start Plugin Module
static void startPlugins()
{
    for (int i=0; i<plugins->count; i++){
        int res = 0;
        char path[MAX_PLUGIN_PATH];
        strcpy(path, plugins->paths[i]);
        // Load Module
        int uid = sceKernelLoadModule(path, 0, NULL);
        if (uid<0){ // try in ARK path
            strcpy(path, ark_config->arkpath);
            strcat(path, plugins->paths[i]);
            uid = sceKernelLoadModule(path, 0, NULL);
        }
        if (uid > 0){
            // Call handler
            if (plugin_handler){
                res = plugin_handler(path, uid);
                // Unload Module on Error
                if (res < 0){
                    sceKernelUnloadModule(uid);
                    continue;
                }
            }
            // Start Module
            res = sceKernelStartModule(uid, strlen(path) + 1, path, NULL, NULL);
            // Unload Module on Error
            if (res < 0) sceKernelUnloadModule(uid);
        }
    }
}

static int isVshRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype >= 0x200) cur_runlevel = RUNLEVEL_VSH;
    }
    return cur_runlevel == RUNLEVEL_VSH;
}

static int isPopsRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype == 0x144 || apitype == 0x155) cur_runlevel = RUNLEVEL_POPS;
    }
    return cur_runlevel == RUNLEVEL_POPS;
}

static int isUmdRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype == 0x120 || apitype == 0x160
                || (apitype >= 0x123 && apitype <= 0x126)
                || apitype == 0x130 || apitype == 0x160
                || (apitype >= 0x110 && apitype <= 0x115))
            cur_runlevel = RUNLEVEL_UMD;
    }
    return cur_runlevel == RUNLEVEL_UMD;
}

static int isHomebrewRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype == 0x141 || apitype == 0x152) cur_runlevel = RUNLEVEL_HOMEBREW;
    }
    return cur_runlevel == RUNLEVEL_HOMEBREW;
}

static int isLauncher(){
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    if (ark_config->launcher[0]) strcat(path, ark_config->launcher);
    else                         strcat(path, VBOOT_PBP);
    return (strcmp(path, sceKernelInitFileName())==0);
}

static int isPath(char* runlevel){
    return (
        strcasecmp(runlevel, sceKernelInitFileName())==0 ||
        strcasecmp(runlevel, sctrlSEGetUmdFile())==0
    );
}

static int isGameId(char* runlevel){
    if (rebootex_config.game_id[0] == 0) return 0;
    char gameid[10]; memset(gameid, 0, sizeof(gameid));
    memcpy(gameid, rebootex_config.game_id, 9);
    lowerString(gameid, gameid, strlen(gameid)+1);
    return (strstr(runlevel, gameid) != NULL);
}

// Runlevel Check
static int matchingRunlevel(char * runlevel)
{
    
    lowerString(runlevel, runlevel, strlen(runlevel)+1);

    int ret = 0;

    if (strcasecmp(runlevel, "all") == 0 || strcasecmp(runlevel, "always") == 0) return 1; // always on

    if (strchr(runlevel, '/')){
        // it's a path
        return isPath(runlevel);
    }

    if(isVshRunlevel()){
        return (strstr(runlevel, "vsh") != NULL || strstr(runlevel, "xmb") != NULL);
    }

    if(isPopsRunlevel()){
        // check if plugin loads on specific game
        if (isGameId(runlevel)) return 1;
        // check keywords
        return (strstr(runlevel, "pops") != NULL || strstr(runlevel, "ps1") != NULL || strstr(runlevel, "psx") != NULL); // PS1 games only
    }
    
    if(isHomebrewRunlevel()) {
        if (strstr(runlevel, "launcher") != NULL){
        	// check if running custom launcher
        	if (isLauncher()) return 1;
        }
        if (strstr(runlevel, "app") != NULL || strstr(runlevel, "homebrew") != NULL || strstr(runlevel, "game") != NULL) return 1; // homebrews only
    }

    if(isUmdRunlevel()) {
        // check if plugin loads on specific game
        if (isGameId(runlevel)) return 1;
        // check keywords
        if(strstr(runlevel, "umd") != NULL || strstr(runlevel, "psp") != NULL || strstr(runlevel, "umdemu") != NULL || strstr(runlevel, "game") != NULL) return 1; // Retail games only
    }
    
    // Unsupported Runlevel (we don't touch those to keep stability up)
    return 0;
}

// Boolean String Parser
static int booleanOn(char * text)
{
    // Different Variations of "true"
    if(stricmp(text, "true") == 0 || stricmp(text, "on") == 0 ||
        strcmp(text, "1") == 0 || stricmp(text, "enabled") == 0)
            return 1;
    
    // Default to False
    return 0;
}

// Whitespace Detection
int isspace(int c)
{
    // Whitespaces
    if(c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f' || c == '\n')
        return 1;
    
    // Normal Character
    return 0;
}

// Trim Leading and Trailing Whitespaces
char * strtrim(char * text)
{
    // Invalid Argument
    if(text == NULL) return NULL;
    
    // Remove Leading Whitespaces
    while(isspace(text[0])) text++;
    
    // Scan Position
    int pos = strlen(text)-1;
    if (pos<0) return text;

    // Find Trailing Whitespaces
    while(isspace(text[pos])) pos--;
    
    // Terminate String
    text[pos+1] = (char)0;
    
    // Return Trimmed String
    return text;
}

// Read Line from File Descriptor
static char * readLine(int fd, char * buf, unsigned int buflen)
{
    // Valid Arguments
    if(fd >= 0 && buf != NULL && buflen > 0)
    {
        // Clean Memory
        memset(buf, 0, buflen);
        
        // Buffer Position
        unsigned int pos = 0;
        
        // Read Text
        while(pos < buflen - 1 && sceIoRead(fd, buf + pos, 1) == 1)
        {
            // Carriage Return (Windows)
            if(buf[pos] == '\r')
            {
                // Next Symbol
                char c = 0;
                
                // Read Next Symbol (to prevent double tapping)
                if(sceIoRead(fd, &c, 1) == 1)
                {
                    // Newline
                    if(c == '\n') break;
                    
                    // Rewind File
                    sceIoLseek32(fd, -1, PSP_SEEK_CUR);
                }
                
                // Handle as Newline
                break;
            }
            
            // Newline
            if(buf[pos] == '\n') break;
            
            // Move Position
            pos++;
        }
        
        // End of File
        if(pos == 0 && buf[pos] == 0) return NULL;
        
        // Remove \r\n
        if(buf[pos] == '\r' || buf[pos] == '\n') buf[pos] = 0;
        
        // Return Line Buffer
        return buf;
    }
    
    // Invalid Arguments
    return NULL;
}

// Parse and Process Line
static void processLine(char * line, void (*enabler)(char*), void (*disabler)(char*))
{
    // Skip Comment Lines
    if(!enabler || line == NULL || strncmp(line, "//", 2) == 0 || line[0] == ';' || line[0] == '#')
        return;
    
    // String Token
    char * runlevel = line;
    char * path = NULL;
    char * enabled = NULL;
    
    // Original String Length
    unsigned int length = strlen(line);
    
    // Fetch String Token
    unsigned int i = 0; for(; i < length; i++)
    {
        // Got all required Token
        if(enabled != NULL)
        {
            // Handle Trailing Comments as Terminators
            if(strncmp(line + i, "//", 2) == 0 || line[i] == ';' || line[i] == '#')
            {
                // Terminate String
                line[i] = 0;
                
                // Stop Token Scan
                break;
            }
        }
        
        // Found Delimiter
        if(line[i] == LINE_TOKEN_DELIMITER)
        {
            // Terminate String
            line[i] = 0;
            
            // Path Start
            if(path == NULL) path = line + i + 1;
            
            // Enabled Start
            else if(enabled == NULL) enabled = line + i + 1;
            
            // Got all Data
            else break;
        }
    }
    
    // Unsufficient Plugin Information
    if(enabled == NULL) return;
    
    // Trim Whitespaces
    runlevel = strtrim(runlevel);
    path = strtrim(path);
    enabled = strtrim(enabled);
    
    // Matching Plugin Runlevel
    if(matchingRunlevel(runlevel))
    {
        // Enabled Plugin
        if(booleanOn(enabled))
        {
            // Start Plugin
            enabler(path);
        }
        else{
            if (disabler) disabler(path);
        }
    }
}

// Load Plugins
static int ProcessConfigFile(char* path, void (*enabler)(char*), void (*disabler)(char*))
{

    int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    
    // Opened Plugin Config
    if(fd >= 0)
    {
        // Allocate Line Buffer
        char * line = (char *)oe_malloc(LINE_BUFFER_SIZE);
        
        // Buffer Allocation Success
        if(line != NULL)
        {
            // Read Lines
            while(readLine(fd, line, LINE_BUFFER_SIZE) != NULL)
            {
                if (line[0] == 0) continue; // empty line
                // Process Line
                processLine(strtrim(line), enabler, disabler);
            }
            
            // Free Buffer
            oe_free(line);
        }
        
        // Close Plugin Config
        sceIoClose(fd);
        return 0;
    }
    return -1;
}

static void settingsHandler(char* path, u8 enabled){
    int apitype = sceKernelInitApitype();
    if (strcasecmp(path, "overclock") == 0){ // set CPU speed to max
        if (enabled)
            se_config.cpubus_clock = 1;
        else if (se_config.cpubus_clock == 1) se_config.cpubus_clock = 0;
    }
    else if (strcasecmp(path, "powersave") == 0){ // underclock to save battery
        if (apitype != 0x144 && apitype != 0x155){ // prevent operation in pops
            if (enabled)
                se_config.cpubus_clock = 2;
            else if (se_config.cpubus_clock == 2) se_config.cpubus_clock = 0;
        }
    }
    else if (strcasecmp(path, "defaultclock") == 0){
        if (apitype != 0x144 && apitype != 0x155){ // prevent operation in pops
            if (enabled)
                se_config.cpubus_clock = 3;
            else if (se_config.cpubus_clock == 3) se_config.cpubus_clock = 0;
        }
    }
    else if (strcasecmp(path, "wpa2") == 0){ // wpa2 support
        se_config.wpa2 = enabled;
    }
    else if (strcasecmp(path, "usbcharge") == 0){ // enable usb charging
        se_config.usbcharge = enabled;
    }
    else if (strcasecmp(path, "highmem") == 0){ // enable high memory
        if ( (apitype == 0x120 || (apitype >= 0x123 && apitype <= 0x126)) && sceKernelFindModuleByName("sceUmdCache_driver") != NULL){
            // don't allow high memory in UMD when cache is enabled
            return;
        }
        se_config.force_high_memory = enabled;
    }
    else if (strcasecmp(path, "mscache") == 0){
        se_config.msspeed = enabled; // enable ms cache for speedup
    }
    else if (strcasecmp(path, "disablepause") == 0){ // disable pause game feature on psp go
        if (apitype != 0x144 && apitype != 0x155 && apitype !=  0x210 && apitype !=  0x220) // prevent in pops and vsh
            se_config.disable_pause = enabled;
    }
    else if (strcasecmp(path, "launcher") == 0){ // replace XMB with custom launcher
        se_config.launcher_mode = enabled;
    }
    else if (strcasecmp(path, "oldplugin") == 0){ // redirect ms0 to ef0 on psp go
        se_config.oldplugin = enabled;
    }
    else if (strncasecmp(path, "infernocache", 12) == 0){
        char* c = strchr(path, ':');
        se_config.iso_cache = enabled;
        if (enabled && c){
            if (strcasecmp(c+1, "lru") == 0) se_config.iso_cache = 1;
            else if (strcasecmp(c+1, "rr") == 0) se_config.iso_cache = 2;
        }
    }
    else if (strcasecmp(path, "noled") == 0){
        se_config.noled = enabled;
    }
    else if (strcasecmp(path, "noumd") == 0){
        se_config.noumd = enabled;
    }
    else if (strcasecmp(path, "noanalog") == 0){
        se_config.noanalog = enabled;
    }
    else if (strcasecmp(path, "region_jp") == 0){
        se_config.umdregion = (enabled)?UMD_REGION_JAPAN:0;
    }
    else if (strcasecmp(path, "region_us") == 0){
        se_config.umdregion = (enabled)?UMD_REGION_AMERICA:0;
    }
    else if (strcasecmp(path, "region_eu") == 0){
        se_config.umdregion = (enabled)?UMD_REGION_EUROPE:0;
    }
    else if (strncasecmp(path, "fakeregion_", 11) == 0){
        int r = atoi(path+11);
        se_config.vshregion = (enabled)?r:0;
    }
    else if (strncasecmp(path, "umdseek_", 8) == 0){
        int r = atoi(path+8);
        se_config.umdseek = (enabled)?r:0;
    }
    else if (strncasecmp(path, "umdspeed_", 9) == 0){
        int r = atoi(path+9);
        se_config.umdspeed = (enabled)?r:0;
    }
    else if (strcasecmp(path, "hibblock") == 0){ // block hibernation
        se_config.hibblock = enabled;
    }
    else if (strncasecmp(path, "skiplogos", 9) == 0){
        char* c = strchr(path, ':');
        se_config.skiplogos = enabled;
        if (enabled && c){
            if (strcasecmp(c+1, "gameboot") == 0) se_config.skiplogos = 2;
            else if (strcasecmp(c+1, "coldboot") == 0) se_config.skiplogos = 3;
        }
    }
    else if (strncasecmp(path, "hidepics", 8) == 0){ // hide PIC0 and PIC1
        char* c = strchr(path, ':');
        se_config.hidepics = enabled;
        if (enabled && c){
            if (strcasecmp(c+1, "pic0") == 0) se_config.hidepics = 2;
            else if (strcasecmp(c+1, "pic1") == 0) se_config.hidepics = 3;
        }
    }
    else if (strcasecmp(path, "hidemac") == 0){ // hide mac address
        se_config.hidemac = enabled;
    }
    else if (strcasecmp(path, "hidedlc") == 0){ // hide mac address
        se_config.hidedlc = enabled;
    }
    else if (strcasecmp(path, "qaflags") == 0){ // QA Flags
        se_config.qaflags = enabled;
    }
}

static void settingsEnabler(char* path){
    settingsHandler(path, 1);
}

static void settingsDisabler(char* path){
    settingsHandler(path, 0);
}

void LoadPlugins(){
    if (disable_plugins || sceKernelFindModuleByName("DesCemManager")!=NULL)
        return; // don't load plugins in recovery mode
    is_plugins_loading = 1;
    // allocate resources
    plugins = oe_malloc(sizeof(Plugins));
    plugins->count = 0; // initialize plugins table
    // Open Plugin Config from ARK's installation folder
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, PLUGINS_FILE);
    ProcessConfigFile(path, &addPlugin, &removePlugin);
    // Open Plugin Config from SEPLUGINS
    ProcessConfigFile(PLUGINS_PATH, addPlugin, removePlugin);
    // On PSP Go (only if ms0 isn't already redirected to ef0)
    if (!sctrlKernelMsIsEf()) ProcessConfigFile(PLUGINS_PATH_GO, addPlugin, removePlugin);
    // Flash0 plugins
    ProcessConfigFile(PLUGINS_PATH_FLASH, addPlugin, removePlugin);
    // start all loaded plugins
    startPlugins();
    // free resources
    oe_free(plugins);
    plugins = NULL;
    is_plugins_loading = 0;
}

void loadSettings(){
    if (disable_settings)
        return; // don't load settings in recovery mode
    // process settings file
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, ARK_SETTINGS);
    if (ProcessConfigFile(path, settingsEnabler, settingsDisabler) < 0) // try external settings
        ProcessConfigFile(ARK_SETTINGS_FLASH, settingsEnabler, settingsDisabler); // retry flash1 settings
    se_config.magic = ARK_CONFIG_MAGIC;

    if (!se_config.force_high_memory){
        int apitype = sceKernelInitApitype();
        if (apitype == 0x141 || apitype == 0x152){
            int paramsize=4;
            int use_highmem = 0;
            if (sctrlGetInitPARAM("MEMSIZE", NULL, &paramsize, &use_highmem) >= 0 && use_highmem){
                se_config.force_high_memory = 1;
            }
        }
    }
}

static void patch_devicename(SceUID modid)
{
    SceModule2 *mod;
    int i;

    mod = (SceModule2*)sceKernelFindModuleByUID(modid);

    if(mod == NULL) {
        return;
    }

    for(i=0; i<mod->nsegment; ++i) {
        u32 addr;
        u32 end;

        end = mod->segmentaddr[i] + mod->segmentsize[i];

        for(addr = mod->segmentaddr[i]; addr < end; addr ++) {
        	char *str = (char*)addr;

        	if (0 == strncmp(str, "ms0", 3)) {
        		str[0] = 'e';
        		str[1] = 'f';
        	} else if (0 == strncmp(str, "fatms", 5)) {
        		str[3] = 'e';
        		str[4] = 'f';
        	}
        }
    }
    
    u32 start = mod->text_addr+mod->text_size;
    u32 end = start + mod->data_size;
    for (u32 addr=start; addr<end; addr++){
        char *str = (char*)addr;
        if (0 == strncmp(str, "ms0", 3)) {
        	str[0] = 'e';
        	str[1] = 'f';
        } else if (0 == strncmp(str, "fatms", 5)) {
        	str[3] = 'e';
        	str[4] = 'f';
        }
    }

    flushCache();
}

static int ef0PluginHandler(const char* path, int modid){
    if(se_config.oldplugin && path[0] == 'e' && path[1] == 'f') {
        patch_devicename(modid);
    }
    return 0;
}
