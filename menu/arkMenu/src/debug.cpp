#include "debug.h"
#include <pspthreadman.h>

static SceUID debugFileSema = -1;

void debugFile(const char* text){

	if (debugFileSema < 0)
		debugFileSema = sceKernelCreateSema("debug_file_sema",  0, 1, 1, NULL);
	
	sceKernelWaitSema(debugFileSema, 1, NULL);
	FILE* fp = fopen("DEBUG.TXT", "a+");
	fwrite(text, 1, strlen(text), fp);
	fclose(fp);
	sceKernelSignalSema(debugFileSema, 1);
}
