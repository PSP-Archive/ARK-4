#include "main.h"
#include <functions.h>

#define BUF_SIZE 1024*32

extern ARKConfig* ark_config;

u8 seed[0x100];
// sigcheck keys
u8 check_keys0[0x10] = {
    0x71, 0xF6, 0xA8, 0x31, 0x1E, 0xE0, 0xFF, 0x1E,
    0x50, 0xBA, 0x6C, 0xD2, 0x98, 0x2D, 0xD6, 0x2D
};

u8 check_keys1[0x10] = {
    0xAA, 0x85, 0x4D, 0xB0, 0xFF, 0xCA, 0x47, 0xEB,
    0x38, 0x7F, 0xD7, 0xE4, 0x3D, 0x62, 0xB0, 0x10
};
u8* bigbuf = NULL;

int (*BufferCopyWithRange)() = NULL;
int (*IdStorageReadLeaf)(u16, u8*) = NULL;
int (*KernelGetUserLevel)();

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

static int Decrypt(u32 *buf, int size)
{
 buf[0] = 5;
 buf[1] = buf[2] = 0;
 buf[3] = 0x100;
 buf[4] = size;

 if (BufferCopyWithRange((u8*)buf, size+0x14, (u8*)buf, size+0x14, 8) != 0)
  return -1;
 
 return 0;
}

int pspUnsignCheck(u8 *buf)
{
 u8 enc[0xD0+0x14];
 int iXOR, res;

 memcpy(enc+0x14, buf+0x80, 0xD0);

 for (iXOR = 0; iXOR < 0xD0; iXOR++)
 {
  enc[iXOR+0x14] ^= check_keys1[iXOR&0xF]; 
 }

 if ((res = Decrypt((u32 *)enc, 0xD0)) < 0)
 {
  return res;
 }

 for (iXOR = 0; iXOR < 0xD0; iXOR++)
 {
  enc[iXOR] ^= check_keys0[iXOR&0xF];
 }

 memcpy(buf+0x80, enc+0x40, 0x90);
 memcpy(buf+0x110, enc, 0x40);

 return 0;
}

void copyFile(char* path, char* destination){

    SceUID src = k_tbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);
    SceUID dst = k_tbl->KernelIOOpen(destination, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    int len = strlen(path);
    int read;

    if (strcmp(&path[len-4], ".prx") == 0){
        size_t fsize = k_tbl->KernelIOLSeek(src, 0, PSP_SEEK_END);
        k_tbl->KernelIOLSeek(src, 0, PSP_SEEK_SET);
        read = k_tbl->KernelIORead(src, bigbuf, fsize);
        pspUnsignCheck(bigbuf);
        k_tbl->KernelIOWrite(dst, bigbuf, read);
    }
    else{
        do {
            read = k_tbl->KernelIORead(src, bigbuf, BUF_SIZE);
            k_tbl->KernelIOWrite(dst, bigbuf, read);
        } while (read > 0);
    }

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

// Set User Level
int sctrlKernelSetUserLevel(int level)
{
    
    // Backup User Level
    int previouslevel = KernelGetUserLevel();
    
    
    u32 _sceKernelReleaseThreadEventHandler = FindFunction("sceThreadManager", "ThreadManForKernel", 0x72F3C145);
    
    u32 threadman_userlevel_struct = _lh(_sceKernelReleaseThreadEventHandler + 0x4)<<16;
    threadman_userlevel_struct += (short)_lh(_sceKernelReleaseThreadEventHandler + 0x18);
    
    
    // Set User Level
    _sw((level ^ 8) << 28, *(unsigned int *)(threadman_userlevel_struct) + 0x14);
    
    // Flush Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    
    // Return previous User Level
    return previouslevel;
}

int dcIdStorageReadLeaf(u16 leafid, u8 *buf)
{
    int level = sctrlKernelSetUserLevel(8);

    int res = IdStorageReadLeaf(leafid, buf);

    sctrlKernelSetUserLevel(level);

    return res;
}

void dump_idStorage(){
    static u8 buf[512];
    int fd = k_tbl->KernelIOOpen("ms0:/idStorage.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    
    for (int i=0; i<0x140; i++){
        dcIdStorageReadLeaf(i, buf);
        k_tbl->KernelIOWrite(fd, buf, 512);
    }
    k_tbl->KernelIOClose(fd);
}

int kthread(SceSize args, void *argp){

    PRTSTR("Dumping idStorage");
    dump_idStorage();

    open_flash();

    PRTSTR("Dumping flash0");
    copy_folder_recursive("flash0:/", "ms0:/flash0");

    PRTSTR("Flash0 Dumped");
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

    KernelGetUserLevel = FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665);
    IdStorageReadLeaf = FindFunction("sceIdStorage_Service", "sceIdStorage_driver", 0xEB00C509);
    BufferCopyWithRange = FindFunction("sceMemlmd", "semaphore", 0x4C537C72);

    if (BufferCopyWithRange == NULL){
        PRTSTR("ERROR: cannot find import BufferCopyWithRange");
        return;
    }

    if (IdStorageReadLeaf == NULL){
        PRTSTR("ERROR: cannot find import IdStorageReadLeaf");
        return;
    }

    if (KernelGetUserLevel == NULL){
        PRTSTR("ERROR: cannot find import KernelGetUserLevel");
        return;
    }

    memcpy(seed, (void*)0xBFC00200, 0x100);

    int memid = k_tbl->KernelAllocPartitionMemory(2, "ark heap", PSP_SMEM_Low, 1024*1024, NULL);
    bigbuf = k_tbl->KernelGetBlockHeadAddr(memid);

    if((u32)bigbuf & 63) // align 64
            bigbuf = (void*)(((u32)bigbuf & (~63)) + 64);

    initKernelThread();

    k_tbl->KernelFreePartitionMemory(memid);

    PRTSTR("Done");
    
    return;
}