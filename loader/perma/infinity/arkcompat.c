/*
    arkcompat.prx : a compatibility layer between infinity and ark cfw
    Copyright (C) 2015 David "Davee" Morgan

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <pspmacro.h>

#include <pspinit.h>
#include <pspsdk.h>

#include <string.h>

#include <compat_interface.h>

#define SENSE_MASK                                                                                 \
    (SYSCON_CTRL_TRIANGLE | SYSCON_CTRL_CIRCLE | SYSCON_CTRL_CROSS | SYSCON_CTRL_RECTANGLE |       \
     SYSCON_CTRL_SELECT | SYSCON_CTRL_LTRG | SYSCON_CTRL_RTRG | SYSCON_CTRL_START |                \
     SYSCON_CTRL_HOME | SYSCON_CTRL_VOL_UP | SYSCON_CTRL_VOL_DN | SYSCON_CTRL_LCD |                \
     SYSCON_CTRL_NOTE | SYSCON_CTRL_ALLOW_UP | SYSCON_CTRL_ALLOW_RT | SYSCON_CTRL_ALLOW_DN |       \
     SYSCON_CTRL_ALLOW_LT)

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

// ARK Runtime configuration
#define ARK_CONFIG 0x08800010
#define ARK_CONFIG_MAGIC 0xB00B1E55
#define ARK_PATH_SIZE 128

#define REBOOT_START 0x88600000
#define REBOOTEX_START 0x88FC0000
#define REBOOTEX_MAX_SIZE 0x4000
#define BTCNF_MAGIC 0x0F803001
#define BOOTCONFIG_TEMP_BUFFER 0x88FB0200

// ARK Reboot Buffer Configuration Address
#define REBOOTEX_CONFIG (REBOOTEX_START - 0x10000)
#define REBOOTEX_CONFIG_MAXSIZE 0x200

// ARK Reboot Buffer ISO Path (so we don't lose that information)
#define REBOOTEX_CONFIG_ISO_PATH_MAXSIZE 0x100

// ARK Reboot Buffer Configuration
typedef struct RebootBufferConfiguration {
    unsigned int magic;
    unsigned int reboot_buffer_size;
    unsigned char iso_mode;
    unsigned char iso_disc_type;
    char iso_path[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE];
} RebootBufferConfiguration;

// ARK execution modes
typedef enum{
    DEV_UNK = 0b0000,
    PSP_ORIG = 0b0100,
    PS_VITA = 0b1000,
    PSV_POPS = 0b1010,
    DEV_MASK = 0b1100,
}ExecMode;

// ARK runtime configuration
typedef struct ARKConfig{
    u32 magic;
    char arkpath[ARK_PATH_SIZE-20]; // ARK installation folder, leave enough room to concatenate files
    char exploit_id[12]; // ID of the game exploit, or name of the bootloader
    char launcher[20]; // run ARK in launcher mode if launcher specified
    unsigned char exec_mode; // ARK execution mode (PSP, PS Vita, Vita POPS, etc)
    unsigned char recovery; // run ARK in recovery mode (disables settings, plugins and autoboots RECOVERY.PBP)
} ARKConfig;

ARKConfig _arkconf = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = "ms0:/PSP/SAVEDATA/ARK_01234/", // default path for ARK files
    .exploit_id = "Infinity",
    .launcher = {0},
    .exec_mode = PSP_ORIG, // run ARK in PSP mode
    .recovery = 0,
};

int (*memlmd_E42AFE2E)(void* buf, void* check, void* s) = NULL;
int (*memlmd_3F2AC9C6)(void* a0, void* a1) = NULL;

void* memset(void* b, int c, size_t len)
{
    void* pRet = b;
    unsigned char* ub = (unsigned char*)b;

    while (len > 0)
    {
        *ub++ = (unsigned char)c;
        len--;
    }

    return pRet;
}

int _memcpy(char* to, char* from, unsigned int length)
{
    // result
    int result = 0;

    // loop copy
    unsigned int pos = 0;
    for (; pos < length; pos++)
    {
        // copy byte
        to[pos] = from[pos];

        // increment result
        result++;
    }

    return result;
}

// memlmd_E42AFE2E_patched
int memlmd_Decrypt_patched(PSP_Header* buf, int* check, int* s)
{
    if (buf->oe_tag == 0xC01DB15D
        || (_lb((unsigned)buf + 0x150) == 0x1F && _lb((unsigned)buf + 0x151) == 0x8B ) )
    {
        _memcpy((char*)buf, (char*)&(buf->main_data), buf->comp_size);
        *s = buf->comp_size;
        return 0;
    }

    return memlmd_E42AFE2E(buf, check, s);
}

// memlmd_3F2AC9C6_patched
int memlmd_Sigcheck_patched(void* a0, void* a1)
{
    PSP_Header* head = (PSP_Header*)a0;
    int i;

    for (i = 0; i < 0x30; i++)
    {
        if (head->scheck[i] != 0)
            return memlmd_3F2AC9C6(a0, a1);
    }

    return 0;
}

int PatchLoadCore(void* a0, void* a1, void* a2, int (*module_start)(void*, void*, void*))
{
    u32 text_addr = ((u32)module_start) - 0x00000AF8;

    MAKE_CALL(text_addr + 0x00005994, memlmd_Sigcheck_patched);
    MAKE_CALL(text_addr + 0x00005970, memlmd_Decrypt_patched);

    memlmd_3F2AC9C6 = (void*)(text_addr + 0x00007824);
    memlmd_E42AFE2E = (void*)(text_addr + 0x0000783C);

    return module_start(a0, a1, a2);
}

int compat_entry(BtcnfHeader* btcnf,
                 int btcnf_size,
                 INSERT_BTCNF insert_btcnf,
                 REPLACE_BTCNF replace_btcnf,
                 int model,
                 int is_recovery,
                 int firmware)
{
    // clear location for configuration and set default akin to CIPL
    memset((void*)REBOOTEX_CONFIG, 0, REBOOTEX_CONFIG_MAXSIZE);

    RebootBufferConfiguration* conf = (RebootBufferConfiguration*)(REBOOTEX_CONFIG);
    conf->magic = ARK_CONFIG_MAGIC;
    conf->reboot_buffer_size = 0;
    
    _arkconf.recovery = is_recovery; // let ARK handle recovery mode
    if (model == PSP_MODEL_PSPGO){ // switch ARK install path to Go's internal memory
        _arkconf.arkpath[0] = 'e';
        _arkconf.arkpath[1] = 'f';
    }

    insert_btcnf("/kd/ark_systemctrl.prx", // add ARK's SystemControl
                 "/kd/init.prx",
                 btcnf,
                 &btcnf_size,
                 (BOOTLOAD_VSH | BOOTLOAD_GAME | BOOTLOAD_POPS | BOOTLOAD_UPDATER |
                  BOOTLOAD_UMDEMU | BOOTLOAD_APP | BOOTLOAD_MLNAPP));
    
    insert_btcnf("/kd/ark_pspcompat.prx", // add ARK's PSP Compat layer
                 "/kd/init.prx",
                 btcnf,
                 &btcnf_size,
                 (BOOTLOAD_VSH | BOOTLOAD_GAME | BOOTLOAD_POPS | BOOTLOAD_UPDATER |
                  BOOTLOAD_UMDEMU | BOOTLOAD_APP | BOOTLOAD_MLNAPP));
    
    // Insert VSH Control              
    insert_btcnf("/kd/ark_vshctrl.prx", "/kd/vshbridge.prx", btcnf, &btcnf_size, (BOOTLOAD_VSH));

    // copy ARK configuration for SystemControl to find
    _memcpy(ARK_CONFIG, &_arkconf, sizeof(ARKConfig));

    // PRO patches work well with ARK...
    // need to patch loadcore too
    // MIPS_ADDU( 7 , 15 , 0 )
    switch (model)
    {
        case PSP_MODEL_PHAT:
            // Prepare LoadCore Patch Part #1 - addu $a3, $zr, $s1 - Stores module_start ($s1) as
            // fourth argument.
            _sw(0x00113821, REBOOT_START + 0x00005644 - 4);
            // Prepare LoadCore Patch Part #2 - jal PatchLoadCore
            MAKE_JUMP(REBOOT_START + 0x00005644, PatchLoadCore);
            // Prepare LoadCore Patch Part #3 - move $sp, $s5 - Backed up instruction.
            _sw(0x02A0E821, REBOOT_START + 0x00005644 + 4);
            break;

        case PSP_MODEL_SLIM:
        case PSP_MODEL_BRITE:
        case PSP_MODEL_BRITE4G:
        case PSP_MODEL_BRITE7G:
        case PSP_MODEL_BRITE9G:
        case PSP_MODEL_STREET:
        case PSP_MODEL_PSPGO:
        default:
            // Prepare LoadCore Patch Part #1 - addu $a3, $zr, $s1 - Stores module_start ($s1) as
            // fourth argument.
            _sw(0x00113821, REBOOT_START + 0x00005704 - 4);
            // Prepare LoadCore Patch Part #2 - jal PatchLoadCore
            MAKE_JUMP(REBOOT_START + 0x00005704, PatchLoadCore);
            // Prepare LoadCore Patch Part #3 - move $sp, $s5 - Backed up instruction.
            _sw(0x02A0E821, REBOOT_START + 0x00005704 + 4);
    }

    // return the new btcnf size
    return btcnf_size;
}
