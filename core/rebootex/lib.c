#include <psptypes.h>

/*
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
*/

void memcpy_b(void *dst, void *src, int len)
{
	u8 *d = (u8 *)dst;
	u8 *s = (u8 *)src;
	while(len--) 
	{
		d[len] = s[len];
	} 
}

/*
int memcmp(char *m1, char *m2, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		if (m1[i] != m2[i])
			return m2[i] - m1[i];
	}

	return 0;
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

int strlen(char *str)
{
	char *p;
	for(p=str; *p; ++p)
		;

	return (p - str);
}
*/
