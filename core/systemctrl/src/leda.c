#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <pspopenpsid.h>
#include <psputilsforkernel.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ark.h>
#include <macros.h>
#include <systemctrl.h>


int leda_running = 0;
SceUID (* KernelLoadModuleMs2_hook)() = NULL;
SceUID (* KernelLoadModuleMs2_orig)() = NULL;
static int execute_apitype = 0x141;

SceUID sceKernelLoadModuleMs2_bridge(const char *path, int flags, SceKernelLMOption *option)
{
    SceUID ret = KernelLoadModuleMs2_orig(execute_apitype, path , flags , option);
    leda_running = 0; // re-enable checkexec in modman
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

// patch leda
void patchLedaPlugin(void* handler){
    
    SceModule2* init = sceKernelFindModuleByName("sceInit");

    // register handler
    KernelLoadModuleMs2_hook = handler;

    // patch leda
    u32 text_addr = ((u32)handler) - 0xCE8;

    // Remove version check
    _sw(0, text_addr + 0xC58);

    // Fix sceKernelLoadModuleMs2 call
    _sw(JUMP(sceKernelLoadModuleMs2_bridge), text_addr + 0x2E28);
    _sw(NOP, text_addr + 0x2E28 + 4);

    // Remove patch of sceKernelGetUserLevel on sceLFatFs_Driver
    _sw(NOP, text_addr + 0x1140);

    // patch init sceKernelLoadModuleMs2
    KernelLoadModuleMs2_orig = sctrlHENFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x7BD53193);
    hookImportByNID(init, "ModuleMgrForKernel", 0x7BD53193, sceKernelLoadModuleMs2_patched);

    // register handler for custom fixes to legacy games
    leda_previous = sctrlHENSetStartModuleHandler( LedaModulePatch );

    leda_running = 1; // disable checkexec in modman
    
    flushCache();
}
