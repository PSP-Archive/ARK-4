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

#define FAKE_UID_XMB_VIDEO_ISO (FAKE_UID|0xD1)

static int file_pos = 0;
static SceUID video_dd = -1;
static SceUID isovideo_fd = -1;
extern SceCtrlData *last_control_data;

static char* video_dir = "ms0:/VIDEO";
static char* isovideo_dir = "ms0:/ISO/VIDEO";
static const char* video_icon_path = "/UMD_VIDEO/ICON0.PNG";

static u8* icon_data = NULL;
static int icon_size = 0;

static void launch_umdvideo_mount(const char *path) {
    if (path == NULL)
        return;

    int type = vshDetectDiscType(path);
    if (type < 0)
        return;

    sctrlSESetUmdFile(path);
    sctrlSESetBootConfFileIndex(MODE_VSHUMD);
    sctrlSESetDiscType(type);
    sctrlKernelExitVSH(NULL);
}

static void getIconStatFromISO(const char* isopath){
    int res, size, lba;

    icon_size = 0;

    res = isoOpen(isopath);
    if (res<0) return;

    res = isoGetFileInfo(video_icon_path, &size, &lba);
    isoClose();
    if (res<0) return;
    
    icon_size = size;
}

static void readIconFromISO(const char* isopath){
    int res, size, lba;

    res = isoOpen(isopath);
    if (res<0) return;

    res = isoGetFileInfo(video_icon_path, &size, &lba);
    if (res<0) {
        isoClose();
        return;
    }

    u8* data = vsh_malloc(size);

    res = isoRead(data, lba, 0, size);
    isoClose();

    if (res<0){
        vsh_free(data);
        return;
    }

    if (icon_data){
        vsh_free(icon_data);
    }

    icon_data = data;
    icon_size = size;

    // patch smallvid
    size += 24;
    smallvid[0x9DE] = icon_data[0x13];
    smallvid[0x9E0] = icon_data[0x17];
    smallvid[0xB19] = (unsigned char)((size&0xFF00)>>8);
    smallvid[0xB1A] = (unsigned char)(size&0xFF);
    smallvid[0x6E2] = 0;
    smallvid[0x71C] = 0;
    smallvid[0x770] = 0;
    smallvid[0x7AC] = 0;
    smallvid[0x7EE] = 0;
    smallvid[0x832] = 0;
    smallvid[0x86A] = 0;
    smallvid[0x8AB] = 0;
    smallvid[0x8AC] = 0;
    smallvid[0x8AD] = 0;
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
                          || (pad&PSP_CTRL_RIGHT)  == PSP_CTRL_RIGHT
                        ){
                            launch_umdvideo_mount(isopath);
                        }
                    }
                    readIconFromISO(isopath);
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
                    getIconStatFromISO(isopath);
                    stat->st_size = sizeof(smallvid) + icon_size;
                }
            }
        }
        pspSdkSetK1(k1);
    }

    return res;
}

int videoIoRead(SceUID fd, void* buf, u32 size){

    if (fd == FAKE_UID_XMB_VIDEO_ISO){
        if (file_pos < sizeof(smallvid)){
            if (file_pos+size > sizeof(smallvid)){
                int size_1 = sizeof(smallvid)-file_pos;
                int size_2 = file_pos+size-sizeof(smallvid);
                memcpy(buf, &smallvid[file_pos], size_1);
                memcpy((u8*)buf+size_1, icon_data, size_2);
            }
            else {
                memcpy(buf, &smallvid[file_pos], size);
            }
        }
        else{
            memcpy(buf, &icon_data[file_pos-sizeof(smallvid)], size);
        }
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
    if (fd != FAKE_UID_XMB_VIDEO_ISO)
        return sceIoClose(fd);

    if (icon_data){
        vsh_free(icon_data);
        icon_data = 0;
        icon_size = 0;
    }
    return 0;
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
        case PSP_SEEK_END: file_pos = icon_size+sizeof(smallvid)-offset; break;
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