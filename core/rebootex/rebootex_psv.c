#include "rebootex.h"


int (*pspemuLfatOpen)(char **filename, int unk) = NULL;

// Load Core module_start Hook
int loadcoreModuleStartVita(unsigned int args, void * argp, int (* start)(SceSize, void *))
{
    loadCoreModuleStartCommon();
    flushCache();
    return start(args, argp);
}

int _pspemuLfatOpen(char **filename, int unk)
{
    if (filename != NULL && 0 == strcmp(*filename, "pspbtcnf.bin")){
        RebootBufferConfiguration * conf = (RebootBufferConfiguration*)REBOOTEX_CONFIG;
        char *p = filename;

        p[2] = 'v'; // custom btcnf for PS Vita

        if (IS_VITA_POPS(ark_conf->exec_mode)){
            p[5] = 'x'; // Vita PSX emulator uses custom setup for POPS
        }
        else{
            switch(conf->iso_mode)
            {
                case MODE_UMD:
                case MODE_NP9660:
                    // pspbtnnf.bin
                    p[5] = 'n';
                    break;
                case MODE_INFERNO:
                default:
                    // pspbtinf.bin
                    p[5] = 'i';
                    break;
            }
        }
    }
    return pspemuLfatOpen(filename, unk);
}

void patchRebootBufferVita(u32 reboot_start, u32 reboot_end){
    pspemuLfatOpen = origLfatOpen;
    u32 lFatOpenCall = JAL(pspemuLfatOpen);
    for (u32 addr = reboot_start; addr<reboot_end; addr+=4){
        u32 data = _lw(addr);
        if (data == lFatOpenCall){
            _sw(JAL(_pspemuLfatOpen), addr); // Hook pspLfatOpen
        }
        else if (data == 0x00600008){ // found loadcore jump on Vita
            // Move LoadCore module_start Address into third Argument
            _sw(0x00603021, addr); // move a2, v1
            // Hook LoadCore module_start Call
            _sw(JUMP(loadcoreModuleStartVita), addr+8);
        }
        else if ((data == _lw(addr+4)) && (data & 0xFC000000) == 0xAC000000){ // Patch ~PSP header check
            // Returns size of the buffer on loading whatever modules
            _sw(0xAFA50000, addr+4); // sw a1, 0(sp)
            _sw(0x20A30000, addr+8); // move v1, a1
        }
        else if ((data & 0x0000FFFF) == 0x8B00){
            _sb(0xA0, addr); // Link Filesystem Buffer to 0x8BA00000
        }
    }
}
