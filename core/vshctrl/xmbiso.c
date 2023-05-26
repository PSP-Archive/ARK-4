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

#include "xmbiso.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "isoreader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include "virtual_pbp.h"
#include "strsafe.h"
#include "dirent_track.h"
#include "macros.h"
//#include "ansi_c_functions.h"

#define MAGIC_DFD_FOR_DELETE 0x9000
#define MAGIC_DFD_FOR_DELETE_2 0x9001

extern u32 psp_model;
extern SEConfig* se_config;
static char g_iso_dir[128];
static char g_temp_delete_dir[128];
static int g_delete_eboot_injected = 0;

static const char *game_list[] = {
	"ms0:/PSP/GAME/"		,"ef0:/PSP/GAME/"		,
};

static int CorruptIconPatch(char *name)
{
	char path[256];
	SceIoStat stat;


    for (int i=0; i<NELEMS(game_list); i++){

        const char *hidden_path = game_list[i];
        strcpy(path, hidden_path);
        strcat(path, name);
        strcat(path, "%/EBOOT.PBP");

        memset(&stat, 0, sizeof(stat));
        if (sceIoGetstat(path, &stat) >= 0)
        {
            strcpy(name, "__SCE"); // hide icon
            return 1;
        }
    }

	return 0;
}

static int HideDlc(char *name) {
	char path[256];
    SceIoStat stat;

    for (int i=0; i<NELEMS(game_list); i++){
        const char *hidden_path = game_list[i];
        sprintf(path, "%s%s/PARAM.PBP", hidden_path, name);
        memset(&stat, 0, sizeof(stat));
        if (sceIoGetstat(path, &stat) >= 0) {
            sprintf(path, "%s%s/EBOOT.PBP", hidden_path, name);

            memset(&stat, 0, sizeof(stat));
            if (sceIoGetstat(path, &stat) < 0) {
                strcpy(name, "__SCE"); // hide icon
                return 1;
            }
        }
    }
	return 0;
}

static int is_iso_dir(const char *path)
{
    const char *p;

    if (path == NULL)
        return 0;

    p = strchr(path, '/');

    if (p == NULL)
        return 0;

    if (p <= path + 1 || p[-1] != ':')
        return 0;

    p = strstr(p, ISO_ID);

    if (NULL == p) {
        return 0;
    }

    p = strrchr(path, '@') + 1;
    p += 8;

    while(*p != '\0' && *p == '/')
        p++;

    if (*p != '\0')
        return 0;

    return 1;
}

static int is_iso_eboot(const char* path)
{
    const char *p;

    if (path == NULL)
        return 0;

    p = strchr(path, '/');

    if (p == NULL)
        return 0;

    if (p <= path + 1 || p[-1] != ':')
        return 0;

    p = strstr(p, ISO_ID);

    if (NULL == p) {
        return 0;
    }

    p = strrchr(path, '@') + 1;
    p += 8;

    if (0 != strcmp(p, "/EBOOT.PBP"))
        return 0;

    return 1;
}

static inline int is_game_dir(const char *dirname)
{
    const char *p;
    char path[256];
    SceIoStat stat;

    p = strchr(dirname, '/');

    if (p == NULL) {
        return 0;
    }

    if (0 != strnicmp(p, "/PSP/GAME", sizeof("/PSP/GAME")-1)) {
        return 0;
    }

    if (0 == strnicmp(p, "/PSP/GAME/_DEL_", sizeof("/PSP/GAME/_DEL_")-1)) {
        return 0;
    }

    STRCPY_S(path, dirname);
    STRCAT_S(path, "/EBOOT.PBP");

    if(0 == sceIoGetstat(path, &stat)) {
        return 0;
    }

    STRCPY_S(path, dirname);
    STRCAT_S(path, "/PARAM.PBP");

    if(0 == sceIoGetstat(path, &stat)) {
        return 0;
    }

    return 1;
}

