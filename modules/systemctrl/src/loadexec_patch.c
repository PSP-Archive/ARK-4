#include "loadexec_patch.h"
#include <functions.h>

void patchLoadExecCommon(SceModule2* loadexec, u32 LoadReboot){

	u32 addr = 0;
	u32 topaddr = loadexec->text_addr+loadexec->text_size;

	u32 rebootcall = JAL(loadexec->text_addr);
	u32 GetUserLevelJump = JUMP(FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665));
	int k1_patches = 0;

	for (addr=loadexec->text_addr; addr<topaddr; addr+=4){
		u32 data = _lw(addr);
		if (data == 0x3C018860)
			_sb(0xFC, addr); // Patch Reboot Buffer Loader
		else if (data == rebootcall)
			_sw(JAL(LoadReboot), addr); // Patch Reboot Buffer Entry Point
		else if (data == GetUserLevelJump){
			_sw(JR_RA, addr);  // patch sceKernelGetUserLevel stub to make it return 4
			_sw(LI_V0(4), addr+4);
		}
		else if ((data & 0xFFFF0000) == 0x07600000 && k1_patches < 3){
			_sh(0x1000, addr+2); // patch k1 check
			k1_patches++;
		}
	}
}
