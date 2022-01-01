#include "rebootex.h"

int (*pspemuLfatOpen)(char** filename, int unk) = NULL;
extern int UnpackBootConfigPatched(char **p_buffer, int length);
extern int (* UnpackBootConfig)(char * buffer, int length);

// Load Core module_start Hook
int loadcoreModuleStartVita(unsigned int args, void* argp, int (* start)(SceSize, void *))
{
    loadCoreModuleStartCommon();
    flushCache();
    return start(args, argp);
}

int _pspemuLfatOpen(char** filename, int unk)
{
    if (filename != NULL && 0 == strcmp(*filename, "pspbtcnf.bin")){
        char* p = *filename;
        p[2] = 'v'; // custom btcnf for PS Vita
        if (IS_VITA_POPS(ark_config)){
            p[5] = 'x'; // psvbtxnf.bin for PS1 exploits
        }
        else{
            if (reboot_conf->iso_mode == MODE_INFERNO){
                p[5] = 'i'; // use inferno ISO mode
            }
        }
    }
    return pspemuLfatOpen(filename, unk);
}

int UnpackBootConfigDummy(char **p_buffer, int length){
    return (*UnpackBootConfig)(*p_buffer, length);
}

// patch reboot on ps vita
void patchRebootBufferVita(){

    for (u32 addr = reboot_start; addr<reboot_end; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(pspemuLfatOpen)){
            _sw(JAL(_pspemuLfatOpen), addr); // Hook pspemuLfatOpen
            // dummy out _UnpackBootConfig since we are using pspemuLfatOpen
            u32 _UnpackBootConfigPatched = &UnpackBootConfigPatched;
            _sw(JUMP(UnpackBootConfigDummy), _UnpackBootConfigPatched);
            _sw(NOP, _UnpackBootConfigPatched+4);
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
        
        /*
        else if (data == 0x24040004) {
            extern int PatchSysMem(void *a0, void *sysmem_config);
            _sw(0x02402021, addr); //move $a0, $s2
            _sw(JAL(PatchSysMem), addr + 0x64); // Patch call to SysMem module_bootstart
        }
        */
    }
}
