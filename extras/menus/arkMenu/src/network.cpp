#include <pspkernel.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspthreadman.h>
#include <psppower.h>
#include <psputility.h>
#include <psputility_netconf.h>
#include <pspwlan.h>
#include <pspdisplay.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <pspnet_apctl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <psphttp.h>
#include "network.h"
#include "common.h"

static int net_users = 0; // count number of network users
static bool ap_conn = false;
static SceUID ftpmod = -1;
static int ftp_users = 0;

void apctl_handler(int prev_state, int new_state, int event, int error, void *arg)
{
    // Do nothing
    // loginfo("%08X - %08X - %08X - %08X - %08X", prev_state, new_state, event, error, (unsigned int)arg);
}

int initializeNetwork(void)
{
    if (++net_users > 1) return 0; // already initialized
    
    int ret;

    ret = sceUtilityLoadModule(PSP_MODULE_NET_COMMON);

    if (ret < 0)
        return ret;

    ret = sceUtilityLoadModule(PSP_MODULE_NET_INET);

    if (ret < 0)
        return ret;

    ret = sceNetInit(256 * 1024, 42, 0, 42, 0);
    
    if (ret < 0)
        return ret;

    ret = sceNetInetInit();
    
    if (ret < 0)
        return ret;

    ret = sceNetResolverInit();
    
    if (ret < 0)
        return ret;

    ret = sceNetApctlInit(10 * 1024, 48);

    if (ret < 0)
        return ret;
    
    ret = sceNetApctlAddHandler(apctl_handler, NULL);

    return ret;
}

int shutdownNetwork(){
    if (--net_users > 0) return 0; // network still has users
    sceNetApctlDisconnect();
    sceNetApctlTerm();
    sceNetResolverTerm();
    sceNetInetTerm();
    sceNetTerm();
    sceUtilityUnloadModule(PSP_MODULE_NET_INET);
    sceUtilityUnloadModule(PSP_MODULE_NET_COMMON);
    ap_conn = false;
}

char* resolveHostAddress(char* hostname){
    struct hostent* host = gethostbyname(hostname);
    return inet_ntoa(*((struct in_addr*)host->h_addr));
}

/* Connect to an access point */
int connect_to_apctl(void)
{

    if (ap_conn) return 1;

    SceUtilityNetconfParam p;
    SceUtilityNetconfAdhocParam a;
    memset(&p, 0, sizeof(p));
    memset(&a, 0, sizeof(a));
    
    p.base.size = 0x44;
    p.base.message_lang = 1;
    p.base.ctrl_assign = !(int)common::getConf()->swap_buttons;
    p.base.main_thread_priority = 0x11;
    p.base.sub_thread_priority = 0x13;
    p.base.font_thread_priority = 0x12;
    p.base.sound_thread_priority = 0x10;
    p.base.result = 0;
    p.base.reserved1 = 0;
    p.base.reserved2 = 0;
    p.base.reserved3 = 0;
    p.base.reserved4 = 0;
    p.type = 3;
    p.adhoc_param = &a;
    p.browser_available = 1;
    p.browser_flag = 1;
    p.wifisvc_available = 0;
    a.timeout = 10;
    
    int init = sceUtilityNetconfInitStart((pspUtilityNetconfData *)&p);
    
    if(init == 0)
    {
        // Wait for Initialization to complete
        while(sceUtilityNetconfGetStatus() == PSP_UTILITY_DIALOG_INIT) sceKernelDelayThread(100000);
        
        // Render Screen
        while(sceUtilityNetconfGetStatus() == PSP_UTILITY_DIALOG_VISIBLE)
        {
            common::clearScreen(CLEAR_COLOR);
            common::getImage(IMAGE_BG)->draw(0, 0);
        
            sceGuFinish();
            sceGuSync(0,0);
        
            // Update Screen
            sceUtilityNetconfUpdate(1);
            
            common::flipScreen();
            
        }
        
        // Shutdown Utility
        int stop = sceUtilityNetconfShutdownStart();
        
        // Wait for Shutdown
        while(sceUtilityNetconfGetStatus() != PSP_UTILITY_DIALOG_NONE) sceKernelDelayThread(100000);
        
        // Connect Success
        if(p.base.result == 0){
            ap_conn = true;
            return 1;
        }
    }
    return 0;
}

int wget(char* url, char* saveAs, SceULong64* cur_download, SceULong64* max_download){
    int tpl, cnx, req, ret;
    u8 buf[16*1024];
    if((tpl=sceHttpCreateTemplate("ARK-Launcher/1.0", 1, 1))<0)return tpl;
    if((cnx=sceHttpCreateConnectionWithURL(tpl, url, 0))<0)return cnx;
    if((req=sceHttpCreateRequestWithURL(cnx, PSP_HTTP_METHOD_GET, url, 0))<0)return req;
    if((ret=sceHttpSendRequest(req, 0, 0))<0)return ret;
    if (cur_download) *cur_download = 0;
    if (max_download) sceHttpGetContentLength(req, max_download);
    if(saveAs){
        SceUID fd=sceIoOpen(saveAs, PSP_O_WRONLY | PSP_O_CREAT, 0777);
        while((ret=sceHttpReadData(req,buf,sizeof(buf)))>0){
            sceIoWrite(fd,buf,ret);
            if (cur_download) *cur_download += ret;
        }
        ret=sceIoClose(fd);
    }else{//store in ram
        ret=sceHttpReadData(req,buf,sizeof(buf));
    }
    sceHttpDeleteRequest(req);
    sceHttpDeleteConnection(cnx);
    sceHttpDeleteTemplate(tpl);
    return ret;
}
