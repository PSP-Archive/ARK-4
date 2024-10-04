#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pspkernel.h>

#include <systemctrl.h>
#include <rebootconfig.h>
#include <macros.h>

#include <common/include/reboot150.h>
#include <rebootex150/rebootex150.h>

PSP_MODULE_INFO("Reboot150", PSP_MODULE_KERNEL | PSP_MODULE_SINGLE_START | PSP_MODULE_SINGLE_LOAD, 1, 0);

int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
	ARKConfig *ark_config = sctrlHENGetArkConfig(NULL);
	RebootConfigARK *rebootex_config = sctrlHENGetRebootexConfig(NULL);

	rebootex_config->boot_from_fw_version = sceKernelDevkitVersion();

	// clean reboot memory
    memset((char *)REBOOT150_TEXT, 0, 0x400000);

    memcpy(REBOOTEX_TEXT, rebootex150, size_rebootex150);

	// Restore Reboot Buffer Configuration
    memcpy((void *)REBOOTEX_CONFIG, &rebootex_config, sizeof(RebootConfigARK));

    // Restore ARK Configuration
    memcpy(ARK_CONFIG, ark_config, sizeof(ARKConfig));

	return sceKernelGzipDecompress((void *)REBOOT150_TEXT, REBOOT150_SIZE, reboot150 + 0x10, 0);
}

int module_start(SceSize args, void *argp)
{
	int dreg = sceIoOpen("ms0:/TM/DCARK/150/registry/system.dreg", PSP_O_RDONLY, 0);
	int ireg = sceIoOpen("ms0:/TM/DCARK/150/registry/system.ireg", PSP_O_RDONLY, 0);
	if(dreg < 0 && ireg < 0) {
		dreg = sceIoOpen("ms0:/TM/DCARK/150/registry/system.dreg", PSP_O_WRONLY | PSP_O_CREAT, 0777);
		ireg = sceIoOpen("ms0:/TM/DCARK/150/registry/system.ireg", PSP_O_WRONLY | PSP_O_CREAT, 0777);
		int flash_dreg = sceIoOpen("flash1:/registry/system.dreg", PSP_O_RDONLY, 0);
		int flash_ireg = sceIoOpen("flash1:/registry/system.ireg", PSP_O_RDONLY, 0);

		static u8 buf[200000]; __attribute__((aligned(0x40)));
		memset((u8 *)buf, 0, sizeof(buf));

		int size = sceIoRead(flash_dreg, buf, sizeof(buf));
		sceIoWrite(dreg, buf, size);
		sceIoClose(dreg);
		sceIoClose(flash_dreg);

		memset((u8 *)buf, 0, sizeof(buf));

		size = sceIoRead(flash_ireg, buf, sizeof(buf));
		sceIoWrite(ireg, buf, size);
		sceIoClose(ireg);
		sceIoClose(flash_ireg);
	}
	sceIoClose(dreg);
	sceIoClose(ireg);


	sctrlHENSetLoadRebootOverrideHandler(LoadReboot);

	return 0;
}

int module_stop(void)
{
	return 0;
}
