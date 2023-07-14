#include "menu.h"
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <sstream>

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;
static SEConfig _se_conf;
SEConfig* se_config = &_se_conf;
static string ark_version;

Menu::Menu(){

    this->readEbootList("ms0:/PSP/GAME/");
    this->readEbootList("ms0:/PSP/APPS/");
    this->index = 0;
    this->start = 0;
    this->txt = NULL;
}

void Menu::readEbootList(string path){

    struct dirent* dit;
    DIR* dir = opendir(path.c_str());
    
    if (dir == NULL)
        return;
        
    while ((dit = readdir(dir))){
    
        string fullpath = fullPath(path, dit->d_name);
        if (strcmp(dit->d_name, ".") == 0) continue;
        if (strcmp(dit->d_name, "..") == 0) continue;
        if (strcmp(dit->d_name, "SCPS10084") == 0) continue;
        if (common::fileExists(path+dit->d_name)) continue;
        if (!isPOPS(fullpath)) continue;
        
        this->eboots.push_back(new Entry(fullpath));
    }
    closedir(dir);
}

string Menu::fullPath(string path, string app){
    // Return the full path of a homebrew given only the homebrew name
    if (common::fileExists(app))
        return app; // it's already a full path

    else if (common::fileExists(path+app+"/FBOOT.PBP"))
        return path+app+"/FBOOT.PBP";

    else if (common::fileExists(path+app+"/EBOOT.PBP"))
        return path+app+"/EBOOT.PBP";
    
    else if (common::fileExists(path+app+"/VBOOT.PBP"))
        return path+app+"/VBOOT.PBP";

    return "";
}

int Menu::getEbootType(const char* path){

    int ret = UNKNOWN_TYPE;

    FILE* fp = fopen(path, "rb");
    if (fp == NULL)
        return ret;
    
    u32 magic;
    fseek(fp, 0, SEEK_SET);
    fread(&magic, 1, sizeof(magic), fp);

    if (magic != PBP_MAGIC){
        fclose(fp);
        return ret;
    }

    fseek(fp, 48, SEEK_SET);
    
    u32* labelstart = new u32;
    u32* valuestart = new u32;
    u32* valueoffset = new u32;
    u32* entries = new u32;
    u16* labelnameoffset = new u16;
    char* labelname = (char*)malloc(9);
    u16* categoryType = new u16;
    int cur;

    fread(labelstart, 4, 1, fp);
    fread(valuestart, 4, 1, fp);
    fread(entries, 4, 1, fp);
    while (*entries>0 && ret == UNKNOWN_TYPE){
    
        (*entries)--;
        cur = ftell(fp);
        fread(labelnameoffset, 2, 1, fp);
        fseek(fp, *labelnameoffset + *labelstart + 40, SEEK_SET);
        fread(labelname, 8, 1, fp);

        if (!strncmp(labelname, "CATEGORY", 8)){
            fseek(fp, cur+12, SEEK_SET);
            fread(valueoffset, 1, 4, fp);
            fseek(fp, *valueoffset + *valuestart + 40, SEEK_SET);
            fread(categoryType, 2, 1, fp);
            switch(*categoryType){
            case HMB_CAT:        ret = TYPE_HOMEBREW;    break;
            case PSN_CAT:        ret = TYPE_PSN;            break;
            case PS1_CAT:        ret = TYPE_POPS;        break;
            default:                                    break;
            }
        }
        fseek(fp, cur+16, SEEK_SET);
    }
    fclose(fp);
    return ret;
}

bool Menu::isPOPS(string path){
    return getEbootType(path.c_str()) == TYPE_POPS;
}

