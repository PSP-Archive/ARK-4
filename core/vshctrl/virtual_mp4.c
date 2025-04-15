#include <pspctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "systemctrl_private.h"
#include "isoreader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include "strsafe.h"
#include "macros.h"

#include "smallvid.h"

#define FAKE_UID_XMB_VIDEO_ISO FAKE_UID|0xD1

static int file_pos = 0;
static SceUID video_dd = -1;
static SceUID isovideo_fd = -1;
extern SceCtrlData *last_control_data;

static char* video_dir = "ms0:/VIDEO";
static char* isovideo_dir = "ms0:/ISO/VIDEO";

static void launch_umdvideo_mount(const char *path) {
    SceIoStat stat;
    int type;

    if (path == NULL)
        return;

    if (sceIoGetstat(path, &stat) < 0)
        return;

    type = vshDetectDiscType(path);

    if (type < 0)
        return;

    sctrlSESetUmdFile(path);
    sctrlSESetBootConfFileIndex(MODE_VSHUMD);
    sctrlSESetDiscType(type);
    sctrlKernelExitVSH(NULL);
}

SceUID videoIoOpen(const char* file, u32 flags, u32 mode){
    SceUID res = sceIoOpen(file, flags, mode);

    if (res < 0){
        int k1 = pspSdkSetK1(0);
        char isopath[128]; strcpy(isopath, isovideo_dir);
        isopath[0] = file[0]; isopath[1] = file[1];
        char* filename = strrchr(file, '/');
        if (filename){
            strcat(isopath, filename);
            char* ext = strstr(isopath, ".mp4");
            if (ext){
                SceIoStat stat;
                strcpy(ext, ".iso");
                if (sceIoGetstat(isopath, &stat)>=0){
                    res = FAKE_UID_XMB_VIDEO_ISO;
                    file_pos = 0;
                    if (last_control_data){
                        u32 pad = last_control_data->Buttons;
                        if(  (pad&PSP_CTRL_CROSS)  == PSP_CTRL_CROSS
                          || (pad&PSP_CTRL_CIRCLE) == PSP_CTRL_CIRCLE
                          || (pad&PSP_CTRL_START)  == PSP_CTRL_START
                          || (pad&PSP_CTRL_LEFT)   == PSP_CTRL_LEFT
                        ){
                            launch_umdvideo_mount(isopath);
                        }
                    }
                }
            }
        }
        pspSdkSetK1(k1);
    }

    return res;
}

SceUID videoIoDopen(const char* dir){
    SceUID res = sceIoDopen(dir);

    video_dir[0] = dir[0];
    video_dir[1] = dir[1];
    if (strcasecmp(dir, video_dir) == 0) {
        last_control_data = NULL;
        video_dd = res;
        int k1 = pspSdkSetK1(0);
        isovideo_dir[0] = dir[0];
        isovideo_dir[1] = dir[1];
        isovideo_fd = sceIoDopen(isovideo_dir);
        pspSdkSetK1(k1);
    }

    return res;
}

int videoIoGetstat(const char* path, SceIoStat* stat){
    int res = sceIoGetstat(path, stat);

    if (res < 0){
        int k1 = pspSdkSetK1(0);
        char isopath[128]; strcpy(isopath, isovideo_dir);
        isopath[0] = path[0]; isopath[1] = path[1];
        char* filename = strrchr(path, '/');
        if (filename){
            strcat(isopath, filename);
            char* ext = strstr(isopath, ".mp4");
            if (ext){
                strcpy(ext, ".iso");
                res = sceIoGetstat(isopath, stat);
                if (res>=0){
                    stat->st_size = sizeof(smallvid);
                }
            }
        }
        pspSdkSetK1(k1);
    }

    return res;
}

int videoIoRead(SceUID fd, void* buf, u32 size){

    if (fd == FAKE_UID_XMB_VIDEO_ISO){
        memcpy(buf, &smallvid[file_pos], size);
        file_pos += size;
        return size;
    }

    return sceIoRead(fd, buf, size);
}

int videoIoDread(SceUID fd, SceIoDirent *dir){
    int res = sceIoDread(fd, dir);

    if (res == 0){
        int k1 = pspSdkSetK1(0);
        res = sceIoDread(isovideo_fd, dir);
        if (res>0 && dir->d_name[0] != '.'){
            char* ext = strrchr(dir->d_name, '.');
            if (ext) strcpy(ext, ".mp4");
            if (dir->d_private){
                pspMsPrivateDirent *pri_dirent = dir->d_private;
                ext = strrchr(pri_dirent->l_name, '.');
                if (ext) strcpy(ext, ".mp4");
                ext = strrchr(pri_dirent->s_name, '.');
                if (ext) strcpy(ext, ".MP4");
            }
        }
        pspSdkSetK1(k1);
    }

    return res;
}

int videoIoClose(SceUID fd){
    if (fd == FAKE_UID_XMB_VIDEO_ISO) return 0;
    return sceIoClose(fd);
}

int videoIoDclose(SceUID fd){
    int res = sceIoDclose(fd);
    if (fd == video_dd){
        int k1 = pspSdkSetK1(0);
        sceIoDclose(isovideo_fd);
        pspSdkSetK1(k1);
        video_dd = -1;
        isovideo_fd = -1;
    }
    return res;
}

SceOff videoIoLseek(SceUID fd, SceOff offset, int whence){
    if (fd != FAKE_UID_XMB_VIDEO_ISO) return sceIoLseek(fd, offset, whence);
    switch (whence){
        case PSP_SEEK_SET: file_pos = offset; break;
        case PSP_SEEK_CUR: file_pos += offset; break;
        case PSP_SEEK_END: file_pos = sizeof(smallvid)-offset; break;
    }
    return file_pos;
}

int is_video_path(const char* path){
    video_dir[0] = path[0]; video_dir[1] = path[1];
    return strncasecmp(path, video_dir, 10) == 0;
}

int is_video_file(SceUID fd){
    return fd == FAKE_UID_XMB_VIDEO_ISO;
}

int is_video_folder(SceUID dd){
    return dd == video_dd;
}