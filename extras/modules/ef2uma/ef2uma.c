#include <taihen.h>
#include <vitasdk.h>
#include <stdint.h>
#include <stdbool.h>

void _start() __attribute__ ((weak, alias ("module_start")));

#define DEBUG 0

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

int module_start(SceSize argc, const void *args) {

    // patch IO
    sceIoOpenHook = taiHookFunctionImport(&sceIoOpenRef, "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoOpenPatched);
    sceIoRemoveHook = taiHookFunctionImport(&sceIoRemoveRef, "ScePspemu", 0xCAE9ACE6, 0xE20ED0F3, sceIoRemovePatched);
    sceIoMkdirHook = taiHookFunctionImport(&sceIoMkdirRef, "ScePspemu", 0xCAE9ACE6, 0x9670D39F, sceIoMkdirPatched);
    sceIoRmdirHook = taiHookFunctionImport(&sceIoRmdirRef, "ScePspemu", 0xCAE9ACE6, 0xE9F91EC8, sceIoRmdirPatched);
    sceIoDopenHook = taiHookFunctionImport(&sceIoDopenRef, "ScePspemu", 0xCAE9ACE6, 0xA9283DD0, sceIoDopenPatched);
    sceIoGetstatHook = taiHookFunctionImport(&sceIoGetstatRef, "ScePspemu", 0xCAE9ACE6, 0xBCA5B623, sceIoGetstatPatched);
    sceIoChstatHook = taiHookFunctionImport(&sceIoChstatRef, "ScePspemu", 0xCAE9ACE6, 0x29482F7F, sceIoChstatPatched);
    sceIoRenameHook = taiHookFunctionImport(&sceIoRenameRef, "ScePspemu", 0xCAE9ACE6, 0xF737E369, sceIoRenamePatched);

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
