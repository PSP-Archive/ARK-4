#ifndef PSPBTCNF_H
#define PSPBTCNF_H

#define PATH_FLASH0 "flash0:/"
#define PATH_SYSTEMCTRL PATH_FLASH0 "kd/ark_systemctrl.prx"
#define PATH_PSPCOMPAT PATH_FLASH0 "kd/ark_pspcompat.prx"
#define PATH_VITACOMPAT PATH_FLASH0 "kd/ark_vitacompat.prx"
#define PATH_VITAPOPS PATH_FLASH0 "kd/ark_vitapops.prx"
#define PATH_VSHCTRL PATH_FLASH0 "kd/ark_vshctrl.prx"
#define PATH_STARGATE PATH_FLASH0 "kd/ark_stargate.prx"
#define PATH_INFERNO PATH_FLASH0 "kd/ark_inferno.prx"
#define PATH_POPCORN PATH_FLASH0 "kd/ark_popcorn.prx"
#define PATH_TMCTRL PATH_FLASH0 "tmctrl.prx"

typedef struct _btcnf_header
{
    unsigned int signature; // 0
    unsigned int devkit; // 4
    unsigned int unknown[2]; // 8
    unsigned int modestart; // 0x10
    int nmodes; // 0x14
    unsigned int unknown2[2]; // 0x18
    unsigned int modulestart; // 0x20
    int nmodules; // 0x24
    unsigned int unknown3[2]; // 0x28
    unsigned int modnamestart; // 0x30
    unsigned int modnameend; // 0x34
    unsigned int unknown4[2]; // 0x38
} _btcnf_header __attribute((packed));

typedef struct _btcnf_module
{
    unsigned int module_path; // 0x00
    unsigned int unk_04; //0x04
    unsigned int flags; //0x08
    unsigned int unk_C; //0x0C
    unsigned char hash[0x10]; //0x10
} _btcnf_module __attribute((packed));

enum {
    VSH_RUNLEVEL     =     0x01,
    GAME_RUNLEVEL    =     0x02,
    UPDATER_RUNLEVEL =     0x04,
    POPS_RUNLEVEL    =     0x08,
    APP_RUNLEVEL     =     0x20,
    UMDEMU_RUNLEVEL  =     0x40,
    MLNAPP_RUNLEVEL  =     0x80,
};

#endif