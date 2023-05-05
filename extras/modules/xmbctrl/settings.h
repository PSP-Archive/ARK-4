#ifndef SETTINGS_H
#define SETTINGS_H

#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

enum{
    DISABLED,
    ALWAYS_ON,
    GAME_ONLY,
    UMD_ONLY,
    HOMEBREW_ONLY,
    POPS_ONLY,
    VSH_ONLY,
    LAUNCHER_ONLY,
    CUSTOM
};

int isRunlevelEnabled(char* line);

int runlevelConvert(char* runlevel, char* enable);

void ProcessConfigFile(char* path, int (process_line)(char*, char*, char*), void (*process_custom)(char*));

#endif
