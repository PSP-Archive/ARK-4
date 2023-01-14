#include "common.h"
#include <ctime>
#include <dirent.h>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include "controller.h"
#include "systemctrl.h"
#include "animations.h"

#define CONFIG_PATH "ARKMENU.CFG"
#define RESOURCES_LOAD_PLACE YA2D_PLACE_VRAM

using namespace common;

static ARKConfig ark_config = {0};
static Image* images[MAX_IMAGES];
static intraFont* font;
static MP3* sound_mp3;
static int argc;
static char **argv;
static float scrollX = 0.f;
static float scrollY = 0.f;
static float scrollXTmp = 0.f;
static int currentFont = 0;
/* Instance of the animations that are drawn on the menu */
static Anim* animations[ANIM_COUNT];

static bool flipControl = false;

char* fonts[] = {
    "FONT.PGF",
    "flash0:/font/ltn0.pgf",
    "flash0:/font/ltn1.pgf",
    "flash0:/font/ltn2.pgf",
    "flash0:/font/ltn3.pgf",
    "flash0:/font/ltn4.pgf",
    "flash0:/font/ltn5.pgf",
    "flash0:/font/ltn6.pgf",
    "flash0:/font/ltn7.pgf",
    "flash0:/font/ltn8.pgf",
    "flash0:/font/ltn9.pgf",
    "flash0:/font/ltn19.pgf",
    "flash0:/font/ltn11.pgf",
    "flash0:/font/ltn12.pgf",
    "flash0:/font/ltn13.pgf",
    "flash0:/font/ltn14.pgf",
    "flash0:/font/ltn15.pgf",
    "flash0:/font/jpn0.pgf",
    "flash0:/font/kr0.pgf"
};

static t_conf config;

void setArgs(int ac, char** av){
    argc = ac;
    argv = av;
}

void loadConfig(){
    FILE* fp = fopen(CONFIG_PATH, "rb");
    if (fp == NULL){
        resetConf();
        return;
    }
    fseek(fp, 0, SEEK_END);
    
    if (ftell(fp) != sizeof(t_conf)){
        fclose(fp);
        resetConf();
        return;
    }
    fseek(fp, 0, SEEK_SET);
    fread(&config, 1, sizeof(t_conf), fp);
    fclose(fp);
}

ARKConfig* common::getArkConfig(){
    if (ark_config.magic != ARK_CONFIG_MAGIC){
        sctrlHENGetArkConfig(&ark_config);
    }
    return &ark_config;
}

struct tm common::getDateTime(){
    struct tm  ts;
    time_t now = sceKernelLibcTime(NULL);
    ts = *localtime(&now);
    return ts;
}

void common::saveConf(){

    if (currentFont != config.font){
        if (!fileExists(fonts[config.font]))
            config.font = 1;
    
        intraFont* aux = font;
        font = NULL;
        intraFontUnload(aux);
        font = intraFontLoad(fonts[config.font], 0);
    }
    
    FILE* fp = fopen(CONFIG_PATH, "wb");
    fwrite(&config, 1, sizeof(t_conf), fp);
    fclose(fp);
}

t_conf* common::getConf(){
    return &config;
}

void common::resetConf(){
    config.fast_gameboot = 0;
    config.language = 0;
    config.font = 1;
    config.plugins = 1;
    config.scan_save = 0;
    config.scan_cat = 0;
    config.scan_dlc = 0;
    config.swap_buttons = 0;
    config.animation = 0;
    config.main_menu = 0;
    config.sort_entries = 1;
    config.show_recovery = 1;
    config.show_fps = 0;
    config.text_glow = 1;
    config.screensaver = 2;
}

void common::launchRecovery(){
    struct SceKernelLoadExecVSHParam param;
    char cwd[128];
    string recovery_path = string(getcwd((char*)cwd, sizeof(cwd))) + "/" + "RECOVERY.PBP";
    
    memset(&param, 0, sizeof(param));
    
    param.args = strlen(recovery_path.c_str()) + 1;
    param.argp = (char*)recovery_path.c_str();
    param.key = "game";
    sctrlKernelLoadExecVSHWithApitype(0x141, recovery_path.c_str(), &param);
}

