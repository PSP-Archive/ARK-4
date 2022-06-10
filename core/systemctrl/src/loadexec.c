#include "loadexec_patch.h"
#include <functions.h>

void patchLoadExec(SceModule2 *loadexec, u32 LoadReboot, u32 GetUserLevel,
                   int k1_patches) {
  u32 addr = 0;
  u32 text_addr = loadexec->text_addr;
  u32 topaddr = text_addr + loadexec->text_size;

  u32 rebootcall = JAL(text_addr);
  u32 GetUserLevelJump = JUMP(GetUserLevel);

  int patches = 3 + k1_patches;
  for (addr = text_addr; addr < topaddr && patches; addr += 4) {
    u32 data = _lw(addr);
    if (data == 0x3C018860) {
      _sb(0xFC, addr); // Patch Reboot Buffer Entry Point
      patches--;
    } else if (data == rebootcall) {
      _sw(JAL(LoadReboot), addr); // Patch Reboot Buffer Loader
      patches--;
    } else if (data == GetUserLevelJump) {
      _sw(JR_RA, addr); // patch sceKernelGetUserLevel stub to make it return 4
      _sw(LI_V0(4), addr + 4);
      patches--;
    } else if ((data & 0xFFFF0000) == 0x07600000 && k1_patches) {
      _sh(0x1000, addr + 2); // patch k1 check
      k1_patches--;
      patches--;
    }
  }
}
