// system entry trickery
#ifndef PSP_LOADUTIL_H
#define PSP_LOADUTIL_H

#ifdef __cplusplus
extern "C"{
#endif

// if library must be loaded into user memory/partition, but from
//   a kernel thread, we must manually patch the entry table
SceUID LoadAndStartAndPatch(SceModuleInfo* modInfoPtr, const char* szFile);
void FlushCaches();

#ifdef __cplusplus
}
#endif

#endif