//open directory
SceUID gamedopen(const char * dirname)
{
    SceUID result;
    u32 k1;

    if(is_iso_dir(dirname)) {
        result = MAGIC_DFD_FOR_DELETE;
        g_delete_eboot_injected = 0;
        strncpy(g_iso_dir, dirname, sizeof(g_iso_dir));
        g_iso_dir[sizeof(g_iso_dir)-1] = '\0';
        return result;
    }

    if(0 == strcmp(dirname, g_temp_delete_dir)) {
        result = MAGIC_DFD_FOR_DELETE_2;
        return result;
    }
   
    result = sceIoDopen(dirname);
    
    if(is_game_dir(dirname)) {
        char path[256];
        const char *p;
        int iso_dfd, ret;
        
        get_device_name(path, sizeof(path), dirname);
        STRCAT_S(path, "/ISO");

        p = strstr(dirname, "/PSP/GAME");

        if(p != NULL) {
            p += sizeof("/PSP/GAME") - 1;
            STRCAT_S(path, p);
        }

        k1 = pspSdkSetK1(0);
        iso_dfd = vpbp_dopen(path);
        pspSdkSetK1(k1);

        if(iso_dfd < 0) {
            goto exit;
        }

        if(result < 0) {
            result = iso_dfd;
        }
        
        ret = dirent_add(result, iso_dfd, dirname); 

        if(ret < 0) {
            #ifdef DEBUG
            printk("%s: dirent_add -> %d\n", __func__, ret);
            #endif
            result = -1;
            goto exit;
        }
    }

exit:
    #ifdef DEBUG
    printk("%s: %s -> 0x%08X\n", __func__, dirname, result);
    #endif
    return result;
}

//read directory
int gamedread(SceUID fd, SceIoDirent * dir)
{
    int result;
    u32 k1;

    if(fd == MAGIC_DFD_FOR_DELETE || fd == MAGIC_DFD_FOR_DELETE_2) {
        if (0 == g_delete_eboot_injected) {
            u32 k1;
           
            memset(dir, 0, sizeof(*dir));
            k1 = pspSdkSetK1(0);
            result = vpbp_getstat(g_iso_dir, &dir->d_stat);
            pspSdkSetK1(k1);

            if(fd == MAGIC_DFD_FOR_DELETE) {
                strcpy(dir->d_name, "EBOOT.PBP");
            } else {
                strcpy(dir->d_name, "_EBOOT.PBP");
            }

            g_delete_eboot_injected = 1;
            result = 1;
        } else {
            result = 0;
        }
        
        
        return result;
    }

    result = sceIoDread(fd, dir);

    if(result <= 0) {
        struct IoDirentEntry *entry;

        entry = dirent_search(fd);

        if(entry != NULL) {
            k1 = pspSdkSetK1(0);
            result = vpbp_dread(fd, dir);
            pspSdkSetK1(k1);
        }
    }
    else {
        int k1 = pspSdkSetK1(0);
        CorruptIconPatch(dir->d_name);
        if (se_config->hidedlc)
    		HideDlc(dir->d_name);
        pspSdkSetK1(k1);
    }
    #ifdef DEBUG
    printk("%s: 0x%08X %s -> 0x%08X\n", __func__, fd, dir->d_name, result);
    #endif
    return result;
}

//directory descriptor closer
int gamedclose(SceUID fd)
{
    int result;
    u32 k1;
    struct IoDirentEntry *entry;
   
    if(fd == MAGIC_DFD_FOR_DELETE || fd == MAGIC_DFD_FOR_DELETE_2) {
        result = 0;
        g_delete_eboot_injected = 0;
        
        return result;
    }
    
    entry = dirent_search(fd);

    if(entry != NULL) {
        if(entry->iso_dfd == fd) {
            k1 = pspSdkSetK1(0);
            vpbp_dclose(fd);
            pspSdkSetK1(k1);

            result = 0;
        } else if (entry->dfd == fd) {
            k1 = pspSdkSetK1(0);
            result = vpbp_dclose(fd);
            pspSdkSetK1(k1);
        } else {
            result = sceIoDclose(fd);
        }

        dirent_remove(entry);
    } else {
        result = sceIoDclose(fd);
    }
    #ifdef DEBUG
    printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, result);
    #endif
    return result;
}

