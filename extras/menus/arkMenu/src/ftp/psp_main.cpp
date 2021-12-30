/* PSPFTPD */

#include <string>

#include "controller.h"
#include "psp_main.h"
#include "std.h"
#include "util.h"
#include "ftp.h"
#include "ftpd.h"
#include "psp_init.h"
#include "psp_cfg.h"

#include <pspkernel.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspthreadman.h>
#include <psppower.h>
#include <psputility.h>
#include <psputility_netconf.h>
#include <pspwlan.h>

#include "common.h"

using namespace std;

static int loc_info_thread_end = 0;

# define STATUS_MESSAGE_LENGTH    55
# define STATUS_MESSAGE_LINE      20

static char status_messages[STATUS_MESSAGE_LINE][STATUS_MESSAGE_LENGTH];
static int  status_index = 0;

typedef struct sDevCtl
{
  s32 max_clusters;
  s32 free_clusters;
  s32 max_sectors;  // ???
  s32 sector_size;
  s32 sector_count;
} sDevCtl;

typedef struct sDevCommand
{
  sDevCtl * p_dev_inf;
} sDevCommand;

typedef struct {
	unsigned int size;
	int message_lang;
	int ctrl_assign;
	int main_thread_priority;
	int sub_thread_priority;
	int font_thread_priority;
	int sound_thread_priority;
	int result; 
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
} SceUtilityParamBase;

typedef struct
{
	unsigned char group_name[8];
	unsigned int timeout;
} SceUtilityNetconfAdhocParam;

typedef struct
{
	SceUtilityParamBase base;
	int type;
	SceUtilityNetconfAdhocParam * adhoc_param;
	unsigned int browser_available;
	unsigned int browser_flag;
	unsigned int wifisvc_available;
} SceUtilityNetconfParam;

void
mftpAddNewStatusMessage(char *Message)
{
  int length = strlen(Message);
  int index  = 0;

  strncpy(status_messages[status_index], Message, STATUS_MESSAGE_LENGTH);

  status_messages[status_index][STATUS_MESSAGE_LENGTH-1] = 0;

  for (index = length; index < (STATUS_MESSAGE_LENGTH-1); index++) {
    status_messages[status_index][index] = ' ';
  }

  if ((status_index + 1) == STATUS_MESSAGE_LINE) status_index = 0;
  else status_index++;
}


void
mftpDisplayStatusMessage()
{
  int index;

  for (index = 0; index < STATUS_MESSAGE_LINE; index++) {
    common::printText(1, 40 + index*10, status_messages[(status_index + index) % STATUS_MESSAGE_LINE]);
  }
}

void
info_thread(SceSize args, void *argp)
{

  while (!loc_info_thread_end) {

    common::clearScreen(CLEAR_COLOR);
    common::getImage(IMAGE_BG)->draw(0, 0);
    common::printText(0, 10, (char*)argp);
    mftpDisplayStatusMessage();
    common::flipScreen();

    sceKernelDelayThread(0);
  }
}

void apctl_handler(int prev_state, int new_state, int event, int error, void *arg)
{
	// Do nothing
	// loginfo("%08X - %08X - %08X - %08X - %08X", prev_state, new_state, event, error, (unsigned int)arg);
}

int initializeNetwork(void)
{

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

char* generic_ok = "FTP closed";

static void 
DoInetNetFtpd(void)
{
  int info_thid = 0;
  int last_state;
  int err;
  char szMyIPAddr[32];
  int connectionConfig = -1;
  PICKER pickConn; // connection picker
  int iNetIndex;
  int iPick;
  
  char* error = generic_ok;

  err = initializeNetwork();

  if (err < 0) {
  	error = "Internet Library Initialization Error!";
  	sceKernelDelayThread(1000000);
    	goto close_connection;
  }

  //info_thid = sceKernelCreateThread( "info_thread", (SceKernelThreadEntry)info_thread, 0x18, 0x10000, 0, 0 );

  //if(info_thid >= 0) {
  //  sceKernelStartThread(info_thid, 0, 0);
  //}

  while (sceWlanGetSwitchState() != 1)  //switch = off
  {
    error = "Activate WiFi on your VITA!";
    sceKernelDelayThread(1000000);
    goto close_connection;
  }
	if(connect_to_apctl() == 0)
	{
		error = "Router Connection failed!";
		sceKernelDelayThread(1000000);
		goto close_connection;
	}

  // get IP address
  if (sceNetApctlGetInfo(8, (SceNetApctlInfo*)szMyIPAddr) != 0) {
    error = "Couldn't aquire valid IP Address!";
    sceKernelDelayThread(1000000);
    goto close_connection;
  }

  ftpdLoop(szMyIPAddr);

close_connection:
  err = sceNetApctlDisconnect();

  common::clearScreen(CLEAR_COLOR);
  common::getImage(IMAGE_BG)->draw(0, 0);
  common::printText(0, 20, error);
  if (error != generic_ok)
  	common::printText(0, 30, "Press X or ()");
  else
    common::printText(0, 30, "Exiting FTP Server");
  common::flipScreen();
  
  if (error != generic_ok){
  	sceKernelDelayThread(100000);
  	Controller pad;
  	pad.wait();
  }
  
  pspSdkInetTerm();

}

int ftp_thread(SceSize args, void *argp)
{
  DoInetNetFtpd();
  sceKernelExitDeleteThread(0);
  return 0;
}


int 
ftpStart(int argc, char **argv)
{
  int user_thid;

  psp_init_stuff(argc, argv);

  psp_read_config();

  user_thid = sceKernelCreateThread("ftp_thread", ftp_thread, 0x16, 32*1024, PSP_THREAD_ATTR_USER, 0);
  if(user_thid >= 0) {
    sceKernelStartThread(user_thid, 0, 0);
  }
  
  sceKernelWaitThreadEnd(user_thid, 0);

  sceUtilityUnloadModule(PSP_MODULE_NET_INET);
  sceUtilityUnloadModule(PSP_MODULE_NET_COMMON);

  return 0;
}

