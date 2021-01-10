/***************************************************************************
                            oss.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include "out.h"

////////////////////////////////////////////////////////////////////////
// oss globals
////////////////////////////////////////////////////////////////////////

#define OSS_MODE_STEREO	    1
#define OSS_MODE_MONO       0

#define OSS_SPEED_44100     44100

static int oss_audio_fd = -1;
extern int errno;

////////////////////////////////////////////////////////////////////////
// SETUP SOUND
////////////////////////////////////////////////////////////////////////

static int oss_init(void)
{
 int pspeed=44100;
 int pstereo;
 int format;
 int fragsize = 0;
 int myfrag;
 int oss_speed, oss_stereo;

 pstereo = OSS_MODE_STEREO;
 oss_speed = pspeed;
 oss_stereo = pstereo;

 if((oss_audio_fd=open("/dev/dsp",O_WRONLY,0))==-1)
  {
   printf("OSS device not available\n");
   return -1;
  }

 if(ioctl(oss_audio_fd,SNDCTL_DSP_RESET,0)==-1)
  {
   printf("Sound reset failed\n");
   return -1;
  }

 // we use 64 fragments with 1024 bytes each
 // rearmed: now using 10*4096 for better latency

 fragsize=12;
 myfrag=(10<<16)|fragsize;

 if(ioctl(oss_audio_fd,SNDCTL_DSP_SETFRAGMENT,&myfrag)==-1)
  {
   printf("Sound set fragment failed!\n");
   return -1;
  }

 format = AFMT_S16_NE;

 if(ioctl(oss_audio_fd,SNDCTL_DSP_SETFMT,&format) == -1)
  {
   printf("Sound format not supported!\n");
   return -1;
  }

 if(format!=AFMT_S16_NE)
  {
   printf("Sound format not supported!\n");
   return -1;
  }

 if(ioctl(oss_audio_fd,SNDCTL_DSP_STEREO,&oss_stereo)==-1 || !oss_stereo)
  {
   printf("Stereo mode not supported!\n");
   return -1;
  }

 if(ioctl(oss_audio_fd,SNDCTL_DSP_SPEED,&oss_speed)==-1)
  {
   printf("Sound frequency not supported\n");
   return -1;
  }

 if(oss_speed!=pspeed)
  {
   printf("Sound frequency not supported\n");
   return -1;
  }

 return 0;
}

////////////////////////////////////////////////////////////////////////
// REMOVE SOUND
////////////////////////////////////////////////////////////////////////

static void oss_finish(void)
{
 if(oss_audio_fd != -1 )
  {
   close(oss_audio_fd);
   oss_audio_fd = -1;
  }
}

////////////////////////////////////////////////////////////////////////
// GET BUFFERED STATUS
////////////////////////////////////////////////////////////////////////

static int oss_busy(void)
{
 audio_buf_info info;
 unsigned long l;

 if(oss_audio_fd == -1) return 1;
 if(ioctl(oss_audio_fd,SNDCTL_DSP_GETOSPACE,&info)==-1)
  l=0;
 else
  {
   if(info.fragments<(info.fragstotal>>1))             // can we write in at least the half of fragments?
        l=1;                                           // -> no? wait
   else l=0;                                           // -> else go on
  }

 return l;
}

////////////////////////////////////////////////////////////////////////
// FEED SOUND DATA
////////////////////////////////////////////////////////////////////////

static void oss_feed(void *buf, int bytes)
{
 if(oss_audio_fd == -1) return;
 write(oss_audio_fd, buf, bytes);
}

void out_register_oss(struct out_driver *drv)
{
	drv->name = "oss";
	drv->init = oss_init;
	drv->finish = oss_finish;
	drv->busy = oss_busy;
	drv->feed = oss_feed;
}
