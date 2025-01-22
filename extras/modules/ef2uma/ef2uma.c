#include <taihen.h>
#include <vitasdk.h>
#include <stdint.h>
#include <stdbool.h>

void _start() __attribute__ ((weak, alias ("module_start")));

#define DEBUG 0
#define USE_XMC0 0

#define MAX_HOOKS 8
struct{
  SceUID uid;
  tai_hook_ref_t ref;
} hooks[MAX_HOOKS];

#if DEBUG
void logtext(char* text){
  int fd = sceIoOpen("ux0:/iolog.txt", SCE_O_WRONLY|SCE_O_APPEND|SCE_O_CREAT, 0777);
  sceIoWrite(fd, text, strlen(text));
  sceIoClose(fd);
}
#endif

bool checkEfPath(const char* path, int* fstart)
{
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

void fixEfPath(newpath, oldpath)
{
  int fstart = 0;
  if (checkEfPath(oldpath, &fstart)){
      #if USE_XMC0
      strcpy(newpath, "xmc0:pspemu");
      #else
      strcpy(newpath, "uma0:pspemu");
      #endif
      strcat(newpath, oldpath+fstart);
      
      #if DEBUG
      logtext("fixed ");
      logtext(oldpath);
      logtext(" -> ");
      logtext(newpath);
      logtext("\n");
      #endif 
  }
  else{
    strcpy(newpath, oldpath);
  }
}

// IO Open patched
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode)
{
    char path[256];
    fixEfPath(path, file);
    return TAI_CONTINUE(SceUID, hooks[0].ref, path, flags, mode);
}

int sceIoRemovePatched(const char * file)
{
    char path[256];
    fixEfPath(path, file);
    return TAI_CONTINUE(int, hooks[1].ref, path);
}

int sceIoMkdirPatched(char * file, SceMode mode)
{
	  char path[256];
    fixEfPath(path, file);
    return TAI_CONTINUE(int, hooks[2].ref, path, mode);
}

int sceIoRmdirPatched(char * file)
{
	  char path[256];
    fixEfPath(path, file);
    return TAI_CONTINUE(int, hooks[3].ref, path);
}

int sceIoDopenPatched(char * file)
{
	  char path[256];
    fixEfPath(path, file);
    return TAI_CONTINUE(int, hooks[4].ref, path);
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat)
{
    char path[256];
    fixEfPath(path, file);
    return TAI_CONTINUE(int, hooks[5].ref, path, stat);
}

int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits)
{
    char path[256];
    fixEfPath(path, file);
    return TAI_CONTINUE(int, hooks[6].ref, path, stat, bits);
}

int sceIoRenamePatched(const char* oldfile, const char* newfile)
{  
    char oldpath[256];
    char newpath[256];
    fixEfPath(oldpath, oldfile);
    fixEfPath(newpath, newfile);
    return TAI_CONTINUE(int, hooks[7].ref, oldpath, newpath);
}

int module_start(SceSize argc, const void *args) {

    // patch IO
    hooks[0].uid = taiHookFunctionImport(&(hooks[0].ref), "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoOpenPatched);
    hooks[1].uid = taiHookFunctionImport(&(hooks[1].ref), "ScePspemu", 0xCAE9ACE6, 0xE20ED0F3, sceIoRemovePatched);
    hooks[2].uid = taiHookFunctionImport(&(hooks[2].ref), "ScePspemu", 0xCAE9ACE6, 0x9670D39F, sceIoMkdirPatched);
    hooks[3].uid = taiHookFunctionImport(&(hooks[3].ref), "ScePspemu", 0xCAE9ACE6, 0xE9F91EC8, sceIoRmdirPatched);
    hooks[4].uid = taiHookFunctionImport(&(hooks[4].ref), "ScePspemu", 0xCAE9ACE6, 0xA9283DD0, sceIoDopenPatched);
    hooks[5].uid = taiHookFunctionImport(&(hooks[5].ref), "ScePspemu", 0xCAE9ACE6, 0xBCA5B623, sceIoGetstatPatched);
    hooks[6].uid = taiHookFunctionImport(&(hooks[6].ref), "ScePspemu", 0xCAE9ACE6, 0x29482F7F, sceIoChstatPatched);
    hooks[7].uid = taiHookFunctionImport(&(hooks[7].ref), "ScePspemu", 0xCAE9ACE6, 0xF737E369, sceIoRenamePatched);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

  for (int i=0; i<MAX_HOOKS; i++){
    taiHookRelease(hooks[i].uid, hooks[i].ref);
  }

  return SCE_KERNEL_STOP_SUCCESS;
}
