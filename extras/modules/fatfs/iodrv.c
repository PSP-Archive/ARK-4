#include <pspkernel.h>
#include <pspreg.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>
#include <pspumd.h>
#include <psprtc.h>
#include <pspinit.h>
#include "systemctrl_private.h"
#include "inferno.h"
#include <ark.h>
#include "macros.h"
#include "ff.h"

static FATFS fatfs;

static void fileInfoToStat(FILINFO *fno, SceIoStat *stat)
{
    WORD fdate = fno->fdate;
    WORD ftime = fno->ftime;
    unsigned char stime[8];
    u16 year;

    stat->attr           = 0777;
    stat->size           = (unsigned int)(fno->fsize);
    stat->hisize         = (unsigned int)(fno->fsize>>32);

    stat->mode = FIO_S_IROTH | FIO_S_IXOTH;
    if (fno->fattrib & AM_DIR) {
        stat->mode |= FIO_S_IFDIR;
    } else {
        stat->mode |= FIO_S_IFREG;
    }
    if (!(fno->fattrib & AM_RDO)) {
        stat->mode |= FIO_S_IWOTH;
    }

    // Since the VFAT file system does not support timezones, the timezone offset will not be applied.
    // exFAT does support timezones, but the feature is not used/exposed in the FatFs library.
    // Thus, conversion to/from JST may be incorrect.
    // For simplicity's sake, the timezone is not read from the system configuration and timezone conversion is not done.

    stime[0] = 0; // Padding

    stime[4] = (fdate & 31); // Day
    stime[5] = (fdate >> 5) & 15; // Month

    year = (fdate >> 9) + 1980;
    stime[6] = year & 0xff; // Year (low bits)
    stime[7] = (year >> 8) & 0xff; // Year (high bits)

    stime[3] = (ftime >> 11); // Hours
    stime[2] = (ftime >> 5) & 63; // Minutes
    stime[1] = (ftime << 1) & 31; // Seconds (multiplied by 2)

    memcpy(stat->ctime, stime, sizeof(stime));
    memcpy(stat->atime, stime, sizeof(stime));
    memcpy(stat->mtime, stime, sizeof(stime));
}

static int IoInit(PspIoDrvArg* arg){
    return 0;
}

static int IoExit(PspIoDrvArg* arg){
    return 0;
}

static int IoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode){
    BYTE f_mode = FA_OPEN_EXISTING;
    (void)mode;

    // check if the slot is free
    arg->arg = fs_find_free_fil_structure();
    if (arg->arg == NULL) {
        _fs_unlock();
        return 0x80010018; // SCE_ERROR_ERRNO_TOO_MANY_OPEN_SYSTEM_FILES
    }

    // translate mode
    if (flags & PSP_O_RDONLY)
        f_mode |= FA_READ;
    if (flags & PSP_O_WRONLY)
        f_mode |= FA_WRITE;
    if (flags & PSP_O_CREAT)
        f_mode |= FA_OPEN_ALWAYS;
    if (flags & PSP_O_TRUNC)
        f_mode |= FA_CREATE_ALWAYS;
    if (flags & PSP_O_APPEND)
        f_mode |= FA_OPEN_APPEND;

    int ret = f_open(fd->privdata, file, f_mode);

    if (ret != FR_OK){
        arg->arg = NULL;
    }

    _fs_unlock();

    switch (ret){
        case FR_OK: return 0;
        case FR_DISK_ERR: return 0x80010005; // SCE_ERROR_ERRNO_IO_ERROR
        case FR_NO_FILE: case FR_NO_PATH: return 0x80010002; // SCE_ERROR_ERRNO_FILE_NOT_FOUND
        default: return -ret;
    }
}

static int IoClose(PspIoDrvFileArg *arg){
    int ret = FR_OK;

    _fs_lock();

    if (arg->arg) {
        ret = f_close(arg->arg);
        arg->arg = NULL;
    }

    _fs_unlock();

    if (ret != FR_OK) return 0x80010009; // SCE_ERROR_ERRNO_INVALID_FILE_DESCRIPTOR
    
    return 0;
}

static int IoRead(PspIoDrvFileArg *arg, char *data, int len){
    int ret;
    UINT br = 0;

    if (arg->arg == NULL)
        return 0x80010009; // SCE_ERROR_ERRNO_INVALID_FILE_DESCRIPTOR

    _fs_lock();

    ret = f_read(arg->arg, data, len, &br);

    _fs_unlock();
    return (ret == FR_OK) ? br : 0x80010005; // SCE_ERROR_ERRNO_IO_ERROR
}

static int IoWrite(PspIoDrvFileArg *arg, char *data, int len){
    int ret;
    UINT bw = 0;

    if (arg->arg == NULL)
        return 0x80010009; // SCE_ERROR_ERRNO_INVALID_FILE_DESCRIPTOR

    _fs_lock();

    ret = f_write(arg->arg, data, len, &bw);

    _fs_unlock();
    return (ret == FR_OK) ? bw : 0x80010005; // SCE_ERROR_ERRNO_IO_ERROR
}

static SceOff IoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence){
    int res;

    if (arg->arg == NULL)
        return 0x80010009; // SCE_ERROR_ERRNO_INVALID_FILE_DESCRIPTOR
    
    _fs_lock();

    FIL *file = (FIL *)(arg->arg);

    FSIZE_t off = offset;

    switch (whence) {
        case SEEK_CUR:
            off += file->fptr;
            break;
        case SEEK_END:
            off = file->obj.objsize - offset;
            break;
    }

    res = f_lseek(file, off);

    _fs_unlock();
    return (res == FR_OK) ? (SceOff)(file->fptr) : 0x80010005; // SCE_ERROR_ERRNO_IO_ERROR
}

