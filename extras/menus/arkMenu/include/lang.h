#ifndef LANG_H
#define LANG_H

#include <string>
#include <cstring>

#define TR(s) Translations::translate(s)

namespace Translations{
    extern bool loadLanguage(string lang_file);
    extern string translate(string orig);
};

#endif