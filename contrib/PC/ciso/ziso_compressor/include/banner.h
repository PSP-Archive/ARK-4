#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////

void banner(void)
{
    printf(TITLE "\n"
                 "  " COPYR "\n"
                 "Improved version " VERSI
                 " (%d-bit "

#if defined(__CYGWIN__)
                 "Windows, Cygwin"
#elif defined(__MINGW32__)
                 "Windows, MinGW"
#elif defined(_WIN32) && defined(_MSC_VER) && (defined(__alpha) || defined(__ALPHA) || defined(__Alpha_AXP))
                 "Windows, Digital AXP C"
#elif defined(_WIN32) && defined(_MSC_VER) && defined(_M_ALPHA)
                 "Windows, Microsoft C, Alpha"
#elif defined(_WIN32) && defined(_MSC_VER) && defined(_M_MRX000)
                 "Windows, Microsoft C, MIPS"
#elif defined(_WIN32) && defined(_MSC_VER)
                 "Windows, Microsoft C"
#elif defined(__WIN32__) || defined(_WIN32)
                 "Windows"
#elif defined(__DJGPP__)
                 "DOS, DJGPP"
#elif defined(__MSDOS__) && defined(__TURBOC__)
                 "DOS, Turbo C"
#elif defined(_DOS) && defined(__WATCOMC__)
                 "DOS, Watcom"
#elif defined(__MSDOS__) || defined(MSDOS) || defined(_DOS)
                 "DOS"
#elif defined(__APPLE__)
                 "Mac OS"
#elif defined(__linux) || defined(__linux__) || defined(__gnu_linux__) || defined(linux)
                 "Linux"
#elif defined(__OpenBSD__)
                 "OpenBSD"
#elif defined(BSD)
                 "BSD"
#elif defined(human68k) || defined(HUMAN68K) || defined(__human68k) || defined(__HUMAN68K) || defined(__human68k__) || defined(__HUMAN68K__)
                 "Human68k"
#elif defined(__unix__) || defined(__unix) || defined(unix)
                 "unknown Unix"
#else
                 "unknown platform"
#endif

                 "%s)\n"
                 "  https://www.electrosoftcloud.com/\n"
                 "\n"
                 "  Binary Build date: %s @ %s\n"
                 "\n",
           (int)(sizeof(size_t) * 8),
           (sizeof(off_t) > 4 && sizeof(off_t) > sizeof(size_t)) ? ", large file support" : "",
           __DATE__, __TIME__);
}

////////////////////////////////////////////////////////////////////////////////