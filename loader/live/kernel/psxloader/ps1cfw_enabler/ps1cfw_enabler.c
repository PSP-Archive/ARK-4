#include <taihen.h>
#include <vitasdk.h>
#include <stdint.h>

#include "main.h"

void _start() __attribute__ ((weak, alias ("module_start")));

#define SCE_PSPEMU_CACHE_NONE 0x1
#define THUMB_SHUFFLE(x) ((((x) & 0xFFFF0000) >> 16) | (((x) & 0xFFFF) << 16))

uint32_t module_nid;
SceUID sceIoOpenHook = -1;
SceUID sceIoStatHook = -1;
SceUID titleIdHook = -1;
tai_hook_ref_t sceIoOpenRef;
tai_hook_ref_t sceIoGetstatRef;
tai_hook_ref_t ScePspemuGetTitleidRef;

SceUID io_patch_path = -1;
SceUID io_patch_size = -1;

uint32_t nop_nop_opcode = 0xBF00BF00;
uint32_t mov_r2_r4_mov_r4_r2 = 0x46224614;
uint16_t b_send_resp = 0xEAFFFFEA;
uint32_t mips_move_a2_0 = 0x00003021;
uint32_t mips_nop = 0;

typedef struct PopsConfig{
    uint32_t magic;
    char title_id[20];
    char path[256];
}PopsConfig;

#define ARK_MAGIC 0xB00B1E55

PopsConfig popsconfig;

int (* ScePspemuDivide)(uint64_t x, uint64_t y);
int (* ScePspemuErrorExit)(int error);
int (* ScePspemuConvertAddress)(uint32_t addr, int mode, uint32_t cache_size);
int (* ScePspemuWritebackCache)(void *addr, int size);
int (* ScePspemuInitPocs)();

void get_functions(uint32_t text_addr) {
  ScePspemuDivide                     = (void *)(text_addr + 0x39F0 + 0x1);
  ScePspemuErrorExit                  = (void *)(text_addr + 0x4104 + 0x1);
  ScePspemuConvertAddress             = (void *)(text_addr + 0x6364 + 0x1);
  ScePspemuWritebackCache             = (void *)(text_addr + 0x6490 + 0x1);
  
  ScePspemuInitPocs                   = (void *)(text_addr + 0x30678 + 0x1);
}

// Tile ID patched
char *ScePspemuGetTitleidPatched() {
  if (popsconfig.magic == ARK_MAGIC && popsconfig.title_id[0])
      return popsconfig.title_id;
  return TAI_CONTINUE(char*, ScePspemuGetTitleidRef);
}

#if 0
void logtext(char* text){
    SceUID fd = sceIoOpen("ux0:pspemu/ps1log.txt", SCE_O_WRONLY|SCE_O_CREAT|SCE_O_APPEND, 0777);
    sceIoWrite(fd, text, strlen(text));
    sceIoClose(fd);
}
#endif

