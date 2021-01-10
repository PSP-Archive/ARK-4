#include "sutils.h"
#include <string.h>

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
		*s=toupper(*s);

		s++;
	}
}
