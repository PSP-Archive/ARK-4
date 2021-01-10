
#ifndef CSO_H

#define CSO_H


#include "entry.h"

#include <stdio.h>

#include <stdlib.h>

#include <zlib.h>

#include <pspdebug.h>

#include <string.h>


#define CSO_MAGIC 0x4F534943


typedef struct 

{

	unsigned magic;

	unsigned header_size;

	unsigned long long file_size;

	unsigned block_size;

	unsigned char version;

	unsigned char align;

	char reserved[2];

} cso_header;


typedef struct

{

    char type;

    char id[5];

    char version;

} volumeDescriptor;


typedef struct

{

    unsigned pathTableSize;

    unsigned pathTableSizeLE;

    unsigned pathTableOffset;

    unsigned pathTableOptionalOffset;

} primaryVolumeDescriptor;


typedef struct

{

    char length;

    char extended;

    unsigned location;

    short parent;

} pathTable; //8


typedef struct

{

    char length;

	char extended;

    unsigned location;

	unsigned locationLE;

	unsigned size;

	unsigned sizeLE;

	char trash[7];

	char flags;

	char other_size;

	char intervale;

	unsigned vol_seq;

    char nameLen;

} dirRecord;


#define SECTOR_SIZE 0x800


class Cso : public Entry{


	private:


		unsigned identifyEntry(const char * name, unsigned block, unsigned * fileSize);

	

		FILE * file;

		unsigned * indices;

		unsigned char * data, * read_buffer;

		unsigned total_blocks, indices_len, current_index, current_index2;

		cso_header file_header, head;

		primaryVolumeDescriptor pvd;

		z_stream dec;


		bool open(const char * path);

		void clear();

		

		int getPrimaryVolumeDescriptor();

		unsigned findFile(const char * file, unsigned * fileSize);

		unsigned char * getDataBlock(unsigned block);

		

		void readFile(void* dst, unsigned block, unsigned size);

		void extractFile(const char * name, unsigned block, unsigned size);

		

		u8* block_out;

		unsigned block_size, start_read;

		void getInitialBlock(FILE* fp);


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

		

		void execute();


};


#endif

