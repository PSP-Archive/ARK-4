#ifndef _UTILS_H_
#define _UTILS_H_

#include <pspkernel.h>
#include <pspdisplay.h>
#include <psputility.h>
#include <pspctrl.h>

#define printf pspDebugScreenPrintf
#define cls    pspDebugScreenClear

#define CONNECT_TIMEOUT 10.0

int select_netconfig();
void list_netconfigs();
int connect_net(int config_n);
int get_ip(char *ip);

int load_net_modules();
int unload_net_modules();
int init_net();
int deinit_net();

int is_net_common_mod_loaded();
int is_net_inet_mod_loaded();

#endif 
