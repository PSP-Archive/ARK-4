#include "mp3.h"
#include "debug.h"

#define SAMPLE_PER_FRAME 1152
#define MP3BUF_SIZE (8*1024)
#define PCMBUF_SIZE (SAMPLE_PER_FRAME*2*4)

//static char mp3Buf[MP3BUF_SIZE]  __attribute__((aligned(64)));
//static short pcmBuf[PCMBUF_SIZE]  __attribute__((aligned(64)));

static bool running = false;
static bool eof = false;
static SceUID mp3Thread = -1;
static SceUID mp3_mutex = sceKernelCreateSema("mp3_mutex", 0, 1, 1, 0);
static bool paused = false;


bool fillStreamBuffer(int fd, int handle, void* buffer, int buffer_size)
{
    bool res = 0;
    char* dst;
    SceInt32 write;
    SceInt32 pos;

    if (eof) return false;

    // Get Info on the stream (where to fill to, how much to fill, where to fill from)
    int status = sceMp3GetInfoToAddStreamData(handle, (SceUChar8**)&dst, &write, &pos);
    if (status < 0){
        return 0;
    }

    // read from file
    if (fd >= 0){

        // Seek file to position requested
        status = sceIoLseek32( fd, pos, SEEK_SET );
        if (status < 0)
            return false;

        // Read the amount of data
        int read = sceIoRead( fd, dst, write );
        if (read <= 0){
            // End of file?
            eof = true;
            return false;
        }
        write = read;
        res = (pos > 0);
    }
    // read from memory buffer
    else{
        
        if (pos >= buffer_size){
            eof = true;
            return false;
        }

        if (pos + write > buffer_size){
            write = buffer_size-pos;
            eof = true;
        }
        memcpy(dst, (u8*)buffer+pos, write);
        res = (pos > 0);
    }

    // Notify mp3 library about how much we really wrote to the stream buffer
    status = sceMp3NotifyAddStreamData( handle, write);
    if (status < 0){
        return false;
    }
    return res;
}

u32 findMP3StreamStart(int file_handle, void* buffer, int buffer_size, char* tmp_buf){
    u8* buf = (u8*)buffer;
    if (file_handle>=0){
        buf = (u8*)tmp_buf;
        sceIoRead(file_handle, buf, MP3BUF_SIZE);
    }
    if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3'){
        u32 header_size = (buf[9] | (buf[8]<<7) | (buf[7]<<14) | (buf[6]<<21));
        return header_size+10;
    }
    else if (buf[0] == 'A' && buf[1] == 'P' && buf[2] == 'E'){
        u32 header_size = (buf[12] | (buf[13]<<8) | (buf[14]<<16) | (buf[15]<<24));
        return header_size+32;
    }
    return 0;
}

