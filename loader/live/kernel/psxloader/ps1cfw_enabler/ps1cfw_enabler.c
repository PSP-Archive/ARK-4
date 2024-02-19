#include <taihen.h>
#include <vitasdk.h>
#include <stdint.h>

void _start() __attribute__ ((weak, alias ("module_start")));

#define SCE_PSPEMU_CACHE_NONE 0x1

tai_module_info_t info;
uint32_t module_nid;

SceUID sceIoOpenHook = -1;
SceUID sceIoStatHook = -1;
SceUID titleIdHook = -1;
tai_hook_ref_t sceIoOpenRef;
tai_hook_ref_t sceIoGetstatRef;

SceUID io_patch_path = -1;
SceUID io_patch_size = -1;
SceUID ctrl_patch = -1;

uint32_t movs_a1_0_nop_opcode = 0xBF002000;
uint32_t nop_nop_opcode = 0xBF00BF00;
uint32_t mov_r2_r4_mov_r4_r2 = 0x46224614;
uint32_t mips_move_a2_0 = 0x00003021;
uint32_t mips_nop = 0;

typedef struct PopsConfig{
    uint32_t magic;
    char title_id[20];
    char path[256];
}PopsConfig;

#define ARK_MAGIC 0xB00B1E55

PopsConfig popsconfig;

int (* ScePspemuErrorExit)(int error);
int (* ScePspemuConvertAddress)(uint32_t addr, int mode, uint32_t cache_size);
int (* ScePspemuWritebackCache)(void *addr, int size);
int (* ScePspemuPausePops)(int pause);


void get_functions(uint32_t text_addr) {
    ScePspemuErrorExit                  = (void *)(text_addr + 0x4104 + 0x1);
    ScePspemuConvertAddress             = (void *)(text_addr + 0x6364 + 0x1);
    ScePspemuWritebackCache             = (void *)(text_addr + 0x6490 + 0x1);
  
    if (module_nid == 0x2714F07D) {
        ScePspemuPausePops                  = (void *)(text_addr + 0x300C0 + 0x1);
    }
    else {
        ScePspemuPausePops                  = (void *)(text_addr + 0x300D4 + 0x1);
    }
}

