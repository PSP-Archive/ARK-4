/* Copyright 2015 the unarr project authors (see AUTHORS file).
   License: LGPLv3 */

/* demonstration of most of the public unarr API:
   parses and decompresses an archive into memory (integrity test) */

#include "unarr.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

#include <pspsdk.h>
#include <psprtc.h>
#include <pspiofilemgr.h>

PSP_MODULE_INFO("LibUnarchivePSP", PSP_MODULE_SINGLE_LOAD|PSP_MODULE_SINGLE_START, 1, 0);


struct tm* gmtime(const time_t* timer){
    struct tm* res = malloc(sizeof(struct tm));
    memset(res, 0, sizeof(struct tm));
    pspTime date;
    sceRtcSetWin32FileTime(&date, timer);
    res->tm_sec = date.seconds;
    res->tm_min = date.minutes;
    res->tm_hour = date.hour;
    res->tm_mday = date.day;
    res->tm_mon = date.month;
    res->tm_year = date.year;
    return res;
}

time_t mktime(struct tm *timeptr){
    u64 time;
    pspTime date;
    date.seconds = timeptr->tm_sec;
    date.minutes = timeptr->tm_min;
    date.hour = timeptr->tm_hour;
    date.day = timeptr->tm_mday;
    date.month = timeptr->tm_mon;
    date.year = timeptr->tm_year;
    sceRtcGetWin32FileTime(&date, &time);
    return time;
}

void createDirsForFile(char* path){
    int len = strlen(path);
    char* tmp = NULL;
    while ((tmp=strrchr(path, '/')) != NULL){
        *tmp = 0;
        sceIoMkdir(path, 0777);
    }
    for (int i=0; i<len; i++){
        if (path[i] == 0) path[i] = '/';
    }
}

ar_archive *ar_open_any_archive(ar_stream *stream, const char *fileext)
{
    ar_archive *ar = ar_open_rar_archive(stream);
    if (!ar)
        ar = ar_open_zip_archive(stream, fileext && (strcmp(fileext, ".xps") == 0 || strcmp(fileext, ".epub") == 0));
    if (!ar)
        ar = ar_open_7z_archive(stream);
    if (!ar)
        ar = ar_open_tar_archive(stream);
    return ar;
}

int unarchiveFile(const char* filepath, const char* parent)
{
    ar_stream *stream = NULL;
    ar_archive *ar = NULL;
    int entry_count = 1;
    int entry_skips = 0;
    int error_step = 1;

    stream = ar_open_file(filepath);

    ar = ar_open_any_archive(stream, strrchr(filepath, '.'));

    while (ar_parse_entry(ar)) {
        size_t size = ar_entry_get_size(ar);
        const char *raw_filename = ar_entry_get_raw_name(ar);
        char full_path[255];
        strcpy(full_path, parent);

        int parent_slash = parent[strlen(parent)-1] == '/';
        int raw_slash = raw_filename[0] == '/';
        if (parent_slash && raw_slash){
            strcat(full_path, &raw_filename[1]);
        }
        else if (!parent_slash && !raw_slash){
            strcat(full_path, "/");
            strcat(full_path, raw_filename);
        }
        else {
            strcat(full_path, raw_filename);
        }
        createDirsForFile(full_path);
        int fd = sceIoOpen(full_path, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
        while (size > 0) {
            unsigned char buffer[1024];
            size_t count = size < sizeof(buffer) ? size : sizeof(buffer);
            if (!ar_entry_uncompress(ar, buffer, count))
                break;
            sceIoWrite(fd, buffer, count);
            size -= count;
        }
        sceIoClose(fd);
        if (size > 0) {
            entry_skips++;
        }
    }
    error_step = entry_skips > 0 ? 1000 + entry_skips : 0;

CleanUp:
    ar_close_archive(ar);
    ar_close(stream);
    return error_step;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(){
    return 0;
}
