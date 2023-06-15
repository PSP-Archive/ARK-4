#ifndef LANG_H
#define LANG_H

#include <string>
#include <cstring>

#define TR(s) Translations::translate(s)

namespace Translations{
    extern bool loadLanguage(std::string lang_file);
    extern std::string translate(std::string orig);
};

#endif