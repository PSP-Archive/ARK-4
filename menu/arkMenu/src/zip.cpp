#include "zip.h"
#include "common.h"

Zip::Zip(string path)
{
	this->path = path;
	size_t lastSlash = path.rfind("/", string::npos);
	this->name = path.substr(lastSlash+1, string::npos);
	this->icon0 = common::getImage(IMAGE_ZIP);
	this->subtype = NULL;
};

void Zip::loadIcon(){
	sceKernelDelayThread(50000);
	this->icon0 = common::getImage(IMAGE_ZIP);
}

void Zip::getTempData1(){
	this->pic0 = NULL;
	this->pic1 = NULL;
}

void Zip::getTempData2(){
	this->icon1 = NULL;
	this->snd0 = NULL;
}

void Zip::execute(){
}
		
char* Zip::getType(){
	return "ZIP";
}

char* Zip::getSubtype(){
	char* zip = "ZIP";
	char* rar = "RAR";
	return (isZip(this->path.c_str()))? zip : rar;
}

void Zip::extract(string dir){
	if (isZip(this->path.c_str()))
		unzipToDir(this->path.c_str(), dir.c_str(), NULL);
	else
		DoExtractRAR(this->path.c_str(), dir.c_str(), NULL);
}

Zip::~Zip()
{
};

bool Zip::isZip(const char* path){
	return (common::getMagic(path, 0) == ZIP_MAGIC);
}

bool Zip::isRar(const char* path){
	return (common::getMagic(path, 0) == RAR_MAGIC);
}
