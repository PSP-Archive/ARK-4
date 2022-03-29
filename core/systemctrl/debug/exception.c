/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psppower.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include "imports.h"
#include "exception.h"
#include "graphics.h"
#include "macros.h"

extern ARKConfig* ark_config;

// Exception Handler
PspDebugErrorHandler curr_handler = NULL;

// Register Snapshot
PspDebugRegBlock * exception_regs = NULL;

// ASM Exception Handler Payload
extern void _pspDebugExceptionHandler(void);

// Bluescreen Register Snapshot
static PspDebugRegBlock cpuRegs;

// Exception Code String Literals
static const char * codeTxt[32] =
{
    "Interrupt", "TLB modification", "TLB load/inst fetch", "TLB store",
    "Address load/inst fetch", "Address store", "Bus error (instr)",
    "Bus error (data)", "Syscall", "Breakpoint", "Reserved instruction",
    "Coprocessor unusable", "Arithmetic overflow", "Unknown 14",
    "Unknown 15", "Unknown 16", "Unknown 17", "Unknown 18", "Unknown 19",
    "Unknown 20", "Unknown 21", "Unknown 22", "Unknown 23", "Unknown 24",
    "Unknown 25", "Unknown 26", "Unknown 27", "Unknown 28", "Unknown 29",
    "Unknown 31"
};

// Register String Literals
static const unsigned char regName[32][5] =
{
    "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

// Bluescreen Exception Handler
static void ARKExceptionHandler(PspDebugRegBlock * regs)
{
    initScreen(DisplaySetFrameBuf);
    colorDebug(0xFF0000); // Blue Screen of Death
    PRTSTR("Exception caught!");
    PRTSTR1("Exception - %s", codeTxt[(regs->cause >> 2) & 31]);
    PRTSTR1("EPC       - %p", regs->epc);
    PRTSTR1("Cause     - %p", regs->cause);
    PRTSTR1("Status    - %p", regs->status);
    PRTSTR1("BadVAddr  - %p", regs->badvaddr);
    for(int i = 0; i < 32; i+=4){
        PRTSTR8("%s:%p %s:%p %s:%p %s:%p", regName[i], regs->r[i],
                regName[i+1], regs->r[i+1], regName[i+2], regs->r[i+2], regName[i+3], regs->r[i+3]);
    }
    u32 epc = regs->epc;
    {
        SceModule2* mod = sceKernelFindModuleByAddress(epc);
        if (mod != NULL){
            PRTSTR1("Instruction at EPC: %p", _lw(epc)); // TODO: disassemble instruction
            PRTSTR2("From Module: %s @ %p", mod->modname, epc-mod->text_addr);
        }
    }
    u32 ra = regs->r[31];
    {        
        SceModule2* mod = sceKernelFindModuleByAddress(ra);
        if (mod != NULL){
            PRTSTR1("Instruction at RA:  %p", _lw(ra)); // TODO: disassemble instruction
            PRTSTR2("From Module: %s @ %p", mod->modname, ra-mod->text_addr);
        }
    }
    
    // present user menu for recovery options
    int (*CtrlPeekBufferPositive)(SceCtrlData *, int) = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x3A622550);
    if (CtrlPeekBufferPositive){
        PRTSTR("Press cross to soft reset");
        PRTSTR("Press circle to launch recovery");
        PRTSTR("Press square to hard reset");
        PRTSTR("Press triangle to shudown");
    }

    // await user input
    SceCtrlData data;
    memset(&data, 0, sizeof(data));
    while (1){
        if (CtrlPeekBufferPositive){
            CtrlPeekBufferPositive(&data, 1);
            if((data.Buttons & PSP_CTRL_CROSS) == PSP_CTRL_CROSS){
                sctrlKernelExitVSH(NULL);
            }
            else if((data.Buttons & PSP_CTRL_CIRCLE) == PSP_CTRL_CIRCLE){
                extern void exitLauncher();
                ark_config->recovery = 1;
                exitLauncher();
            }
            else if((data.Buttons & PSP_CTRL_SQUARE) == PSP_CTRL_SQUARE){
                void (*ColdReset)(int) = sctrlHENFindFunction("scePower_Service", "scePower", 0x0442D852);
                if (ColdReset) ColdReset(0);
            }
            else if((data.Buttons & PSP_CTRL_TRIANGLE) == PSP_CTRL_TRIANGLE){
                void (*Shutdown)() = sctrlHENFindFunction("scePower_Service", "scePower", 0x2B7C7CF4);
                if (Shutdown) Shutdown();
            }
        }
    };
}

// Register Exception Handler
void registerExceptionHandler(PspDebugErrorHandler handler, PspDebugRegBlock * regs)
{
    // Valid Arguments
    if(handler != NULL && regs != NULL)
    {
        // Save Arguments
        curr_handler = handler;
        exception_regs = regs;
    }
    
    // Register default screen Handler
    else
    {
        // Save Arguments
        curr_handler = &ARKExceptionHandler;
        exception_regs = &cpuRegs;
    }
    
    // Register Exception Handler
    sceKernelRegisterDefaultExceptionHandler((void *)_pspDebugExceptionHandler);
}
