#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>
#include <macros.h>
#include <ark.h>
#include <systemctrl_private.h>
#include "modulemanager.h"

// Module Start Handler
void (* g_module_start_handler)(SceModule2 *) = NULL;

// Prologue Module Function
int (* prologue_module)(void *, SceModule2 *) = NULL;

// Prologue Module Hook
int prologue_module_hook(void * unk0, SceModule2 * mod)
{
    // Forward Call
    int result = prologue_module(unk0, mod);
    
    // Load Success
    if(result >= 0)
    {
        // Module Start Handler
        if(g_module_start_handler != NULL)
        {
            // Call Module Start Handler
            g_module_start_handler(mod);
        }
    }
    
    // Return Result
    return result;
}

void patchModuleManager()
{
    SceModule2 *mod = sceKernelFindModuleByName("sceModuleManager");

    /* bne t4, zero, +43 -> beq zero, zero, +43 : 
        Force always to take the size of the data.psp instead of
        the size of the PBP to avoid the error 0x80020001 */
    _sw(0x1000002A, mod->text_addr + 0x3f28);	

    prologue_module = (int (*)(void *, SceModule2 *))(mod->text_addr + 0x3b34);

    MAKE_CALL(mod->text_addr + 0x2d90, prologue_module_hook);
}