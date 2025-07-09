/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 */

#ifndef _PSP_INIT_H_
#define _PSP_INIT_H_

#ifdef __cplusplus
extern "C"{
#endif

  extern char *psp_home_dir;

  extern void psp_init_stuff(int argc, char **argv);
  extern void pspDebugPrintf(const char *Format, ...);
  extern int psp_exit(int status);
  extern unsigned short htons(unsigned short wIn);

#ifdef __cplusplus
}
#endif

# endif
