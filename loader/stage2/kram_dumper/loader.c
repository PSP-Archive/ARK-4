#include "main.h"
#include "flash_dump.h"
#include <functions.h>


// Kernel Permission Function
void loadKernelArk(){
    PRTSTR("Dumping kernel ram");

    char kmem[ARK_PATH_SIZE];
    strcpy(kmem, ark_config->arkpath);
    strcat(kmem, "KMEM.BIN");
    
    char seed[ARK_PATH_SIZE];
    strcpy(seed, ark_config->arkpath);
    strcat(seed, "SEED.BIN");
    
    SceUID fd;

	fd = k_tbl->KernelIOOpen(kmem, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd >= 0) {
		k_tbl->KernelIOWrite(fd, (void*)0x88000000, 0x400000);
		k_tbl->KernelIOClose(fd);
	}

	fd = k_tbl->KernelIOOpen(seed, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd >= 0) {
		k_tbl->KernelIOWrite(fd, (void*)0xBFC00200 0x100);
		k_tbl->KernelIOClose(fd);
	}
}
