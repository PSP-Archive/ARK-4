#include <pspsdk.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <ark.h> 
#include "rebootex.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "interruptman.h"
#include "cryptography.h"
#include "syspatch.h"
#include "sysmem.h"
#include "exception.h"

#ifndef DEBUG
// dummy debug exports
void colorDebug(u32 c){}
void initScreen(){}
void PRTSTR11(){}
int printkCached(char *fmt, ...){return 0;}
int printk(char *fmt, ...){return 0;}
int printkInit(const char *output){return 0;}
int printkSync(void){return 0;}
void installJALTrace(unsigned int address){}
void installMemoryJALTrace(unsigned int start, unsigned int size){}
void installModuleJALTrace(SceModule2 * module){}
void dumpJAL(unsigned int target, unsigned int ra, unsigned int result){}
#endif
