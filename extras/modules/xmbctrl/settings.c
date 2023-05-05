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
    if (!isRunlevelEnabled(enable)) return DISABLED;
    else if (strcasecmp(runlevel, "always") == 0 || strcasecmp(runlevel, "all") == 0){
        return ALWAYS_ON;
    }
    else if (strcasecmp(runlevel, "game") == 0){
        return GAME_ONLY;
    }
    else if (strcasecmp(runlevel, "umd") == 0 || strcasecmp(runlevel, "psp") == 0){
        return UMD_ONLY;
    }
    else if (strcasecmp(runlevel, "homebrew") == 0){
        return HOMEBREW_ONLY;
    }
    else if (strcasecmp(runlevel, "pops") == 0 || strcasecmp(runlevel, "psx") == 0 || strcasecmp(runlevel, "ps1") == 0){
        return POPS_ONLY;
    }
    else if (strcasecmp(runlevel, "vsh") == 0 || strcasecmp(runlevel, "xmb") == 0){
        return VSH_ONLY;
    }
    else if (strcasecmp(runlevel, "launcher") == 0){
        return LAUNCHER_ONLY;
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

// Read Line from File Descriptor
char * readLine(int fd, char * buf, unsigned int buflen)
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
        // Allocate Line Buffer
        char* line = my_malloc(LINE_BUFFER_SIZE);        
        // Buffer Allocation Success
        if(line != NULL)
        {
            // Read Lines
            while(readLine(fd, line, LINE_BUFFER_SIZE) != NULL)
            {
                char* dupline = my_malloc(strlen(line)+1);
                strcpy(dupline, line);
                // Process Line
                if (processLine(strtrim(line), process_line)){
                    my_free(dupline);
                }
                else{
                    process_custom(dupline);
                }
            }
            my_free(line);
        }
        // Close Plugin Config
        sceIoClose(fd);
    }
}
