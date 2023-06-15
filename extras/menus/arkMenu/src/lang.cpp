#include "lang.h"
#include "cJSON.h"
#include "common.h"

static cJSON* cur_lang = NULL;

bool Translations::loadLanguage(string lang_file){

    // read language file from PKG
    unsigned size = 0;
    void* buf = common::readFromPKG(lang_file.c_str(), &size, "LANG.ARK");

    if (buf && size){
        if (cur_lang != NULL){
            // cleanup old language
            cJSON_Delete(cur_lang);
        }
        // parse new language file
        cur_lang = cJSON_ParseWithLength((const char*)buf, size);

        // check if language file requires a font
        cJSON* val = cJSON_GetObjectItem(cur_lang, "__font__");
        if (val){
            int font = (int)cJSON_GetNumberValue(val);
            t_conf* conf = common::getConf();
            conf->font = font;
            common::saveConf();
        }
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