//open file
SceUID gameopen(const char * file, int flags, SceMode mode)
{
    //forward to firmware
    SceUID result;
   
    if (is_iso_eboot(file)) {
        u32 k1 = pspSdkSetK1(0);
        result = vpbp_open(file, flags, mode);
        pspSdkSetK1(k1);
    } else {
        result = sceIoOpen(file, flags, mode);
    }

    return result;
}

//read file
int gameread(SceUID fd, void * data, SceSize size)
{
    //forward to firmware
    int result;
   
    if (vpbp_is_fd(fd)) {
        u32 k1 = pspSdkSetK1(0);
        result = vpbp_read(fd, data, size);
        pspSdkSetK1(k1);
    } else {
        result = sceIoRead(fd, data, size);
    }

    return result;
}

//close file
int gameclose(SceUID fd)
{
    int result;

    if (vpbp_is_fd(fd)) {
        u32 k1 = pspSdkSetK1(0);
        result = vpbp_close(fd);
        pspSdkSetK1(k1);
    } else {
        result = sceIoClose(fd);
    }
    
    return result;
}

SceOff gamelseek(SceUID fd, SceOff offset, int whence)
{
    SceOff result = 0;

    if (vpbp_is_fd(fd)) {
        u32 k1 = pspSdkSetK1(0);
        result = vpbp_lseek(fd, offset, whence);
        pspSdkSetK1(k1);
    } else {
        result = sceIoLseek(fd, offset, whence);
    }

    return result;
}

//get file status
int gamegetstat(const char * file, SceIoStat * stat)
{
    int result;
   
    //virtual iso eboot detected
    if (is_iso_eboot(file)) {
        u32 k1 = pspSdkSetK1(0);
        result = vpbp_getstat(file, stat);
        pspSdkSetK1(k1);
    } else {
        result = sceIoGetstat(file, stat);
    }

    return result;
}

//remove file
int gameremove(const char * file)
{
    int result;
   
    if(g_temp_delete_dir[0] != '\0' && 
            0 == strncmp(file, g_temp_delete_dir, strlen(g_temp_delete_dir))) {
        result = 0;
        #ifdef DEBUG
        printk("%s:<virtual> %s -> 0x%08X\n", __func__, file, result);
        #endif
        return result;
    }
    
    result = sceIoRemove(file);
    #ifdef DEBUG
    printk("%s: %s -> 0x%08X\n", __func__, file, result);
    #endif
    return result;
}

char game150_delete[256];

void recursiveFolderDelete(char* path){
    //try to open directory
    SceUID d = sceIoDopen(path);
    
    if(d >= 0)
    {
        SceIoDirent entry;
        memset(&entry, 0, sizeof(SceIoDirent));
        
        //allocate memory to store the full file paths
        char new_path[256];

        //start reading directory entries
        while(sceIoDread(d, &entry) > 0)
        {
            //skip . and .. entries
            if (!strcmp(".", entry.d_name) || !strcmp("..", entry.d_name)) 
            {
                memset(&entry, 0, sizeof(SceIoDirent));
                continue;
            };
            
            //build new file path
            strcpy(new_path, path);
            strcat(new_path, entry.d_name);

            if (FIO_SO_ISDIR(entry.d_stat.st_attr)){
                strcat(new_path, "/");
                recursiveFolderDelete(new_path);
            }
            else{
                sceIoRemove(new_path);
            }
            
        };
        sceIoDclose(d); //close directory
        int len = strlen(path);
        if (path[len-1] == '/') path[len-1] = 0;
        sceIoRmdir(path); //delete empty folder
    };
}

