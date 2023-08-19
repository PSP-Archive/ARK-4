#include "kirk.h"
#include "cpu.h"

typedef struct
{
    volatile unsigned int signature;
    volatile unsigned int version;
    volatile unsigned int error;
    volatile unsigned int proc_phase;
    volatile unsigned int command;
    volatile unsigned int result;
    volatile unsigned int unk_18;
    volatile unsigned int status;
    volatile unsigned int status_async;
    volatile unsigned int status_async_end;
    volatile unsigned int status_end;
    volatile unsigned int src_addr;
    volatile unsigned int dst_addr;
} PspKirkRegs;

#define MAKE_PHYS_ADDR(_addr)    (((unsigned int)_addr) & 0x1FFFFFFF)
#define SYNC()                   __asm ("sync")
#define KIRK_HW_REGISTER_ADDR    ((PspKirkRegs *)0xBDE00000)
 
 void kirk_hwreset(void) 
{
    sysreg_reset_enable_kirk();
    sysreg_busclk_enable_kirk();
    sysreg_busclk_disable_kirk();
    sysreg_reset_disable_kirk();
    sysreg_busclk_enable_kirk();
}

int kirk1(void *dst, const void *src)
{
    cpu_dcache_wb_inv_all();
    PspKirkRegs *const kirk = KIRK_HW_REGISTER_ADDR;
 
    kirk->command = 1; // decrypt operation
    kirk->src_addr = MAKE_PHYS_ADDR(src);
    kirk->dst_addr = MAKE_PHYS_ADDR(dst);
 
    SYNC();
    kirk->proc_phase = 1; // start processing
 
    while((kirk->status & 0x11) == 0); // wait until processing complete
 
    if (kirk->status & 0x10) // error occured
    {
        kirk->proc_phase = 2;
 
        while((kirk->status & 2) == 0);
 
        kirk->status_end = kirk->status;
        SYNC();
        return -1;
    }
 
    kirk->status_end = kirk->status;
    SYNC();
    return kirk->result;
}

int kirk4(void *dst, const void *src)
{
    cpu_dcache_wb_inv_all();
    PspKirkRegs *const kirk = KIRK_HW_REGISTER_ADDR;
 
    kirk->command = 4; // encrypt operation
    kirk->src_addr = MAKE_PHYS_ADDR(src);
    kirk->dst_addr = MAKE_PHYS_ADDR(dst);
 
    SYNC();
    kirk->proc_phase = 1; // start processing
 
    while((kirk->status & 0x11) == 0); // wait until processing complete
 
    if (kirk->status & 0x10) // error occured
    {
        kirk->proc_phase = 2;
 
        while((kirk->status & 2) == 0);
 
        kirk->status_end = kirk->status;
        SYNC();
        return -1;
    }
 
    kirk->status_end = kirk->status;
    SYNC();
    return kirk->result;
}

int kirk7(void *dst, const void *src)
{
    cpu_dcache_wb_inv_all();
    PspKirkRegs *const kirk = KIRK_HW_REGISTER_ADDR;

    kirk->command = 7; // decryption
    kirk->src_addr = MAKE_PHYS_ADDR(dst);
    kirk->dst_addr = MAKE_PHYS_ADDR(dst);

    // begin processing
    kirk->proc_phase = 1;
    SYNC();

    // wait until we advance from the initial phase
    while ((kirk->proc_phase & 1) != 0);

    // wait until status is set
    while (kirk->status == 0);

    kirk->status_end = kirk->status & kirk->status_async;
    SYNC();
    return kirk->result;
}

int kirkE(void *dst)
{
    cpu_dcache_wb_inv_all();
    PspKirkRegs *const kirk = KIRK_HW_REGISTER_ADDR;

    kirk->command = 0xE; // RNG
    kirk->dst_addr = MAKE_PHYS_ADDR(dst);

    // begin processing
    kirk->proc_phase = 1;
    SYNC();

    // wait until we advance from the initial phase
    while ((kirk->proc_phase & 1) != 0);

    // wait until status is set
    while (kirk->status == 0);

    kirk->status_end = kirk->status & kirk->status_async;
    SYNC();
    return kirk->result;
}

int kirkF(void *dst)
{
    cpu_dcache_wb_inv_all();
    PspKirkRegs *const kirk = KIRK_HW_REGISTER_ADDR;

    kirk->command = 0xF; // initialisation?
    kirk->src_addr = MAKE_PHYS_ADDR(dst);
    kirk->dst_addr = MAKE_PHYS_ADDR(dst);

    // begin processing
    kirk->proc_phase = 1;
    SYNC();

    // wait until we advance from the initial phase
    while ((kirk->proc_phase & 1) != 0);

    // wait until status is set
    while (kirk->status == 0);

    kirk->status_end = kirk->status & kirk->status_async;
    SYNC();
    return kirk->result;
}
