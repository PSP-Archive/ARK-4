#include "mp3.h"
#include "debug.h"

#define printf(x) sceIoWrite(2, x, strlen(x))

#define MP3BUF_SIZE 128*1024
#define PCMBUF_SIZE 128*(1152/2)

static char mp3Buf[MP3BUF_SIZE]  __attribute__((aligned(64)));
static short pcmBuf[PCMBUF_SIZE]  __attribute__((aligned(64)));

static int running = 0;
static int eof = 0;
static int bufferCounter = 0;
static SceUID mp3Thread = -1;
static SceUID mp3_mutex = sceKernelCreateSema("mp3_mutex", 0, 1, 1, 0);


int fillStreamBuffer(int fd, int handle, void* buffer, int buffer_size)
{

	// read from memory buffer
	if (buffer != NULL){
	
		if (eof || bufferCounter >= buffer_size){
			return 0;
		}
	
		char* dst;
		SceInt32 write;
		SceInt32 pos;
		
		int status = sceMp3GetInfoToAddStreamData(handle, (SceUChar8**)&dst, &write, &pos);
		if (status < 0){
			return 0;
		}
			
		if (pos + write > buffer_size){
			write = buffer_size-pos;
			eof = true;
		}
		bufferCounter += write;
		memcpy(dst, (u8*)buffer+pos, write);
		status = sceMp3NotifyAddStreamData(handle, write);
		if (status < 0){
			return 0;
		}
		return (pos > 0);
	}
	// read from file
	else{
		char* dst;
		SceInt32 write;
		SceInt32 pos;
		// Get Info on the stream (where to fill to, how much to fill, where to fill from)
		int status = sceMp3GetInfoToAddStreamData(handle, (SceUChar8**)&dst, &write, &pos);
		if (status < 0)
			return 0;

		// Seek file to position requested
		status = sceIoLseek32( fd, pos, SEEK_SET );
		if (status < 0)
			return 0;

		// Read the amount of data
		int read = sceIoRead( fd, dst, write );
		if (read < 0)
			return 0;
		else if (read == 0){
			// End of file?
			eof = true;
			return 0;
		}

		// Notify mp3 library about how much we really wrote to the stream buffer
		status = sceMp3NotifyAddStreamData( handle, read );
		if (status < 0)
			return 0;

		return (pos > 0);
	}
}

