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

// Fake UID to identify Virtual MP4
#define FAKE_UID_XMB_VIDEO_ISO (FAKE_UID|0xD1)

// I/O related variables
static int file_pos = 0;
static SceUID video_dd = -1;
static SceUID isovideo_fd = -1;
static u8* icon_data = NULL;
static int icon_size = 0;
static char iso_launched[128] = {0};

// Controller data
extern SceCtrlData *last_control_data;

// Various important paths
static char* video_dir = "ms0:/VIDEO";
static char* isovideo_dir = "ms0:/ISO/VIDEO";
static const char* video_icon_path = "/UMD_VIDEO/ICON0.PNG";
static const char* music_icon_path = "/UMD_AUDIO/ICON0.PNG"; // does this even exist?

// Mount an ISO on XMB (reboots the system with Inferno driver instead of UMD)
static void launch_umdvideo_mount(const char *path) {

    int type = vshDetectDiscType(path);
    if (type < 0)
        return;

    sctrlSESetUmdFile(path);
    sctrlSESetBootConfFileIndex(MODE_VSHUMD);
    sctrlSESetDiscType(type);
    sctrlKernelExitVSH(NULL);
}

// Get icon0.png size in a Video ISO
static void getIconStatFromISO(const char* isopath){
    int res, size, lba;

    icon_size = 0;

    res = isoOpen(isopath);
    if (res<0) return;

    res = isoGetFileInfo(video_icon_path, &size, &lba);
    if (res<0) res = isoGetFileInfo(music_icon_path, &size, &lba); // retry with UMD_AUDIO
    isoClose();
    if (res<0) return;
    
    icon_size = size;
}

// Read icon0.png from a Video ISO
static void readIconFromISO(const char* isopath){
    int res, size, lba;

    res = isoOpen(isopath);
    if (res<0) return;

    res = isoGetFileInfo(video_icon_path, &size, &lba);
    if (res<0) res = isoGetFileInfo(music_icon_path, &size, &lba); // retry with UMD_AUDIO
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
    smallvid[0x9DE] = icon_data[0x13]; // icon width
    smallvid[0x9E0] = icon_data[0x17]; // icon size
    smallvid[0xB1A] = (unsigned char)size; // size of png + size of tag (24)
    smallvid[0xB19] = (unsigned char)(size>>8);
    smallvid[0xA2E] = (unsigned char)icon_size; // size of png
    smallvid[0xA2D] = (unsigned char)(icon_size>>8);
    smallvid[0x6E2] = 0; // nullify titles so it uses filename instead
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

int videoMpegCreate(void* Mpeg, void* pData, int iSize, void* Ringbuffer, int iFrameWidth, int iUnk1, int iUnk2)
{
    if (iso_launched[0]){
        launch_umdvideo_mount(iso_launched); // launch ISO file
    }
    // passthrough
    return sceMpegCreate(Mpeg, pData, iSize, Ringbuffer, iFrameWidth, iUnk1, iUnk2);
}

// sceIoOpen for Video ISO
SceUID videoIoOpen(const char* file, u32 flags, u32 mode){
    SceUID res = sceIoOpen(file, flags, mode); // passthrough

    if (res < 0){ // failed, retry with ISO
        int k1 = pspSdkSetK1(0);
        char isopath[128]; strcpy(isopath, isovideo_dir);
        isopath[0] = file[0]; isopath[1] = file[1]; // adjust device (psp go)
        char* filename = strrchr(file, '/');
        if (filename){ // filename found
            strcat(isopath, filename); // redirect to /ISO/VIDEO/
            char* ext = strstr(isopath, ".mp4");
            if (ext){ // .mp4 extension found
                SceIoStat stat;
                strcpy(ext, ".iso"); // replace .mp4 with .iso
                if (sceIoGetstat(isopath, &stat)>=0){ // ISO file exists
                    res = FAKE_UID_XMB_VIDEO_ISO; // adjust error value with fake uid
                    file_pos = 0; // reset file position
                    iso_launched[0] = 0;
                    // super nasty solution for detecting that you are trying to play a Video ISO file
                    if (last_control_data && video_dd < 0 && isovideo_fd < 0){
                        u32 swap_xo;
                        u32 pad = last_control_data->Buttons;
                        vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &swap_xo);
                        // detect that some button has been pressed, user will have pressed any of these when wanting to play the file
                        if(  ( ((pad&PSP_CTRL_CROSS)  == PSP_CTRL_CROSS) && swap_xo)
                          || ( ((pad&PSP_CTRL_CIRCLE) == PSP_CTRL_CIRCLE) && !swap_xo)
                          ||    (pad&PSP_CTRL_START)  == PSP_CTRL_START
                          ||    (pad&PSP_CTRL_RIGHT)  == PSP_CTRL_RIGHT
                        ){
                            strcpy(iso_launched, isopath); // remember ISO file
                        }
                    }
                    // read icon0.png from the ISO to make it viable in the XMB
                    readIconFromISO(isopath);
                }
            }
        }
        pspSdkSetK1(k1);
    }

    return res;
}

