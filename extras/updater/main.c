#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <kubridge.h>
#include "globals.h"

#include <tmctrl/tmctrl.h>
#include <msipl/mainbinex/payload.h>
#include <installer/tm_msipl.h>
#include <installer/tm_mloader.h>

#include <installer/pspbtcnf_dc.h>
#include <installer/pspbtcnf_02g_dc.h>
#include <installer/dcman.h>
#include <installer/ipl_update.h>
#include <installer/iop.h>
#include <installer/pspdecryptmod.h>
#include <installer/intrafont.h>
#include <installer/resurrection.h>
#include <installer/vlf.h>

PSP_MODULE_INFO("ARKUpdater", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4096);

#define BUF_SIZE 16*1024
#define KERNELIFY(a) ((u32)a|0x80000000)
#define DEFAULT_THEME_SIZE 614050

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
    {IDSREG_PRX, IDSREG_PRX_FLASH},
    {XMBCTRL_PRX, XMBCTRL_PRX_FLASH},
    {USBDEV_PRX, USBDEV_PRX_FLASH},
    {VSH_MENU, VSH_MENU_FLASH},
    {RECOVERY_PRX, RECOVERY_PRX_FLASH},
    {UPDATER_FILE, UPDATER_FILE_FLASH},
};
static const int N_FLASH_FILES = (sizeof(flash_files)/sizeof(flash_files[0]));

static int isVitaFile(char* filename){
    return (strstr(filename, "psv")!=NULL // PS Vita btcnf replacement, not used on PSP
            || strstr(filename, "660")!=NULL // PSP 6.60 modules can be used on Vita, not needed for PSP
            || strstr(filename, "vita")!=NULL // Vita modules
    );
}

void checkArkConfig(ARKConfig* ark_config){
    // check if ARK is using SEPLUGINS folder due to lack of savedata folder
    SceUID fd = -1;
    char path[ARK_PATH_SIZE]; strcpy(path, ark_config->arkpath); path[strlen(path)-1] = 0; // remove trailing '/' or else sceIoGetstat won't work
    if (strcmp(ark_config->arkpath, SEPLUGINS_MS0) == 0 || (fd = sceIoDopen(path)) < 0){
        // create savedata folder, first attempt on ef0 for PSP Go
        strcpy(ark_config->arkpath, SAVEDATA_EF0 DEFAULT_ARK_FOLDER); // ef0:/PSP/SAVEDATA/ARK_01234
        sceIoMkdir(ark_config->arkpath, 0777);
        if ((fd = sceIoDopen(ark_config->arkpath)) < 0){
            // second attempt on ms0 for every other device
            ark_config->arkpath[0] = 'm';
            ark_config->arkpath[1] = 's';
            sceIoMkdir(ark_config->arkpath, 0777);
            fd = sceIoDopen(ark_config->arkpath);
        }
        // creation worked?
        if (fd >= 0){
            // notify SystemControl of the new arkpath
            struct KernelCallArg args;
            args.arg1 = ark_config;
            u32 setArkConfig = sctrlHENFindFunction("SystemControl", "SystemCtrlPrivate", 0x6EAFC03D);    
            kuKernelCall((void*)setArkConfig, &args);

            // move settings file to arkpath
            static char* orig = SEPLUGINS_MS0 ARK_SETTINGS; // ms0:/SEPLUGINS/SETTINGS.TXT
            static char* dest = DEFAULT_ARK_PATH ARK_SETTINGS; // ms0:/PSP/SAVEDATA/ARK_01234/SETTINGS.TXT
            dest[0] = ark_config->arkpath[0];
            dest[1] = ark_config->arkpath[1];
            copy_file(orig, dest);
            sceIoRemove(orig);

            strcat(ark_config->arkpath, "/");
        }
    }
    sceIoDclose(fd);
}

