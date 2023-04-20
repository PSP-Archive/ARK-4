#include <psptypes.h>

void* memcpy(void *dst,void *src,int size)
{
    u8 *p1 = (u8 *)dst;
    u8 *p2 = (u8 *)src;
    while(size--)
    {
        *p1++ = *p2++;
    }
    return dst;
}

int strcmp(const char *s, const char *t)
{
    while( *s == *t )
    {
        if( *s == '\0' )
            return 0;

        s++;t++;
    }
    return (*s - *t);
}

int strncmp(const char *a, const char *b, unsigned int count)
{
    while(count>0 && *a && *b && *a == *b)
    {
        a++, b++;
        count--;
    }
    if (count == 0) return 0;
    return *a - *b;
}


void* memset(void *dst, u8 code, int size)
{
    u8 *p1 = (u8 *)dst;
    while(size--)
    {
        *p1++ = code;
    }
    return p1;
}

int strcpy(char* dest, char* orig){
    int i = 0;
    do{
        dest[i] = orig[i];
    } while (orig[i++]);
    return i-1;
}

void *memmove(void * to_, const void * from_, unsigned int length)
{
    char *to = to_;
    const char *from = from_;

    if (to > from) {
        //back buffer
        char * tob = to + length;
        const char * fromb = from + length;

        //loop copy
        unsigned int pos = 0; for(; pos < length; pos++)
        {
            //copy byte
            *--tob = *--fromb;
        }

        return to_;
    }

    return memcpy(to, from, length);
}

#ifdef MS_IPL

int memcmp(const void *_a, const void* _b, unsigned int size)
{
    const unsigned char *a, *b;

    a = _a, b = _b;

    while(size > 0 && *a == *b)
    {
        a++, b++;
        size--;
    }

    if(size == 0)
        return 0;

    return *a - *b;
}

int tolower(int s)
{
    if((s >= 'A') && (s <= 'Z'))
        s = 'a' + (s - 'A');

    return s;
}

int strncasecmp(const char *s1, const char *s2, unsigned int n)
{
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;
    unsigned char c1, c2;

    if (p1 == p2 || n == 0)
        return 0;

    do {
        c1 = tolower(*p1);
        c2 = tolower(*p2);

        if (--n == 0 || c1 == '\0' || c2 == '\0')
            break;

        ++p1;
        ++p2;
    } while (c1 == c2);

    return c1 - c2;
}

int strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, (unsigned int)-1);
}

int strlen(const char *s)
{
    int len = 0;

    while (s[len] != '\0')
        len++;

    return len;
}

char *strcat(char *dest, const char *source)
{
    if (!dest || !source) {
        return 0;
    }

    char *ptr = dest + strlen(dest);

    while (*source != '\0') {
        *ptr++ = *source++;
    }

    *ptr = '\0';

    return dest;
}

#endif