#include <stdlib.h>

char * strchr(const char *s, int c)
{
    while (*s != '\0' && *s != (char)c)
        s++;
    return ( (*s == c) ? (char *) s : NULL );
}

char * strrchr(const char *cp, int ch)
{
    char *save;
    char c;

    for (save = (char *) NULL; (c = *cp); cp++) {
        if (c == ch)
            save = (char *) cp;
    }

    return save;
}