void playMP3File(char* filename, void* buffer, int buffer_size)
{

    if (filename == NULL && buffer == NULL){
        running = false;
        return;
    }

    int file_handle = -1;
    int mp3_handle;

    int status;

    eof = false;

    if (filename != NULL){
        file_handle = sceIoOpen(filename, PSP_O_RDONLY, 0777 );
        if(file_handle < 0) {
            running = false;
            return;
        }
    }

    status = sceMp3InitResource();
    if(status < 0) {
        running = false;
        return;
    }


    // Reserve a mp3 handle for our playback
    SceMp3InitArg mp3Init;
    char* mp3Buf = (char*)memalign(64, MP3BUF_SIZE);
    short* pcmBuf = (short*)memalign(64, PCMBUF_SIZE);
    memset(mp3Buf, 0, MP3BUF_SIZE);
    memset(pcmBuf, 0, PCMBUF_SIZE);
    mp3Init.mp3StreamStart = findMP3StreamStart(file_handle, buffer, buffer_size, mp3Buf);
    mp3Init.mp3StreamEnd = (file_handle >= 0)? sceIoLseek32( file_handle, 0, SEEK_END ) : buffer_size;
    mp3Init.unk1 = 0;
    mp3Init.unk2 = 0;
    mp3Init.mp3Buf = mp3Buf;
    mp3Init.mp3BufSize = MP3BUF_SIZE;
    mp3Init.pcmBuf = pcmBuf;
    mp3Init.pcmBufSize = PCMBUF_SIZE;

    int channel = -1;
    int lastDecoded = 0;
    int samplingRate, numChannels, max_sample;

    mp3_handle = sceMp3ReserveMp3Handle( &mp3Init );

    if (mp3_handle < 0){
        goto mp3_terminate;
    }

    // Fill the stream buffer with some data so that sceMp3Init has something to
    // work with
    fillStreamBuffer(file_handle, mp3_handle, buffer, buffer_size);
    
    status = sceMp3Init( mp3_handle );
    
    if (status < 0){
        goto mp3_terminate;
    }

    if (!running)
        running = true;

    sceMp3SetLoopNum(mp3_handle, 0);

    samplingRate = sceMp3GetSamplingRate(mp3_handle);
    numChannels = sceMp3GetMp3ChannelNum(mp3_handle);
    max_sample = sceMp3GetMaxOutputSample(mp3_handle);

    sceAudioSRCChRelease();
    channel = sceAudioSRCChReserve(max_sample, samplingRate, numChannels);

    if (channel < 0) goto mp3_terminate;

    while (running) {

        if (paused){
            sceKernelDelayThread(10000);
            continue;
        }
        
         // Check if we need to fill our stream buffer
        if (sceMp3CheckStreamDataNeeded( mp3_handle ) > 0){
            if (!fillStreamBuffer(file_handle, mp3_handle, buffer, buffer_size)){
                break;
            }
        }

        // Decode some samples
        short* buf;
        unsigned int bytesDecoded;

        bytesDecoded = sceMp3Decode(mp3_handle, &buf);

        // Nothing more to decode? Must have reached end of input buffer
        if (bytesDecoded <= 0)
        {
            break;
        }

        // Output the decoded samples and accumulate the 
        // number of played samples to get the playtime
        sceAudioSRCOutputBlocking(PSP_AUDIO_VOLUME_MAX, buf );
    }

    mp3_terminate:

    // Cleanup time...
    if (channel >= 0){
        while (sceAudioSRCChRelease() < 0){ // wait for the audio to be outputted
            sceKernelDelayThread(100);
        }
    }

    status = sceMp3ReleaseMp3Handle( mp3_handle );

    status = sceMp3TermResource();

    if (file_handle >= 0)
        status = sceIoClose( file_handle );

    file_handle = -1;
    mp3_handle = -1;
    running = false;
    free(mp3Buf);
    free(pcmBuf);
}

MP3::MP3(void* buffer, int size){
    this->filename = NULL;
    this->buffer_size = size;
    this->buffer = buffer;
    this->on_music_end = NULL;
}

MP3::MP3(char* filename, bool to_buffer){
    this->on_music_end = NULL;
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
    sceKernelWaitSema(mp3_mutex, 1, 0);
    if (!running){
        running = true;
        void* self = (void*)this;
        mp3Thread = sceKernelCreateThread("", (SceKernelThreadEntry)MP3::playThread, 0x10, 0x10000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
        sceKernelStartThread(mp3Thread,  sizeof(self), &self);
    }
    sceKernelSignalSema(mp3_mutex, 1);
}

void MP3::stop(){
    running = false;
    sceKernelWaitThreadEnd(mp3Thread, 0);
}

void MP3::pauseResume(){
    paused = !paused;
}

int MP3::isPlaying(){
    return running;
}

int MP3::isPaused(){
    return paused;
}

int MP3::playThread(SceSize _args, void** _argp)
{
    MP3* self = (MP3*)(*_argp);
    playMP3File(self->filename, self->buffer, self->buffer_size);
    if (self->on_music_end) self->on_music_end(self);
    sceKernelExitDeleteThread(0);
    return 0;
}

void MP3::fullStop(){
    running = false;
    sceKernelWaitThreadEnd(mp3Thread, NULL);
}