#include <taihen.h>
#include <vitasdk.h>
#include <stdint.h>
#include <stdbool.h>

void _start() __attribute__ ((weak, alias ("module_start")));

#define DEBUG 0
#define USE_XMC0 0

/* for chstat cbit */
#define PSP_CST_MODE    0x0001
#define PSP_CST_ATTR    0x0002
#define PSP_CST_SIZE    0x0004
#define PSP_CST_CT      0x0008
#define PSP_CST_AT      0x0010
#define PSP_CST_MT      0x0020
#define PSP_CST_PRVT    0x0040

#define MAX_HOOKS 9
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

int (* ScePspemuConvertStatTimeToUtc)(SceIoStat *stat);
int (* ScePspemuConvertStatTimeToLocaltime)(SceIoStat *stat);

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

void convertFileStat(SceIoStat *stat) {
  if (stat->st_mode & SCE_S_IFDIR)
    stat->st_attr |= SCE_SO_IFDIR;

  if (stat->st_mode & SCE_S_IFREG)
    stat->st_attr |= SCE_SO_IFREG;

  ScePspemuConvertStatTimeToLocaltime(stat);
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
    int res = TAI_CONTINUE(int, hooks[5].ref, path, stat);
    convertFileStat(stat);
    return res;
}

int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits)
{
    char path[256];
    fixEfPath(path, file);

    bits &= ~(PSP_CST_MODE | PSP_CST_ATTR | PSP_CST_SIZE | PSP_CST_PRVT);
    stat->st_attr = 0;
    ScePspemuConvertStatTimeToUtc(stat);

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

int sceIoDreadPatched(SceUID fd, SceIoDirent *dir){
  int res = TAI_CONTINUE(int, hooks[8].ref, fd, dir);
  convertFileStat(&dir->d_stat);
  return res;
}

int module_start(SceSize argc, const void *args) {


    tai_module_info_t tai_info;
    tai_info.size = sizeof(tai_module_info_t);
    taiGetModuleInfo("ScePspemu", &tai_info);

    // Module info
    SceKernelModuleInfo mod_info;
    mod_info.size = sizeof(SceKernelModuleInfo);
    sceKernelGetModuleInfo(tai_info.modid, &mod_info);

    uint32_t text_addr = (uint32_t)mod_info.segments[0].vaddr;
    ScePspemuConvertStatTimeToUtc       = (void *)(text_addr + 0x8664 + 0x1);
    ScePspemuConvertStatTimeToLocaltime = (void *)(text_addr + 0x8680 + 0x1);

    // patch IO
    hooks[0].uid = taiHookFunctionImport(&(hooks[0].ref), "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoOpenPatched);
    hooks[1].uid = taiHookFunctionImport(&(hooks[1].ref), "ScePspemu", 0xCAE9ACE6, 0xE20ED0F3, sceIoRemovePatched);
    hooks[2].uid = taiHookFunctionImport(&(hooks[2].ref), "ScePspemu", 0xCAE9ACE6, 0x9670D39F, sceIoMkdirPatched);
    hooks[3].uid = taiHookFunctionImport(&(hooks[3].ref), "ScePspemu", 0xCAE9ACE6, 0xE9F91EC8, sceIoRmdirPatched);
    hooks[4].uid = taiHookFunctionImport(&(hooks[4].ref), "ScePspemu", 0xCAE9ACE6, 0xA9283DD0, sceIoDopenPatched);
    hooks[5].uid = taiHookFunctionImport(&(hooks[5].ref), "ScePspemu", 0xCAE9ACE6, 0xBCA5B623, sceIoGetstatPatched);
    hooks[6].uid = taiHookFunctionImport(&(hooks[6].ref), "ScePspemu", 0xCAE9ACE6, 0x29482F7F, sceIoChstatPatched);
    hooks[7].uid = taiHookFunctionImport(&(hooks[7].ref), "ScePspemu", 0xCAE9ACE6, 0xF737E369, sceIoRenamePatched);
    hooks[8].uid = taiHookFunctionImport(&(hooks[8].ref), "ScePspemu", 0xCAE9ACE6, 0x9C8B6624, sceIoDreadPatched);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

  for (int i=0; i<MAX_HOOKS; i++){
    taiHookRelease(hooks[i].uid, hooks[i].ref);
  }

  return SCE_KERNEL_STOP_SUCCESS;
}
