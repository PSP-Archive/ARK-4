#include <vitasdkkern.h>
#include <taihen.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <psp2/kernel/clib.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/sysmem.h>
#include <sys/types.h>

static tai_hook_ref_t req_ref;
static tai_hook_ref_t alloc_ref;

typedef struct SceCompatAllocCdramWithHoleParam { // size is 8 on FW 3.60
    void *pCachedCdramBase;
    void *pUncachedCdramBase;
} SceCompatAllocCdramWithHoleParam;

static SceCompatAllocCdramWithHoleParam gCdram;


typedef struct {
        uint32_t cmd; //0x0
        SceUID sema_id; //0x4
        uint64_t *response; //0x8
        uint32_t padding; //0xC
        uint64_t args[14]; // 0x10
} SceKermitRequest; //0x80

// this function is in kernel space
int sceCompatWaitAndGetRequest_patched(SceUInt32 req_id, SceUInt32 a2) {
  int fd = TAI_CONTINUE(int, req_ref, req_id, a2);
//  ksceDebugPrintf("sceCompatWaitAndGetRequest(%x, %x): %x\n", req_id, a2, fd);

  // if cached == null - read from uncached, else read from cached
  void* addr = gCdram.pCachedCdramBase;
  if (addr == NULL)
    addr = gCdram.pUncachedCdramBase;


  int32_t pvVar1 = fd;

  uint16_t* puVar3 = 0;


  int32_t uVar2 = (pvVar1 * 2) & 0x1fffffff;

  if (uVar2 + 0xf8000000 < 0x4000000)
  {
    if ((pvVar1 * 2 < 1) || (uVar2 - 0x8400000 < 0x1c00000))
    {
      puVar3 = (uint16_t*)(uVar2 + addr + 0xF8000000);

      SceKermitRequest request;
      ksceKernelCopyFromUser(&request, puVar3, sizeof(SceKermitRequest));
      ksceDebugPrintf("facility: %02x, cmd: 0x%02X\n", req_id, request.cmd);
    }
  }
  else {
    if (uVar2 + 0xfc000000 < 0x800000)
    {
      ksceDebugPrintf("uVar2 + 0xfc000000 < 0x800000\n");

      puVar3 = (uint16_t*)(uVar2 + addr + -0x200000);
    }
    else if (uVar2 - 0x10000 < 0x4000)
    {
      ksceDebugPrintf("uVar2 - 0x10000 < 0x4000\n");
      puVar3 = (uint16_t*)(uVar2 + addr + 0x3cf0000);
    }
    else
    {
        ksceDebugPrintf("AAAAAAAAAAAAAAAAAAAAAA\n");
    }
  }

  return fd;
}


int sceCompatAllocCdramWithHole_patched(SceCompatAllocCdramWithHoleParam *pCdram) {
  int fd = TAI_CONTINUE(int, alloc_ref, pCdram);
  ksceKernelCopyFromUser(&gCdram, pCdram, sizeof(SceCompatAllocCdramWithHoleParam));
  ksceDebugPrintf("sceCompatAllocCdramWithHole: 0x%08x, 0x%08x\n", gCdram.pCachedCdramBase, gCdram.pUncachedCdramBase);
  return fd;
}

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);
int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);

static int (* _ksceKernelGetModuleInfo)(SceUID pid, SceUID modid, SceKernelModuleInfo *info) = NULL;

static int get_module_info_func() {
  int res;

  res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0xC445FA63,
                               0xD269F915, (uintptr_t *)&_ksceKernelGetModuleInfo);
  if (res < 0)
    res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0x92C9FFC2,
                                 0xDAA90093, (uintptr_t *)&_ksceKernelGetModuleInfo);
  return res;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {

  get_module_info_func();

  int res;
  tai_module_info_t tai_info;
  tai_info.size = sizeof(tai_module_info_t);
  res = taiGetModuleInfoForKernel(KERNEL_PID, "SceCompat", &tai_info);

  if (res >= 0)
  {
      int hook = taiHookFunctionExportForKernel(KERNEL_PID,      // Kernel process
                                 &req_ref,       // Output a reference
                                 "SceCompat",  // Name of module being hooked
                                 TAI_ANY_LIBRARY, // If there's multiple libs exporting this
                                 0x8176C238,      // NID
                                 sceCompatWaitAndGetRequest_patched); // Name of the hook function

      ksceDebugPrintf("compat hook: %x\n", hook);

      hook = taiHookFunctionExportForKernel(KERNEL_PID,      // Kernel process
                                 &alloc_ref,       // Output a reference
                                 "SceCompat",  // Name of module being hooked
                                 TAI_ANY_LIBRARY, // If there's multiple libs exporting this
                                 0xA5039FFA,      // NID
                                 sceCompatAllocCdramWithHole_patched); // Name of the hook function

      ksceDebugPrintf("compat hook: %x\n", hook);

  }



    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    return SCE_KERNEL_STOP_SUCCESS;
}