void playMP3File(char* filename, void* buffer, int buffer_size)
{

	if (filename == NULL && buffer == NULL)
		return;

	int file_handle = -1;
	int mp3_handle;

	int status;

	sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	sceUtilityLoadModule(PSP_MODULE_AV_MP3);

	eof = 0;
	bufferCounter = 0;

	if (buffer == NULL){
		file_handle = sceIoOpen(filename, PSP_O_RDONLY, 0777 );
		if(file_handle < 0) {
			return;
		}
	}

	status = sceMp3InitResource();
	if(status < 0) {
		return;
	}


	// Reserve a mp3 handle for our playback
	SceMp3InitArg mp3Init;
	memset(mp3Buf, 0, sizeof(mp3Buf));
	memset(pcmBuf, 0, sizeof(pcmBuf));
	mp3Init.mp3StreamStart = 0;
	mp3Init.mp3StreamEnd = (buffer == NULL)? sceIoLseek32( file_handle, 0, SEEK_END ) : buffer_size;
	mp3Init.unk1 = 0;
	mp3Init.unk2 = 0;
	mp3Init.mp3Buf = mp3Buf;
	mp3Init.mp3BufSize = sizeof(mp3Buf);
	mp3Init.pcmBuf = pcmBuf;
	mp3Init.pcmBufSize = sizeof(pcmBuf);

	mp3_handle = sceMp3ReserveMp3Handle( &mp3Init );

	if (mp3_handle < 0){
		return;
	}

	// Fill the stream buffer with some data so that sceMp3Init has something to
	// work with
	fillStreamBuffer(file_handle, mp3_handle, buffer, buffer_size);

	status = sceMp3Init( mp3_handle );
	if (status < 0){
		return;
	}

	int channel = -1;
	int samplingRate = sceMp3GetSamplingRate( mp3_handle );
	int numChannels = sceMp3GetMp3ChannelNum( mp3_handle );
	int lastDecoded = 0;
	int volume = PSP_AUDIO_VOLUME_MAX;
	int numPlayed = 0;

	if (!running)
		running = 1;

	int loopContinue = mp3Init.mp3StreamEnd/100;
	
	sceAudioSRCChRelease();

	//sceKernelDelayThread(10000);
	while (running) {
		
		 // Check if we need to fill our stream buffer
		if (sceMp3CheckStreamDataNeeded( mp3_handle ) > 0){
			if (!fillStreamBuffer(file_handle, mp3_handle, buffer, buffer_size)){
				break;
			}
			loopContinue = mp3Init.mp3StreamEnd/100;
		}
		else {
			loopContinue--;
		}

		// Decode some samples
		short* buf;
		unsigned int bytesDecoded;
		int retries = 0;

		bytesDecoded = sceMp3Decode(mp3_handle, &buf);

		/*
		if (bytesDecoded < 0 && bytesDecoded != 0x80671402)
		{
			running = 0;
			break;
		}
		*/
		// Nothing more to decode? Must have reached end of input buffer
		if (bytesDecoded <= 0)
		{
			break;
		} else {
			// Reserve the Audio channel for our output if not yet done
			if (channel < 0 || lastDecoded != bytesDecoded)
			{
				if (channel >= 0)
					sceAudioSRCChRelease();

				channel = sceAudioSRCChReserve( bytesDecoded/(2*numChannels),
					samplingRate, numChannels );
			}

			// Output the decoded samples and accumulate the 
			// number of played samples to get the playtime
			numPlayed += sceAudioSRCOutputBlocking( volume, buf );
		}
		//sceKernelDelayThread(10000);
		
		if (loopContinue<=0)
			break;
		
	}

	// Reset the state of the player to the initial starting state
	sceMp3ResetPlayPosition( mp3_handle );
	numPlayed = 0;

	// Cleanup time...
	if (channel>=0)
	  sceAudioSRCChRelease();

	status = sceMp3ReleaseMp3Handle( mp3_handle );

	status = sceMp3TermResource();

	if (buffer == NULL)
		status = sceIoClose( file_handle );

	file_handle = -1;
	mp3_handle = -1;
	running = 0;
}

MP3::MP3(void* buffer, int size){
	this->filename = NULL;
	this->buffer_size = size;
	this->buffer = buffer;
}

MP3::MP3(char* filename, bool to_buffer){
	if (!to_buffer){
		this->filename = filename;
		this->buffer = NULL;
		this->buffer_size = 0;
	}
	else {
		this->filename = NULL;
		SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0777 );
		this->buffer_size = sceIoLseek32(fd, 0, SEEK_END);
		sceIoLseek(fd, 0, SEEK_SET);
		this->buffer = malloc(this->buffer_size);
		sceIoRead(fd, this->buffer, this->buffer_size);
		sceIoClose(fd);
	}
}

MP3::~MP3(){
	if (this->buffer != NULL)
		free(this->buffer);
}

void* MP3::getBuffer(){
	return this->buffer;
}

int MP3::getBufferSize(){
	return this->buffer_size;
}

void MP3::play(){
	if (running){
		//stop();
		sceKernelWaitThreadEnd(mp3Thread, 0);
	}
	printf("starting mp3 thread\n");
	running = true;
	mp3Thread = sceKernelCreateThread("mp3_thread", MP3::playThread, 0x3D, 0x10000, PSP_THREAD_ATTR_USER, NULL);
	sceKernelStartThread(mp3Thread,  sizeof(this), this);
}

void MP3::stop(){
	running = false;
}

int MP3::playThread(SceSize _args, void *_argp)
{
	MP3* self = (MP3*)_argp;
	sceKernelWaitSema(mp3_mutex, 1, 0);
	playMP3File(self->filename, self->buffer, self->buffer_size);
	sceKernelSignalSema(mp3_mutex, 1);
	sceKernelExitDeleteThread(0);
	return 0;
}
