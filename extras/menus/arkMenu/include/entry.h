#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <malloc.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "common.h"
#include "controller.h"
#include "gfx.h"

#define PSN_DRIVER 0
#define ISO_DRIVER 3

#define HOMEBREW_RUNLEVEL 0x141
#define HOMEBREW_RUNLEVEL_GO 0x152
#define ISO_RUNLEVEL 0x123
#define ISO_RUNLEVEL_GO 0x125
#define ISO_PBOOT_RUNLEVEL 0x124
#define ISO_PBOOT_RUNLEVEL_GO 0x126
#define POPS_RUNLEVEL 0x144
#define POPS_RUNLEVEL_GO 0x155
#define RECOVERY_RUNLEVEL 0x141
#define UPDATER_RUNLEVEL 0x140

#define ZIP_MAGIC 0x04034b50
#define RAR_MAGIC 0x21726152

int loadIconThread(SceSize _args, void *_argp);

typedef struct  __attribute__((packed)) {
    u32 signature;
    u32 version;
    u32 fields_table_offs;
    u32 values_table_offs;
    int nitems;
} SFOHeader;

typedef struct __attribute__((packed)) {
    u16 field_offs;
    u8  unk;
    u8  type; // 0x2 -> string, 0x4 -> number
    u32 unk2;
    u32 unk3;
    u16 val_offs;
    u16 unk4;
} SFODir;

typedef struct SfoInfo {
    char title[128];
    char gameid[10];
}SfoInfo;

class Entry{

    private:

        void gameBoot();
        void animAppear();
        void animDisappear();

    protected:

        string name;
        string path;
        Image* icon0;
        void* icon1;
        Image* pic0;
        Image* pic1;
        void* snd0;
        
        int icon1_size;
        int at3_size;
        
        virtual void doExecute()=0;
                
    public:
        Entry();
        Entry(string path);
        virtual ~Entry()=0;
        
        virtual string getName();
        void setName(string name);
        string getPath();
        void setPath(string path);
        
        Image* getIcon();
        void* getIcon1();
        int getIcon1Size();
        Image* getPic1();
        Image* getPic0();
        void* getSnd();
        int getSndSize();
        
        void setIcon(Image* icon){ this->icon0 = icon; };
        virtual void freeIcon();

        virtual SfoInfo getSfoInfo(){
            SfoInfo info;
            strcpy(info.title, name.c_str());
            strcpy(info.gameid, "HOMEBREW");
            return info;
        };
        
        virtual void loadIcon()=0;
        virtual void loadPics()=0;
        virtual void loadAVMedia()=0;
        
        void freeTempData();
        
        void drawBG();
        
        bool pmfPrompt();
        void execute(bool isAutoboot = false);
        
        virtual char* getType()=0;
        virtual char* getSubtype()=0;
        
        static bool isPRX(const char* path);
        static bool isARK(const char* path);
        static bool isTXT(const char* path);
        static bool isIMG(const char* path);
        static bool isMusic(const char* path);
        static bool isVideo(const char* path);
        static bool isArchive(const char* path);

        static bool getSfoParam(unsigned char* sfo_buffer, int buf_size, char* param_name, unsigned char* var, int* var_size);
        
        static bool cmpEntriesForSort (Entry* i, Entry* j);
        
};

#endif
