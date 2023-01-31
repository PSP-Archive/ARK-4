#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

#include "globals.h"

#include "main.h"

#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

extern CFWConfig config;
extern ARKConfig* ark_config;

char** custom_config = NULL;
int n_custom_config = 0;
int max_custom_config = 0;

// Allocate Line Buffer
static char line[LINE_BUFFER_SIZE];

enum{
    DISABLED,
    ALWAYS_ON,
    GAME_ONLY,
    UMD_ONLY,
    HOMEBREW_ONLY,
    POPS_ONLY,
    VSH_ONLY,
    CUSTOM
};

void add_custom_config(char* config){
    if (custom_config == NULL){
        custom_config = my_malloc(sizeof(char*)*8);
        memset(custom_config, 0, sizeof(char*)*8);
        max_custom_config = 8;
        n_custom_config = 0;
    }
    if (n_custom_config >= max_custom_config){
        char** old_table = custom_config;
        max_custom_config *= 2;
        custom_config = my_malloc(sizeof(char*)*max_custom_config);
        memset(custom_config, 0, sizeof(char*)*max_custom_config);
        for (int i=0; i<n_custom_config; i++){
            custom_config[i] = old_table[i];
        }
        my_free(old_table);
    }
    custom_config[n_custom_config++] = config;
}

void clear_custom_config(){
    if (custom_config == NULL) return;
    for (int i=0; i<n_custom_config; i++){
        my_free(custom_config[i]);
    }
    my_free(custom_config);
    custom_config = NULL;
    n_custom_config = 0;
    max_custom_config = 0;
}

int isRunlevelEnabled(char* line){
    return (strcasecmp(line, "on") == 0 || strcasecmp(line, "1") == 0 || strcasecmp(line, "enabled") == 0 || strcasecmp(line, "true") == 0);
}

int runlevelConvert(char* runlevel, char* enable){
    if (!isRunlevelEnabled(enable)) return DISABLED;
    else if (strcasecmp(runlevel, "always") == 0 || strcasecmp(runlevel, "all") == 0){
        return ALWAYS_ON;
    }
    else if (strcasecmp(runlevel, "game") == 0){
        return GAME_ONLY;
    }
    else if (strcasecmp(runlevel, "umd") == 0){
        return UMD_ONLY;
    }
    else if (strcasecmp(runlevel, "homebrew") == 0){
        return HOMEBREW_ONLY;
    }
    else if (strcasecmp(runlevel, "pops") == 0){
        return POPS_ONLY;
    }
    else if (strcasecmp(runlevel, "vsh") == 0){
        return VSH_ONLY;
    }
    return CUSTOM;
}

int processConfigLine(char* runlevel, char* path, char* enabled){
    
    int opt = runlevelConvert(runlevel, enabled);

    if (opt == CUSTOM) return 0;

    if (strcasecmp(path, "usbcharge") == 0){
        config.usbcharge = opt;
        return 1;
    }
    else if (strcasecmp(path, "overclock") == 0){
        config.overclock = opt;
        return 1;
    }
    else if (strcasecmp(path, "powersave") == 0){
        config.powersave = opt;
        return 1;
    }
    else if (strcasecmp(path, "launcher") == 0){
        config.launcher = opt;
        return 1;
    }
    else if (strcasecmp(path, "disablepause") == 0){
        config.disablepause = opt;
        return 1;
    }
    else if (strcasecmp(path, "highmem") == 0){
        config.highmem = opt;
        return 1;
    }
    else if (strcasecmp(path, "mscache") == 0){
        config.mscache = opt;
        return 1;
    }
    else if (strcasecmp(path, "infernocache") == 0){
        config.infernocache = opt;
        return 1;
    }
    else if (strcasecmp(path, "oldplugin") == 0){
        config.oldplugin = opt;
        return 1;
    }
    else if (strcasecmp(path, "skiplogos") == 0){
        config.skiplogos = opt;
        return 1;
    }
    else if (strcasecmp(path, "hidepics") == 0){
        config.hidepics = opt;
        return 1;
    }
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
static int processLine(char * line)
{
    // Skip Comment Lines
    if(line == NULL || strncmp(line, "//", 2) == 0 || line[0] == ';' || line[0] == '#')
        return 0;
    
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
    if(enabled == NULL) return 0;
    
    // Trim Whitespaces
    runlevel = strtrim(runlevel);
    path = strtrim(path);
    enabled = strtrim(enabled);

    return processConfigLine(runlevel, path, enabled);
}

// Load Plugins
static void ProcessConfigFile(char* path)
{

    int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    
    // Opened Plugin Config
    if(fd >= 0)
    {
        
        // Buffer Allocation Success
        if(line != NULL)
        {
            // Read Lines
            while(readLine(fd, line, LINE_BUFFER_SIZE) != NULL)
            {
                char* dupline = my_malloc(strlen(line)+1);
                strcpy(dupline, line);
                // Process Line
                if (processLine(strtrim(line))){
                    my_free(dupline);
                }
                else{
                    add_custom_config(dupline);
                }
            }
        }
        // Close Plugin Config
        sceIoClose(fd);
    }
}

void loadSettings(){
    clear_custom_config();

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "SETTINGS.TXT");
    ProcessConfigFile(path);
}

void processSetting(char* line, char* name, int setting){
    switch (setting){
    default:
    case DISABLED:            snprintf(line, LINE_BUFFER_SIZE, "always, %s, off\n", name);   break;
    case ALWAYS_ON:           snprintf(line, LINE_BUFFER_SIZE, "always, %s, on\n", name);   break;
    case GAME_ONLY:           snprintf(line, LINE_BUFFER_SIZE, "game, %s, on\n", name);     break;
    case UMD_ONLY:            snprintf(line, LINE_BUFFER_SIZE, "umd, %s, on\n", name);      break;
    case HOMEBREW_ONLY:       snprintf(line, LINE_BUFFER_SIZE, "homebrew, %s, on\n", name); break;
    case POPS_ONLY:           snprintf(line, LINE_BUFFER_SIZE, "pops, %s, on\n", name);     break;
    case VSH_ONLY:            snprintf(line, LINE_BUFFER_SIZE, "vsh, %s, on\n", name);      break;
    }
}

void saveSettings(){

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "SETTINGS.TXT");

    int fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    if (fd < 0){
        return;
    }

    processSetting(line, "usbcharge", config.usbcharge);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "overclock", config.overclock);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "powersave", config.powersave);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "launcher", config.launcher);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "disablepause", config.disablepause);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "highmem", config.highmem);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "mscache", config.mscache);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "infernocache", config.infernocache);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "oldplugin", config.oldplugin);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "skiplogos", config.skiplogos);
    sceIoWrite(fd, line, strlen(line));
    processSetting(line, "hidepics", config.hidepics);
    sceIoWrite(fd, line, strlen(line));

    for (int i=0; i<n_custom_config; i++){
        sceIoWrite(fd, custom_config[i], strlen(custom_config[i]));
        sceIoWrite(fd, "\n", 1);
    }

    sceIoClose(fd);

    my_free(line);

    //clear_custom_config();
}