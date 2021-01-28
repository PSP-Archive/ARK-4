/***************************************************************************
		pulseaudio.c  -  description
		     -------------------
begin                : Thu Feb 04 2010
copyright            : (C) 2010 by Tristin Celestin
email                : cetris1@umbc.edu
comment              : Much of this was taken from simple.c, in the pulseaudio
                       library
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

#include "stdafx.h"

#define _IN_OSS

#include "externals.h"
#include <pulse/pulseaudio.h>

////////////////////////////////////////////////////////////////////////
// pulseaudio structs
////////////////////////////////////////////////////////////////////////

typedef struct {
     pa_threaded_mainloop *mainloop;
     pa_context *context;
     pa_mainloop_api *api;
     pa_stream *stream;
     pa_sample_spec spec;
     int first;
} Device;

typedef struct {
     unsigned int frequency;
     unsigned int latency_in_msec;
} Settings;

////////////////////////////////////////////////////////////////////////
// pulseaudio globals
////////////////////////////////////////////////////////////////////////

static Device device = {
     .mainloop = NULL,
     .api = NULL,
     .context = NULL,
     .stream = NULL
};

static Settings settings = {
     .frequency = 44100,
     .latency_in_msec = 20,
};

// the number of bytes written in SoundFeedStreamData
const int mixlen = 3240;

// used to calculate how much space is used in the buffer, for debugging purposes
//int maxlength = 0;

////////////////////////////////////////////////////////////////////////
// CALLBACKS FOR THREADED MAINLOOP
////////////////////////////////////////////////////////////////////////
static void context_state_cb (pa_context *context, void *userdata)
{
     Device *dev = userdata;

     if ((context == NULL) || (dev == NULL))
	  return;

     switch (pa_context_get_state (context))
     {
     case PA_CONTEXT_READY:
     case PA_CONTEXT_TERMINATED:
     case PA_CONTEXT_FAILED:
	  pa_threaded_mainloop_signal (dev->mainloop, 0);
	  break;

     case PA_CONTEXT_UNCONNECTED:
     case PA_CONTEXT_CONNECTING:
     case PA_CONTEXT_AUTHORIZING:
     case PA_CONTEXT_SETTING_NAME:
	  break;
     }
}

static void stream_state_cb (pa_stream *stream, void * userdata)
{
     Device *dev = userdata;
    
     if ((stream == NULL) || (dev == NULL))
	  return;

     switch (pa_stream_get_state (stream))
     {
     case PA_STREAM_READY:
     case PA_STREAM_FAILED:
     case PA_STREAM_TERMINATED:
	  pa_threaded_mainloop_signal (dev->mainloop, 0);
	  break;

     case PA_STREAM_UNCONNECTED:
     case PA_STREAM_CREATING:
	  break;
     }
}

static void stream_latency_update_cb (pa_stream *stream, void *userdata)
{
     Device *dev = userdata;

     if ((stream == NULL) || (dev == NULL))
	  return;

     pa_threaded_mainloop_signal (dev->mainloop, 0);
}

static void stream_request_cb (pa_stream *stream, size_t length, void *userdata)
{
     Device *dev = userdata;

     if ((stream == NULL) || (dev == NULL))
	  return;
     pa_threaded_mainloop_signal (dev->mainloop, 0);
}

////////////////////////////////////////////////////////////////////////
// SETUP SOUND
////////////////////////////////////////////////////////////////////////

static void pulse_init(void)
{
     int error_number;

     // Acquire mainloop ///////////////////////////////////////////////////////
     device.mainloop = pa_threaded_mainloop_new ();
     if (device.mainloop == NULL)
     {
	  fprintf (stderr, "Could not acquire PulseAudio main loop\n");
	  return -1;
     }

     // Acquire context ////////////////////////////////////////////////////////
     device.api = pa_threaded_mainloop_get_api (device.mainloop);
     device.context = pa_context_new (device.api, "PCSX");
     pa_context_set_state_callback (device.context, context_state_cb, &device);

     if (device.context == NULL)
     {
	  fprintf (stderr, "Could not acquire PulseAudio device context\n");
	  return -1;
     }

     // Connect to PulseAudio server ///////////////////////////////////////////
     if (pa_context_connect (device.context, NULL, 0, NULL) < 0)
     {
	  error_number = pa_context_errno (device.context);
	  fprintf (stderr, "Could not connect to PulseAudio server: %s\n", pa_strerror(error_number));
	  return -1;
     }

     // Run mainloop until sever context is ready //////////////////////////////
     pa_threaded_mainloop_lock (device.mainloop);
     if (pa_threaded_mainloop_start (device.mainloop) < 0)
     {
	  fprintf (stderr, "Could not start mainloop\n");
	  return -1;
     }

     pa_context_state_t context_state;
     context_state = pa_context_get_state (device.context);
     while (context_state != PA_CONTEXT_READY)
     {
	  context_state = pa_context_get_state (device.context);
	  if (! PA_CONTEXT_IS_GOOD (context_state))
	  {
	       error_number = pa_context_errno (device.context);
	       fprintf (stderr, "Context state is not good: %s\n", pa_strerror (error_number));
	       return -1;
	  }
	  else if (context_state == PA_CONTEXT_READY)
	       break;
	  else
	       fprintf (stderr, "PulseAudio context state is %d\n", context_state);
	  pa_threaded_mainloop_wait (device.mainloop);
     }

     // Set sample spec ////////////////////////////////////////////////////////
     device.spec.format = PA_SAMPLE_S16NE;
     device.spec.channels = 2;
     device.spec.rate = settings.frequency;

     pa_buffer_attr buffer_attributes;
     buffer_attributes.tlength = pa_bytes_per_second (& device.spec) / 5;
     buffer_attributes.maxlength = buffer_attributes.tlength * 3;
     buffer_attributes.minreq = buffer_attributes.tlength / 3;
     buffer_attributes.prebuf = buffer_attributes.tlength;

     //maxlength = buffer_attributes.maxlength;
     //fprintf (stderr, "Total space: %u\n", buffer_attributes.maxlength);
     //fprintf (stderr, "Minimum request size: %u\n", buffer_attributes.minreq);
     //fprintf (stderr, "Bytes needed before playback: %u\n", buffer_attributes.prebuf);
     //fprintf (stderr, "Target buffer size: %lu\n", buffer_attributes.tlength);

     // Acquire new stream using spec //////////////////////////////////////////
     device.stream = pa_stream_new (device.context, "PCSX", &device.spec, NULL);
     if (device.stream == NULL)
     {
	  error_number = pa_context_errno (device.context);
	  fprintf (stderr, "Could not acquire new PulseAudio stream: %s\n", pa_strerror (error_number));
	  return -1;
     }

     // Set callbacks for server events ////////////////////////////////////////
     pa_stream_set_state_callback (device.stream, stream_state_cb, &device);
     pa_stream_set_write_callback (device.stream, stream_request_cb, &device);
     pa_stream_set_latency_update_callback (device.stream, stream_latency_update_cb, &device);

     // Ready stream for playback //////////////////////////////////////////////
     pa_stream_flags_t flags = (pa_stream_flags_t) (PA_STREAM_ADJUST_LATENCY | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE);
     //pa_stream_flags_t flags = (pa_stream_flags_t) (PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_EARLY_REQUESTS);
     if (pa_stream_connect_playback (device.stream, NULL, &buffer_attributes, flags, NULL, NULL) < 0)
     {
	  pa_context_errno (device.context);
	  fprintf (stderr, "Could not connect for playback: %s\n", pa_strerror (error_number));
	  return -1;
     }

     // Run mainloop until stream is ready /////////////////////////////////////
     pa_stream_state_t stream_state;
     stream_state = pa_stream_get_state (device.stream);
     while (stream_state != PA_STREAM_READY)
     {
	  stream_state = pa_stream_get_state (device.stream);

	  if (stream_state == PA_STREAM_READY)
	       break;

	  else if (! PA_STREAM_IS_GOOD (stream_state))
	  {
	       error_number = pa_context_errno (device.context);
	       fprintf (stderr, "Stream state is not good: %s\n", pa_strerror (error_number));
	       return -1;
	  }
	  else
	       fprintf (stderr, "PulseAudio stream state is %d\n", stream_state);
	  pa_threaded_mainloop_wait (device.mainloop);
     }

     pa_threaded_mainloop_unlock (device.mainloop);

     fprintf  (stderr, "PulseAudio should be connected\n");
     return 0;
}

////////////////////////////////////////////////////////////////////////
// REMOVE SOUND
////////////////////////////////////////////////////////////////////////
static void pulse_finish(void)
{
     if (device.mainloop != NULL)
	  pa_threaded_mainloop_stop (device.mainloop);

     // Release in reverse order of acquisition
     if (device.stream != NULL)
     {
	  pa_stream_unref (device.stream);
	  device.stream = NULL;

     }
     if (device.context != NULL)
     {
	  pa_context_disconnect (device.context);
	  pa_context_unref (device.context);
	  device.context = NULL;
     }

     if (device.mainloop != NULL)
     {
	  pa_threaded_mainloop_free (device.mainloop);
	  device.mainloop = NULL;
     }

}

////////////////////////////////////////////////////////////////////////
// GET BYTES BUFFERED
////////////////////////////////////////////////////////////////////////

static int pulse_busy(void)
{
     int free_space;
     int error_code;
     long latency;
     int playing = 0;

     if ((device.mainloop == NULL) || (device.api == NULL) || ( device.context == NULL) || (device.stream == NULL))
     	  return 1;

     pa_threaded_mainloop_lock (device.mainloop);
     free_space = pa_stream_writable_size (device.stream);
     pa_threaded_mainloop_unlock (device.mainloop);

     //fprintf (stderr, "Free space: %d\n", free_space);
     //fprintf (stderr, "Used space: %d\n", maxlength - free_space);
     if  (free_space < mixlen * 3)
     {
	  // Don't buffer anymore, just play
	  //fprintf (stderr, "Not buffering.\n");
     	  return 1;
     }
     else 
     {
	  // Buffer some sound
	  //fprintf (stderr, "Buffering.\n");
     	  return 0;
     }
}

////////////////////////////////////////////////////////////////////////
// FEED SOUND DATA
////////////////////////////////////////////////////////////////////////

static void pulse_feed(void *pSound, int lBytes)
{
     int error_code;
     int size;

     if (device.mainloop != NULL)
     {
	  pa_threaded_mainloop_lock (device.mainloop);
	  if (pa_stream_write (device.stream, pSound, lBytes, NULL, 0LL, PA_SEEK_RELATIVE) < 0)
	  {
	       fprintf (stderr, "Could not perform write\n");
	  }
	  else
	  {
	       //fprintf (stderr, "Wrote %d bytes\n", lBytes);
	       pa_threaded_mainloop_unlock (device.mainloop);
	  }
     }
}

void out_register_pulse(struct out_driver *drv)
{
	drv->name = "pulseaudio";
	drv->init = pulse_init;
	drv->finish = pulse_finish;
	drv->busy = pulse_busy;
	drv->feed = pulse_feed;
}
