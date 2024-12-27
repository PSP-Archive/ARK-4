#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <pspkernel.h>
#include <psputility.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <pspctrl.h>
#include <pspumd.h>

#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include <systemctrl.h>
#include <systemctrl_se.h>

#include "vsh.h"

typedef struct _pspMsPrivateDirent {
	SceSize size;
	char s_name[16];
	char l_name[1024];
} pspMsPrivateDirent;

void exec_recovery_menu(vsh_Menu *vsh);
void exec_random_game(vsh_Menu *vsh);
void launch_umdvideo_mount(vsh_Menu *vsh);

#endif