void Menu::updateScreen(){
    clearScreen(CLEAR_COLOR);
    blitAlphaImageToScreen(0, 0, 480, 272, common::getBG(), 0, 0);
    
    for (int i=this->start; i<min(this->start+3, (int)eboots.size()); i++){
        int offset = 8 + (90 * (i-this->start));
        blitAlphaImageToScreen(0, 0, eboots[i]->getIcon()->imageWidth, \
            eboots[i]->getIcon()->imageHeight, eboots[i]->getIcon(), 10, offset);
        if (i == this->index)
            fillScreenRect(WHITE_COLOR, 200, offset+30+TEXT_HEIGHT, min((int)eboots[i]->getPath().size()*TEXT_WIDTH, 280), 1);
    }
    guStart();
    for (int i=this->start; i<min(this->start+3, (int)eboots.size()); i++){
        int offset = 8 + (90 * (i-this->start));
        if (i == this->index)
            this->txt->draw(offset);
        else
            common::printText(200, offset+30, eboots[i]->getName().c_str());
    }

    common::printText(2, 2, ark_version.c_str());

    common::flip();
}

void Menu::updateTextAnim(){
    if (this->txt != NULL)
        delete this->txt;
    this->txt = new TextAnim(eboots[this->index]->getName(), eboots[this->index]->getPath());
}

void Menu::moveDown(){
    if (this->index == eboots.size())
        return;
    else if (this->index-this->start == 2){
        if (this->index+1 < eboots.size()-1)
            this->index++;
        if (this->start+4 < eboots.size())
            this->start++;
    }
    else if (this->index+1 < eboots.size())
        this->index++;
    updateTextAnim();
}

void Menu::moveUp(){
    if (this->index == 0)
        return;
    else if (this->index == this->start){
        this->index--;
        if (this->start>0)
            this->start--;
    }
    else
        this->index--;
    updateTextAnim();
}

void Menu::control(){

    Controller control;
    
    while(true){
        updateScreen();
        control.update();
        if (control.down())
            moveDown();
        else if (control.up())
            moveUp();
        else if (control.cross()){
            if (eboots[this->index]->run()){
                loadGame();
                break;
            }
        }        
        else if (control.circle()){
            break;
        }
        else if (control.select()){
            rebootMenu();
        }
    }
}

void Menu::loadGame(){

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(SceKernelLoadExecVSHParam));
    
    char path[256];
    strcpy(path, eboots[this->index]->getPath().c_str());
    
    int runlevel = (path[0]=='e' && path[1]=='f')? POPS_RUNLEVEL_GO : POPS_RUNLEVEL;
    
    param.args = strlen(path) + 1;
    param.argp = path;
    param.key = "pops";
    debugScreen(path);
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}

void Menu::rebootMenu(){

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(SceKernelLoadExecVSHParam));

    char path[256];
    strcpy(path, ark_config->arkpath);
    strcat(path, ark_config->launcher);

    int runlevel = 0x141;
    
    param.args = strlen(path) + 1;
    param.argp = path;
    param.key = "game";
    debugScreen(path);
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}

void Menu::run(){
    if (eboots.size() == 0)
        return;
    
    // get ARK config and version
    sctrlSEGetConfig(se_config);
    sctrlHENGetArkConfig(ark_config);
    u32 ver = sctrlHENGetVersion(); // ARK's full version number
    u32 major = (ver&0xFF000000)>>24;
    u32 minor = (ver&0xFF0000)>>16;
    u32 micro = (ver&0xFF00)>>8;
    u32 rev   = sctrlHENGetMinorVersion();

    stringstream version;
	version << "ARK " << major << "." << minor;
    if (micro>9) version << "." << micro;
    else if (micro>0) version << ".0" << micro;
    if (rev) version << " r" << rev;
    version << " " << ark_config->exploit_id;
    #ifdef DEBUG
	version << " DEBUG";
	#endif

    version << " - Memory Stick Speedup: " << ((se_config->msspeed)? "Enabled" : "Disabled");

    ark_version = version.str();

    updateTextAnim();
    control();
}

Menu::~Menu(){
    delete this->txt;
    this->eboots.clear();
}
