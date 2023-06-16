#include "lang.h"
#include "cJSON.h"
#include "common.h"

static cJSON* cur_lang = NULL;
intraFont* altFont = NULL;

bool Translations::loadLanguage(string lang_file){

    // cleanup old language and font
    if (cur_lang){
        cJSON* aux = cur_lang;
        cur_lang = NULL;
        cJSON_Delete(aux);
    }
    if (altFont){
        intraFontUnload(altFont);
        altFont = NULL;
        t_conf* conf = common::getConf();
        conf->font = 0;
    }

    // read language file from PKG
    unsigned size = 0;
    void* buf = common::readFromPKG(lang_file.c_str(), &size, "LANG.ARK");

    if (buf && size){
        // parse new language file
        cur_lang = cJSON_ParseWithLength((const char*)buf, size);

        // check if language file requires a font
        cJSON* val = cJSON_GetObjectItem(cur_lang, "__font__");
        if (val){
            int font = (int)cJSON_GetNumberValue(val);
            t_conf* conf = common::getConf();
            conf->font = font;
            if (altFont == NULL){
                altFont = intraFontLoad("flash0:/font/ltn0.pgf", 0);
                intraFontSetEncoding(altFont, INTRAFONT_STRING_UTF8);
            }
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