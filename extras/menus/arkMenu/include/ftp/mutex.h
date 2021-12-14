#ifndef _MUTEX_H_
#define _MUTEX_H_

#define PSP_MUTEX_ATTR_FIFO 0
#define PSP_MUTEX_ATTR_PRIORITY 0x100
#define PSP_MUTEX_ATTR_ALLOW_RECURSIVE 0x200

#ifdef __cplusplus
extern "C" {
#endif

int sceKernelCreateMutex(const char *name, uint attributes, int initial_count, void* options);
int sceKernelDeleteMutex(int mutexId);
int sceKernelLockMutex(int mutexId, int count, uint* timeout);
int sceKernelTryLockMutex(int mutexId, int count);
int sceKernelUnlockMutex(int mutexId, int count);

#ifdef __cplusplus
}
#endif


#endif
