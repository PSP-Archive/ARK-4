#include "../common.h"
#include "start.h"

#include "intraFont.h"

extern "C" {

PSP_MODULE_INFO("SaveStateUser", 0, 1, 0);
PSP_HEAP_SIZE_KB(8 * 1024); //-4096

int main();

PspGeContext __attribute__((aligned(16))) context;

int pmode, pwidth, pheight, pbufferwidth, ppixelformat;
void *vram_buffer, *pvram, *pvram_bak;

int running = 1;

SceUID user_thid = -1;

SceUID sceKernelAllocPartitionMemory(SceUID partitionid, const char *name, int type, SceSize size, void *addr)
{
	return _sceKernelAllocPartitionMemory(11, name, type, size, addr);
}

int user_thread(SceSize args, void *argp)
{
	/* Suspend threads */
	GetThreads();
	SuspendThreads();

	sceDisplayWaitVblankStart(); //stability purpose

	/* Save vram */
	sceDisplayGetMode(&pmode, &pwidth, &pheight);
	sceDisplayGetFrameBuf(&pvram, &pbufferwidth, &ppixelformat, PSP_DISPLAY_SETBUF_IMMEDIATE);

	pvram_bak = malloc(pbufferwidth);
	sceDmacMemcpy(pvram_bak, pvram, pbufferwidth);

	void *vram_addr = sceGeEdramGetAddr();
	u32 vram_size = sceGeEdramGetSize();

	vram_buffer = malloc(vram_size);
	sceDmacMemcpy(vram_buffer, vram_addr, vram_size);

	sceDisplayWaitVblankStart(); //stability purpose

	/* Save GE context */
	sceGeSaveContext(&context);

	/* Main */
	main();

	/* Restore GE context */
	sceGeRestoreContext(&context);

	sceDisplayWaitVblankStart(); //stability purpose

	/* Restore vram */
	sceDmacMemcpy(vram_addr, vram_buffer, vram_size);
	free(vram_buffer);

	sceDisplaySetMode(pmode, pwidth, pheight);
	sceDisplaySetFrameBuf(pvram, pbufferwidth, ppixelformat, PSP_DISPLAY_SETBUF_NEXTFRAME);

	sceDisplayWaitVblankStart(); //stability purpose

	/* Resume threads */
	ResumeThreads();

	__psp_free_heap();
	sceKernelSelfStopUnloadModule(0, 0, NULL);
	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	user_thid = sceKernelCreateThread("user_thread", user_thread, 32, 0x10000, PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU, NULL);
	if(user_thid >= 0) sceKernelStartThread(user_thid, 0, NULL);
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	running = 0;

	SceUInt timeout = 100 * 1000;
	if(sceKernelWaitThreadEnd(user_thid, &timeout) < 0)
	{
		sceKernelTerminateDeleteThread(user_thid);
	}

	return 0;
}

}