// IO Open patched
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
  
  // Virtual Kernel Exploit (allow easy escalation of priviledge)
    if (file != NULL && strstr(file, "__dokxploit__") != 0){
	    uint32_t *m;
	    
	    // remove k1 checks in IoRead (lets you write into kram)
	    m = (uint32_t *)ScePspemuConvertAddress((module_nid==0x2714F07D)?0x8805769c:0x8805769C, SCE_PSPEMU_CACHE_NONE, 4);
	    *m = mips_move_a2_0; // move $a2, 0
	    ScePspemuWritebackCache(m, 4);

	    // remove k1 checks in IoWrite (lets you read kram)
	    m = (uint32_t *)ScePspemuConvertAddress((module_nid==0x2714F07D)?0x880577b0:0x880577B0, SCE_PSPEMU_CACHE_NONE, 4);
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
        strcpy(popsconfig.path, "ms0:");
        strcat(popsconfig.path, path);
        popsconfig.magic = ARK_MAGIC;
        return -101;
    }

    // Clear configuration 
    if (strstr(file, "__popsclear__")){
        memset(&popsconfig, 0, sizeof(PopsConfig));
        return -102;
    }
    
    // Exit
    if (strstr(file, "__popsexit__")){
        ScePspemuErrorExit(0);
        return 0;
    }
    
    // unlock PS button when boot complete
    if (strstr(file, "__popsbooted__")){
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
        sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
        return -103;
    }

    // Redirect files for memory card manager
    if (popsconfig.magic == ARK_MAGIC && popsconfig.title_id[0] && popsconfig.path[0]){
      char *p = strrchr(file, '/');
      if (p) {
        static char new_file[256];

        if (strcmp(p+1, "__sce_menuinfo") == 0) {
          char *filename = popsconfig.path;
          if (strncmp(filename, "ms0:/", 5) == 0) {
            char *q = strrchr(filename, '/');
            if (q) {
              char path[128];
              strncpy(path, filename+5, q-(filename+5));
              path[q-(filename+5)] = '\0';

              snprintf(new_file, sizeof(new_file), "%s/%s/__sce_menuinfo", getPspemuMemoryStickLocation(), path);
              file = new_file;
            }
          }
        } else if (strstr(file, "pspemu/PSP/SAVEDATA/") &&
                  (strcmp(p+1, "PARAM.SFO") == 0 ||
                   strcmp(p+1, "SCEVMC0.VMP") == 0 ||
                   strcmp(p+1, "SCEVMC1.VMP") == 0)) {
          snprintf(new_file, sizeof(new_file), "%s/PSP/SAVEDATA/%s/%s", getPspemuMemoryStickLocation(), popsconfig.title_id, p+1);
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

        if (strstr(file, "pspemu/PSP/SAVEDATA/") &&
           (strcmp(p+1, "PARAM.SFO") == 0 ||
            strcmp(p+1, "SCEVMC0.VMP") == 0 ||
            strcmp(p+1, "SCEVMC1.VMP") == 0)) {
          snprintf(new_file, sizeof(new_file), "%s/PSP/SAVEDATA/%s/%s", getPspemuMemoryStickLocation(), popsconfig.title_id, p+1);
          file = new_file;
        }
      }
  }
  return TAI_CONTINUE(int, sceIoGetstatRef, file, stat);
}

int module_start(SceSize argc, const void *args) {
    tai_module_info_t info;
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

    if (module_nid == 0x2714F07D) { // 3.60 retail
        titleIdHook = taiHookFunctionOffset(&ScePspemuGetTitleidRef, info.modid, 0, 0x205FC, 0x1, ScePspemuGetTitleidPatched);
    }
    else if (module_nid == 0x3F75D4D3) { // 3.65-3.70 retail
        titleIdHook = taiHookFunctionOffset(&ScePspemuGetTitleidRef, info.modid, 0, 0x20600, 0x1, ScePspemuGetTitleidPatched);
    }

    #if 0
    uint32_t movs_a4_1_nop_opcode = 0xBF002301;
    uint32_t movs_a1_0_nop_opcode = 0xBF002000;
    uint32_t movs_a1_1_nop_opcode = 0xBF002001;
    uint32_t uids[64]; int n_uids = 0;
    
        // Resume stuff. PROBABLY SHOULD DO POPS AND PSP MODE STUFF
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x42F0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown. Mode 4, 5
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x572E, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Set cache address for pops stuff
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x57C0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Read savedata and menu info. Should be enabled, otherwise an error will occur
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x5BBA, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Get app state for pops
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x5C52, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x5E4A, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    ///////////////////////////

    // isPops patches

    // Peripheral

    // Use vibration
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x169F6, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for POPS mode
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16AEC, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0x80010089
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16B6C, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16B86, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16C3E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    ////////////////////

    // Init ScePspemuMenuWork
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x1825E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Read savedata and menu info
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x2121E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // POPS Settings menu function
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x17B32, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));
    #endif

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

  if(sceIoOpenHook >= 0) taiHookRelease(sceIoOpenHook, sceIoOpenRef);
  if (sceIoStatHook >= 0) taiHookRelease(sceIoStatHook, sceIoGetstatRef);
  if (titleIdHook >= 0) taiHookRelease(titleIdHook, ScePspemuGetTitleidRef);
  if (io_patch_path) taiInjectRelease(io_patch_path);
  if (io_patch_size) taiInjectRelease(io_patch_size);

  return SCE_KERNEL_STOP_SUCCESS;
}
