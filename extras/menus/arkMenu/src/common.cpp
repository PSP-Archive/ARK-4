#include "common.h"
#include <ctime>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include "controller.h"
#include "systemctrl.h"
#include "animations.h"

#define RESOURCES_LOAD_PLACE YA2D_PLACE_VRAM

using namespace common;

extern "C" int kuKernelGetModel();

static ARKConfig ark_config = {0};
static Image* images[MAX_IMAGES];
/* Common browser images */
static Image* checkbox[2];
static Image* icons[MAX_FILE_TYPES];

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

static int psp_model;

static string theme_path = THEME_NAME;

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
    memset(&config, 0, sizeof(config));
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
    config.redirect_ms0 = 0;
    config.startbtn = 0;
    config.menusize = 0;
}

void common::launchRecovery(){
    struct SceKernelLoadExecVSHParam param;
    char cwd[128];
    string recovery_path = string(ark_config.arkpath) + "RECOVERY.PBP";
    
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
    
    FILE* pkg = fopen(theme_path.c_str(), "rb");
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
    missingFileHandler(theme_path.c_str());
    return 0;
}

void* common::readFromPKG(const char* filename, unsigned* size){

    unsigned mySize;
    
    if (size == NULL)
        size = &mySize;

    unsigned offset = findPkgOffset(filename, size);
    
    FILE* fp = fopen(theme_path.c_str(), "rb");
    
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

int common::getPspModel(){
    return psp_model;
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

static bool loading_theme = false;

static int loading_theme_thread(SceSize argc, void* argp){
    float angle = 1.0;
    while (loading_theme){
        common::clearScreen(CLEAR_COLOR);
        images[IMAGE_BG]->draw(0, 0);
        images[IMAGE_WAITICON]->draw_rotate(
            (480 - images[IMAGE_WAITICON]->getTexture()->width)/2,
            (272 - images[IMAGE_WAITICON]->getTexture()->height)/2,
            angle
        );
        angle+=0.2;
        common::flipScreen();
    }
    sceKernelExitDeleteThread(0);
    return 0;
}

void common::loadTheme(){
    images[IMAGE_BG] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("DEFBG.PNG"));
    images[IMAGE_WAITICON] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("WAIT.PNG"));
    
    loading_theme = true;
    SceUID loading_thread = sceKernelCreateThread("theme_thread", &loading_theme_thread, 0x10, 0x8000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(loading_thread, 0, NULL);

    
    images[IMAGE_LOADING] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("LOADING.PNG"));
    images[IMAGE_SPRITE] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("SPRITE.PNG"));
    images[IMAGE_NOICON] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("NOICON.PNG"));
    images[IMAGE_GAME] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("GAME.PNG"));
    images[IMAGE_FTP] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("FTP.PNG"));
    images[IMAGE_SETTINGS] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("SETTINGS.PNG"));
    images[IMAGE_BROWSER] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("BROWSER.PNG"));
    images[IMAGE_DIALOG] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("BOX.PNG"));
    images[IMAGE_EXIT] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("EXIT.PNG"));
    images[IMAGE_PLUGINS] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("PLUGINS.PNG"));

    icons[FOLDER] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("FOLDER.PNG"));
    icons[FILE_BIN] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("FILE.PNG"));
    icons[FILE_TXT] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("TXT.PNG"));
    icons[FILE_PBP] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("PBP.PNG"));
    icons[FILE_PRX] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("PRX.PNG"));    
    icons[FILE_ISO] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("ISO.PNG"));
    icons[FILE_ZIP] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("ZIP.PNG"));
    icons[FILE_MUSIC] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("MUSIC.PNG"));
    icons[FILE_PICTURE] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("PICTURE.PNG"));

    checkbox[1] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("CHECK.PNG"));
    checkbox[0] = new Image(theme_path, YA2D_PLACE_VRAM, common::findPkgOffset("UNCHECK.PNG"));
    
    for (int i=0; i<MAX_IMAGES; i++){
        images[i]->swizzle();
        images[i]->is_system_image = true;
    }
    
    unsigned mp3_size;
    void* mp3_buffer = readFromPKG("SOUND.MP3", &mp3_size);
    sound_mp3 = new MP3(mp3_buffer, mp3_size);

    loading_theme = false;
    sceKernelWaitThreadEnd(loading_thread, NULL);
    sceKernelDeleteThread(loading_thread);
}

void common::loadData(int ac, char** av){

    argc = ac;
    argv = av;

    psp_model = kuKernelGetModel();

    sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
    sceUtilityLoadModule(PSP_MODULE_AV_MP3);
    
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
    
    loadTheme();
    
    loadConfig();
    
    if (!fileExists(fonts[config.font]))
        config.font = 1;
    font = intraFontLoad(fonts[config.font], INTRAFONT_CACHE_ALL);
    
    currentFont = config.font;
    
}

void common::deleteTheme(){
    for (int i=0; i<MAX_IMAGES; i++){
        delete images[i];
    }
    for (int i=0; i<MAX_FILE_TYPES; i++){
        delete icons[i];
    }
    delete checkbox[0];
    delete checkbox[1];
    delete sound_mp3;
}

void common::deleteData(){
    deleteTheme();
    intraFontUnload(font);
}

void common::setThemePath(char* path){
    if (path == NULL) theme_path = THEME_NAME;
    else theme_path = path;
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
    return rc == 0 ? stat_buf.st_size : 0;
}

u64 common::deviceSize(const std::string path){
    struct DeviceSize {
        u32 maxClusters;
        u32 freeClusters;
        u32 maxSectors;
        u32 sectorSize;
        u32 sectorCount;
    } devsize;
    void* command = (void*)&devsize;
    memset(&devsize, 0, sizeof(devsize));
    string devpath = path.substr(0, path.find(":")+1);
    sceIoDevctl(devpath.c_str(), 0x02425818, &command, sizeof(command), NULL, 0);
    return (u64)devsize.freeClusters*(u64)devsize.sectorSize*(u64)devsize.sectorCount;
}

string common::beautifySize(u64 size){
    ostringstream txt;

    if (size < 1024)
        txt<<size<<" Bytes";
    else if (1024 < size && size < 1048576)
        txt<<float(size)/1024.f<<" KB";
    else if (1048576 < size && size < 1073741824)
        txt<<float(size)/1048576.f<<" MB";
    else
        txt<<float(size)/1073741824.f<<" GB";
    return txt.str();
}

Image* common::getImage(int which){
    return (which < MAX_IMAGES)? images[which] : images[IMAGE_LOADING];
}

Image* common::getIcon(int which){
    return (which < MAX_FILE_TYPES)? icons[which] : images[IMAGE_LOADING];
}

Image* common::getCheckbox(int which){
    return checkbox[which&1];
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
    sound_mp3->play();
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

int common::calcTextWidth(const char* text, float size){
    intraFontSetStyle(font, size, 0, 0, 0.f, INTRAFONT_WIDTH_VAR);
    float w = intraFontMeasureText(font, text) + size*strlen(text);
    return (int)ceil(w);
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
