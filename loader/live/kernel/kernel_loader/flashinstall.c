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

#include <macros.h>
#include <globals.h>
#include "functions.h"

#define BUF_SIZE 16*1024

#define PRTSTR11(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11) {if (prtstr) prtstr(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11);}
#define PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) PRTSTR11(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, 0)
#define PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, x9) PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, 0)
#define PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, x8) PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, 0)
#define PRTSTR7(text, x1, x2, x3, x4, x5, x6, x7) PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, 0)
#define PRTSTR6(text, x1, x2, x3, x4, x5, x6) PRTSTR7(text, x1, x2, x3, x4, x5, x6, 0)
#define PRTSTR5(text, x1, x2, x3, x4, x5) PRTSTR6(text, x1, x2, x3, x4, x5, 0)
#define PRTSTR4(text, x1, x2, x3, x4) PRTSTR5(text, x1, x2, x3, x4, 0)
#define PRTSTR3(text, x1, x2, x3) PRTSTR4(text, x1, x2, x3, 0)
#define PRTSTR2(text, x1, x2) PRTSTR3(text, x1, x2, 0)
#define PRTSTR1(text, x1) PRTSTR2(text, x1, 0)
#define PRTSTR(text) PRTSTR1(text, 0)


static inline void open_flash(){
    while(k_tbl->IoUnassign("flash0:") < 0) {
        k_tbl->KernelDelayThread(500000);
    }
    while (k_tbl->IoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
        k_tbl->KernelDelayThread(500000);
    }
}

static int dummyFilter(char* filename){
    return 0;
}

void extractFlash0Archive(SceSize args, void** argp){

    void (*prtstr)(const char* A, unsigned long B, unsigned long C, unsigned long D, unsigned long E, unsigned long F, unsigned long G, unsigned long H, unsigned long I, unsigned long J, unsigned long K, unsigned long L) = NULL;
    int (*filter)(char*) = NULL;

    int nargs = args/sizeof(void*);
    if (nargs!=3) return;
    char* archive = argp[0];
    filter = argp[1];
    prtstr = argp[2];
    
    if (filter == NULL) filter = &dummyFilter;

    char* dest_path = "flash0:/";
    unsigned char buf[BUF_SIZE];
    int path_len = strlen(dest_path);
    static char filepath[ARK_PATH_SIZE];
    static char filename[ARK_PATH_SIZE];
    strcpy(filepath, dest_path);

    PRTSTR1("Extracting %s", archive);
    
    int fdr = k_tbl->KernelIOOpen(archive, PSP_O_RDONLY, 0777);
    
    if (fdr>=0){
        open_flash();
        int filecount;
        k_tbl->KernelIORead(fdr, &filecount, sizeof(filecount));
        PRTSTR1("Processing %d files", filecount);
        for (int i=0; i<filecount; i++){
            filepath[path_len] = '\0';
            int filesize;
            k_tbl->KernelIORead(fdr, &filesize, sizeof(filesize));

            char namelen;
            k_tbl->KernelIORead(fdr, &namelen, sizeof(namelen));

            k_tbl->KernelIORead(fdr, filename, namelen);
            filename[namelen] = '\0';
            
            if (filter(filename)){ // check if file is not needed on PSP
                k_tbl->KernelIOLSeek(fdr, filesize, 1); // skip file
            }
            else{
                strcat(filepath, (filename[0]=='/')?filename+1:filename);
                PRTSTR1("Extracting file %s", filepath);
                int fdw = k_tbl->KernelIOOpen(filepath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
                if (fdw < 0){
                    PRTSTR("ERROR: could not open file for writing");
                    k_tbl->KernelIOClose(fdr);
                    while(1){};
                    return;
                }
                while (filesize>0){
                    int read = k_tbl->KernelIORead(fdr, buf, (filesize>BUF_SIZE)?BUF_SIZE:filesize);
                    k_tbl->KernelIOWrite(fdw, buf, read);
                    filesize -= read;
                }
                k_tbl->KernelIOClose(fdw);
            }
        }
        k_tbl->KernelIOClose(fdr);
    }
    else{
        PRTSTR("Nothing to be done");
    }
    PRTSTR("Done");
    k_tbl->KernelExitThread(0);
}
