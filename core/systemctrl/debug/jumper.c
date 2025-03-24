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
#include <pspthreadman_kernel.h>
#include <systemctrl.h>
#include <macros.h>
#include <module2.h>

#if DEBUG >= 2

// Log info
int printk(char *fmt, ...)__attribute__((format (printf, 1, 2)));

// JAL ASM Stub Size
#define JUMPER_STUB_SIZE 17

// JAL Table Size
#define JUMPER_MAX_COUNT 2048

// JAL Table Size (for User Calls)
unsigned int JUMPER_MAX_COUNT_USER = 0;

// JAL Hook Count
unsigned int hook_count = 0;

// JAL Hook Count (for User Calls)
unsigned int hook_count_user = 0;

// Kernel Address & Capacity Check
#define KERNEL_OKAY(address) (address & 0x80000000 && hook_count < JUMPER_MAX_COUNT)

// User Address & Capacity Check
#define USER_OKAY(address) (address & 0x8000000 && hook_count_user < JUMPER_MAX_COUNT_USER)

// Jumper ASM Table Buffer
unsigned int jumper_table_kernel[JUMPER_STUB_SIZE * JUMPER_MAX_COUNT];

// Jumper ASM Table Buffer (for User Calls)
unsigned int  * jumper_table_user = NULL;

// dumpJAL System Call Number
int dumpJALSyscallNumber = -1;

// Log Function
void dumpJAL(unsigned int target, unsigned int ra, unsigned int result)
{
    // Find Caller
    SceModule2 * caller = (SceModule2 *)sceKernelFindModuleByAddress(ra);
    
    // Find Callee
    SceModule2 * callee = (SceModule2 *)sceKernelFindModuleByAddress(target);
    
    // Craft Caller Information
    const char * caller_name = "UNK";
    unsigned int caller_textaddr = 0;
    if(caller)
    {
        caller_name = caller->modname;
        caller_textaddr = caller->text_addr;
    }
    
    // Craft Callee Information
    const char * callee_name = "UNK";
    unsigned int callee_textaddr = 0;
    if(callee)
    {
        callee_name = callee->modname;
        callee_textaddr = callee->text_addr;
    }
    
    // Log JAL
    printk("TRACE %08X: %s_%08X to %s_%08X ret = %08X\r\n", sceKernelGetThreadId(), caller_name, ra - caller_textaddr - 8, callee_name, target - callee_textaddr, result);
}

