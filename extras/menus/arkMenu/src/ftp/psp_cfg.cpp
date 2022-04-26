/*
 *  Copyright (C) 2006 Zx / Ludovic Jacomme (ludovic.jacomme@gmail.com)
 */

#include <stdio.h>
#include "std.h"
#include "util.h"
#include "ftp.h"
#include "psp_init.h"
//#include "nlh.h"
#include <string.h>
#include "psp_cfg.h"

 mftpConfig_t mftp_config;

static mftpUser_t*
cfg_add_new_user()
{
  mftpUser_t *NewUser;

  NewUser = (mftpUser_t *)malloc(sizeof(mftpUser_t));
  memset(NewUser,0,sizeof(mftpUser_t));
  strcpy(NewUser->root,"ms0:");
  NewUser->next = mftp_config.head_user;
  mftp_config.head_user = NewUser;

  return NewUser;
}

mftpUser_t*
cfg_get_user(char *User, char *Password)
{
  mftpUser_t *ScanUser;

  for (ScanUser  = (mftpUser_t *)mftp_config.head_user;
       ScanUser != (mftpUser_t *)0;
       ScanUser  = ScanUser->next)
  {
    if ((!strcmp(ScanUser->user    , User    )) && 
        (!strcmp(ScanUser->password, Password))) 
    {
      return ScanUser;
    }
  }
  return (mftpUser_t *)0;
}

int
psp_read_config(void)
{
  char        cfg_filename[128];
  char        Buffer[512];
  FILE       *CfgFile;
  char       *Begin;
  char       *Scan;
  mftpUser_t *NewUser;

  mftp_config.auth_required = 0;
  mftp_config.head_user = (mftpUser_t *)0;
  NewUser = (mftpUser_t *)0;

  strcpy(cfg_filename,psp_home_dir);
  strcat(cfg_filename,"/psp-ftpd.cfg");

  CfgFile = fopen(cfg_filename, "r");

  if (CfgFile != (FILE*)0) {
    while (fgets(Buffer,512,CfgFile) != (char *)0) {
      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';

      if (Buffer[0] == '#') continue;

      if (!strncasecmp(Buffer,"auth=",5)) {
        Begin = Buffer + 5;
        Scan = strchr(Begin,':');
        if (Scan) {
          NewUser = cfg_add_new_user();

          strncpy(NewUser->user,Begin,Scan - Begin);
          strcpy(NewUser->password, Scan + 1);

          if (strlen(NewUser->user) && strlen(NewUser->password)) {
            mftp_config.auth_required = 1;
          }
        }
      } else 
      if (!strncasecmp(Buffer,"root=",5)) {
        if (NewUser == (mftpUser_t *)0) {
          NewUser = cfg_add_new_user();
        }
        strcpy(NewUser->root, Buffer + 5);
        NewUser = (mftpUser_t *)0;
      }
    }

    fclose(CfgFile);
  }

  if (mftp_config.head_user == (mftpUser_t *)0) {
    NewUser = cfg_add_new_user();
  } else {
    pspDebugScreenPrintf("\nAdd the following user(s) to the database:\n\n");
    for (NewUser  = mftp_config.head_user;
         NewUser != (mftpUser_t*)0;
         NewUser  = NewUser->next) {
       pspDebugScreenPrintf("  > user='%s' with root='%s'\n", NewUser->user, NewUser->root);
    }
    sceKernelDelayThread(5000000); 
  }

  return 0;
}
