#include <pspsdk.h>
#include <pspkernel.h>
#include <pspatrac3.h>
#include <pspaudio.h>
#include <psputility.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTEALIGN  (64)
#define MAX_SAMPLE             (2048)
#define AUDIO_OUTPUT_SAMPLE    MAX_SAMPLE
#define BUFFER_NUM             (3)
#define WRITEBUF_SIZE          (BUFFER_NUM  * (MAX_SAMPLE))

int sceAtracGetSecondBufferInfo(int atracID, u32 *puiPosition, u32 *puiDataByte);
int sceAtracGetNextDecodePosition(int atracID, u32 *puiSamplePosition);

static int* decode_data;

static int* abortT = NULL;

static char *at3_data = NULL;
static int at3_size = 0;
static int threadDelay = 0;

void resetAt3Data(){
	at3_data = NULL;
	at3_size = 0;
	abortT = NULL;
	threadDelay = 0;
}

void setAt3Data(char* data, int size, int* abortVar, int delay){
	at3_data = data;
	at3_size = size;
	abortT = abortVar;
	threadDelay = delay;
}

unsigned getAT3Frequency(){
	unsigned frequency = *(int*)(at3_data+0x18);
	if (frequency != 48000 && frequency != 44100)
		frequency = 44100;
	return frequency;
}

int AT3_T(SceSize argc, void* argv) {

	if (abortT == NULL || at3_data == NULL || at3_size == 0){ // check if needed data is set
		return 0;
	}

	// allocate space for output buffer, must be 64 bit aligned
	decode_data = (int*)memalign(64, (WRITEBUF_SIZE+MAX_SAMPLE-1)*sizeof(int));
	
	memset(decode_data, 0, (WRITEBUF_SIZE+MAX_SAMPLE-1)*sizeof(int));
	
	int write_pos = 0; // position of buffer to write
	int play_pos = 0; // position of buffer to read

	int atracID;
	int channel;

	// load the needed utilities if not done already
	sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	sceUtilityLoadModule(PSP_MODULE_AV_ATRAC3PLUS);
	
	// set the atrac3 data and its size
	atracID = sceAtracSetDataAndGetID(at3_data, at3_size);
	
	// set to infinite loop playback
	sceAtracSetLoopNum(atracID, -1);

	int max_samples = 0;
	sceAtracGetMaxSample(atracID, &max_samples);

	sceAudioSRCChRelease(); // release the channel if it's already taken
	channel = sceAudioSRCChReserve(max_samples, getAT3Frequency(), 2); // reserve the channel
	
	int end = 0;
	while (!end && !*abortT) {
		int remainFrame = 0;
		int samples = 0;

		sceAtracDecodeData(atracID, (u16 *)&decode_data[write_pos], &samples, &end, &remainFrame); // decode next frame
		
		write_pos += samples; // update current write buffer
		
		if (write_pos - play_pos >= samples){
		
			sceAudioSRCOutputBlocking(PSP_AUDIO_VOLUME_MAX, (void*)&decode_data[play_pos]); // output data from the play buffer
		
			while (sceAudioGetChannelRestLen(channel) > 0){ // wait for the audio to be outputted
				sceKernelDelayThread(0);
			}
			play_pos += samples;
		}
		if (write_pos >= WRITEBUF_SIZE) { // we've played the last buffer
			write_pos -= play_pos;
			memcpy(decode_data, &decode_data[play_pos],
				   sizeof(int) * write_pos);
			play_pos = 0;
		}
		sceKernelDelayThread(threadDelay); // allow other threads to run
	}
	// release all resources
	sceAudioSRCChRelease();
	sceAtracReleaseAtracID(atracID);
	free(decode_data);

	return 0;
}
