#include "umd.h"
#include "iso.h"
#include <systemctrl.h>

extern "C" int sctrlKernelLoadExecVSHDisc(const char*, struct SceKernelLoadExecVSHParam*);

UMD::UMD(){
    this->name = "UMD Drive";
    this->path = "disc0:/";
    this->icon0 = common::getImage(IMAGE_WAITICON);
    sceUmdActivate(1, "disc0:");
}

UMD::~UMD(){
    if (this->icon0 != common::getImage(IMAGE_NOICON) && this->icon0 != common::getImage(IMAGE_WAITICON))
        delete this->icon0;
    
    sceUmdDeactivate(1, "disc0:");
}

string UMD::getName(){
    return name;
}

void UMD::loadIcon(){
    Image* icon = NULL;
    
    // el hefy UMD's are slooooowwww, but if it is initialized it does not wait the full 20 secs which is nice.
    sceUmdWaitDriveStatWithTimer(UMD_WAITFORINIT, 20000000);

    if (common::fileExists(UMD_GAME_ICON0_PATH)) {
        icon = new Image(UMD_GAME_ICON0_PATH, YA2D_PLACE_RAM);
    }

    icon = (icon == NULL)? common::getImage(IMAGE_NOICON) : icon;
    icon->swizzle();
    this->icon0 = icon;
}

SfoInfo UMD::getSfoInfo(){
    SfoInfo info = this->Entry::getSfoInfo();
    unsigned int size = 0;
    string UMD_SFO_PATH;
    if(common::fileExists(UMD_GAME_SFO_PATH)) {
        size = common::fileSize(UMD_GAME_SFO_PATH);
        UMD_SFO_PATH = UMD_GAME_SFO_PATH;
    }
    
    unsigned char* sfo_buffer = (unsigned char*)malloc(size);

    if (sfo_buffer){

        FILE* fp = fopen(UMD_SFO_PATH.c_str(), "rb");
        fread(sfo_buffer, 1, size, fp);
        fclose(fp);

        int title_size = sizeof(info.title);
        Entry::getSfoParam(sfo_buffer, size, "TITLE", (unsigned char*)(info.title), &title_size);
        
        int id_size = sizeof(info.gameid);
        Entry::getSfoParam(sfo_buffer, size, "DISC_ID", (unsigned char*)(info.gameid), &id_size);

        free(sfo_buffer);
    }
    return info;
}

void UMD::loadPics(){
    this->pic0 = NULL;
    this->pic1 = NULL;
    
    // grab pic0.png
    if (common::fileExists(UMD_GAME_PIC0_PATH))
        this->pic0 = new Image(UMD_GAME_PIC0_PATH, YA2D_PLACE_RAM);
    // grab pic1.png
    if (common::fileExists(UMD_GAME_PIC1_PATH))
        this->pic1 = new Image(UMD_GAME_PIC1_PATH, YA2D_PLACE_RAM);
}

void UMD::loadAVMedia(){

    this->icon1 = NULL;
    this->snd0 = NULL;
    this->at3_size = 0;
    this->icon1_size = 0;

    int size;

    // grab snd0.at3
    if (common::fileExists(UMD_GAME_SND0_PATH)){
        size = common::fileSize(UMD_GAME_SND0_PATH);
        this->snd0 = malloc(size);
        memset(this->snd0, 0, size);
        this->at3_size = size;
        // read file
        FILE* src = fopen(UMD_GAME_SND0_PATH, "rb");
        fread(this->snd0, size, 1, src);
        fclose(src);
    }
   
    // grab icon1.pmf
    if (common::fileExists(UMD_GAME_ICON1_PATH)){
        size = common::fileSize(UMD_GAME_ICON1_PATH);
        this->icon1 = malloc(size);
        memset(this->icon1, 0, size);
        this->icon1_size = size;
        // read file
        FILE* src = fopen(UMD_GAME_ICON1_PATH, "rb");
        fread(this->icon1, size, 1, src);
        fclose(src);
    }
}

void UMD::doExecute(){
    int apitype = 0x160;
    char path[128];

    if (!hasUpdate(path)){
        apitype = 0x120;
        strcpy(path, UMD_EBOOT_BIN);
    }

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.argp = path;
    param.args = strlen(path)+1;
    param.key = "game";
   
    sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
    sctrlSESetBootConfFileIndex(MODE_UMD);

    sctrlSESetUmdFile("");
    
    sctrlKernelLoadExecVSHWithApitype(apitype, path, &param);
}

bool UMD::hasUpdate(char* update_path){
    char game_id[10];
    memset(game_id, 0, sizeof(game_id));
    readGameId(game_id);

    // try to find the update file
    sprintf(update_path, "ms0:/PSP/GAME/%s/PBOOT.PBP", game_id);
    SceIoStat stat;
    int res = sceIoGetstat(update_path, &stat);
    if (res >= 0){
        // found
        return 1;
    }
    return 0;
}

int UMD::readGameId(char* game_id){
    // Open Disc Identifier
    int disc = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);
    // Opened Disc Identifier
    if(disc >= 0)
    {
        // Read Country Code
        sceIoRead(disc, game_id, 4);
        
        // Skip Delimiter
        sceIoLseek32(disc, 1, PSP_SEEK_CUR);
        
        // Read Game ID
        sceIoRead(disc, game_id + 0x4, 5);
        
        // Close Disc Identifier
        sceIoClose(disc);
        return 1;
    }
    return 0;
}

char* UMD::getType(){
    return "UMD";
}

char* UMD::getSubtype(){
    return "UMD";
}

bool UMD::isUMD(){
    return (bool)sceUmdCheckMedium();
}
