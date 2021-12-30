#ifndef PSP_MAIN_H
#define PSP_MAIN_H

extern "C"{
typedef void (* sceNetApctlHandler)(int oldState, int newState, int event, int error, void *pArg);

int sceNetInit(int poolsize, int calloutprio, int calloutstack, int netintrprio, int netintrstack);
int sceNetInetInit(void);
int sceNetResolverInit(void);
int sceNetApctlInit(int stackSize, int initPriority);
int sceNetApctlAddHandler(sceNetApctlHandler handler, void *pArg);
int sceNetApctlGetInfo(int code, union SceNetApctlInfo *pInfo);
int sceNetApctlDisconnect(void);

}

void mftpAddNewStatusMessage(char *Message);

#endif
