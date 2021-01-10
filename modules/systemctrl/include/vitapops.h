#ifndef VITA_POPS
#define VITA_POPS

extern int (* _sceDisplaySetFrameBufferInternal)(int pri, void *topaddr, int width, int format, int sync);

int sctrlHENIsVitaPops();
void patchVitaPopsDisplay();
int sceDisplaySetFrameBufferInternalHook(int pri, void *topaddr, int width, int format, int sync);

#endif