// sceIoDopen for Video ISO
SceUID videoIoDopen(const char* dir){
    SceUID res = sceIoDopen(dir);

    video_dir[0] = dir[0]; // adjust device (psp go)
    video_dir[1] = dir[1];
    if (strcasecmp(dir, video_dir) == 0) { // check if /VIDEO/ folder has been opened
        last_control_data = NULL;
        video_dd = res;
        int k1 = pspSdkSetK1(0);
        isovideo_dir[0] = dir[0]; // adjust device (psp go)
        isovideo_dir[1] = dir[1];
        isovideo_fd = sceIoDopen(isovideo_dir); // open /ISO/VIDEO/ folder too
        pspSdkSetK1(k1);
    }

    return res;
}

// sceIoGetstat for Video ISO
int videoIoGetstat(const char* path, SceIoStat* stat){
    int res = sceIoGetstat(path, stat);

    if (res < 0){ // failed, retry
        int k1 = pspSdkSetK1(0);
        char isopath[128]; strcpy(isopath, isovideo_dir); // retry with /ISO/VIDEO/
        isopath[0] = path[0]; isopath[1] = path[1]; // adjust device (psp go)
        char* filename = strrchr(path, '/');
        if (filename){ // file name found
            strcat(isopath, filename);
            char* ext = strstr(isopath, ".mp4");
            if (ext){ // .mp4 extension found
                strcpy(ext, ".iso"); // retry with .iso
                res = sceIoGetstat(isopath, stat);
                if (res>=0){ // ISO file found
                    getIconStatFromISO(isopath); // get size of ISO's icon0.png
                    stat->st_size = sizeof(smallvid) + icon_size; // adjust virtual mp4 file size
                }
            }
        }
        pspSdkSetK1(k1);
    }

    return res;
}

// sceIoRead for Video ISO
int videoIoRead(SceUID fd, void* buf, u32 size){

    if (fd == FAKE_UID_XMB_VIDEO_ISO){ // Fake UID of Virtual MP4 file
        if (file_pos >= sizeof(smallvid)+icon_size) // Out of Bouds read
            return 0;
        if (file_pos+size > sizeof(smallvid)+icon_size) // more data requested than available
            size = sizeof(smallvid)+icon_size - file_pos;
        
        if (file_pos < sizeof(smallvid)){ // current position is within mp4 header
            if (file_pos+size > sizeof(smallvid)){ // data to be read is split between mp4 header and png icon
                int size_1 = sizeof(smallvid)-file_pos; // calculate how much from mp4 header to read
                int size_2 = file_pos+size-sizeof(smallvid); // calculate how much from png to read
                memcpy(buf, &smallvid[file_pos], size_1); // read from mp4 header
                memcpy((u8*)buf+size_1, icon_data, size_2); // read from png icon
            }
            else { // only mp4 header needs to be read
                memcpy(buf, &smallvid[file_pos], size);
            }
        }
        else{ // only png icon needs to be read
            memcpy(buf, &icon_data[file_pos-sizeof(smallvid)], size);
        }

        file_pos += size; // adjust virtual file position
        return size;
    }

    return sceIoRead(fd, buf, size);
}

