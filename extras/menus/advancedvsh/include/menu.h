#ifndef _MENU_H
#define _MENU_H


#include "vsh.h"


int menu_draw(void);
static inline const char *get_enable_disable(int opt);
int menu_setup(void);
int menu_ctrl(u32 button_on);
void button_func(vsh_Menu *vsh);


#endif