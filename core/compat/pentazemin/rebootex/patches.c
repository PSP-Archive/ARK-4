#include "rebootex.h"

// patch reboot on ps vita
void patchRebootBuffer(){
    _sw(0x44000000, 0xBC800100);
    colorDebug(0xff);
}