static void missingFileHandler(const char* filename){
    
    static char msg[64];
    snprintf(msg, 64, "Error, missing file %s", filename);
    
    Controller pad;
    
    while (true){
    
        common::clearScreen(0);
        common::printText(10, 10, msg, GRAY_COLOR, SIZE_BIG, true);
        common::printText(10, 30, "Press cross or circle to launch recovery", GRAY_COLOR, SIZE_LITTLE, false);
        common::printText(10, 50, "Press triangle to exit", GRAY_COLOR, SIZE_LITTLE, false);
        common::flipScreen();
    
        pad.update();
        if (pad.cross() || pad.circle())
            common::launchRecovery();
        else if (pad.triangle())
            sceKernelExitGame();
    }
}

SceOff common::findPkgOffset(const char* filename, unsigned* size){
    
    FILE* pkg = fopen(PKG_PATH, "rb");
    if (pkg == NULL)
        return 0;
     
    fseek(pkg, 0, SEEK_END);
     
    unsigned pkgsize = ftell(pkg);
    unsigned size2 = 0;
     
    fseek(pkg, 0, SEEK_SET);

    if (size != NULL)
        *size = 0;

    unsigned offset = 0;
    char name[64];
           
    while (offset != 0xFFFFFFFF){
        fread(&offset, 1, 4, pkg);
        if (offset == 0xFFFFFFFF){
            fclose(pkg);
            missingFileHandler(filename);
            return 0;
        }
        unsigned namelength;
        fread(&namelength, 1, 4, pkg);
        fread(name, 1, namelength+1, pkg);
                   
        if (!strncmp(name, filename, namelength)){
            fread(&size2, 1, 4, pkg);
    
            if (size2 == 0xFFFFFFFF)
                size2 = pkgsize;

            if (size != NULL)
                *size = size2 - offset;
     
            fclose(pkg);
            return offset;
        }
    }
    missingFileHandler(PKG_PATH);
    return 0;
}

void* common::readFromPKG(const char* filename, unsigned* size){

    unsigned mySize;
    
    if (size == NULL)
        size = &mySize;

    unsigned offset = findPkgOffset(filename, size);
    
    FILE* fp = fopen(PKG_PATH, "rb");
    
    if (offset == 0 || fp == NULL){
        fclose(fp);
        return NULL;
    }
    
    void* data = malloc(*size);
    fseek(fp, offset, SEEK_SET);
    fread(data, 1, *size, fp);
    fclose(fp);
    return data;
}


int common::getArgc(){
    return argc;
}

char** common::getArgv(){
    return argv;
}

bool common::has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

u32 common::getMagic(const char* filename, unsigned int offset){
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    u32 magic;
    fseek(fp, offset, SEEK_SET);
    fread(&magic, 4, 1, fp);
    fclose(fp);
    return magic;
}

void common::loadData(int ac, char** av){

    argc = ac;
    argv = av;
    
    animations[0] = new PixelAnim();
    animations[1] = new Waves();
    animations[2] = new Sprites();
    animations[3] = new Fire();
    animations[4] = new Tetris();
    animations[5] = new Matrix();
    animations[6] = new Hacker();
    animations[7] = new BSoD();
    animations[8] = new SnowAnim();
    animations[9] = new GoLAnim();
    animations[10] = new NoAnim();
    
    images[IMAGE_BG] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("DEFBG.PNG"));
    images[IMAGE_WAITICON] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("WAIT.PNG"));
    
    common::clearScreen(CLEAR_COLOR);
    images[IMAGE_BG]->draw(0, 0);
    images[IMAGE_WAITICON]->draw(
        (480 - images[IMAGE_WAITICON]->getTexture()->width)/2,
        (272 - images[IMAGE_WAITICON]->getTexture()->height)/2
    );
    common::flipScreen();
    
    images[IMAGE_LOADING] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("LOADING.PNG"));
    images[IMAGE_SPRITE] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("SPRITE.PNG"));
    images[IMAGE_NOICON] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("NOICON.PNG"));
    images[IMAGE_GAME] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("GAME.PNG"));
    images[IMAGE_FTP] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("FTP.PNG"));
    images[IMAGE_SETTINGS] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("SETTINGS.PNG"));
    images[IMAGE_BROWSER] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("BROWSER.PNG"));
    images[IMAGE_DIALOG] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("BOX.PNG"));
    images[IMAGE_EXIT] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("EXIT.PNG"));
    images[IMAGE_PLUGINS] = new Image(PKG_PATH, RESOURCES_LOAD_PLACE, findPkgOffset("PLUGINS.PNG"));
    
    for (int i=0; i<MAX_IMAGES; i++){
        images[i]->swizzle();
        images[i]->is_system_image = true;
    }
    
    unsigned mp3_size;
    void* mp3_buffer = readFromPKG("SOUND.MP3", &mp3_size);
    sound_mp3 = new MP3(mp3_buffer, mp3_size);
    
    loadConfig();
    
    if (!fileExists(fonts[config.font]))
        config.font = 1;
    font = intraFontLoad(fonts[config.font], INTRAFONT_CACHE_ALL);
    
    currentFont = config.font;
    
}

