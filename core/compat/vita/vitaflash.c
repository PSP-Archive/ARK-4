#include "vitaflash.h"
#include <graphics.h>
#include "kermit.h"

extern ARKConfig* ark_config;
static KernelFunctions* ktbl = NULL;

int (* Kermit_driver_4F75AA05)(void* kermit_packet, u32 cmd_mode, u32 cmd, u32 argc, u32 allow_callback, u64 *resp) = NULL;

int installFlash0Archive(char* path)
{
    int fd;

    // Base Address
    uint32_t procfw = ARK_FLASH;
    uint32_t sony = FLASH_SONY;

    // Cast PROCFW flash0 Filesystem
    VitaFlashBufferFile * prof0 = (VitaFlashBufferFile *)procfw;
    
    // Cast Sony flash0 Filesystem
    VitaFlashBufferFile * f0 = (VitaFlashBufferFile *)sony;

    // flash0 Filecounts
    uint32_t procfw_filecount = 0;
    uint32_t flash0_filecount = 0;

    fd = ktbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);

    if(fd < 0){
        return fd;
    }

    ktbl->KernelIORead(fd, &procfw_filecount, sizeof(procfw_filecount));
    ktbl->KernelIOClose(fd);

    // Count Sony flash0 Files
    while(f0[flash0_filecount].content != NULL) flash0_filecount++;

    // Copy Sony flash0 Filesystem into PROCFW flash0
    memcpy(&prof0[procfw_filecount], f0, (flash0_filecount + 1) * sizeof(VitaFlashBufferFile));
    
    // Cast Namebuffer
    char * namebuffer = (char *)sony;
    
    // Cast Contentbuffer
    unsigned char * contentbuffer = (unsigned char *)&prof0[procfw_filecount + flash0_filecount + 1];
    
    // Ammount of linked in Files
    unsigned int linked = 0;
    
    fd = ktbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);

    if(fd < 0)
        return fd;

    int fileSize, ret, i;
    unsigned char lenFilename;

    // skip file counter
    ktbl->KernelIORead(fd, &fileSize, sizeof(fileSize));

    for(i=0; i<procfw_filecount; ++i)
    {
        ret = ktbl->KernelIORead(fd, &fileSize, sizeof(fileSize));

        if(ret != sizeof(fileSize))
            break;

        ret = ktbl->KernelIORead(fd, &lenFilename, sizeof(lenFilename));

        if(ret != sizeof(lenFilename))
            break;

        ret = ktbl->KernelIORead(fd, namebuffer, lenFilename);

        if(ret != lenFilename)
            break;

        namebuffer[lenFilename] = '\0';

        // Content Buffer 64 Byte Alignment required
        // (if we don't align this buffer by 64 PRXDecrypt will fail on 1.67+ FW!)
        if((((unsigned int)contentbuffer) % 64) != 0)
        {
            // Align Content Buffer
            contentbuffer += 64 - (((unsigned int)contentbuffer) % 64);
        }
        
        ret = ktbl->KernelIORead(fd, contentbuffer, fileSize);

        if(ret != fileSize)
            break;

        // Link File into virtual flash0 Filesystem
        prof0[linked].name = namebuffer;
        prof0[linked].content = contentbuffer;
        prof0[linked++].size = fileSize;

        // Move Buffer
        namebuffer += lenFilename + 1;
        contentbuffer += fileSize;
    }

    ktbl->KernelIOClose(fd);

    // Injection Error
    if(procfw_filecount == 0 || linked != procfw_filecount) return -1;
    
    // Return Number of Injected Files
    return linked;
}

// kermit_peripheral's sub_000007CC clone, called by loadexec + 0x0000299C with a0=8 (was a0=7 for fw <210)
// Returns 0 on success
u64 kermit_flash_load(int cmd)
{
    u8 buf[128];
    u64 resp;
    void *alignedBuf = (void*)ALIGN_64((int)buf + 63);
    ktbl->KernelDcacheInvalidateRange(alignedBuf, 0x40);
    KermitPacket *packet = (KermitPacket *)KERMIT_PACKET((int)alignedBuf);
    u32 argc = 0;
    Kermit_driver_4F75AA05(packet, KERMIT_MODE_PERIPHERAL, cmd, argc, KERMIT_CALLBACK_DISABLE, &resp);
    return resp;
}

int flashLoadPatch(int cmd)
{
    int ret = kermit_flash_load(cmd);
    // Custom handling on loadFlash mode, else nothing
    if ( cmd == KERMIT_CMD_ERROR_EXIT || cmd == KERMIT_CMD_ERROR_EXIT_2 )
    {
        int linked;
        // Wait for flash to load
        ktbl->KernelDelayThread(10000);
        onVitaFlashLoaded();
        // Patch flash0 Filesystem Driver
        char path[ARK_PATH_SIZE];
        strcpy(path, ark_config->arkpath);
        strcat(path, FLASH0_ARK);
        linked = installFlash0Archive(path);
        ktbl->KernelIcacheInvalidateAll();
        ktbl->KernelDcacheWritebackInvalidateAll();
    }
    return ret;
}

u32 findKermitFlashDriver(){
    u32 nids[] = {0x4F75AA05, 0x36666181};
    for (int i=0; i<NELEMS(nids) && Kermit_driver_4F75AA05 == NULL; i++){
        Kermit_driver_4F75AA05 = sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver", nids[i]);
    }
    return Kermit_driver_4F75AA05;
}

int patchKermitPeripheral(KernelFunctions* kf)
{
    ktbl = kf;
    findKermitFlashDriver();
    // Redirect KERMIT_CMD_ERROR_EXIT loadFlash function
    u32 knownnids[2] = { 0x3943440D, 0x0648E1A3 /* 3.3X */ };
    u32 swaddress = 0;
    u32 i;
    for (i = 0; i < 2; i++)
    {
        swaddress = findFirstJAL(sctrlHENFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral_driver", knownnids[i]));
        if (swaddress != 0)
            break;
    }
    _sw(JUMP(flashLoadPatch), swaddress);
    _sw(NOP, swaddress+4);
    
    return 0;
}
