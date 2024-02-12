#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <cstdio>
#include <malloc.h>
#include "controller.h"
#include "graphics.h"

typedef struct
{
    u32 magic;
    u32 version;
    u32 param_offset;
    u32 icon0_offset;
    u32 icon1_offset;
    u32 pic0_offset;
    u32 pic1_offset;
    u32 snd0_offset;
    u32 elf_offset;
    u32 psar_offset;
} PBPHeader;

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

using namespace std;

class Entry{

    private:

        string name;
        string path;
        string ebootName;
        Image* icon0;
        Image* pic0;
        Image* pic1;
        PBPHeader header;

        void readHeader();
        Image* loadIcon();
        
        void animAppear();
        void animDisappear();
        
        static bool getSfoParam(unsigned char* sfo_buffer, int buf_size, char* param_name, unsigned char* var, int* var_size);
        void findNameInParam();

    public:
    
        Entry(string path);
        ~Entry();
        
        string getName();
        
        string getPath();
        
        string getEbootName();
        
        Image* getIcon();
        
        Image* getPic0();
        
        Image* getPic1();
        
        bool run();

        static bool cmpEntriesForSort (Entry* i, Entry* j);
};

#endif
