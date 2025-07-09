#include "sutils.h"
#include <string.h>
#include <pspiofilemgr_dirent.h>

char* skipWS(char* s) {
    while ((*s)==' ' || (*s)=='\t') {
        s++;
    }
    return s;
}

void trimEndingChar(char* s, char c) {
    char* p=s+strlen(s)-1;

    while ( p>=s && ((*p)==c) ) {
        p--;
    }

    (*(++p))=0;
}

void trimEndingWS(char* s) {
    char* p=s+strlen(s)-1;

    while ( p>=s && ((*p)==' ' || (*p)=='\t') ) {
        p--;
    }

    (*(++p))=0;
}

int strStartsWith(char* s, char* start) {

    while ((*start)!=0 && (*s)!=0 && (*start)==(*s)) {
        *start++; *s++;
    }

    return ((*start)==0);
}

int endsWith(char* s, char* end) {
    if (strlen(end)>strlen(s)) {
        return 0;
    } else {
        char* sEnd=s+strlen(s)-strlen(end);

        return (strcmp(sEnd, end)==0);
    }
}

char *itoa(char *buf, int n)
{
  int radix = 10;
  char         *ret = buf;
  char         tmp[33];
  int          i = 0, j, r;

  /* validate the conversion number base. */
  if ((radix >= 2) && (radix <= 36)) {
    if ((radix == 10) && (n < 0)) {
      /* negative integer value. */
      *buf++ = '-';
      n = -n;
    }
    do {
      /* calculate the current digit. */
      r = (int)((unsigned int)n % radix);
      tmp[i++] = ((r < 10) ? (r + '0') : (r - 10 + 'a'));
    } while ((n /= radix) != 0);
    /* reverse the buffer string. */
    for (--i, j = 0; (i >= 0); --i, ++j) buf[j] = tmp[i];
    buf[j] = 0;
  }
  return (ret);
}

void strReplaceChar(char* str, char s, char d) {
    while (*str!=0) {
        if (*str==s) *str=d;

        str++;
    }
}

void toUpperCase(char* s) {
    while (*s!=0) {
        char c = *s;
        if (c >= 'a' && c <= 'z'){
            c -= 0x20;
            *s = c;
        }
        s++;
    }
}

int fileExists(const char* path){
    SceIoStat sb;
    return (sceIoGetstat(path, &sb) == 0 && FIO_SO_ISREG(sb.st_mode));
}

int isFolder(struct SceIoDirent* dit){
    return FIO_SO_ISDIR(dit->d_stat.st_attr) || FIO_S_ISDIR(dit->d_stat.st_mode);
}

void recursiveFolderDelete(char* path){
    //try to open directory
    int d = sceIoDopen(path);
    
    if(d >= 0)
    {
        struct SceIoDirent entry;
        memset(&entry, 0, sizeof(SceIoDirent));

        //start reading directory entries
        while(sceIoDread(d, &entry) > 0)
        {
            //skip . and .. entries
            if (!strcmp(".", entry.d_name) || !strcmp("..", entry.d_name)) 
            {
                memset(&entry, 0, sizeof(SceIoDirent));
                continue;
            };
            
            //build new file path
            char new_path[255];
            strcpy(new_path, path);
            strcat(new_path, entry.d_name);

            if (isFolder(&entry)){
                strcat(new_path, "/");
                recursiveFolderDelete(new_path);
            }
            else{
                sceIoRemove(new_path);
            }
            
        };
        
        sceIoDclose(d); //close directory
        int len = strlen(path);
        if (path[len-1] == '/') path[len-1] = 0;
        sceIoRmdir(path); //delete empty folder
    };
}