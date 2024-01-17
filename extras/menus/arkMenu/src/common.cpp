#include "common.h"
#include <ctime>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include "controller.h"
#include <systemctrl.h>
#include <pspiofilemgr.h>
#include <kubridge.h>
#include "animations.h"
#include "system_mgr.h"
#include "lang.h"
#include "browser.h"

#define RESOURCES_LOAD_PLACE YA2D_PLACE_VRAM

using namespace common;

bool common::is_recovery = false;

extern "C"{
    int kuKernelGetModel();
}

struct tm today;

static ARKConfig ark_config = {0};
static Image* images[MAX_IMAGES];
/* Common browser images */
static Image* checkbox[2];
static Image* icons[MAX_FILE_TYPES];

extern float text_size;
extern int altFontId;
extern intraFont* altFont;
extern intraFont* font;
static MP3* sound_mp3 = NULL;
static int argc;
static char **argv;
static int currentFont = 0;
static int currentLang = 0;
static int currentApp = 0; // Games
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
    "flash0:/font/ltn10.pgf",
    "flash0:/font/ltn11.pgf",
    "flash0:/font/ltn12.pgf",
    "flash0:/font/ltn13.pgf",
    "flash0:/font/ltn14.pgf",
    "flash0:/font/ltn15.pgf",
    //"flash0:/font/jpn0.pgf",
    //"flash0:/font/kr0.pgf"
};

static char* lang_files[] = {
    "lang_en.json",
    "lang_es.json",
    "lang_de.json",
    "lang_fr.json",
    "lang_pt.json",
    "lang_it.json",
    "lang_nl.json",
    "lang_ru.json",
    "lang_ukr.json",
    "lang_ro.json",
    "lang_lat.json",
    "lang_jp.json",
    "lang_ko.json",
    "lang_cht.json",
    "lang_chs.json",
    "lang_pol.json",
    "lang_latgr.json"
    //"lang_grk.json",
    //"lang_thai.json",
};

static t_conf config;

static volatile bool do_loading_thread = false;
static volatile SceUID load_thread_id = -1;

static void dummyMissingHandler(const char* filename){

}

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
    memset(&config, 0, sizeof(t_conf));
    fseek(fp, 0, SEEK_SET);
    fread(&config, 1, sizeof(t_conf), fp);
    fclose(fp);
    if (today.tm_mday == 1 && today.tm_mon == 3)
        config.language = 10;
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

static void loadFont(){
    unsigned offset=0, size=0;
    if (config.font == 0){
        offset = findPkgOffset(fonts[0], &size, "LANG.ARK", &dummyMissingHandler);
        if (offset && size){
            fonts[0] = "LANG.ARK"; // found font in lang package
        }
        else if ((offset = findPkgOffset(fonts[0], &size, "THEME.ARK", &dummyMissingHandler)) != 0 ){
            fonts[0] = "THEME.ARK"; // found font in theme package
        }
        else if (!fileExists(fonts[0])){
            if (altFont){
                intraFont* aux = font;
                if (aux) intraFontUnload(aux);
                font = altFont;
                config.font = altFontId;
                return;
            }
            else{
                config.font = 1;
            }
        }
    }

    // offload current font
    intraFont* aux = font;
    if (aux) intraFontUnload(aux);
    // load new font
    if (config.font == 0 && !altFont) altFont = intraFontLoadEx(fonts[1], INTRAFONT_CACHE_ALL, 0, 0);
    font = intraFontLoadEx(fonts[config.font], INTRAFONT_CACHE_ALL, offset, size);
    intraFontSetEncoding(font, INTRAFONT_STRING_UTF8);
    // set alt font
    if (altFont) intraFontSetAltFont(font, altFont);
    currentFont = config.font;
}

