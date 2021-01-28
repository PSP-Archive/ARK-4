#include <pspkernel.h>
#include <string.h>
#include "utils.h"
#include "systemctrl.h"
#include "main.h"

int vshCtrlDeleteHibernation(void)
{
	SceUID fd;
	char buf[512 + 64], *p;
	int ret, level;
	u32 k1;

	if(psp_model != PSP_GO) {
		return -1;
	}

	k1 = pspSdkSetK1(0);
	level = sctrlKernelSetUserLevel(8);
	p = (char*)((((u32)buf) & ~(64-1)) + 64);
	memset(p, 0, 512);

	fd = sceIoOpen("eflash0a:__hibernation", PSP_O_RDWR | 0x04000000, 0777);

	if(fd < 0) {
		sctrlKernelSetUserLevel(level);
		pspSdkSetK1(k1);
		return fd;
	}

	ret = sceIoWrite(fd, p, 512);

	if(ret < 0) {
		sceIoClose(fd);
		sctrlKernelSetUserLevel(level);
		pspSdkSetK1(k1);
		return ret;
	}

	sceIoClose(fd);
	sctrlKernelSetUserLevel(level);
	pspSdkSetK1(k1);

	return 0;
}
