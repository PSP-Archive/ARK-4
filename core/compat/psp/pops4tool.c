#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <ark.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <functions.h>

 typedef struct SceStubLibrary {
     u32 unk0; //0
     struct SceStubLibrary *next; //4
     const char *libName; //8
     u8 version[2]; //12
     u16 attribute; //14
     u8 stubEntryTableLen; //16
     u8 vStubCount; //17
     u16 stubCount; //18
     u32 *nidTable; //20
     void *stubTable; //24
     void *vStubTable; //28
     u16 unk32; //32
     void *libStubTable; //36
     u32 status; //40
     u32 isUserLib; //44
     char *libName2; //48
     u32 libNameInHeap; //52
 } SceStubLibrary; //size = 56

static s32 (*aLinkLibEntries)(SceStubLibrary *stubLib);
static s32 myLinkLibEntries(SceStubLibrary *stubLib){

    int res = aLinkLibEntries(stubLib);
    if (res < 0){
        // this should not get executed in normal circumstances
        // we do this for toolkits to load just enough of pops to trigger popsloader
        if (sceKernelFindModuleByName("pops") != NULL && sceKernelFindModuleByName("popscore") == NULL){
            return 0;
        }
    }

    return res;
}

void patchPops4Tool(){
    SceModule2* mod = sceKernelFindModuleByName("sceLoaderCore");
    u32 start_addr = mod->text_addr;
    u32 topaddr = mod->text_addr+mod->text_size;
    
    for (u32 addr = start_addr; addr<topaddr; addr+=4){
        if (_lw(addr) == 0x3452013C){
            HIJACK_FUNCTION(addr-44, myLinkLibEntries, aLinkLibEntries);
            break;
        }
    }
}