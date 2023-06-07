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
#include <globals.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
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

void (*plugin_handler)(const char* path, int modid) = NULL;

int disable_plugins = 0;
int disable_settings = 0;
int is_plugins_loading = 0;

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
        char* path = plugins->paths[i];
        // Load Module
        int uid = sceKernelLoadModule(path, 0, NULL);
        // Call handler
        if (plugin_handler) plugin_handler(path, uid);
        // Start Module
        int res = sceKernelStartModule(uid, strlen(path) + 1, path, NULL, NULL);
        // Unload Module on Error
        if (res < 0) sceKernelUnloadModule(uid);
    }
}

// Runlevel Check
static int matchingRunlevel(char * runlevel)
{

    // Fetch Apitype
    int apitype = sceKernelInitApitype();
    
    if (stricmp(runlevel, "all") == 0 || stricmp(runlevel, "always") == 0) return 1; // always on
    else if (stricmp(runlevel, "vsh") == 0 || stricmp(runlevel, "xmb") == 0) // VSH only
        return (apitype == 0x200 || apitype ==  0x210 || apitype ==  0x220 || apitype == 0x300);
    else if (stricmp(runlevel, "pops") == 0 || stricmp(runlevel, "ps1") == 0 || stricmp(runlevel, "psx") == 0) // PS1 games only
        return (apitype == 0x144 || apitype == 0x155);
    else if (stricmp(runlevel, "umd") == 0 || stricmp(runlevel, "psp") == 0 || stricmp(runlevel, "umdemu") == 0) // Retail games only
        return (apitype == 0x120 || (apitype >= 0x123 && apitype <= 0x126) || apitype == 0x130 || apitype == 0x160 || (apitype >= 0x110 && apitype <= 0x115));
    else if (stricmp(runlevel, "game") == 0) // retail+homebrew
        return (apitype == 0x120 || (apitype >= 0x123 && apitype <= 0x126) || apitype == 0x141 || apitype == 0x152 || apitype == 0x130 || apitype == 0x160 || (apitype >= 0x110 && apitype <= 0x115));
    else if (stricmp(runlevel, "app") == 0 || stricmp(runlevel, "homebrew") == 0) // homebrews only
        return (apitype == 0x141 || apitype == 0x152);
    else if (stricmp(runlevel, "launcher") == 0){
        // check if running custom launcher
        static char path[ARK_PATH_SIZE];
        strcpy(path, ark_config->arkpath);
        strcat(path, ark_config->launcher);
        return (strcmp(path, sceKernelInitFileName())==0);
    }
    else if (apitype == 0x120 || (apitype >= 0x123 && apitype <= 0x126) || apitype == 0x130 || apitype == 0x160 || (apitype >= 0x110 && apitype <= 0x115)){
        char gameid[10]; memset(gameid, 0, sizeof(gameid)); // check if plugin loads on specific game
        return (getGameId(gameid) && strstr(runlevel, gameid) != NULL);
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
static void ProcessConfigFile(char* path, void (*enabler)(char*), void (*disabler)(char*))
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
                // Process Line
                processLine(strtrim(line), enabler, disabler);
            }
            
            // Free Buffer
            oe_free(line);
        }
        
        // Close Plugin Config
        sceIoClose(fd);
    }
}

static void settingsHandler(char* path, u8 enabled){
    int apitype = sceKernelInitApitype();
    if (strcasecmp(path, "overclock") == 0){ // set CPU speed to max
        if (enabled)
            se_config.clock = 1;
        else if (se_config.clock == 1) se_config.clock = 0;
    }
    else if (strcasecmp(path, "powersave") == 0){ // underclock to save battery
        if (apitype != 0x144 && apitype != 0x155) // prevent operation in pops
            if (enabled)
                se_config.clock = 2;
            else if (se_config.clock == 2) se_config.clock = 0;
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
    else if (strcasecmp(path, "infernocache") == 0){
        if (apitype == 0x123 || apitype == 0x125 || (apitype >= 0x112 && apitype <= 0x115)){
            se_config.iso_cache = enabled;
        }
    }
    else if (strcasecmp(path, "noled") == 0){
        se_config.noled = enabled;
    }
    else if (strcasecmp(path, "skiplogos") == 0){
        se_config.skiplogos = enabled;
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
    else if (strcasecmp(path, "hidepics") == 0){ // hide PIC0 and PIC1
        se_config.hidepics = enabled;
    }
    else if (strcasecmp(path, "hibblock") == 0){ // block hibernation
        se_config.hibblock = enabled;
    }
    else if (strcasecmp(path, "skiplogos") == 0){ // skip sony logo and gameboot
        se_config.skiplogos = enabled;
    }
    else if (strcasecmp(path, "hidemac") == 0){ // hide mac address
        se_config.hidemac = enabled;
    }
    else if (strcasecmp(path, "hidedlc") == 0){ // hide mac address
        se_config.hidedlc = enabled;
    }
}

static void settingsEnabler(char* path){
    settingsHandler(path, 1);
}

static void settingsDisabler(char* path){
    settingsHandler(path, 0);
}

static int isRecoveryMode(){
    if (ark_config->recovery) return 1;
    // check if launching recovery menu
    static char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, ARK_RECOVERY);
    return (strcmp(path, sceKernelInitFileName())==0);
}

void LoadPlugins(){
    if (disable_plugins || isRecoveryMode())
        return; // don't load plugins in recovery mode
    is_plugins_loading = 1;
    // allocate resources
    plugins = oe_malloc(sizeof(Plugins));
    plugins->count = 0; // initialize plugins table
    // Open Plugin Config from ARK's installation folder
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "PLUGINS.TXT");
    ProcessConfigFile(path, &addPlugin, &removePlugin);
    // Open Plugin Config from SEPLUGINS
    ProcessConfigFile("ms0:/SEPLUGINS/PLUGINS.TXT", &addPlugin, &removePlugin);
    // On PSP Go (only if ms0 isn't already redirected to ef0)
    ProcessConfigFile("ef0:/SEPLUGINS/PLUGINS.TXT", &addPlugin, &removePlugin);
    // start all loaded plugins
    startPlugins();
    // free resources
    oe_free(plugins);
    plugins = NULL;
    is_plugins_loading = 0;
}

void loadSettings(){
    if (disable_settings || isRecoveryMode())
        return; // don't load settings in recovery mode
    // process settings file
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "SETTINGS.TXT");
    ProcessConfigFile(path, settingsEnabler, settingsDisabler);
    se_config.magic = ARK_CONFIG_MAGIC;
}
