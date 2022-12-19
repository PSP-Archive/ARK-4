#include "rebootex.h"

int (*pspemuLfatOpen)(char** filename, int unk) = NULL;
void (*SetMemoryPartitionTable)(void *sysmem_config, SceSysmemPartTable *table) = NULL;
extern int UnpackBootConfigPatched(char **p_buffer, int length);
extern int (* UnpackBootConfig)(char * buffer, int length);

// Load Core module_start Hook
int loadcoreModuleStartVita(unsigned int args, void* argp, int (* start)(SceSize, void *))
{
    loadCoreModuleStartCommon(start);
    flushCache();
    return start(args, argp);
}

int _pspemuLfatOpen(BootFile* file, int unk)
{
    char* p = file->name;
    if (strcmp(p, "pspbtcnf.bin") == 0){
        p[2] = 'v'; // custom btcnf for PS Vita
        p[5] = 'x'; // psvbtxnf.bin for PS1 exploits
    }
    else if (strcmp(p, REBOOT_MODULE) == 0){
        file->buffer = (void *)0x89000000;
		file->size = reboot_conf->rtm_mod.size;
		memcpy(file->buffer, reboot_conf->rtm_mod.buffer, file->size);
		reboot_conf->rtm_mod.buffer = NULL;
        reboot_conf->rtm_mod.size = 0;
		return 0;
    }
    return pspemuLfatOpen(file, unk);
}

int UnpackBootConfigVita(char **p_buffer, int length){
    int res = (*UnpackBootConfig)(*p_buffer, length);
    if(reboot_conf->rtm_mod.before && reboot_conf->rtm_mod.buffer && reboot_conf->rtm_mod.size)
    {
        //add reboot prx entry
        res = AddPRX(*p_buffer, reboot_conf->rtm_mod.before, REBOOT_MODULE, reboot_conf->rtm_mod.flags);
    }
    return res;
}


// patch reboot on ps vita
void patchRebootBuffer(){

    // hijack UnpackBootConfig to insert modules at runtime
    _sw(0x27A40004, UnpackBootConfigArg); // addiu $a0, $sp, 4
    _sw(JAL(UnpackBootConfigVita), UnpackBootConfigCall); // Hook UnpackBootConfig

    for (u32 addr = reboot_start; addr<reboot_end; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(pspemuLfatOpen)){
            _sw(JAL(_pspemuLfatOpen), addr); // Hook pspemuLfatOpen
        }
        else if (data == 0x3A230001){ // found pspemuLfatOpen
            u32 a = addr;
            do {a-=4;} while (_lw(a) != 0x27BDFFF0);
            pspemuLfatOpen = (void*)a;
        }
        else if (data == 0x00600008){ // found loadcore jump on Vita
            // Move LoadCore module_start Address into third Argument
            _sw(0x00603021, addr); // move a2, v1
            // Hook LoadCore module_start Call
            _sw(JUMP(loadcoreModuleStartVita), addr+8);
        }
        else if ((data & 0x0000FFFF) == 0x8B00){
            _sb(0xA0, addr); // Link Filesystem Buffer to 0x8BA00000
        }
    }
    // Flush Cache
    flushCache();
}
