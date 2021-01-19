#include <stdlib.h>

char* strstr(const char* source, const char* search){
    int len = strlen(search)-1;
    if (len > 0){
        int i = 0;
        while (source[i] != 0){
            if (strncmp(&source[i], search, len)==0){
                return &source[i];
            }
            i++;
        }
    }
    return NULL;
}
