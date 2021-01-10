#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <cstdio>
#include <malloc.h>
#include "controller.h"
extern "C"{
#include "../graphics/graphics.h"
}

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

using namespace std;

class Entry{

	private:

		string name;
		string path;
		string ebootName;
		Image* icon0;
		Image* pic0;
		Image* pic1;
		PBPHeader* header;

		void readHeader();
		Image* loadIcon();
		
		void animAppear();
		void animDisappear();
		
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
		
};

#endif
