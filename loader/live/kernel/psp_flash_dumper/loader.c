#include "main.h"
#include <functions.h>

#define BUF_SIZE 1024*4

extern ARKConfig* ark_config;

static inline void open_flash(){
    while(k_tbl->IoUnassign("flash0:") < 0) {
        k_tbl->KernelDelayThread(500000);
    }
    while (k_tbl->IoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
        k_tbl->KernelDelayThread(500000);
    }
}

int fileExists(const char* path){
    int fp = k_tbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);
    if (fp < 0)
        return 0;
    k_tbl->KernelIOClose(fp);
    return 1;
}

int folderExists(const char* path){
    int fp = k_tbl->KernelIODopen(path);
    if (fp < 0)
        return 0;
    k_tbl->KernelIODclose(fp);
    return 1;
}

void copyFile(char* path, char* destination){

    SceUID src = k_tbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);
    SceUID dst = k_tbl->KernelIOOpen(destination, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    int read;
    static u8 buffer[BUF_SIZE];
    
    do {
        read = k_tbl->KernelIORead(src, buffer, BUF_SIZE);
        k_tbl->KernelIOWrite(dst, buffer, read);
    } while (read > 0);

    k_tbl->KernelIOClose(src);
    k_tbl->KernelIOClose(dst);
}

int copy_folder_recursive(const char * source, const char * destination)
{

    //create new folder
    k_tbl->KernelIOMkdir(destination, 0777);
    
    int src_len = strlen(source);
    int dst_len = strlen(destination);

    char new_destination[255];
    strcpy(new_destination, destination);
    if (new_destination[dst_len-1] != '/'){
        new_destination[dst_len] = '/';
        new_destination[dst_len+1] = 0;
    }
    
    char new_source[255];
    strcpy(new_source, source);
    if (new_source[src_len-1] != '/'){
        new_source[src_len] = '/';
        new_source[src_len+1] = 0;
    }

    //try to open source folder
    SceUID dir = k_tbl->KernelIODopen(source);
    
    if(dir >= 0)
    {
        SceIoDirent entry;
        memset(&entry, 0, sizeof(SceIoDirent));
        
        //start reading directory entries
        while(k_tbl->KernelIODread(dir, &entry) > 0)
        {
            //skip . and .. entries
            if (!strcmp(".", entry.d_name) || !strcmp("..", entry.d_name)) 
            {
                memset(&entry, 0, sizeof(SceIoDirent));
                continue;
            };

            char src[255];
            strcpy(src, new_source);
            strcat(src, entry.d_name);

            char dst[255];
            strcpy(dst, new_destination);
            strcat(dst, entry.d_name);

            if (fileExists(src))
            { //is it a file
                PRTSTR1("Copying file %s", src);
                copyFile(src, dst); //copy file
            }
            else if (folderExists(src))
            {
                //try to copy as a folder
                PRTSTR1("Copying folder %s", src);
                copy_folder_recursive(src, dst);
            }

        };
        //close folder
        k_tbl->KernelIODclose(dir);
    };
    
    return 1;
};

int kthread(SceSize args, void *argp){

    open_flash();

    copy_folder_recursive("flash0:/", "ms0:/flash0");

    PRTSTR("Done");
    k_tbl->KernelExitThread(0);

    return 0;
}

void initKernelThread(){
    SceUID kthreadID = k_tbl->KernelCreateThread( "arkflasher", (void*)KERNELIFY(&kthread), 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    if (kthreadID >= 0){
        // start thread and wait for it to end
        k_tbl->KernelStartThread(kthreadID, 0, NULL);
        k_tbl->waitThreadEnd(kthreadID, NULL);
        k_tbl->KernelDeleteThread(kthreadID);
    }
}

// Kernel Permission Function
void loadKernelArk(){
    PRTSTR("Dumping flash0");
    initKernelThread();
    PRTSTR("Flash0 Dumped");
    return;
}