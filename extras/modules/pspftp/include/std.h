#ifdef __cplusplus
extern "C"{
#endif

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>

#include <pspiofilemgr.h>
#include <pspiofilemgr_fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define bool int
#define false 0
#define true 1

// don't use 'printf' or 'pspDebugScreenPrintf'
// use my_print/my_printn instead which prints to ms0:/err.txt log file

#ifdef __cplusplus
}
#endif