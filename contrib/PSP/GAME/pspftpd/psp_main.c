/* PSPFTPD */

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspthreadman.h>

#include <pspwlan.h>
#include <pspkernel.h>
#include <psppower.h>

PSP_MODULE_INFO("PSPFTPD", PSP_MODULE_USER, 1, 1);
PSP_HEAP_SIZE_KB(12 * 1024);
PSP_MAIN_THREAD_ATTR(0);
PSP_MAIN_THREAD_STACK_SIZE_KB(64);

#include "std.h"
#include "util.h"
#include "ftp.h"
#include "ftpd.h"
#include "psp_init.h"
#include "psp_pg.h"
#include "psp_cfg.h"

#include <pspwlan.h>
#include <pspkernel.h>
#include <psppower.h>
#include <psputility.h>
#include <psputility_netconf.h>

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

long
psp_get_memory_stick_free()
{
	return 0;
/*
  sDevCtl     dev_ctl;
  sDevCommand command;

  command.p_dev_inf = &dev_ctl;

  if ( sceIoDevctl( "ms0:", 0x02425818, &command, sizeof( sDevCommand ), NULL, 0 ) == SCE_KERNEL_ERROR_OK ) {
    return dev_ctl.free_clusters * dev_ctl.sector_count * dev_ctl.sector_size;
  }
  return -1;
  */
}

static char loc_batt_str[128];

char *
psp_get_battery_string()
{
  char tmp[128];
  int ret;

  strcpy(loc_batt_str, "none");
  if (scePowerIsBatteryExist()) {
    ret = scePowerGetBatteryLifePercent();

    if (ret >= 0) {
      sprintf(tmp, "%d", ret);
      strcpy(loc_batt_str,tmp);
      strcat(loc_batt_str,"%");
      if(!scePowerIsPowerOnline()){
        if((ret=scePowerGetBatteryLifeTime()) >= 0){
          sprintf(tmp, " %dh", ret/60);
          strcat(loc_batt_str,tmp);
          sprintf(tmp, "%d", (ret%60) + 100);
          strcat(loc_batt_str,tmp+1);
        }
      }
    }
  }
  return loc_batt_str;
}

int
psp_is_low_battery()
{
  int ret = 0;
  if (scePowerIsBatteryExist()) {
    ret = scePowerGetBatteryLifePercent();
    if (ret < 4) return 1;
  }
  return 0;
}

void
mftpDisplayBatteryMessage(void)
{
  char buffer[64];
  long space_free;
  int  color;

  snprintf(buffer, 50, " [%4s] ", psp_get_battery_string());

  if (psp_is_low_battery()) color = PG_TEXT_RED;
  else                      color = PG_TEXT_GREEN;

  pgFramePrint(25,29, buffer, color);

  space_free  = psp_get_memory_stick_free();
  space_free /= 1024 * 1024;

  if (space_free < 1) color = PG_TEXT_RED;
  else                color = PG_TEXT_YELLOW;
  snprintf(buffer, 20, "%ld MB free", space_free );

  /* Fill with space */
  int length = strlen(buffer);
  while (length < 20) buffer[length++] = ' ';
  buffer[length] = 0;
  
  pgFramePrint(0,29, buffer, color);
}

void
mftpDisplayMemoryMessage(void)
{
  char buffer[64];
  int color;

  pgFramePrint(25,29, buffer, color);
}

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
    pgFramePrint(1, 5 + index, 
    status_messages[(status_index + index) % STATUS_MESSAGE_LINE],
    PG_TEXT_WHITE);
  }
}


void
info_thread(SceSize args, void *argp)
{
  while (!loc_info_thread_end) {

    mftpDisplayStatusMessage();
    mftpDisplayBatteryMessage();

    sceKernelDelayThread(100000);
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

	ret = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);

	if (ret < 0)
		return ret;

	ret = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

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
	p.base.message_lang = 0;
	p.base.ctrl_assign = 0;
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
			sceKernelDelayThread(1000000 / 30);
		}
		
		// Shutdown Utility
		int stop = sceUtilityNetconfShutdownStart();
		
		// Wait for Shutdown
		while(sceUtilityNetconfGetStatus() != PSP_UTILITY_DIALOG_NONE) sceKernelDelayThread(100000);
		
		// Connect Success
		if(p.base.result == 0) return 1;
	}
	
	return 0;
}

static void 
DoInetNetFtpd(void)
{
  int info_thid;
  int last_state;
  int err;
  char szMyIPAddr[32];
  int connectionConfig = -1;
  PICKER pickConn; // connection picker
  int iNetIndex;
  int iPick;

  pgInit();

  err = initializeNetwork();

  if (err < 0) {
  	pgFramePrint(0, 0, "Internet Library Initialization Error!", PG_TEXT_COLOR);
  	sceKernelDelayThread(1000000);
    	goto close_connection;
  }

  info_thid = sceKernelCreateThread( "info_thread", 
     (SceKernelThreadEntry)info_thread, 0x18, 0x10000, 0, 0 );

  if(info_thid >= 0) {
    sceKernelStartThread(info_thid, 0, 0);
  }

  while (sceWlanGetSwitchState() != 1)  //switch = off
  {
    pgFramePrint(0,0, "Activate WiFi on your VITA!", PG_TEXT_COLOR);
    sceKernelDelayThread(1000000);
    goto close_connection;
  }

	if(connect_to_apctl() == 0)
	{
		pgFramePrint(0,0, "Router Connection failed!", PG_TEXT_COLOR);
		sceKernelDelayThread(1000000);
		goto close_connection;
	}

  // get IP address
  if (sceNetApctlGetInfo(8, szMyIPAddr) != 0) {
    pgFramePrint(0, 0, "Couldn't aquire valid IP Address!", PG_TEXT_COLOR);
    sceKernelDelayThread(1000000);
    goto close_connection;
  }

  ftpdLoop(szMyIPAddr);

close_connection:
  err = sceNetApctlDisconnect();

  pgClear();
  pgFramePrint(0,0, "Exiting FTP Server", PG_TEXT_COLOR);
  
  pspSdkInetTerm();
  sceKernelDelayThread(1000000);

  sceKernelExitDeleteThread(info_thid);
  sceKernelWaitThreadEnd(info_thid, NULL);
}

void user_thread(SceSize args, void *argp)
{
  psp_setup_callbacks();
  DoInetNetFtpd();
}

int 
main(int argc, char **argv)
{
  int user_thid;

  pspDebugScreenInit();

  psp_init_stuff(argc, argv);

  //sceUtilityLoadNetModule(1); /* common */
  //sceUtilityLoadNetModule(3); /* inet */

  psp_read_config();

  user_thid = sceKernelCreateThread( "user_thread", 
     (SceKernelThreadEntry)user_thread, 0x16, 256*1024, PSP_THREAD_ATTR_USER, 0 );
  if(user_thid >= 0) {
    sceKernelStartThread(user_thid, 0, 0);
    sceKernelWaitThreadEnd(user_thid, NULL);
  }

  psp_exit(0);

  return 0;
}

