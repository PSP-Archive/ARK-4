#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "globals.h"

PSP_MODULE_INFO("ARKUpdater", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4096);

#define BUF_SIZE 16*1024
#define KERNELIFY(a) ((u32)a|0x80000000)

typedef struct
{
    u32 magic;
    u32 version;
    u32 param_offset;
    u32 icon0_offset;
    u32 icon1_offset;
    u32 pic0_offset;
    u32 pic1_offset;
    u32 snd0_offset;
    u32 elf_offset;
    u32 psar_offset;
} PBPHeader;

struct {
    char* orig;
    char* dest;
} flash_files[] = {
    {"IDSREG.PRX", "flash0:/kd/ark_idsreg.prx"},
    {"XMBCTRL.PRX", "flash0:/kd/ark_xmbctrl.prx"},
    {"USBDEV.PRX", "flash0:/vsh/module/ark_usbdev.prx"},
    {"VSHMENU.PRX", "flash0:/vsh/module/ark_satelite.prx"}
};
static const int N_FLASH_FILES = (sizeof(flash_files)/sizeof(flash_files[0]));

static int isVitaFile(char* filename){
    return (strstr(filename, "psv")!=NULL // PS Vita btcnf replacement, not used on PSP
            || strstr(filename, "660")!=NULL // PSP 6.60 modules can be used on Vita, not needed for PSP
            || strstr(filename, "vita")!=NULL // Vita modules
            || strcmp(filename, "/fake.cso")==0 // fake.cso used on Vita to simulate UMD drive when no ISO available
    );
}

// Entry Point
int main(int argc, char * argv[])
{

    ARKConfig ark_config;

    sctrlHENGetArkConfig(&ark_config);
    
    // Initialize Screen Output
    pspDebugScreenInit();

    if (ark_config.magic != ARK_CONFIG_MAGIC){
        pspDebugScreenPrintf("ERROR: not running ARK\n");
        while (1){};
    }

    pspDebugScreenPrintf("ARK Updater Started\n");

    u32 my_ver = (ARK_MAJOR_VERSION << 16) | (ARK_MINOR_VERSION << 8) | ARK_MICRO_VERSION;
    u32 cur_ver = sctrlHENGetMinorVersion();
    int major = (cur_ver&0xFF0000)>>16;
	int minor = (cur_ver&0xFF00)>>8;
	int micro = (cur_ver&0xFF);

    pspDebugScreenPrintf("Current Version %d.%d.%.2i\n", major, minor, micro);
    pspDebugScreenPrintf("Update Version %d.%d.%.2i\n", ARK_MAJOR_VERSION, ARK_MINOR_VERSION, ARK_MICRO_VERSION);

    if (my_ver < cur_ver){
        pspDebugScreenPrintf("WARNING: downgrading to lower version\n");
    }

    char* eboot_path = argv[0];

    PBPHeader header;

    int my_fd = sceIoOpen(eboot_path, PSP_O_RDONLY, 0777);

    sceIoRead(my_fd, &header, sizeof(header));

    sceIoLseek32(my_fd, header.psar_offset, PSP_SEEK_SET);

    pspDebugScreenPrintf("Extracting ARK_01234\n");

    // extract ARK_01234
    extractArchive(my_fd, ark_config.arkpath, NULL);

    // extract FLASH0.ARK (PSP only)
    ARKConfig* ac = &ark_config;
    if (IS_PSP(ac)){
        char flash0_ark[ARK_PATH_SIZE];
        strcpy(flash0_ark, ark_config.arkpath);
        strcat(flash0_ark, "FLASH0.ARK");
        pspDebugScreenPrintf("Extracting %s\n", flash0_ark);
        open_flash();
        extractArchive(sceIoOpen(flash0_ark, PSP_O_RDONLY, 0777), "flash0:/", &isVitaFile);

        for (int i=0; i<N_FLASH_FILES; i++){
            char path[ARK_PATH_SIZE];
            strcpy(path, ark_config.arkpath);
            strcat(path, flash_files[i].orig);
            int test = sceIoOpen(flash_files[i].dest, PSP_O_RDONLY, 0777);
            if (test >= 0){
                sceIoClose(test);
                pspDebugScreenPrintf("Installing %s to %s\n", flash_files[i].orig, flash_files[i].dest);
                copy_file(path, flash_files[i].dest);
            }
        }
    }

    // Kill Main Thread
    sceKernelExitGame();

    // Exit Function
    return 0;
}

// Exit Point
int module_stop(SceSize args, void * argp)
{
    // Return Success
    return 0;
}

void open_flash(){
    while(sceIoUnassign("flash0:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
        sceKernelDelayThread(500000);
    }
}

static int dummyFilter(char* filename){
    return 0;
}

void extractArchive(int fdr, char* dest_path, int (*filter)(char*)){
    
    if (filter == NULL) filter = &dummyFilter;

    static unsigned char buf[BUF_SIZE];
    int path_len = strlen(dest_path);
    static char filepath[ARK_PATH_SIZE];
    static char filename[ARK_PATH_SIZE];
    strcpy(filepath, dest_path);
    
    
    if (fdr>=0){
        int filecount;
        sceIoRead(fdr, &filecount, sizeof(filecount));
        pspDebugScreenPrintf("Processing %d files\n", filecount);
        for (int i=0; i<filecount; i++){
            filepath[path_len] = '\0';
            int filesize;
            sceIoRead(fdr, &filesize, sizeof(filesize));

            char namelen;
            sceIoRead(fdr, &namelen, sizeof(namelen));

            sceIoRead(fdr, filename, namelen);
            filename[namelen] = '\0';
            
            if (filter(filename)){ // check if file is not needed on PSP
                sceIoLseek32(fdr, filesize, PSP_SEEK_CUR); // skip file
            }
            else{
                strcat(filepath, (filename[0]=='/')?filename+1:filename);
                pspDebugScreenPrintf("Extracting file %s\n", filepath);
                int fdw = sceIoOpen(filepath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
                if (fdw < 0){
                    pspDebugScreenPrintf("ERROR: could not open file for writing\n");
                    sceIoClose(fdr);
                    while(1){};
                    return;
                }
                while (filesize>0){
                    int read = sceIoRead(fdr, buf, (filesize>BUF_SIZE)?BUF_SIZE:filesize);
                    sceIoWrite(fdw, buf, read);
                    filesize -= read;
                }
                sceIoClose(fdw);
            }
        }
        sceIoClose(fdr);
    }
    else{
        pspDebugScreenPrintf("Nothing to be done\n");
    }
    pspDebugScreenPrintf("Done\n");
}

void copy_file(char* orig, char* dest){
    static u8 buf[BUF_SIZE];
    int fdr = sceIoOpen(orig, PSP_O_RDONLY, 0777);
    int fdw = sceIoOpen(dest, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    while (1){
        int read = sceIoRead(fdr, buf, BUF_SIZE);
        if (read <= 0) break;
        sceIoWrite(fdw, buf, read);
    }
    sceIoClose(fdr);
    sceIoClose(fdw);
}
