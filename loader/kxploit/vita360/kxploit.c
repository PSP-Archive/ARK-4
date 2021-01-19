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
This file is a dummy for the 3.60 PBOOT.PBP bubble loader.
*/


int is_exploited = 0;
FunctionTable* tbl;

static void my_func(){
    is_exploited = 1;
}

int stubScanner(FunctionTable* arg0){
    tbl = arg0;
    return 0;
}

void repairInstruction(void){
    _sw(0x3C038801, 0x88010040);
    _sw(0x8C654D84, 0x88010044);
}

int doExploit(void){
    is_exploited = 0;
    return 0;
}

void executeKernel(u32 kfuncaddr){
    tbl->KernelLibcTime(KERNELIFY(kfuncaddr));
}
