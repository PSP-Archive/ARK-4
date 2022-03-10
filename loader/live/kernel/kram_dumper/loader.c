#include "main.h"
#include <functions.h>

unsigned addWriteFile( SceUID packFileID, void *data, unsigned size, char *name)
{

    PRTSTR1("Dumping file %s", name);

    char path[ARK_PATH_SIZE];
    mysprintf11(path, "%s_%p", (unsigned long)name, (unsigned long)data, (unsigned long)0,  (unsigned long)0, (unsigned long)0, (unsigned long)0, (unsigned long)0,  (unsigned long)0, (unsigned long) 0, (unsigned long) 0, (unsigned long) 0);

    unsigned char namelen = strlen(path);

    // Write Data
    k_tbl->KernelIOWrite(packFileID, &size, sizeof(u32));
    k_tbl->KernelIOWrite(packFileID, &namelen, sizeof(u8));
    k_tbl->KernelIOWrite(packFileID, path, namelen);
    unsigned written = k_tbl->KernelIOWrite(packFileID, data, size);
    
    return written;
}

void modulesDump( char *packName )
{
    
    SceUID packFileID = k_tbl->KernelIOOpen(packName, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    
    if (packFileID)
    {
        int c = -1;
        k_tbl->KernelIOWrite(packFileID, &c, 4);
        
        SceModule2* mod = k_tbl->KernelFindModuleByName("sceSystemMemoryManager");
        // Write all found flash files
        while ( mod )
        {
            addWriteFile(packFileID, mod->text_addr, mod->text_size, mod->modname);
            mod = mod->next;
        }
        
        k_tbl->KernelIOClose(packFileID);
    }
    else{
        PRTSTR("Could not open file for writing");
    }
}

// Kernel Permission Function
void loadKernelArk(){
    PRTSTR("Dumping kernel ram");

    char path[ARK_PATH_SIZE];
    SceUID fd;

    // Dump kram
    strcpy(path, ark_config->arkpath);
    strcat(path, "KMEM.BIN");
    fd = k_tbl->KernelIOOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0) {
        k_tbl->KernelIOWrite(fd, (void*)0x88000000, 0x400000);
        k_tbl->KernelIOClose(fd);
    }

    // Dump seed
    strcpy(path, ark_config->arkpath);
    strcat(path, "SEED.BIN");
    fd = k_tbl->KernelIOOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0) {
        k_tbl->KernelIOWrite(fd, (void*)0xBFC00200, 0x100);
        k_tbl->KernelIOClose(fd);
    }
    
    // Dump loaded modules
    strcpy(path, ark_config->arkpath);
    strcat(path, "KMEM.ARK");
    modulesDump(path);
}