//remove folder
int gamermdir(const char * path)
{
    int result;
   
    if(0 == strcmp(path, g_temp_delete_dir)) {
        strcat(g_iso_dir, "/EBOOT.PBP");
        u32 k1 = pspSdkSetK1(0);
        result = vpbp_remove(g_iso_dir);
        pspSdkSetK1(k1);
        #ifdef DEBUG
        printk("%s:<virtual> %s -> 0x%08X\n", __func__, path, result);
        #endif
        g_iso_dir[0] = '\0';
        g_temp_delete_dir[0] = '\0';

        return result;
    }
    if (game150_delete[0]) {
        int k1 = pspSdkSetK1(0);
        recursiveFolderDelete(game150_delete);
        pspSdkSetK1(k1);
        game150_delete[0] = 0;
    }
    result = sceIoRmdir(path);
    #ifdef DEBUG
    printk("%s: %s 0x%08X\n", __func__, path, result);
    #endif
    return result;
}

//load and execute file
int gameloadexec(char * file, struct SceKernelLoadExecVSHParam * param)
{
    //result
    int result = -1;

    //virtual iso eboot detected
    if (is_iso_eboot(file)) {
        u32 k1 = pspSdkSetK1(0);
        result = vpbp_loadexec(file, param);
        pspSdkSetK1(k1);
        return result;
    }

    // fix 1.50 homebrew
    char *perc = strchr(param->argp, '%');
    if (perc) {
        strcpy(perc, perc + 1);
        file = param->argp;
    }

    // homebrew boot
    u32 k1 = pspSdkSetK1(0);
    result = sceKernelLoadExecVSHMs2(file, param);
    pspSdkSetK1(k1);
    return result;
}

int gamerename(const char *oldname, const char *newfile)
{
    int result;

    if(is_iso_dir(oldname)) {
        result = 0;
        strncpy(g_iso_dir, oldname, sizeof(g_iso_dir));
        g_iso_dir[sizeof(g_iso_dir)-1] = '\0';
        strncpy(g_temp_delete_dir, newfile, sizeof(g_temp_delete_dir));
        g_temp_delete_dir[sizeof(g_temp_delete_dir)-1] = '\0';
        #ifdef DEBUG
        printk("%s:<virtual> %s %s -> 0x%08X\n", __func__, oldname, newfile, result);
        #endif
        return 0;
    }

    if(g_temp_delete_dir[0] != '\0' &&
            0 == strncmp(oldname, g_temp_delete_dir, strlen(g_temp_delete_dir))) {
        result = 0;
        #ifdef DEBUG
        printk("%s:<virtual2> %s %s -> 0x%08X\n", __func__, oldname, newfile, result);
        #endif
        return 0;
    }

    char* perc = strchr(oldname, '%');
    if (perc && strstr(newfile, "_DEL_")){
        memset(game150_delete, 0, sizeof(game150_delete));
        strncpy(game150_delete, oldname, perc-oldname);
        strcat(game150_delete, "/");
    }

    result = sceIoRename(oldname, newfile);
    #ifdef DEBUG
    printk("%s: %s %s -> 0x%08X\n", __func__, oldname, newfile, result);
    #endif
    return result;
}

int gamechstat(const char *file, SceIoStat *stat, int bits)
{
    int result;

    if(g_temp_delete_dir[0] != '\0' && 
            0 == strncmp(file, g_temp_delete_dir, strlen(g_temp_delete_dir))) {
        result = 0;
        #ifdef DEBUG
        printk("%s:<virtual> %s -> 0x%08X\n", __func__, file, result);
        #endif
        return 0;
    }

    result = sceIoChstat(file, stat, bits);
    #ifdef DEBUG
    printk("%s: %s -> 0x%08X\n", __func__, file, result);
    #endif
    return result;
}
