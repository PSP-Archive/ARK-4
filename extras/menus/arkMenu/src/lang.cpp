#include "lang.h"
#include "cJSON.h"
#include "common.h"

#define MASKBITS 0x3F
#define MASK2BYTES 0xC0
#define MASK3BYTES 0xE0

static cJSON* cur_lang = NULL;
intraFont* font = NULL;
intraFont* altFont = NULL;
int altFontId = 0;
float text_size = 1.0;
extern char* fonts[];

bool Translations::loadLanguage(string lang_file){

    bool needs_altfont = false;

    // cleanup old language and font
    fonts[0] = "FONT.PGF";
    text_size = 1.0;
    if (cur_lang){
        cJSON* aux = cur_lang;
        cur_lang = NULL;
        cJSON_Delete(aux);
    }
    if (altFont){
        intraFontUnload(font);
        font = altFont;
        altFont = NULL;
        t_conf* conf = common::getConf();
        conf->font = altFontId;
    }

    // read language file from PKG
    unsigned size = 0;
    void* buf = common::readFromPKG(lang_file.c_str(), &size, "LANG.ARK");

    if (buf && size){
        // parse new language file
        cJSON* val;
        cur_lang = cJSON_ParseWithLength((const char*)buf, size);

        // check if language file requires a font
        val = cJSON_GetObjectItem(cur_lang, "__font__");
        if (val){
            char* fontfile = cJSON_GetStringValue(val);
            if (fontfile){
                t_conf* conf = common::getConf();
                altFontId = conf->font;
                conf->font = 0;
                fonts[0] = fontfile;
                needs_altfont = true;
                altFont = font;
                font = NULL;
            }
        }

        val = cJSON_GetObjectItem(cur_lang, "__textsize__");
        if (val){
            text_size = cJSON_GetNumberValue(val);
        }

        // free resources
        free(buf);
    }

    return (cur_lang!=NULL);
}

string Translations::translate(string orig){

    if (cur_lang != NULL){

        cJSON* val = cJSON_GetObjectItem(cur_lang, orig.c_str());
        
        if (val != NULL){
            char* s = cJSON_GetStringValue(val);
            return string(s);
        }

    }

    return orig;
}