// IO Open patched
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
  
  // Virtual Kernel Exploit (allow easy escalation of priviledge)
    if (file != NULL && strstr(file, "__dokxploit__") != 0){
	    uint32_t *m;
	    
	    // remove k1 checks in IoRead (lets you write into kram)
	    m = (uint32_t *)ScePspemuConvertAddress(0x8805769C, SCE_PSPEMU_CACHE_NONE, 4);
	    *m = mips_move_a2_0; // move $a2, 0
	    ScePspemuWritebackCache(m, 4);

	    // remove k1 checks in IoWrite (lets you read kram)
	    m = (uint32_t *)ScePspemuConvertAddress(0x880577B0, SCE_PSPEMU_CACHE_NONE, 4);
	    *m = mips_move_a2_0; // move $a2, 0
	    ScePspemuWritebackCache(m, 4);

	    // allow running any code as kernel (lets us pass function pointer as second argument of libctime)
	    m = (uint32_t *)ScePspemuConvertAddress((module_nid==0x2714F07D)?0x88010044:0x8800FFB4, SCE_PSPEMU_CACHE_NONE, 4);
	    *m = mips_nop; // nop
	    ScePspemuWritebackCache(m, 4);
	    return 0;
    }
  
    // Configure currently loaded game
    char* popsetup = strstr(file, "__popsconfig__");
    if (popsetup){
        char* title_id = strchr(popsetup, '/') + 1;
        char* path = strchr(title_id, '/');
        strncpy(popsconfig.title_id, title_id, (path-title_id));
        strcpy(popsconfig.path, path);
        popsconfig.magic = ARK_MAGIC;
        return -101;
    }

    // Clear configuration 
    if (strstr(file, "__popsclear__")){
        memset(&popsconfig, 0, sizeof(PopsConfig));
        return -102;
    }
    
    // Handle when system has booted
    if (strstr(file, "__popsbooted__")){
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
        sceKernelPowerUnlock(0);
        return -103;
    }
    
    // Pause POPS
    if (strstr(file, "__popspause__")){
        ScePspemuPausePops(1);
        sceDisplayWaitVblankStart();
        return -104;
    }
    
    // Resume POPS
    if (strstr(file, "__popsresume__")){
        ScePspemuPausePops(0);
        sceDisplayWaitVblankStart();
        return -105;
    }
    
    // Clean Exit
    if (strstr(file, "__popsexit__")){
        ScePspemuErrorExit(0);
        return 0;
    }

    // Redirect files for memory card manager
    if (popsconfig.magic == ARK_MAGIC && popsconfig.title_id[0] && popsconfig.path[0]){
      char *p = strrchr(file, '/');
      if (p) {
        static char new_file[256];

        if (strcmp(p+1, "__sce_menuinfo") == 0) {
          char *filename = popsconfig.path;
          char *q = strrchr(filename, '/');
          if (q) {
            char path[128];
            strncpy(path, filename, q-(filename));
            path[q-filename] = '\0';

            snprintf(new_file, sizeof(new_file), "ms0:%s/__sce_menuinfo", path);
            file = new_file;
          }
        } else if (strstr(file, "/SCPS10084/") &&
                  (strcmp(p+1, "PARAM.SFO") == 0 ||
                   strcmp(p+1, "SCEVMC0.VMP") == 0 ||
                   strcmp(p+1, "SCEVMC1.VMP") == 0)) {
          snprintf(new_file, sizeof(new_file), "ms0:PSP/SAVEDATA/%s/%s", popsconfig.title_id, p+1);
          file = new_file;
        }
      }
    }
    return TAI_CONTINUE(SceUID, sceIoOpenRef, file, flags, mode);
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat) {
  if (popsconfig.magic == ARK_MAGIC && popsconfig.title_id[0] && popsconfig.path[0]){
      char *p = strrchr(file, '/');
      if (p) {
        static char new_file[256];
        if (strstr(file, "/SCPS10084/") &&
           (strcmp(p+1, "PARAM.SFO") == 0 ||
            strcmp(p+1, "SCEVMC0.VMP") == 0 ||
            strcmp(p+1, "SCEVMC1.VMP") == 0)) {
          snprintf(new_file, sizeof(new_file), "ms0:PSP/SAVEDATA/%s/%s", popsconfig.title_id, p+1);
          file = new_file;
        }
      }
  }
  return TAI_CONTINUE(int, sceIoGetstatRef, file, stat);
}

SceUID thread_hook = -1;

int module_start(SceSize argc, const void *args) {
    info.size = sizeof(info);

    taiGetModuleInfo("ScePspemu", &info);

    memset(&popsconfig, 0, sizeof(PopsConfig));

    SceKernelModuleInfo mod_info;
    mod_info.size = sizeof(SceKernelModuleInfo);
    int ret = sceKernelGetModuleInfo(info.modid, &mod_info);
    
    module_nid = info.module_nid;

    // Get PspEmu functions
    get_functions((uint32_t)mod_info.segments[0].vaddr);

    // allow opening any path
    io_patch_path = taiInjectData(info.modid, 0x00, 0x839C, &nop_nop_opcode, 0x4);

    // allow opening files of any size
    io_patch_size = taiInjectData(info.modid, 0x00, 0xA13C, &mov_r2_r4_mov_r4_r2, 0x4);

    // patch ioOpen for various functions
    sceIoOpenHook = taiHookFunctionImport(&sceIoOpenRef, "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoOpenPatched);

    // fix memory card manager
    sceIoStatHook = taiHookFunctionImport(&sceIoGetstatRef, "ScePspemu", 0xCAE9ACE6, 0xBCA5B623, sceIoGetstatPatched);

    // fix controller on Vita TV
    if (module_nid == 0x2714F07D){
      ctrl_patch = taiInjectData(info.modid, 0, 0x2073C, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    }
    else {
      ctrl_patch = taiInjectData(info.modid, 0, 0x20740, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

  if (sceIoOpenHook >= 0) taiHookRelease(sceIoOpenHook, sceIoOpenRef);
  if (sceIoStatHook >= 0) taiHookRelease(sceIoStatHook, sceIoGetstatRef);
  if (io_patch_path >= 0) taiInjectRelease(io_patch_path);
  if (io_patch_size >= 0) taiInjectRelease(io_patch_size);
  if (ctrl_patch    >= 0) taiInjectRelease(ctrl_patch);

  return SCE_KERNEL_STOP_SUCCESS;
}
