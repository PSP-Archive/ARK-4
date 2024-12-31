#ifndef __FLASHEMU_H__
#define __FLASHEMU_H__

int InstallFlashEmu();
int UninstallFlashEmu();

#define MAX_FILES 32
#define DIR_FLAG 0xd0d0

typedef struct
{
    int opened;
    SceUID fd;
    int unk_8;
    char path[0xC0];
    SceMode mode;
    int flags;
    SceOff offset;
} FileHandler;

#endif