#include <cstdio>
#include <dirent.h>
#include "browser_entries.h"
#include "eboot.h"
#include "iso.h"


int fileTypeByExtension(string path){
    if (Iso::isISO(path.c_str())){
        return FILE_ISO;
    }
    else if (Eboot::isEboot(path.c_str())){
        return FILE_PBP;
    }
    else if (Entry::isTXT(path.c_str())){
        return FILE_TXT;
    }
    else if (Entry::isZip(path.c_str()) || Entry::isRar(path.c_str())){
        return FILE_ZIP;
    }
    else if (Entry::isPRX(path.c_str())){
        return FILE_PRX;
    }
    else if (Entry::isIMG(path.c_str())){
        return FILE_PICTURE;
    }
    else if (Entry::isMusic(path.c_str())){
        return FILE_MUSIC;
    }
    return FILE_BIN;
}


BrowserFile::BrowserFile(){
}

BrowserFile::BrowserFile(string path, string shortname){
    size_t lastSlash = path.rfind('/', string::npos);
    this->path = path;
    this->name = path.substr(lastSlash+1, string::npos);
    this->parent = path.substr(0, lastSlash) + '/';
    this->icon0 = NULL;
    this->selected = false;
    this->filetype = FOLDER;
    this->setShortName(shortname);
    this->calcSize();
    this->filetype = fileTypeByExtension(getPath());
}

BrowserFile::BrowserFile(string parent, string name, string shortname){
    this->icon0 = NULL;
    this->path = parent + name;
    this->parent = parent;
    this->name = name;
    this->selected = false;
    this->setShortName(shortname);
    this->calcSize();
    this->filetype = fileTypeByExtension(getPath());
}

BrowserFile::~BrowserFile(){
}

void BrowserFile::setShortName(string shortname){
    this->shortname = "";
    if (shortname.size() > 0 && shortname[0] != 0x14){
        if (this->name.size() < shortname.size()){
            this->path = this->parent + shortname;
            this->name = shortname;
        }
        else {
            this->shortname = shortname;
        }
    }
}

unsigned BrowserFile::getFileSize(){
    return common::fileSize(getPath());
}

void BrowserFile::calcSize(){
    // Calculate the size (in Bytes, KB, MB or GB) of a BrowserFile, if it's a BrowserFolder, simply return its type
    if (common::getConf()->show_size){
        unsigned size = this->getFileSize();
        this->fileSize = common::beautifySize(size);
    }
    else {
        this->fileSize = getType();
    }
}

bool BrowserFile::isSelected(){
    return this->selected;
}

void BrowserFile::changeSelection(){
    this->selected = !this->selected;
}

string BrowserFile::getPath(){
    return (shortname.size() > 0)? parent+shortname : path;
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
    this->icon0 = NULL;
    if (filetype == FILE_PBP){
        Eboot* eboot = new Eboot(this->getPath());
        eboot->loadIcon();
        this->icon0 = eboot->getIcon();
        eboot->setIcon(NULL);
        delete eboot;
    }
    else if (filetype == FILE_ISO){
        Iso* iso = new Iso(this->getPath());
        iso->loadIcon();
        this->icon0 = iso->getIcon();
        iso->setIcon(NULL);
        delete iso;
    }
}

void BrowserFile::freeIcon(){
    this->Entry::freeIcon();
    this->icon0 = NULL;
}

void BrowserFile::loadPics(){
}

void BrowserFile::loadAVMedia(){
}

void BrowserFile::doExecute(){
}

BrowserFolder::BrowserFolder(string path, string shortname){
    size_t lastSlash = path.rfind('/', path.length()-2);
    this->path = path;
    this->name = path.substr(lastSlash+1, string::npos);
    this->parent = path.substr(0, lastSlash) + '/';
    this->icon0 = NULL;
    this->selected = false;
    this->fileSize = "Folder";
    this->filetype = FOLDER;
    if (shortname.size() > 0) shortname += '/';
    this->setShortName(shortname);
}

BrowserFolder::BrowserFolder(string parent, string name, string shortname){
    this->icon0 = NULL;
    this->path = parent + name + '/';
    this->name = name + '/';
    this->parent = parent;
    this->selected = false;
    this->fileSize = "Folder";
    this->filetype = FOLDER;
    if (shortname.size() > 0) shortname += '/';
    this->setShortName(shortname);
}

BrowserFolder::~BrowserFolder(){
}

char* BrowserFolder::getType(){
    return "FOLDER";
}

char* BrowserFolder::getSubtype(){
    return getType();
}
