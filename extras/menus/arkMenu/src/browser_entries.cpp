#include <cstdio>
#include <sstream>
#include <dirent.h>
#include "browser_entries.h"


BrowserFile::BrowserFile(){
}

BrowserFile::BrowserFile(string path){
    this->path = path;
    size_t lastSlash = path.rfind("/", string::npos);
    this->name = path.substr(lastSlash+1, string::npos);
    this->selected = false;
    this->calcSize();
}

BrowserFile::BrowserFile(BrowserFile* orig){
    this->path = orig->path;
    this->selected = false;
    this->fileSize = orig->fileSize;
}

BrowserFile::~BrowserFile(){
}

void BrowserFile::calcSize(){
    // Calculate the size (in Bytes, KB, MB or GB) of a BrowserFile, if it's a BrowserFolder, simply return its type
    FILE* fp = fopen(this->getPath().c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    unsigned size = ftell(fp);
    fclose(fp);

    ostringstream txt;

    if (size < 1024)
        txt<<size<<" Bytes";
    else if (1024 < size && size < 1048576)
        txt<<float(size)/1024.f<<" KB";
    else if (1048576 < size && size < 1073741824)
        txt<<float(size)/1048576.f<<" MB";
    else
        txt<<float(size)/1073741824.f<<" GB";
    this->fileSize = txt.str();
}

bool BrowserFile::isSelected(){
    return this->selected;
}

void BrowserFile::changeSelection(){
    this->selected = !this->selected;
}

string BrowserFile::getPath(){
    return this->path;
}

string BrowserFile::getName(){
    return this->name;
}

string BrowserFile::getSize(){
    return this->fileSize;
}

char* BrowserFile::getType(){
    return "File";
}

char* BrowserFile::getSubtype(){
    return getType();
}

void BrowserFile::loadIcon(){
}

void BrowserFile::getTempData1(){
}

void BrowserFile::getTempData2(){
}

void BrowserFile::doExecute(){
}

BrowserFolder::BrowserFolder(string path){
    this->path = path;
    size_t lastSlash = path.rfind("/", path.length()-2);
    this->name = path.substr(lastSlash+1, string::npos);
    this->selected = false;
    this->fileSize = "Folder";
}

BrowserFolder::BrowserFolder(BrowserFolder* orig){
    this->path = orig->path;
    this->name = orig->name;
    this->selected = false;
    this->fileSize = "Folder";
}

BrowserFolder::~BrowserFolder(){
}

char* BrowserFolder::getType(){
    return "Folder";
}

char* BrowserFolder::getSubtype(){
    return getType();
}
