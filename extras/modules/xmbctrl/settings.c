#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

#include "settings.h"

int isRunlevelEnabled(char* line){
    return (strcasecmp(line, "on") == 0 || strcasecmp(line, "1") == 0 || strcasecmp(line, "enabled") == 0 || strcasecmp(line, "true") == 0);
}

int runlevelConvert(char* runlevel, char* enable){
    int enabled = isRunlevelEnabled(enable);
    if (strcasecmp(runlevel, "always") == 0 || strcasecmp(runlevel, "all") == 0){
        return (enabled)?ALWAYS_ON:DISABLED;
    }
    else if (strcasecmp(runlevel, "game") == 0){
        return (enabled)?GAME_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel, "umd") == 0 || strcasecmp(runlevel, "psp") == 0){
        return (enabled)?UMD_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel, "homebrew") == 0){
        return (enabled)?HOMEBREW_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel, "pops") == 0 || strcasecmp(runlevel, "psx") == 0 || strcasecmp(runlevel, "ps1") == 0){
        return (enabled)?POPS_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel, "vsh") == 0 || strcasecmp(runlevel, "xmb") == 0){
        return (enabled)?VSH_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel, "launcher") == 0){
        return (enabled)?LAUNCHER_ONLY:DISABLED;
    }
    return CUSTOM;
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

int readLine(char* source, char *str)
{
    u8 ch = 0;
    int n = 0;
    int i = 0;
    while(1)
    {
        if( (ch = source[i]) == 0){
            *str = 0;
            return n;
        }
        n++; i++;
        if(ch < 0x20)
        {
            *str = 0;
            return n;
        }
        else
        {
            *str++ = ch;
        }
    }
}

// Parse and Process Line
int processLine(char * line, int (process_line)(char*, char*, char*))
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

    return process_line(runlevel, path, enabled);
}

void ProcessConfigFile(char* path, int (process_line)(char*, char*, char*), void (*process_custom)(char*))
{

    int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    
    // Opened Plugin Config
    if(fd >= 0)
    {

        // allocate buffer and read entire file
        int fsize = sceIoLseek(fd, 0, PSP_SEEK_END);
        sceIoLseek(fd, 0, PSP_SEEK_SET);

        u8* buf = vsh_malloc(fsize+1);
        if (buf == NULL){
            sceIoClose(fd);
            return -1;
        }

        sceIoRead(fd, buf, fsize);
        sceIoClose(fd);
        buf[fsize] = 0;

        // Allocate Line Buffer
        char* line = vsh_malloc(LINE_BUFFER_SIZE);        
        // Buffer Allocation Success
        if(line != NULL)
        {
            // Read Lines
            int nread = 0;
            int total_read = 0;
            
            while ((nread=readLine(buf+total_read, line))>0)
            {
                total_read += nread;
                if (line[0] == 0) continue; // empty line
                char* dupline = vsh_malloc(strlen(line)+1);
                strcpy(dupline, line);
                // Process Line
                if (processLine(strtrim(line), process_line)){
                    vsh_free(dupline);
                }
                else{
                    process_custom(dupline);
                }
            }
            vsh_free(line);
        }
        vsh_free(buf);
    }
}