void common::saveConf(){

    SystemMgr::pauseDraw();

    // reload language
    if (currentLang != config.language){
        if (!Translations::loadLanguage(lang_files[config.language])){
            config.language = 0;
        }
        currentLang = config.language;
    }

    // reload font
    if (currentFont != config.font || font == NULL){
        loadFont();
    }

    // swap apps
    if (!is_recovery && currentApp != config.main_menu){
        SystemEntry* ent0 = SystemMgr::getSystemEntry(0);
        SystemEntry* ent1 = SystemMgr::getSystemEntry(1);
        SystemMgr::setSystemEntry(ent1, 0);
        SystemMgr::setSystemEntry(ent0, 1);
        currentApp = config.main_menu;
    }

    SystemMgr::resumeDraw();

    strcpy(config.browser_dir, Browser::getInstance()->getCWD());
    
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
    config.text_glow = 3;
    config.screensaver = 2;
    config.redirect_ms0 = 0;
    config.startbtn = 0;
    config.menusize = 0;
    config.show_path = 0;
    config.browser_icon0 = 1;
}

void common::launchRecovery(const char* path){
    string fakent = string(common::getArkConfig()->arkpath) + VBOOT_PBP;
    if (fakent != path && common::fileExists(path)){
        struct SceKernelLoadExecVSHParam param;
        
        memset(&param, 0, sizeof(param));
        
        int runlevel = HOMEBREW_RUNLEVEL;
        
        param.args = strlen(path) + 1;
        param.argp = (char*)path;
        param.key = "game";
        sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
    }
    else {
        string recovery_prx = string(common::getArkConfig()->arkpath) + RECOVERY_PRX;
        SceUID modid = kuKernelLoadModule(recovery_prx.c_str(), 0, NULL);
        if (modid >= 0){
            int res = sceKernelStartModule(modid, recovery_prx.size() + 1, (void*)recovery_prx.c_str(), NULL, NULL);
            if (res >= 0){
                while (1){sceKernelDelayThread(1000000);}; // wait for recovery to finish
            }
        }
    }
}

static void missingFileHandler(const char* filename){

    if (load_thread_id >= 0){
        stopLoadingThread();
    }

    if (!font){
        font = intraFontLoad(fonts[1], INTRAFONT_CACHE_ASCII);
        intraFontSetEncoding(font, INTRAFONT_STRING_UTF8);
    }
    
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
        if (pad.cross() || pad.circle()){
            string recovery_path = string(ark_config.arkpath) + ARK_RECOVERY;
            common::launchRecovery(recovery_path.c_str());
        }
        else if (pad.triangle())
            sctrlKernelExitVSH(NULL);
    }
}

SceOff common::findPkgOffset(const char* filename, unsigned* size, const char* pkgpath, void (*missinghandler)(const char*)){
    
    if (pkgpath == NULL)
        pkgpath = theme_path.c_str();

    if (missinghandler == NULL)
        missinghandler = &missingFileHandler;

    FILE* pkg = fopen(pkgpath, "rb");
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
            missinghandler(filename);
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
    missinghandler(pkgpath);
    return 0;
}

