
#ifndef CSO_H

#define CSO_H


#include <stdio.h>

#include <stdlib.h>

#include <zlib.h>

#include <pspdebug.h>

#include <string.h>

#include "iso.h"

#define CSO_MAGIC 0x4F534943
#define ZSO_MAGIC 0x4F53495A
#define DAX_MAGIC 0x00584144

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

enum {
    TYPE_CSO,
    TYPE_ZSO,
    TYPE_DAX,
};

class Cso : public Entry{


    private:

        int ciso_type;

        void clear();

    protected:
    
        bool isPatched();
        void doExecute();


    public:

        Cso(string path);

        ~Cso();

        

        void loadIcon();

        void getTempData1();

        void getTempData2();

        

        char* getType();

        char* getSubtype();

        

        static bool isPatched(string path);

        static bool isCSO(const char* filepath);

        void* fastExtract(const char* path, char* file, unsigned* size=NULL);

};


#endif

