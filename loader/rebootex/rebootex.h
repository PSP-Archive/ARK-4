#ifndef REBOOTEX_H
#define REBOOTEX_H

#include "rebootconfig.h"

#define REBOOT_MODULE "/rtm.prx"

typedef struct {
    char *name;
    void *buffer;
    u32 size;
} BootFile;

typedef struct{
    u8 filesize[4];
    char namelen;
    char name[1];
} ArkFlashFile;

// Sony flash0 files
typedef struct {
    int nfiles;
    char bootfile[100][64]; // list of boot files
} FlashBackupList;

typedef struct {
    u32 addr;
    u32 size;
} SceSysmemPartInfo;

typedef struct {
    u32 memSize;
    u32 unk4;
    u32 unk8;
    SceSysmemPartInfo other1; // 12
    SceSysmemPartInfo other2; // 20
    SceSysmemPartInfo vshell; // 28
    SceSysmemPartInfo scUser; // 36
    SceSysmemPartInfo meUser; // 44
    SceSysmemPartInfo extSc2Kernel; // 52
    SceSysmemPartInfo extScKernel; // 60
    SceSysmemPartInfo extMeKernel; // 68
    SceSysmemPartInfo extVshell; // 76
} SceSysmemPartTable;

typedef struct
{
    u32 signature;      // 0
    u16 attribute;      // 4  modinfo
    u16 comp_attribute; // 6
    u8 module_ver_lo;   // 8
    u8 module_ver_hi;   // 9
    char modname[28];   // 0A
    u8 version;         // 26
    u8 nsegments;       // 27
    int elf_size;       // 28
    int psp_size;       // 2C
    u32 entry;          // 30
    u32 modinfo_offset; // 34
    int bss_size;       // 38
    u16 seg_align[4];   // 3C
    u32 seg_address[4]; // 44
    int seg_size[4];    // 54
    u32 reserved[5];    // 64
    u32 devkitversion;  // 78
    u32 decrypt_mode;   // 7C
    u8 key_data0[0x30]; // 80
    int comp_size;      // B0
    int _80;            // B4
    int reserved2[2];   // B8
    u8 key_data1[0x10]; // C0
    u32 tag;            // D0
    u8 scheck[0x58];    // D4
    u32 key_data2;      // 12C
    u32 oe_tag;         // 130
    u8 key_data3[0x1C]; // 134
    u8 main_data;       // 150
} PSP_Header;


extern RebootConfigARK* reboot_conf;
extern ARKConfig* ark_config;

extern u32 reboot_start;
extern u32 reboot_end;

// sceReboot Main Function
extern int (* sceReboot)(int, int, int, int, int, int, int);

// Instruction Cache Invalidator
extern void (* sceRebootIcacheInvalidateAll)(void);

// Data Cache Invalidator
extern void (* sceRebootDacheWritebackInvalidateAll)(void);

// Sony PRX Decrypter Function Pointer
extern int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *);
extern int (* origCheckExecFile)(unsigned char * addr, void * arg2);
extern int (* extraPRXDecrypt)(void *, unsigned int, unsigned int *);
extern int (* extraCheckExec)(unsigned char * addr, void * arg2);

// UnpackBootConfig on PSP
extern int UnpackBootConfigPatched(char **p_buffer, int length);
extern int (* UnpackBootConfig)(char * buffer, int length);
extern u32 UnpackBootConfigCall;
extern u32 UnpackBootConfigArg;

// Rebootex functions
u32 loadCoreModuleStartCommon(u32 entry);
void patchRebootBufferPSP();
#ifdef REBOOTEX
void patchRebootBufferVita();
#endif

// IO functions
int pspemuLfatOpenExtra(BootFile* file);
void patchRebootIoPSP();

#endif