static int IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen){
    // TODO: reverse this from original fatms driver
}

static int IoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen){
    // TODO: reverse this from original fatms driver
}

static int IoRemove(PspIoDrvFileArg *arg, const char *name){
    int ret;

    _fs_lock();
    ret = f_unlink(name);
    _fs_unlock();
    
    return -ret;
}

static int IoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode){
    int ret;
    (void)mode;

    _fs_lock();

    ret = f_mkdir(name);

    _fs_unlock();

    return -ret;
}

int IoRmdir(PspIoDrvFileArg *arg, const char *name){
    int ret;

    _fs_lock();

    ret = f_rmdir(name);

    _fs_unlock();

    return -ret;
}

static int IoDopen(PspIoDrvFileArg *arg, const char *dirname){
    int ret;

    _fs_lock();

    // check if the slot is free
    arg->arg = fs_find_free_dir_structure();
    if (arg->arg == NULL) {
        _fs_unlock();
        return 0x80010018; // SCE_ERROR_ERRNO_TOO_MANY_OPEN_SYSTEM_FILES
    }

    ret = f_opendir(arg->arg, dirname);

    if (ret != FR_OK){
        arg->arg = NULL;
    }

    _fs_unlock();

    switch (ret){
        case FR_OK: return 0;
        case FR_DISK_ERR: return 0x80010005; // SCE_ERROR_ERRNO_IO_ERROR
        case FR_NO_FILE: case FR_NO_PATH: return 0x80010002; // SCE_ERROR_ERRNO_FILE_NOT_FOUND
        default: return -ret;
    }
}

static int IoDclose(PspIoDrvFileArg *arg){
    int ret = 0x80010009; // SCE_ERROR_ERRNO_INVALID_FILE_DESCRIPTOR

    _fs_lock();

    if (arg->arg) {
        ret = f_closedir(arg->arg);
        arg->arg = NULL;
    }

    _fs_unlock();
    return (ret == FR_OK)? 0 : 0x80010009; // SCE_ERROR_ERRNO_INVALID_FILE_DESCRIPTOR
}

static int IoDread(PspIoDrvFileArg *arg, SceIoDirent *dir){
    int ret;
    FILINFO fno;

    if (arg->arg == NULL)
        return 0x80010009; // SCE_ERROR_ERRNO_INVALID_FILE_DESCRIPTOR

    _fs_lock();

    ret = f_readdir(arg->arg, &fno);

    if (ret == FR_OK && fno.fname[0]) {
        strncpy(dir->d_name, fno.fname, 255);
        fileInfoToStat(&fno, &(dir->d_stat));
        ret = 1;
    } else {
        ret = 0;
    }

    _fs_unlock();
    return ret;
}

static int IoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat){
    int ret;
    FILINFO fno;

    // FatFs f_stat doesn't handle the root directory, so we'll handle this case ourselves.
    {
        const char *name_no_leading_slash = name;
        while (*name_no_leading_slash == '/') {
            name_no_leading_slash += 1;
        }
        if ((strcmp(name_no_leading_slash, "") == 0) || (strcmp(name_no_leading_slash, ".") == 0)) {
            // Return data indicating that it is a directory.
            memset(stat, 0, sizeof(*stat));
            stat->mode = FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH | FIO_S_IFDIR;
            return 0;
        }
    }

    _fs_lock();

    ret = f_stat(file, &fno);

    if (ret == FR_OK) {
        fileInfoToStat(&fno, stat);
    } else {
        ret = -ret;
    }

    _fs_unlock();
    return ret;
}

static int IoChstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits){
    // TODO
    return 0;
}

static int IoRename(PspIoDrvFileArg *arg, const char *oldname, const char *newname){
    int ret;

    // If old and new path are the same, no need to do anything
    if (strcmp(oldname, newname) == 0) {
        return 0;
    }

    _fs_lock();

    ret = f_rename(oldname, newname);

    _fs_unlock();

    return -ret;
}

static int IoChdir(PspIoDrvFileArg *arg, const char *dir){
    int ret;

    _fs_lock();

    ret = f_chdir(dir);

    _fs_unlock();

    return -ret;
}

static int IoMount(PspIoDrvFileArg *arg){
    int ret = f_mount(&fatfs, "", 1);
    return -ret;
}

static int IoUmount(PspIoDrvFileArg *arg){
    f_unmount("");
}

PspIoDrvFuncs g_drv_funcs = {
    .IoInit    = &IoInit,
    .IoExit    = &IoExit,
    .IoOpen    = &IoOpen,
    .IoClose   = &IoClose,
    .IoRead    = &IoRead,
    .IoWrite   = &IoWrite,
    .IoLseek   = &IoLseek,
    .IoIoctl   = &IoIoctl,
    .IoRemove  = &IoRemove,
    .IoMkdir   = &IoMkdir,
    .IoRmdir   = &IoRmdir,
    .IoDopen   = &IoDopen,
    .IoDclose  = &IoDclose,
    .IoDread   = &IoDread,
    .IoGetstat = &IoGetstat,
    .IoChstat  = &IoChstat,
    .IoRename  = &IoRename,
    .IoChdir   = &IoChdir,
    .IoMount   = &IoMount,
    .IoUmount  = &IoUmount,
    .IoDevctl  = &IoDevctl,
    .IoUnk21   = NULL,
};

int power_event_handler(int ev_id, char *ev_name, void *param, int *result){
    if( ev_id == 0x400 ) { // sleep
		// TODO
	}
    if(ev_id == 0x400000) { // resume complete
        // TODO
    }

}