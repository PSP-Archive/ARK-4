
#include <pspsdk.h>
#include <pspinit.h>
#include <pspdisplay.h>
#include <stdio.h>


int (* DisplayGetFrameBuf)(void*, void*, void*, int) = NULL;

// CWCHEAT Patch
int sceKernelSuspendThreadPatched(SceUID thid) {
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	if(sceKernelReferThreadStatus(thid, &info) == 0) {
        if (strcmp(info.name, "popsmain") == 0) {
			void *framebuf;
            int width;
			int pixelformat;

			DisplayGetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0xEEDA2E54);
            DisplayGetFrameBuf(&framebuf, &width, &pixelformat, 0);
            memset(framebuf, 0, width * 272 * 4);
		}
	}
    return sceKernelSuspendThread(thid);
}
