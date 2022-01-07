#include "../config/sdk.h"
#include "common.h"


void memcpy(char *dest, char *src, int size)
{
  while (size--)
  {
    *dest++ = *src++;
  }
}


int strlen(char * xistr)
{
  int i = 0;
  while (*xistr)
  {
    i++;
  }
  return(i);
}


void memdump()
{
  int file;

  file = sceIoOpen("ms0:/mem.dmp",PSP_O_CREAT | PSP_O_TRUNC | PSP_O_RDWR, 0777);
  sceIoWrite(file,0x08800000, 0x01800000);
  sceIoClose(file);
}


int Sleep(unsigned int xicount)
{
  int ldummy = 2;
  unsigned int i;
  unsigned int j;

  for (i=0; i < xicount; i++)
  {
    for (j=0; j < 65000; j++)
    {
      ldummy = ldummy * ldummy;
    }
  }

  return(ldummy);
}
