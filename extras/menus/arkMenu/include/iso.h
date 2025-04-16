#ifndef ISO_H
#define ISO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include "entry.h"

#define SECTOR_SIZE 0x800
#define ISO_MAGIC 0x30444301

#define CSO_MAGIC 0x4F534943 // CISO
#define ZSO_MAGIC 0x4F53495A // ZISO
#define DAX_MAGIC 0x00584144 // DAX
#define JSO_MAGIC 0x4F53494A // JISO

#define DAX_BLOCK_SIZE 0x2000
#define DAX_COMP_BUF 0x2400

typedef struct 
{
    unsigned magic;
    unsigned header_size;
    unsigned long long file_size;
    unsigned block_size;
    unsigned char version;
    unsigned char align;
    char reserved[2];
} CSOHeader;

typedef struct{ 
    uint32_t magic;
    uint32_t uncompressed_size;
    uint32_t version; 
    uint32_t nc_areas; 
    uint32_t unused[4]; 
} DAXHeader;

typedef struct _JisoHeader {
    uint32_t magic; // [0x000] 'JISO'
    uint8_t unk_x001; // [0x004] 0x03?
    uint8_t unk_x002; // [0x005] 0x01?
    uint16_t block_size; // [0x006] Block size, usually 2048.
    // TODO: Are block_headers and method 8-bit or 16-bit?
    uint8_t block_headers; // [0x008] Block headers. (1 if present; 0 if not.)
    uint8_t unk_x009; // [0x009]
    uint8_t method; // [0x00A] Method. (See JisoAlgorithm_e.)
    uint8_t unk_x00b; // [0x00B]
    uint32_t uncompressed_size; // [0x00C] Uncompressed data size.
    uint8_t md5sum[16]; // [0x010] MD5 hash of the original image.
    uint32_t header_size; // [0x020] Header size? (0x30)
    uint8_t unknown[12]; // [0x024]
} JisoHeader;

typedef enum {
    JISO_METHOD_LZO        = 0,
    JISO_METHOD_ZLIB    = 1,
} JisoMethod;

typedef struct{
    u32 offset;
    u32 size;
} FileData;

class Iso : public Entry
{
    public:

        Iso();
        Iso(string path);
        ~Iso();
    
        void loadIcon();
        void loadPics();
        void loadAVMedia();
        SfoInfo getSfoInfo();
        
        static bool isISO(const char* filepath);
        
        /* Much faster function for extracting files in PSP_GAME/ */
        void* fastExtract(char* file, unsigned* size=NULL);
        int checkAudioVideo();

        char* getType();
        char* getSubtype();

        static void executeISO(const char* path, char* eboot_path);
        static void executeVideoISO(const char* path);

    protected:

        // keep track to the offset and size of loaded files (icon, pmf, etc) for faster extraction
        map<string, FileData> file_cache;

        // reader information        
        u32 header_size;
        u32 block_size;
        u32 uncompressed_size;
        u32 block_header;
        u32 align;   
        
        // reader functions
        int (Iso::*read_iso_data)(u8* addr, u32 size, u32 offset);
        int read_raw_data(u8* addr, u32 size, u32 offset);
        int read_compressed_data(u8* addr, u32 size, u32 offset);

        // decompressor functions
        void (*ciso_decompressor)(void* src, int src_len, void* dst, int dst_len, u32 is_nc);
        
        void doExecute();
        bool isPatched();
        string getShortName();

        int has_installed_file(const char* installed_file, char* out_path);
};

#endif
