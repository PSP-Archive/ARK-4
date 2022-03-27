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
