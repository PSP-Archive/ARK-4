#include "out.h"

#include <pspaudio.h>
#include <pspiofilemgr.h>
#include <string.h>

// Sample Alignment
#define PSP_NUM_AUDIO_SAMPLES 1024

// Current Channel
int currentChannel = -1;

// Alternate Channel
int alternateChannel = -1;

// Last Feed Silence Sample Count
int silenceSamples = 0;

// SETUP SOUND
static int none_init(void)
{
	// Reserve Main Channel
	currentChannel = sceAudioChReserve(-1, PSP_NUM_AUDIO_SAMPLES, 0);
	
	#ifdef DEBUG
	// Log Main Channel Creation
	printk("sceAudioChReserve(MainChannel): %08X\n", currentChannel);
	#endif
	
	// Main Channel Reservation Error
	if(currentChannel < 0) return -1;
	
	// Reserve Alternate Channel
	alternateChannel = sceAudioChReserve(-1, PSP_NUM_AUDIO_SAMPLES, 0);
	
	#ifdef DEBUG
	// Log Alternate Channel Creation
	printk("sceAudioChReserve(AlternateChannel): %08X\n", alternateChannel);
	#endif
	
	// Alternate Channel Reservation Error
	if(alternateChannel < 0)
	{
		// Release Main Channel
		sceAudioChRelease(currentChannel);
		
		// Return Error to Emulator
		return -1;
	}
	
	// Initialization Success
	return 0;
}

// REMOVE SOUND
static void none_finish(void)
{
	// Release Main Channel
	if(currentChannel >= 0) sceAudioChRelease(currentChannel);
	
	// Release Alternate Channel
	if(alternateChannel >= 0) sceAudioChRelease(alternateChannel);
	
	// Erase References
	currentChannel = -1;
	alternateChannel = -1;
}

// GET BYTES BUFFERED
static int none_busy(void)
{
	// Fetch remaining samples
	int remainingSamples = sceAudioGetChannelRestLength(currentChannel);
	
	#ifdef DEBUG
	// Log remaining Samples
	printk("sceAudioGetChannelRestLength: %08X (%08X)\n", remainingSamples, currentChannel);
	#endif
	
	// Time to switch channels!
  if(remainingSamples >= 0 && remainingSamples <= silenceSamples) return 0;
  
  // Busy playing Audio
  return 1;
}

// FEED SOUND DATA
static void none_feed(void * buf, int bytes)
{
	// Calculate Number of Samples (1 Sample = 2 * 2 (Stereo) Bytes = 4 Bytes)
	int samples = bytes >> 2;
	
	// Uh-oh... Channel can't fit whole Feed!
	if(samples > PSP_NUM_AUDIO_SAMPLES)
	{
		#ifdef DEBUG
		// Log Problem
		printk("SPU Feed exceeds channel capacity! (%d > %d)\n", samples, PSP_NUM_AUDIO_SAMPLES);
		#endif
		
		// Exit Feeder
		return;
	}
	
	// Sample Buffer
	unsigned short * feedBuffer[PSP_NUM_AUDIO_SAMPLES * 2];
	
	// Clear Buffer (fill with silence)
	memset(feedBuffer, 0, sizeof(feedBuffer));
	
	// Copy PSX Samples into Sample Buffer
	memcpy(feedBuffer, buf, bytes);
	
	// Switch Audio Channels
	int lastChannel = currentChannel;
	currentChannel = alternateChannel;
	alternateChannel = lastChannel;
	
	// Set Silence Sample Count
	silenceSamples = PSP_NUM_AUDIO_SAMPLES - samples;
	
	// Output Audio Data
	#ifdef DEBUG
	int output = 
	#endif
	sceAudioOutputPannedBlocking(currentChannel, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, feedBuffer);
	
	// Debug Log
	#ifdef DEBUG
	printk("sceAudioOutputPannedBlocking: %08X (%08X)\n", output, currentChannel);
	#endif
}

void out_register_none(struct out_driver *drv)
{
	drv->name = "none";
	drv->init = none_init;
	drv->finish = none_finish;
	drv->busy = none_busy;
	drv->feed = none_feed;
}

