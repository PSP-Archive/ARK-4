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

#include "network.h"
#include "common.h"

static bool net_initialized = false;

void apctl_handler(int prev_state, int new_state, int event, int error, void *arg)
{
	// Do nothing
	// loginfo("%08X - %08X - %08X - %08X - %08X", prev_state, new_state, event, error, (unsigned int)arg);
}

int initializeNetwork(void)
{

    if (net_initialized) return 0;
    
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

    if (ret>=0) net_initialized = true;

	return ret;
}

int shutdownNetwork(){
    if (!net_initialized) return 0;
    sceNetApctlDisconnect();
    sceNetApctlTerm();
    sceNetResolverTerm();
    sceNetInetTerm();
    sceNetTerm();
    sceUtilityUnloadModule(PSP_MODULE_NET_INET);
    sceUtilityUnloadModule(PSP_MODULE_NET_COMMON);
    net_initialized = false;
}

/* Connect to an access point */
int connect_to_apctl(void)
{

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
			// Update Screen
			sceUtilityNetconfUpdate(1);
			
			// Wait to produce 30fps
			//sceKernelDelayThread(1000000 / 30);
            sceDisplayWaitVblankStart();
		}
		
		// Shutdown Utility
		int stop = sceUtilityNetconfShutdownStart();
		
		// Wait for Shutdown
		while(sceUtilityNetconfGetStatus() != PSP_UTILITY_DIALOG_NONE) sceKernelDelayThread(100000);
		
		// Connect Success
		if(p.base.result == 0){
			return 1;
		}
	}
	return 0;
}
