#include "umd.h"


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

void UMD::loadIcon(){
    Image* icon = NULL;
    
    sceUmdWaitDriveStat(UMD_WAITFORINIT);
    
    if (common::fileExists(UMD_ICON0_PATH))
        icon = new Image(UMD_ICON0_PATH, YA2D_PLACE_RAM);
    
    if (icon == NULL)
        sceKernelDelayThread(50000);
    icon = (icon == NULL)? common::getImage(IMAGE_NOICON) : icon;
    icon->swizzle();
    this->icon0 = icon;
}

SfoInfo UMD::getSfoInfo(){
    SfoInfo info = this->Entry::getSfoInfo();
    unsigned int size = common::fileSize(UMD_SFO_PATH);
    unsigned char* sfo_buffer = (unsigned char*)malloc(size);

    if (sfo_buffer){

        FILE* fp = fopen(UMD_SFO_PATH, "rb");
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

void UMD::getTempData1(){
    this->pic0 = NULL;
    this->pic1 = NULL;
    
    int size;
    
    // grab pic0.png
    if (common::fileExists(UMD_PIC0_PATH))
        this->pic0 = new Image(UMD_PIC0_PATH, YA2D_PLACE_RAM);

    // grab pic1.png
    if (common::fileExists(UMD_PIC1_PATH))
        this->pic1 = new Image(UMD_PIC1_PATH, YA2D_PLACE_RAM);
}

void UMD::getTempData2(){

    this->icon1 = NULL;
    this->snd0 = NULL;
    this->at3_size = 0;
    this->icon1_size = 0;

    int size;

    // grab snd0.at3
    if (common::fileExists(UMD_SND0_PATH)){
        size = common::fileSize(UMD_SND0_PATH);
        this->snd0 = malloc(size);
        memset(this->snd0, 0, size);
        this->at3_size = size;
        // read file
        FILE* src = fopen(UMD_SND0_PATH, "rb");
        fread(this->snd0, size, 1, src);
        fclose(src);
    }

    // grab icon1.pmf
    if (common::fileExists(UMD_ICON1_PATH)){
        size = common::fileSize(UMD_ICON1_PATH);
        this->icon1 = malloc(size);
        memset(this->icon1, 0, size);
        this->icon1_size = size;
        // read file
        FILE* src = fopen(UMD_ICON1_PATH, "rb");
        fread(this->icon1, size, 1, src);
        fclose(src);
    }

}

void UMD::doExecute(){
    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    
    param.size = sizeof(param);
    param.argp = (char*)UMD_EBOOT_BIN;
    param.args = 33;
    param.key = "game";
    
    sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
    sctrlSESetBootConfFileIndex(MODE_UMD);
    sctrlSESetUmdFile("");
    
    sctrlKernelLoadExecVSHWithApitype(UMD_APITYPE, UMD_EBOOT_BIN, &param);
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
