#ifndef _sutils
#define _sutils

#ifdef __cplusplus
extern "C"{
#endif

    char* skipWS(char* s);
    void trimEndingWS(char* s);
    int strStartsWith(char* s, char* start);
    int endsWith(char* s, char* end);
    void trimEndingChar(char* s, char c);
    char *itoa(char *str, int num);
    void strReplaceChar(char* str, char s, char d);

    void toUpperCase(char* s);

#ifdef __cplusplus
}
#endif

#endif