void* common::readFromPKG(const char* filename, unsigned* size, const char* pkgpath){

    unsigned mySize;
    
    if (size == NULL)
        size = &mySize;

    if (pkgpath == NULL)
        pkgpath = theme_path.c_str();

    unsigned offset = findPkgOffset(filename, size, pkgpath, &dummyMissingHandler);
    
    FILE* fp = fopen(pkgpath, "rb");
    
    if (offset == 0 || fp == NULL){
        fclose(fp);
        *size = 0;
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

static int loading_thread(SceSize argc, void* argp){
    float angle = 1.0;
    while (do_loading_thread){
        common::clearScreen(CLEAR_COLOR);
        images[IMAGE_BG]->draw(0, 0);
        images[IMAGE_WAITICON]->draw_rotate(
            (480 - images[IMAGE_WAITICON]->getTexture()->width)/2,
            (272 - images[IMAGE_WAITICON]->getTexture()->height)/2,
            angle
        );
        angle+=0.2;
        common::flipScreen();
        sceKernelDelayThread(0);
    }
    sceKernelExitDeleteThread(0);
    return 0;
}

void common::startLoadingThread(){
    do_loading_thread = true;
    load_thread_id = sceKernelCreateThread("theme_thread", &loading_thread, 0x10, 0x8000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(load_thread_id, 0, NULL);
}

void common::stopLoadingThread(){
    do_loading_thread = false;
    sceKernelWaitThreadEnd(load_thread_id, NULL);
    sceKernelDeleteThread(load_thread_id);
    load_thread_id = -1;
}

void common::loadTheme(){
	SceIoStat stat;
	string path = string(ark_config.arkpath) + "BG.PNG";
	images[IMAGE_BG] = (sceIoGetstat(path.c_str(), &stat) >= 0) ? new Image(path.c_str()) : new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("DEFBG.PNG"));
    images[IMAGE_WAITICON] = new Image(theme_path, RESOURCES_LOAD_PLACE, findPkgOffset("WAIT.PNG"));

    images[0]->swizzle();
    images[1]->swizzle();

    startLoadingThread();

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
    
    for (int i=2; i<MAX_IMAGES; i++){
        images[i]->swizzle();
        images[i]->is_system_image = true;
    }
    
    unsigned mp3_size;
    void* mp3_buffer = readFromPKG("SOUND.MP3", &mp3_size);
    sound_mp3 = new MP3(mp3_buffer, mp3_size);

}

void common::loadData(int ac, char** av){

    argc = ac;
    argv = av;

    today = common::getDateTime();

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

    currentFont = config.font;
    currentLang = config.language;
    currentApp = config.main_menu;

    if (config.language){
        Translations::loadLanguage(lang_files[config.language]);
    }
    
    loadFont();

    if (currentFont != config.font){
        currentFont = config.font;
    }
    
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

bool common::isFolder(SceIoDirent* dit){
    return FIO_SO_ISDIR(dit->d_stat.st_attr) || FIO_S_ISDIR(dit->d_stat.st_mode);
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
        txt<<size<<" B";
    else if (1024 < size && size < 1048576)
        txt<<round(float(size*100)/1024.f)/100.f<<" KB";
    else if (1048576 < size && size < 1073741824)
        txt<<round(float(size*100)/1048576.f)/100.f<<" MB";
    else
        txt<<round(float(size*100)/1073741824.f)/100.f<<" GB";
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

void common::printText(float x, float y, const char* text, u32 color, float size, int glow, TextScroll* scroll, int translate){

    if (font == NULL)
        return;

    string translated = (translate)? TR(text) : text;
    intraFont* textFont = font;

    if (translated != text){
        size *= text_size;
    }
    
    if (!translate && altFont){
        textFont = altFont;
    }

    u32 secondColor = BLACK_COLOR;
    u32 arg5 = INTRAFONT_WIDTH_VAR;
    
    if (glow && config.text_glow){
		int val = 0;
        float t = (float)((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
		if(config.text_glow == 1) {
        	val = (t < 0.5f) ? t*311 : (1.0f-t)*311;
		}
		else if(config.text_glow == 2) {
        	val = (t < 0.5f) ? t*411 : (1.0f-t)*411;
		}
		else {
        	val = (t < 0.5f) ? t*511 : (1.0f-t)*511;
		}
        secondColor = (0xFF<<24)+(val<<16)+(val<<8)+(val);
    }
    if (scroll){
        arg5 = INTRAFONT_SCROLL_LEFT;
    }
    
    intraFontSetStyle(textFont, size, color, secondColor, 0.f, arg5);
    if (altFont) intraFontSetStyle(altFont, size, color, secondColor, 0.f, arg5);

    if (scroll){
        if (x != scroll->x || y != scroll->y){
            scroll->x = x;
            scroll->tmp = x;
            scroll->y = y;
        }
        if (scroll->w <= 0 || scroll->w >= 480) scroll->w = 200;
        scroll->tmp = intraFontPrintColumn(textFont, scroll->tmp, y, scroll->w, translated.c_str());
    }
    else
        intraFontPrint(textFont, x, y, translated.c_str());
    
}

int common::calcTextWidth(const char* text, float size, int translate){
    string translated = (translate)? TR(text) : text;
    intraFont* textFont = font;
    if (translated != text){
        size *= text_size;
    }
    if (!translate && altFont){
        textFont = altFont;
    }
    intraFontSetStyle(textFont, size, 0, 0, 0.f, INTRAFONT_WIDTH_VAR);
    float w = intraFontMeasureText(textFont, translated.c_str());
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
