#include <taihen.h>
#include <vitasdk.h>
#include <stdint.h>
#include <stdbool.h>

void _start() __attribute__ ((weak, alias ("module_start")));

#define SCE_PSPEMU_CACHE_NONE 0x1

#define DEBUG 0

tai_module_info_t info;
uint32_t module_nid;

SceUID sceIoOpenHook = -1;
tai_hook_ref_t sceIoOpenRef;

SceUID sceIoRemoveHook = -1;
tai_hook_ref_t sceIoRemoveRef;

SceUID sceIoMkdirHook = -1;
tai_hook_ref_t sceIoMkdirRef;

SceUID sceIoRmdirHook = -1;
tai_hook_ref_t sceIoRmdirRef;

SceUID sceIoDopenHook = -1;
tai_hook_ref_t sceIoDopenRef;

SceUID sceIoGetstatHook = -1;
tai_hook_ref_t sceIoGetstatRef;

SceUID sceIoChstatHook = -1;
tai_hook_ref_t sceIoChstatRef;

SceUID sceIoRenameHook = -1;
tai_hook_ref_t sceIoRenameRef;

SceUID sceIoChdirHook = -1;
tai_hook_ref_t sceIoChdirRef;

int (* ScePspemuErrorExit)(int error);
int (* ScePspemuConvertAddress)(uint32_t addr, int mode, uint32_t cache_size);
int (* ScePspemuWritebackCache)(void *addr, int size);


void get_functions(uint32_t text_addr) {
    ScePspemuErrorExit                  = (void *)(text_addr + 0x4104 + 0x1);
    ScePspemuConvertAddress             = (void *)(text_addr + 0x6364 + 0x1);
    ScePspemuWritebackCache             = (void *)(text_addr + 0x6490 + 0x1);
}

#if DEBUG
void logtext(char* text){
  int fd = sceIoOpen("ux0:/iolog.txt", SCE_O_WRONLY|SCE_O_APPEND|SCE_O_CREAT, 0777);
  sceIoWrite(fd, text, strlen(text));
  sceIoClose(fd);
}
#endif

bool checkEfPath(const char* path, int* fstart){
  if (strncmp("ms0:__ef0__", path, 11) == 0){
    *fstart = 11;
    return true;
  }
  else if (strncmp("ms0:/__ef0__", path, 12) == 0){
    *fstart = 12;
    return true;
  }
  return false;
}

// IO Open patched
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
    
    char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);
      
      #if DEBUG
      logtext("opening ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }
    
    return TAI_CONTINUE(SceUID, sceIoOpenRef, path, flags, mode);
}

int sceIoRemovePatched(const char * file)
{

    char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);
      
      #if DEBUG
      logtext("removing ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }
    
    // Forward Call
    return TAI_CONTINUE(int, sceIoRemoveRef, path);
}

int sceIoMkdirPatched(char * file, SceMode mode)
{
	  char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);
      
      #if DEBUG
      logtext("mkdir ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }
    
    // Forward Call
    return TAI_CONTINUE(int, sceIoMkdirRef, path, mode);
}

int sceIoRmdirPatched(char * file)
{
	  char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);
      
      #if DEBUG
      logtext("rmdir ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }
    
    // Forward Call
    return TAI_CONTINUE(int, sceIoRmdirRef, path);
}

int sceIoDopenPatched(char * file)
{
	  char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);

      #if DEBUG
      logtext("dopen ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }
    
    // Forward Call
    return TAI_CONTINUE(int, sceIoDopenRef, path);
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat) {

    char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);
      
      #if DEBUG
      logtext("getstat ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }

    return TAI_CONTINUE(int, sceIoGetstatRef, path, stat);
}

int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits) {

    char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);
      
      #if DEBUG
      logtext("chstat ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }

    return TAI_CONTINUE(int, sceIoChstatRef, path, stat, bits);
}