// Install Single JAL Trace
void installJALTrace(unsigned int address)
{
    // User Initialization
    if(jumper_table_user == NULL && (address & 0x8000000))
    {
        // Fetch dumpJAL System Call Number
        dumpJALSyscallNumber = sceKernelQuerySystemCall(dumpJAL);
        
        // Volatile Memory Mode
        if(0)
        {
        	// Required Function Stub
        	int (* scePowerVolatileMemLock)(int, void **, int *) = (void *)sctrlHENFindFunction("scePower_Service", "scePower", 0x23C31FFE);
        	
        	// Volatile Memory Address
        	void ** volmemory = (void **)USER_BASE;
        	
        	// Volatile Memory Size
        	int * volsize = (int *)(USER_BASE + 4);
        	
        	// Reduce Permission Level (to look like User System Call)
        	unsigned int k1 = pspSdkSetK1(0x100000);
        	
        	// Lock Volatile Memory for User Access
        	int result = scePowerVolatileMemLock(0, volmemory, volsize);
        	
        	// Restore Permission Level
        	pspSdkSetK1(k1);
        	
        	// Lock Success
        	if(result >= 0)
        	{
        		// Save Pointer
        		jumper_table_user = *volmemory;
        	
        		// Calculate Maximum Stub Count
        		JUMPER_MAX_COUNT_USER = *volsize / (JUMPER_STUB_SIZE * sizeof(unsigned int));
        	}
        } else if (1)
        {
        	enum { VPL_POOL_SIZE = 64 * 1024, };
        	SceUID vpl = sceKernelCreateVpl("jumperVPL", 2, 0, VPL_POOL_SIZE + 2 * 1024, NULL);

        	if(vpl >= 0)
        	{
        		sceKernelAllocateVpl(vpl, VPL_POOL_SIZE, (void*)&jumper_table_user, NULL);

        		if(jumper_table_user != NULL)
        		{
        			JUMPER_MAX_COUNT_USER = VPL_POOL_SIZE / (JUMPER_STUB_SIZE * sizeof(unsigned int));
        		}
        		else
        		{
        			printk("vpl alloc failed\r\n");
        		}
        	}
        }
        
        // Scratchpad Mode
        else
        {
        	// Save Pointer
        	jumper_table_user = (void *)USER_BASE;
        	
        	// Set Maximum Stub Count (Scratchpad can only fit 240)
        	JUMPER_MAX_COUNT_USER = 240;
        }
    }
    
    // Valid Address & Capacity
    if(KERNEL_OKAY(address) || USER_OKAY(address))
    {
        // Fetch ASM Instruction
        unsigned int inst = *(unsigned int *)(address);
        
        // JAL Instruction
        if(IS_JAL(inst))
        {
        	// JAL Target
        	unsigned int target = 0;
        	
        	// JAL ASM Stub
        	unsigned int * jumper = NULL;
        	
        	// Kernel Address
        	if(KERNEL_OKAY(address))
        	{
        		// Get JAL Target
        		target = KERNELIFY(JUMP_TARGET(inst));
        		
        		// Get JAL ASM Stub
        		jumper = &jumper_table_kernel[hook_count * JUMPER_STUB_SIZE];
        	}
        	
        	// User Address
        	else
        	{
        		// Get JAL Target
        		target = JUMP_TARGET(inst);
        		
        		// Get JAL ASM Stub
        		jumper = &jumper_table_user[hook_count_user * JUMPER_STUB_SIZE];
        	}
        	
        	// Allocate 12 Bytes on Stack
        	jumper[0] = 0x27BDFFF4; //addiu $sp, $sp, -12
        	
        	// Backup $ra (can't place in delay slot, would corrupt register)
        	jumper[1] = 0xAFBF0000; //sw $ra, 0($sp)
        	
        	// Execute JAL
        	jumper[2] = inst; //jal target
        	
        	// Delay Slot - NOP
        	jumper[3] = 0; //nop
        	
        	// Backup JAL Result
        	jumper[4] = 0xAFA20004; //sw $v0, 4($sp)
        	jumper[5] = 0xAFA30008; //sw $v1, 8($sp)
        	
        	// Prepare Log Argument #1 : Target Address
        	jumper[6] = 0x3C040000 | (target >> 16); //lui $a0, (target >> 16)
        	jumper[7] = 0x34840000 | (target & 0xFFFF); //ori $a0, $a0, (target & 0xFFFF)
        	
        	// Prepare Log Argument #2 : $ra
        	jumper[8] = 0x8FA50000; //lw $a1, 0($sp)
        	
        	// Kernel Thread
        	if(KERNEL_OKAY(address))
        	{
        		// Call Log Function
        		jumper[9] = JAL(dumpJAL); //jal dumpJAL
        		
        		// Delay Slot - Prepare Log Argument #3 : JAL Result (32bit trimmed)
        		jumper[10] = 0x00403021; //move $a2, $v0
        	}
        	
        	// User Thread
        	else
        	{
        		// Prepare Log Argument #3 : JAL Result (32bit trimmed)
        		jumper[9] = 0x00403021;
        		
        		// Call Log Function
        		jumper[10] = SYSCALL(dumpJALSyscallNumber);
        	}
        	
        	// Restore $ra
        	jumper[11] = 0x8FBF0000; //lw $ra, 0($sp)
        	
        	// Restore Function Result
        	jumper[12] = 0x8FA20004; //lw $v0, 4($sp)
        	jumper[13] = 0x8FA30008; //lw $v1, 8($sp)
        	
        	// Return to Caller
        	jumper[14] = 0x03E00008; //jr $ra
        	
        	// Delay Slot - Free 12 Bytes from Stack
        	jumper[15] = 0x27BD000C; //addiu $sp, $sp, 12
        	
        	// JAL Instruction Address (for undoing of logger hook)
        	jumper[16] = address;
        	
        	// Overwrite JAL and Link to Jumper
        	_sw(JAL(jumper), address);
        	
        	// Flush Cache
        	flushCache();
        	
        	// Increase Kernel Jumper Count
        	if(KERNEL_OKAY(address)) hook_count++;
        	
        	// Increase User Jumper Count
        	else hook_count_user++;
        }
    }
    
    // Output Capacity Warning
    else
    {
        // Capacity Warning Flood Prevention Variable
        static int flood_prevent = 0;
        
        // First Capacity Warning
        if(!flood_prevent)
        {
        	// Enable Flood Prevention
        	flood_prevent = 1;
        	
        	// Output Capacity Warning
        	printk("Trace Capacity exhausted at %08X\n", address);
        }
    }
}

// Install Memory Region JAL Trace
void installMemoryJALTrace(unsigned int start, unsigned int size)
{
    // Iterate Instructions
    unsigned int pos = start; for(; pos < start + size; pos += 4)
    {
        // Attempt to install Jumper
        installJALTrace(pos);
    }
}

// Install Whole-Module JAL Trace (NOT STABLE! DON'T DO IT IF NOT DESPERATE!)
void installModuleJALTrace(SceModule2 * module)
{
    // Valid Argument
    if(module)
    {
        // Install Memory Region JAL Trace
        installMemoryJALTrace(module->text_addr, module->text_size);
    }
}

// Undo Trace Hack
void clearTraceTable(void)
{
    // Restore JAL Instructions
    unsigned int i = 0; for(; i < hook_count; i++)
    {
        // Fetch JAL ASM Stub
        unsigned int * jumper = &jumper_table_kernel[i * JUMPER_STUB_SIZE];

        // Restore JAL
        _sw(jumper[2], jumper[16]);
    }
    
    // Restore JAL Instructions (User)
    for(i = 0; i < hook_count_user; i++)
    {
        // Fetch JAL ASM Stub
        unsigned int * jumper = &jumper_table_user[i * JUMPER_STUB_SIZE];

        // Restore JAL
        _sw(jumper[2], jumper[16]);
    }
    
    // Reset Hook Counter
    hook_count = 0;
    hook_count_user = 0;
}

#else
void installJALTrace(unsigned int address){}
void installMemoryJALTrace(unsigned int start, unsigned int size){}
void installModuleJALTrace(SceModule2 * module){}
void dumpJAL(unsigned int target, unsigned int ra, unsigned int result){}
#endif