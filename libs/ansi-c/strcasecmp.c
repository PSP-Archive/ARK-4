#include <stdlib.h>
#include "ansi_c_functions.h"

int tolower(int s)
{
    if((s >= 'A') && (s <= 'Z'))
        s = 'a' + (s - 'A');

    return s;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;
    unsigned char c1, c2;

    if (p1 == p2 || n == 0)
        return 0;
    
    if (s1 == NULL || s2 == NULL){
        return (int)s1 - (int)s2;
    }

    do {
        c1 = tolower(*p1);
        c2 = tolower(*p2);

        if (--n == 0 || c1 == '\0')
        	break;

        ++p1;
        ++p2;
    } while (c1 == c2);

    return c1 - c2;
}

int strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, (size_t)-1);
}