int sceIoRenamePatched(const char* oldfile, const char* newfile){
  
    char oldpath[256]; int ofstart = 0;
    char newpath[256]; int nfstart = 0;

    if (checkEfPath(oldpath, &ofstart)){
      strcpy(oldpath, "uma0:pspemu");
      strcat(oldpath, oldfile+ofstart);
    }
    else {
      strcpy(oldpath, oldfile);
    }

    if (checkEfPath(newfile, &nfstart)){
      strcpy(newpath, "uma0:pspemu");
      strcat(newpath, newfile+nfstart);
    }
    else{
      strcpy(newpath, newfile);
    }

    #if DEBUG
    logtext("renaming ");
    logtext(oldpath);
    logtext(" -> ");
    logtext(newpath);
    logtext("\n");
    #endif

    return TAI_CONTINUE(int, sceIoRenameRef, oldpath, newpath);
}

/*
int sceIoChdirPatched(char * file)
{
	  char path[256]; int fstart = 0;

    if (checkEfPath(file, &fstart)){
      strcpy(path, "uma0:pspemu");
      strcat(path, file+fstart);
      
      #if DEBUG
      logtext("chdir ");
      logtext(path);
      logtext("\n");
      #endif
    }
    else {
      strcpy(path, file);
    }
    
    // Forward Call
    return TAI_CONTINUE(int, sceIoChdirRef, path);
}
*/

int module_start(SceSize argc, const void *args) {
    info.size = sizeof(info);

    taiGetModuleInfo("ScePspemu", &info);

    SceKernelModuleInfo mod_info;
    mod_info.size = sizeof(SceKernelModuleInfo);
    int ret = sceKernelGetModuleInfo(info.modid, &mod_info);
    
    module_nid = info.module_nid;

    // Get PspEmu functions
    get_functions((uint32_t)mod_info.segments[0].vaddr);

    // patch IO
    sceIoOpenHook = taiHookFunctionImport(&sceIoOpenRef, "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoOpenPatched);
    sceIoRemoveHook = taiHookFunctionImport(&sceIoRemoveRef, "ScePspemu", 0xCAE9ACE6, 0xE20ED0F3, sceIoRemovePatched);
    sceIoMkdirHook = taiHookFunctionImport(&sceIoMkdirRef, "ScePspemu", 0xCAE9ACE6, 0x9670D39F, sceIoMkdirPatched);
    sceIoRmdirHook = taiHookFunctionImport(&sceIoRmdirRef, "ScePspemu", 0xCAE9ACE6, 0xE9F91EC8, sceIoRmdirPatched);
    sceIoDopenHook = taiHookFunctionImport(&sceIoDopenRef, "ScePspemu", 0xCAE9ACE6, 0xA9283DD0, sceIoDopenPatched);
    sceIoGetstatHook = taiHookFunctionImport(&sceIoGetstatRef, "ScePspemu", 0xCAE9ACE6, 0xBCA5B623, sceIoGetstatPatched);
    sceIoChstatHook = taiHookFunctionImport(&sceIoChstatRef, "ScePspemu", 0xCAE9ACE6, 0x29482F7F, sceIoChstatPatched);
    sceIoRenameHook = taiHookFunctionImport(&sceIoRenameRef, "ScePspemu", 0xCAE9ACE6, 0xF737E369, sceIoRenamePatched);
    //sceIoChdirHook = taiHookFunctionImport(&sceIoChdirRef, "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoChdirPatched);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

  if (sceIoOpenHook >= 0) taiHookRelease(sceIoOpenHook, sceIoOpenRef);
  if (sceIoRemoveHook >= 0) taiHookRelease(sceIoRemoveHook, sceIoRemoveRef);
  if (sceIoMkdirHook >= 0) taiHookRelease(sceIoMkdirHook, sceIoMkdirRef);
  if (sceIoRmdirHook >= 0) taiHookRelease(sceIoRmdirHook, sceIoRmdirRef);
  if (sceIoDopenHook >= 0) taiHookRelease(sceIoDopenHook, sceIoDopenRef);
  if (sceIoGetstatHook >= 0) taiHookRelease(sceIoGetstatHook, sceIoGetstatRef);
  if (sceIoChstatHook >= 0) taiHookRelease(sceIoChstatHook, sceIoChstatRef);
  if (sceIoRenameHook >= 0) taiHookRelease(sceIoRenameHook, sceIoRenameRef);

  return SCE_KERNEL_STOP_SUCCESS;
}
