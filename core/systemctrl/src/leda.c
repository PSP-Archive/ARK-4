#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <pspopenpsid.h>
#include <psputilsforkernel.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "macros.h"
#include "systemctrl.h"

SceUID (* KernelLoadModuleMs2_hook)() = NULL;
SceUID (* KernelLoadModuleMs2_orig)() = NULL;
static int execute_apitype = 0x141;

SceUID sceKernelLoadModuleMs2_bridge(const char *path, int flags, SceKernelLMOption *option)
{
    SceUID ret = KernelLoadModuleMs2_orig(execute_apitype, path , flags , option);
    #ifdef DEBUG
    if (ret == 0x80020148) colorDebug(0xff0000);
    #endif
    return ret;
}

SceUID sceKernelLoadModuleMs2_patched(int apitype, const char *path, int flags, SceKernelLMOption *option)
{    
    execute_apitype = apitype;
    SceUID ret = KernelLoadModuleMs2_hook(path , flags , option);
    return ret;
}

int (*lzrc)(void *outbuf, u32 outcapacity, void *inbuf, void *unk) = NULL;
int get_addr(void *outbuf, u32 outcapacity, void *inbuf, void *unk)
{
    int k1 = pspSdkSetK1(0);
    if( !lzrc )
    {
        u32 *mod = (u32 *)sceKernelFindModuleByName("sceNp9660_driver");        
        if (!mod)
        {
            SceUID modload = sceKernelLoadModule("flash0:/kd/np9660.prx", 0, 0);
            mod = (u32 *)sceKernelFindModuleByUID(modload);
        }
        u32 *code = (u32 *)mod[27];
        
        int i;
        for (i = 0; i < 0x8000; i++) 
        {        
            if (code[i] == 0x27bdf4f0 && code[i+20] == 0x98C90001 ) 
            {    
                lzrc = (void *)&code[i];
            }
        }
    }

    pspSdkSetK1(k1);
    return lzrc( outbuf,outcapacity, inbuf, unk);
}

STMOD_HANDLER leda_previous = NULL;
void LedaModulePatch(SceModule2 *mod)
{
//    u32 text_addr = mod->text_addr;
    char *modinfo=mod->modname;
 
    if (strncmp(modinfo, "Resurssiklunssi", sizeof("Resurssiklunssi")-1 ) == 0) 
    {       
        MAKE_DUMMY_FUNCTION_RETURN_0(0x889007A8);
        _sw((u32)get_addr, 0x8891C300);
        flushCache();
    }
   
    if( leda_previous ) leda_previous( mod );
}

void patchLedaPlugin(void* handler){
    // register handler
    KernelLoadModuleMs2_hook = handler;
    
    SceModule2* init = sceKernelFindModuleByName("sceInit");
    SceModule2* leda = sceKernelFindModuleByAddress(handler);

    // patch leda
    //u32 text_addr = ((u32)handler) - 0xCE8;

    // Remove version check
    //_sw(0, text_addr + 0xC58);
    //_sw(0, text_addr + 0x00000D50);
    //_sw(0, text_addr + 0x00000D64);

    // Remove patch of sceKernelGetUserLevel on sceLFatFs_Driver
    //_sw(0, text_addr + 0x1140);

    for (u32 a=leda->text_addr; a<leda->text_addr+leda->text_size; a+=4){
        u32 d = _lw(a);
        if (d&0x0000FFFF == 0xFE00){
            _sw(NOP, FindFirstBEQ(a)); // Remove version check
            break;
        }
    }

    // Fix sceKernelLoadModuleMs2 call
    //_sw(JUMP(sceKernelLoadModuleMs2_bridge), text_addr + 0x2E28);
    //_sw(NOP, text_addr + 0x2E28 + 4);
    hookImportByNID(leda, "ModuleMgrForKernel", 0x49C5B9E1, sceKernelLoadModuleMs2_bridge);

    // patch init sceKernelLoadModuleMs2
    KernelLoadModuleMs2_orig = sctrlHENFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x7BD53193);
    hookImportByNID(init, "ModuleMgrForKernel", 0x7BD53193, sceKernelLoadModuleMs2_patched);

    // register handler for custom fixes to legacy games
    leda_previous = sctrlHENSetStartModuleHandler( LedaModulePatch );
    
    flushCache();
}
