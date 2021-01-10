#ifndef ZIP_H
#define ZIP_H

#include "entry.h"
#include "unziprar.h"

#define ZIP_MAGIC 0x04034b50
#define RAR_MAGIC 0x21726152

class Zip : public Entry
{

	private:
		char* subtype;

	public:
	
		Zip(string path);
		~Zip();
	
		void loadIcon();
		void getTempData1();
		void getTempData2();


		void execute();
		
		char* getType();
		char* getSubtype();
		
		void extract(string dir);
		
		static bool isZip(const char* path);
		static bool isRar(const char* path);

};

#endif