// Entry Point
int main(int argc, char * argv[])
{

    ARKConfig ark_config;

    sctrlHENGetArkConfig(&ark_config);
    
    // Initialize Screen Output
    pspDebugScreenInit();

    if (ark_config.magic != ARK_CONFIG_MAGIC){
        pspDebugScreenPrintf("ERROR: not running ARK, exiting in 5 seconds...\n");
        sceKernelDelayThread(5000000);
        sceKernelExitGame();
    }

    checkArkConfig(&ark_config);

    pspDebugScreenPrintf("ARK Updater Started\n");

    u32 my_ver = (ARK_MAJOR_VERSION << 24) | (ARK_MINOR_VERSION << 16) | (ARK_MICRO_VERSION << 8) | ARK_REVISION;
    u32 cur_ver = sctrlHENGetVersion(); // ARK's full version number
    u32 major = (cur_ver&0xFF000000)>>24;
    u32 minor = (cur_ver&0xFF0000)>>16;
    u32 micro = (cur_ver&0xFF00)>>8;
    u32 rev   = sctrlHENGetMinorVersion();

    pspDebugScreenPrintf("Current Version %d.%d.%.2i r%d\n", major, minor, micro, rev);
    pspDebugScreenPrintf("Update Version %d.%d.%.2i r%d\n", ARK_MAJOR_VERSION, ARK_MINOR_VERSION, ARK_MICRO_VERSION, ARK_REVISION);

    if (my_ver < cur_ver){
        pspDebugScreenPrintf("WARNING: downgrading to lower version\n");
        pspDebugScreenPrintf("Press X to continue to Downgrade, or press any button to QUIT!\n");
		SceCtrlData pad;
		while(1){
			sceCtrlPeekBufferPositive(&pad, 1);
			if(pad.Buttons & PSP_CTRL_CROSS) break;
			else if(pad.Buttons & (PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE|PSP_CTRL_SQUARE)) sctrlKernelExitVSH(NULL);

    	}
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
        strcat(flash0_ark, FLASH0_ARK);
        pspDebugScreenPrintf("Extracting %s\n", flash0_ark);
        open_flash();
        extractArchive(sceIoOpen(flash0_ark, PSP_O_RDONLY, 0777), "flash0:/", &isVitaFile);

        // test for full installations
        SceIoStat stat; int res = sceIoGetstat(flash_files[0].dest, &stat);
        if (res >= 0){
            for (int i=0; i<N_FLASH_FILES; i++){
                char path[ARK_PATH_SIZE];
                strcpy(path, ark_config.arkpath);
                strcat(path, flash_files[i].orig);
                pspDebugScreenPrintf("Copying %s to %s\n", flash_files[i].orig, flash_files[i].dest);
                copy_file(path, flash_files[i].dest);
            }
        }

        struct {
            char* path;
            void* buf;
            size_t size;
        } dc_files[] = {
            { ARK_DC_PATH "/tmctrl.prx", tmctrl, size_tmctrl },
            { ARK_DC_PATH "/payload_01g.bin", ms_ipl_payload, size_ms_ipl_payload },
            { ARK_DC_PATH "/payload_02g.bin", ms_ipl_payload, size_ms_ipl_payload },
            { ARK_DC_PATH "/tm_mloader.bin", tm_mloader, size_tm_mloader },
            { ARK_DC_PATH "/kd/pspbtcnf_dc.bin", pspbtcnf_dc, size_pspbtcnf_dc },
            { ARK_DC_PATH "/kd/pspbtcnf_02g_dc.bin", pspbtcnf_02g_dc, size_pspbtcnf_02g_dc },
            { ARK_DC_PATH "/kd/dcman.prx", dcman, size_dcman },
            { ARK_DC_PATH "/kd/ipl_update.prx", ipl_update, size_ipl_update },
            { ARK_DC_PATH "/kd/iop.prx", iop, size_iop },
            { ARK_DC_PATH "/kd/pspdecrypt.prx", pspdecrypt, size_pspdecrypt },
            { ARK_DC_PATH "/vsh/module/intrafont.prx", intrafont, size_intrafont },
            { ARK_DC_PATH "/vsh/module/resurrection.prx", resurrection, size_resurrection },
            { ARK_DC_PATH "/vsh/module/vlf.prx", vlf, size_vlf },
        };

        const int N_DC_FILES = (sizeof(dc_files)/sizeof(dc_files[0]));

        // test for dc installation
        if (strncmp(ark_config.arkpath, ARK_DC_PATH, sizeof(ARK_DC_PATH)-1) == 0){
            for (int i=0; i<N_DC_FILES; i++){
                pspDebugScreenPrintf("Installing %s\n", dc_files[i].path);
                int fdw = sceIoOpen(dc_files[i].path, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
                sceIoWrite(fdw, dc_files[i].buf, dc_files[i].size);
                sceIoClose(fdw);
            }
        }
    }

    // delete updater
    sceIoClose(my_fd);
    sceIoRemove(eboot_path);
    char* c = strrchr(eboot_path, '/');
    *c = 0;
    sceIoRmdir(eboot_path);

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
    while(sceIoUnassign("flash1:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", 0, NULL, 0)<0){
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

			else if(strstr(filename, ARK_THEME_FILE) != NULL) {
					int size;
					SceUID theme;

					char fullpath[ARK_PATH_SIZE+10];

					strcpy(fullpath, filepath);
					strcat(fullpath, filename);

					theme = sceIoOpen(fullpath, PSP_O_RDONLY, 0777);

					if (theme < 0) {
						goto _else;
					}

					else{

						size = sceIoLseek32(theme, sizeof(theme), PSP_SEEK_END);
						sceIoClose(theme);
					}


					if (size != DEFAULT_THEME_SIZE) { // Size of default THEME (hack for now, md5/sha was not working)
                		sceIoLseek32(fdr, filesize, PSP_SEEK_CUR); // skip file
					}
					else {
						goto _else;
					}
					
					
			}

            else{
				_else:
                strcat(filepath, (filename[0]=='/')?filename+1:filename);
                pspDebugScreenPrintf("Extracting file %s\n", filepath);
                int fdw = sceIoOpen(filepath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
                if (fdw < 0){
                    pspDebugScreenPrintf("ERROR: could not open file for writing\n");
					char *slash = strrchr(filepath, '/');
					int length = slash-filepath-9;
					char *res = (char*)malloc(length+1);
					strncpy(res, filepath, length);
					res[length] = '\0';
                    pspDebugScreenPrintf("ERROR: Do you have ARK_01234 directory in %s ?\n", res);
					free(res);
                    sceIoClose(fdr);
					SceCtrlData pad;
                    pspDebugScreenPrintf("\nPress any key to exit...\n");
                    while(1){
						sceCtrlPeekBufferPositive(&pad, 1);
						if(pad.Buttons) sctrlKernelExitVSH(NULL);
					}
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