// sceIoDread for Video ISO
int videoIoDread(SceUID fd, SceIoDirent *dir){
    int res = sceIoDread(fd, dir);

    if (res == 0){ // no more items being read in /VIDEO/ folder
        int k1 = pspSdkSetK1(0);
        res = sceIoDread(isovideo_fd, dir); // continue reading items from /ISO/VIDEO/
        if (res>0 && dir->d_name[0] != '.'){ // got valid item
            char* ext = strrchr(dir->d_name, '.');
            if (ext) strcpy(ext, ".mp4"); // replace extension with .mp4
            if (dir->d_private){ // adjust private dirent data
                pspMsPrivateDirent *pri_dirent = dir->d_private;
                ext = strrchr(pri_dirent->l_name, '.'); // long file name
                if (ext) strcpy(ext, ".mp4"); // change extension to .mp4
                ext = strrchr(pri_dirent->s_name, '.'); // short file name
                if (ext) strcpy(ext, ".MP4"); // change extension to .MP4
            }
        }
        pspSdkSetK1(k1);
    }

    return res;
}

// sceIoClose for Video ISO
int videoIoClose(SceUID fd){
    if (fd != FAKE_UID_XMB_VIDEO_ISO)
        return sceIoClose(fd);

    file_pos = 0; // reset file position
    iso_launched[0] = 0;

    // free icon data
    if (icon_data){
        vsh_free(icon_data);
        icon_data = 0;
        icon_size = 0;
    }

    return 0;
}

// sceIoDclose for Video ISO
int videoIoDclose(SceUID fd){
    int res = sceIoDclose(fd);
    if (fd == video_dd){ // when closing /VIDEO/, close /ISO/VIDEO/ too
        int k1 = pspSdkSetK1(0);
        sceIoDclose(isovideo_fd);
        pspSdkSetK1(k1);
        video_dd = -1;
        isovideo_fd = -1;
    }
    return res;
}

// sceIoLseek for Video ISO
SceOff videoIoLseek(SceUID fd, SceOff offset, int whence){
    if (fd != FAKE_UID_XMB_VIDEO_ISO) return sceIoLseek(fd, offset, whence);
    switch (whence){ // adjust virtual mp4 file position
        case PSP_SEEK_SET: file_pos = offset; break;
        case PSP_SEEK_CUR: file_pos += offset; break;
        case PSP_SEEK_END: file_pos = icon_size+sizeof(smallvid)-offset; break;
    }
    return file_pos;
}

// sceIoRemove for Video ISO
int videoRemove(const char * file){
    int res = sceIoRemove(file);

    if (res<0){ // failed, retry with Video ISO
        int k1 = pspSdkSetK1(0);
        char* filename;
        char path[256];
        // redirect file to /ISO/VIDEO/ folder
        isovideo_dir[0] = file[0]; isovideo_dir[1] = file[1]; // redirect device (psp go)
        strcpy(path, isovideo_dir);
        filename = strrchr(file, '/'); // find file name
        if (filename) strcat(path, filename);
        // find .mp4 extension
        filename = strstr(path, ".mp4");
        // replace with .iso
        if (filename) strcpy(filename, ".iso");
        // retry sceIoRemove
        res = sceIoRemove(path);
        pspSdkSetK1(k1);

        extern char mounted_iso[];
        if (res >= 0 && strcasecmp(mounted_iso, path) == 0){
            sctrlKernelExitVSH(NULL); // trigger reboot if we have deleted the currently mounted ISO
        }
    }

    if (last_control_data){
        last_control_data->Buttons = 0;
    }

    return res;
}

// determine if a given path is from /VIDEO/ folder
int is_video_path(const char* path){
    video_dir[0] = path[0]; video_dir[1] = path[1];
    return strncasecmp(path, video_dir, 10) == 0;
}

// determine if an opened file is a Virtual MP4
int is_video_file(SceUID fd){
    return fd == FAKE_UID_XMB_VIDEO_ISO;
}

// determine if an opened folder is /VIDEO/
int is_video_folder(SceUID dd){
    return dd == video_dd;
}
