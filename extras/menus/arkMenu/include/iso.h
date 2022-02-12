#ifndef ISO_H
#define ISO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "entry.h"

#define SECTOR_SIZE 0x800
#define VOLUME_DESCRIPTOR_START_SECTOR 0x10
#define PVD_MAGIC "CD001"
#define ISO_MAGIC 0x30444301

class Iso : public Entry
{
    public:

        Iso();
        Iso(string path);
        ~Iso();
    
        void loadIcon();
        void getTempData1();
        void getTempData2();
        
        static bool isISO(const char* filepath);
        
        /* Much faster function for extracting files in PSP_GAME/ */
        static void* fastExtract(const char* path, char* file, unsigned* size=NULL);

        char* getType();
        char* getSubtype();

        static void executeISO(const char* path, bool is_patched);

    protected:

        void doExecute();
        bool isPatched();
};

#endif
