/*
 *  Copyright (C) 2006 Zx / Ludovic Jacomme (ludovic.jacomme@gmail.com)
 */

#include "std.h"
#include <pspumd.h>
# include "psp_init.h"
//# include "nlh.h"

#include <stdarg.h>

  char *psp_home_dir = (char *)0;

unsigned short htons(unsigned short wIn)
{
    u8 bHi = (wIn >> 8) & 0xFF;
    u8 bLo = wIn & 0xFF;
    return ((unsigned short)bLo << 8) | bHi;
}


# ifdef DEBUG
  static FILE *pspDebugFd = (FILE *)0;


void 
pspDebugPrintf(const char *Format, ...)
{
  va_list  Args;
  va_start( Args, Format );
  if (pspDebugFd == (FILE *)0) {
    pspDebugFd = fopen("ms0:/psp/game/debug.log", "w");
    if (! pspDebugFd) {
      pspDebugScreenPrintf("can\'t open debug file !\n");
      sceKernelExitGame();
    }
  }

  vfprintf(pspDebugFd, Format, Args);
  va_end( Args );
}
# endif

char *
my_dirname(char *filename)
{
  char *dirname = (char *)0;

  int index = strlen(filename);
  while (index >= 0) {
    if (filename[index] == '/') break;
    index--;
  }
  if (index >= 0) {
    dirname = (char *)malloc(sizeof(char) * (index + 1));
    strncpy(dirname, filename, index);
    dirname[index] = '\0';

  } else {
    dirname = (char *)malloc(sizeof(char) * 2);
    dirname[0] = '.';
    dirname[1] = '\0'; 
  }

  return dirname;
}

void
kbd_wait_start(void)
{
  while (1)
  {
    SceCtrlData c;
    sceCtrlReadBufferPositive(&c, 1);
    if (c.Buttons & PSP_CTRL_START) break;
  }
}

int
psp_power_callback(int arg1, int arg2, void *arg3)
{

  return 0;
}

void
psp_init_stuff(int argc, char **argv)
{
  int check_umd;

  psp_home_dir = my_dirname(argv[0]);
}

