#ifndef _SCEPAF_H
#define _SCEPAF_H


int scePaf_memcpy(void *dest, const void *src, int size);
int scePaf_memset(void *ptr, int value, int size);
int scePaf_memcmp(const void *ptr1, const void *ptr2, int size);

int scePaf_sprintf(char *str, const char *format, ...);
int scePaf_snprintf(char *str, int size, const char *format, ...);

int scePaf_strlen(const char *str);
int scePaf_strcpy(char *str1, const char *str2);
int scePaf_strncpy(char *str1, const char *str2, int size);
int scePaf_strcmp(const char *str1, const char *str2);
int scePaf_strncmp(const char *str1, const char *str2, int size);
int scePaf_strchr(const char *str, int character);
int scePaf_strrchr(char *str, int character);
int scePaf_strpbrk(char *str1, const char *str2);
int scePaf_strtoul(const char *str, char **endptr, int base);
// scePaf_strstr?
// scePaf_strcat?


#endif