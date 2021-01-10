#ifndef EBOOT_H
#define EBOOT_H

#include "entry.h"
#include "gfx.h"

#define EBOOT_MAGIC 0x50425000

#define PS1_CAT 0x454D
#define PSN_CAT	0x4745
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

typedef struct
{
	unsigned sign;
	unsigned version;
	unsigned key_table;
	unsigned data_table;
	unsigned entries;
} sfoHeader;

enum { TYPE_HOMEBREW, TYPE_PSN, TYPE_POPS, UNKNOWN_TYPE };

using namespace std;

class Eboot : public Entry{

	private:
		string ebootName;
		PBPHeader* header;
		
		char* subtype;

		void readHeader();
		
		void readFile(void* dst, unsigned offset, unsigned size);
		void extractFile(const char * name, unsigned block, unsigned size);
		
	public:
	
		Eboot(string path);
		~Eboot();
	
		string getEbootName();
		
		void loadIcon();
		void getTempData1();
		void getTempData2();
		
		void execute();
		
		char* getType();
		char* getSubtype();
		
		static bool isEboot(const char* path);
		static int getEbootType(const char* path);
		static string fullEbootPath(string app);
};

#endif
