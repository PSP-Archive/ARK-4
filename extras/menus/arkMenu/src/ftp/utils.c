#include <pspkernel.h>
#include <pspdisplay.h>
#include <psputility.h>
#include <pspctrl.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>

#include "utils.h"

extern int run;
int net_connected = 0;


int select_netconfig()
{
    #define MAX_CONFIG 10
    struct {
        int config_n;
        char *name;
    } net_list[MAX_CONFIG];
    
    /* Read net config list */
    
    int i, used = 0;
    for (i = 1; i <= MAX_CONFIG; ++i) {
        if (sceUtilityCheckNetParam(i) == 0) {
            netData data;
            sceUtilityGetNetParam(i, PSP_NETPARAM_NAME, &data);
            int name_size = strlen(data.asString) + 1;
            net_list[used].config_n = i;
            net_list[used].name = malloc(name_size);
            memcpy(net_list[used].name, data.asString, name_size);
            ++used;
        }
    }
    
    printf("Select an access point:\n");
    
    int selected = 0, last_y = pspDebugScreenGetY();
    SceCtrlData pad, old_pad;
    sceCtrlPeekBufferPositive(&pad, 1);
    old_pad = pad;
    
    while (run) {
        sceCtrlPeekBufferPositive(&pad, 1);
        for (i = 0; i < used; ++i) {
            if (selected == i) {
                printf("\n-> %s\n", net_list[i].name);
            } else {
                printf("\n   %s\n", net_list[i].name);
            }
        }
        
        if (pad.Buttons & PSP_CTRL_UP & ~old_pad.Buttons) {
            --selected;
            if (selected < 0) selected = used-1;    
        } else if (pad.Buttons & PSP_CTRL_DOWN & ~old_pad.Buttons) {
            ++selected;
            if (selected >= used) selected = 0;    
        }
        
        if (pad.Buttons & PSP_CTRL_CROSS & ~old_pad.Buttons) break;
        
        old_pad = pad;
        pspDebugScreenSetXY(0, last_y);
        sceDisplayWaitVblankStart();
    }
    
    return net_list[selected].config_n;
}

int connect_net(int config_n)
{
    int con_state = 0, last_con_state = 0;
    clock_t clock_start = 0;
    sceNetApctlConnect(config_n);
    printf("connection to an access point:\n");
    while (run) {
        sceNetApctlGetState(&con_state);
        if (con_state == PSP_NET_APCTL_STATE_GOT_IP) {
            net_connected = 1;
            printf("Got IP!\n");
            return 0;
        }
            
        if (con_state != last_con_state) {
            switch (con_state) {
            case PSP_NET_APCTL_STATE_DISCONNECTED:
                printf("Disconnected\n"); break;
            case PSP_NET_APCTL_STATE_SCANNING:
                printf("Scanning...\n"); break;
            case PSP_NET_APCTL_STATE_JOINING:
                printf("Joining...\n"); break;
            case PSP_NET_APCTL_STATE_GETTING_IP:
                printf("Getting IP...\n"); break;
            case PSP_NET_APCTL_STATE_EAP_AUTH:
                printf("Authentification\n"); break;
            case PSP_NET_APCTL_STATE_KEY_EXCHANGE:
                printf("Key exchange...\n"); break;
            }
            
            clock_start = sceKernelLibcClock();
            last_con_state = con_state;   
        } else {  /* Check timeout */
            if (((sceKernelLibcClock() - clock_start)/(double)CLOCKS_PER_SEC) > CONNECT_TIMEOUT) {
                
            }
        }
        sceKernelDelayThread(50*1000);
    }
    return -1;
}


int get_ip(char *ip)
{
    int state;
    sceNetApctlGetState(&state);
    
    union SceNetApctlInfo info_ip;
    if (sceNetApctlGetInfo(PSP_NET_APCTL_INFO_IP, &info_ip) != 0) {
        strcpy(ip, "unknown IP");
        return 0;
    }
    memcpy(ip, info_ip.ip, 16);
    return 1;
}

void list_netconfigs()
{
    #define MAX_CONFIG 10
    int i;
    for (i = 1; i <= MAX_CONFIG; ++i) {
        if (sceUtilityCheckNetParam(i) == 0) {
            printf("->Configuration %i\n", i);
            netData data;
            sceUtilityGetNetParam(i, PSP_NETPARAM_NAME, &data);
            printf("\tNAME: %s\n", data.asString);
            sceUtilityGetNetParam(i, PSP_NETPARAM_SSID, &data);
            printf("\tSSID: %s\n", data.asString);
            sceUtilityGetNetParam(i, PSP_NETPARAM_SECURE, &data);
            printf("\tSECURE: %s\n", data.asUint==2?"WEP":(data.asUint==3?"WPA":"NONE"));
        }
    }
}

static void apctl_handler(int oldState, int newState, int event, int error, void *pArg)
{
    if (newState == PSP_NET_APCTL_STATE_DISCONNECTED) {
        printf("Disconnected!\n");
        net_connected = 0;   
    }
}


int load_net_modules()
{
    if (!is_net_common_mod_loaded())
        sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    if (!is_net_inet_mod_loaded())
        sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    return 0;
}

int unload_net_modules()
{
    int ret = sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
    if (ret < 0) return ret;
    ret = sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
    return ret;
}

int init_net()
{
    int ret = sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
    if (ret < 0) return ret;
    ret = sceNetInetInit();
    if (ret < 0) return ret;
    ret = sceNetApctlInit(0x8000, 48);
    sceNetApctlAddHandler(apctl_handler, NULL);
    return ret;
}

int deinit_net()
{
    if (net_connected) sceNetApctlDisconnect();
    int ret = sceNetApctlTerm();
    if (ret < 0) return ret;
    ret = sceNetInetTerm();
    if (ret < 0) return ret;
    ret = sceNetTerm();
    return ret;
}



int is_net_common_mod_loaded()
{
    SceUID uid_list[256];
    int count = 0;
    SceKernelModuleInfo info;
    sceKernelGetModuleIdList(uid_list, 256, &count);
    int i;
    for (i = 0; i < count; ++i) {
        memset(&info, 0, sizeof(info));
        info.size = sizeof(info);
        sceKernelQueryModuleInfo(uid_list[i], &info);
        if (strcmp(info.name, "sceNet_Library") == 0) return 1;
    }
    return 0;
}

int is_net_inet_mod_loaded()
{
    SceUID uid_list[256];
    int count = 0;
    SceKernelModuleInfo info;
    sceKernelGetModuleIdList(uid_list, 256, &count);
    int i;
    for (i = 0; i < count; ++i) {
        memset(&info, 0, sizeof(info));
        info.size = sizeof(info);
        sceKernelQueryModuleInfo(uid_list[i], &info);
        if ((strcmp(info.name, "sceNetInet_Library") == 0)  ||
            (strcmp(info.name, "sceNetApctl_Library") == 0) || 
            (strcmp(info.name, "sceNetResolver_Library") == 0))
                return 1;
    }
    return 0;
}

