#ifndef EBOOT_H
#define EBOOT_H

#include "entry.h"
#include "gfx.h"

#define EBOOT_MAGIC 0x50425000
#define ELF_MAGIC 0x464C457F

#define PS1_CAT 0x454D
#define PSN_CAT 0x4745
#define HMB_CAT 0x474D

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

enum { UNKNOWN_TYPE, TYPE_HOMEBREW, TYPE_PSN, TYPE_POPS, TYPE_UPDATER };

using namespace std;

class Eboot : public Entry{

    private:
        string ebootName;
        PBPHeader header;
        
        char* subtype;

        void readHeader();
        
        void readFile(void* dst, unsigned offset, unsigned size);
        
    public:
    
        Eboot(string path);
        ~Eboot();
    
        string getEbootName();
        
        void loadIcon();
        void loadPics();
        void loadAVMedia();
        SfoInfo getSfoInfo();
        
        void doExecute();
        
        char* getType();
        char* getSubtype();
        
        static bool isEboot(const char* path);
        static int getEbootType(const char* path);
        static string fullEbootPath(string path, string app, bool scan_dlc=false);
        
        static void executeEboot(const char* path);
        static void executeHomebrew(const char* path);
        static void executePSN(const char* path);
        static void executePOPS(const char* path);
        static void executeUpdate(const char* path);
};

#endif