void common::deleteData(){
    for (int i=0; i<MAX_IMAGES; i++)
        delete images[i];
    intraFontUnload(font);
    delete sound_mp3;
}

bool common::fileExists(const std::string &path){
    struct stat sb;
    return (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode));
}

bool common::folderExists(const std::string &path){
    struct stat sb;
    return (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
}

long common::fileSize(const std::string &path){
    struct stat stat_buf;
    int rc = stat(path.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

Image* common::getImage(int which){
    return (which < MAX_IMAGES)? images[which] : images[IMAGE_LOADING];
}

bool common::isSharedImage(Image* img){

    if (img->is_system_image)
        return true;

    for (int i=0; i<MAX_IMAGES; i++){
        if (images[i] == img)
            return true;
    }
    return false;
}

intraFont* common::getFont(){
    return font;
}

MP3* common::getMP3Sound(){
    return sound_mp3;
}

void common::playMenuSound(){
    playMP3File(NULL, common::getMP3Sound()->getBuffer(), common::getMP3Sound()->getBufferSize());
}

void common::printText(float x, float y, const char* text, u32 color, float size, int glow, int scroll){

    if (font == NULL)
        return;

    u32 secondColor = BLACK_COLOR;
    u32 arg5 = INTRAFONT_WIDTH_VAR;
    
    if (glow && config.text_glow){
        float t = (float)((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
        int val = (t < 0.5f) ? t*511 : (1.0f-t)*511;
        secondColor = (0xFF<<24)+(val<<16)+(val<<8)+(val);
    }
    if (int(scroll)){
        arg5 = INTRAFONT_SCROLL_LEFT;
    }
    
    intraFontSetStyle(font, size, color, secondColor, 0.f, arg5);

    if (int(scroll)){
        if (x != scrollX || y != scrollY){
            scrollX = x;
            scrollXTmp = x;
            scrollY = y;
        }
        scrollXTmp = intraFontPrintColumn(font, scrollXTmp, y, 200, text);
    }
    else
        intraFontPrint(font, x, y, text);
    
}

void common::clearScreen(u32 color){
    ya2d_start_drawing();
    ya2d_clear_screen(color);
    flipControl = true;
}

void common::drawBorder(){
    ya2d_draw_rect(10, 20, 80, 254, GRAY_COLOR, 0);
    ya2d_draw_rect(170, 20, 80, 254, GRAY_COLOR, 0);
    ya2d_draw_rect(330, 20, 80, 254, GRAY_COLOR, 0);
}

void common::drawScreen(){
    if (canDrawBackground())
        getImage(IMAGE_BG)->draw(0, 0);

    animations[getConf()->animation]->draw();
}

bool common::canDrawBackground(){
    return animations[getConf()->animation]->drawBackground();
}

void common::flipScreen(){
    if (!flipControl)
        return;

    sceDisplayWaitVblankStart();
    ya2d_finish_drawing();
    ya2d_swapbuffers();
    flipControl = false;
};

void common::upperString(char* str){
    while (*str){
        if (*str >= 'a' && *str <= 'z')
            *str -= 0x20;
        str++;
    }
}

int common::maxString(string* strings, int n_strings){
    int max = 0;
    for (int i = 0; i<n_strings; i++){
        int len = strings[i].length();
        if (len > max)
            max = len;
    }
    return max;
}

std::string common::getExtension(std::string path){
    std::string ext = path.substr(path.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

bool common::canInstallGame(){
    static char* test_dir = "ms0:/PSP/GAME/ARKTEST/";
    sceIoMkdir(test_dir, 0777);
    return (sceIoRmdir(test_dir) >= 0);
}
