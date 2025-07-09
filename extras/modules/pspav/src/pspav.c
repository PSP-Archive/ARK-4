#include <pspkernel.h>
#include <pspsdk.h>
#include <psptypes.h>
#include <psppower.h>

#define DISPLAY_VIDEO 1

#include <psputilsforkernel.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <pspgu.h>
#include <pspaudio.h>
#include <psputility.h>
#include <pspatrac3.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <pspmpeg.h>

#include "pspav.h"
#include "pspav_common.h"
#include "pspav_video.h"
#include "pspav_decoder.h"
#include "pspav_reader.h"
#include "pspav_audio.h"
#include "pspav_entry.h"

#define SWAPINT(x) (((x)<<24) | (((uint)(x)) >> 24) | (((x) & 0x0000FF00) << 8) | (((x) & 0x00FF0000) >> 8))

typedef struct {
    int   packets;
    uint  packetsRead;
    uint  packetsWritten;
    uint  packetsFree;
    uint  packetSize;
    void* data;
    uint  callback;
    void* callbackParameter;
    void* dataUpperBound;
    int   semaId;
    SceMpeg* pspav;
} _SceMpegRingbuffer;

typedef struct {
    uint   magic1;
    uint   magic2;
    uint   magic3;
    uint   unk_m1;
    void*  ringbuffer_start;
    void*  ringbuffer_end;
} _SceMpeg;

int retVal;
SceMpegAvcMode m_MpegAvcMode;

ReaderThreadData                    Reader;
VideoThreadData                     Video;
AudioThreadData                     Audio;
DecoderThreadData                   Decoder;
AT3ThreadData                        Atrac3;
AT3ThreadData*                        AT3 = &Atrac3;

SceInt32                            m_MpegStreamOffset;
SceInt32                            m_MpegStreamSize;

SceMpeg                             m_Mpeg;
SceInt32                            m_MpegMemSize;
ScePVoid                            m_MpegMemData = NULL;

SceInt32                            m_RingbufferPackets;
SceInt32                            m_RingbufferSize;
ScePVoid                            m_RingbufferData = NULL;
SceMpegRingbuffer                   m_Ringbuffer;

SceMpegStream*                      m_MpegStreamAVC;
ScePVoid                            m_pEsBufferAVC;
SceMpegAu                           m_MpegAuAVC;

SceMpegStream*                      m_MpegStreamAtrac;
ScePVoid                            m_pEsBufferAtrac;
SceMpegAu                           m_MpegAuAtrac;

SceInt32                            m_MpegAtracEsSize;
SceInt32                            m_MpegAtracOutSize;

SceInt32                            m_iLastTimeStamp;

int work = 1;

unsigned char playAT3;
unsigned char playAV;
unsigned char playAudio = 1;
unsigned char is_mps = 0;

int at3_thread_started = 0;

void* MPEGdata = NULL;
SceUID pspavfd = -1;
SceOff MPEGsize = 0;
SceOff MPEGcounter = 0;
int MPEGstart = 0;

PSPAVEntry* entry = NULL;
PSPAVCallbacks* av_callbacks = NULL;
u8* ringbuf = NULL;

unsigned char run = 0;

int dx;
int dy;

unsigned char mps_header_injected = 0;
#include "mps_header.h"

static void copyHeader(void* pData){
    memset(pData, 0, 2048);
    memcpy(pData, mps_header, sizeof(mps_header));
    memcpy((u8*)pData+12, &MPEGsize, sizeof(MPEGsize));
    memcpy((u8*)pData+92, &m_iLastTimeStamp, sizeof(m_iLastTimeStamp));
    memcpy((u8*)pData+118, &m_iLastTimeStamp, sizeof(m_iLastTimeStamp));
}

SceInt32 RingbufferCallbackFromBuffer(ScePVoid pData, SceInt32 iNumPackets, ScePVoid pParam)
{

    if (MPEGcounter >= MPEGsize)
        MPEGcounter = MPEGstart;

    unsigned char* pspavData = (unsigned char*)pParam+MPEGcounter;

    int toRead = iNumPackets*2048;
    if (MPEGcounter + toRead > MPEGsize)
        toRead = MPEGsize-MPEGcounter;

    memcpy(pData, pspavData, toRead);

    MPEGcounter += toRead;

    return toRead/2048;
}

SceInt32 RingbufferCallbackFromFile(ScePVoid pData, SceInt32 iNumPackets, ScePVoid pParam)
{

    int res = 0;

    if (is_mps && MPEGcounter == 0 && !mps_header_injected){
        //printf("faking header for mps\n");
        copyHeader(pData);
        mps_header_injected = 1;
        if (iNumPackets == 1) return 1;
        res = 1;
        iNumPackets--;
        pData = (u8*)pData+2048;
    }

    if (MPEGcounter >= MPEGsize){
        //printf("reset MPEGcounter (%d, %d)\n", MPEGcounter, MPEGsize);
        MPEGcounter = MPEGstart;
        sceIoLseek(pspavfd, MPEGstart, PSP_SEEK_SET);
    }

    int toRead = iNumPackets*2048;

    //printf("reading %d bytes at %d\n", toRead, MPEGcounter);

    toRead = sceIoRead(pspavfd, pData, toRead);

    MPEGcounter += toRead;

    //printf("MPEGcounter: %d\n");

    return res+iNumPackets;
}

SceInt32 ParseHeader()
{
    int retVal;

    char * pHeader = (char *)malloc(2048);

    if (MPEGsize < 2048)
    {
        goto error;
    }

    if (is_mps){
        m_MpegStreamOffset = 0;
        m_iLastTimeStamp = (MPEGsize-2048);
        m_MpegStreamSize = MPEGsize;
        MPEGcounter = 0;
        MPEGstart = 0;
        copyHeader(pHeader);
    }
    else if (MPEGdata){
        memcpy(pHeader, MPEGdata, 2048);
    }
    else if (pspavfd >= 0){
        //printf("reading header from file\n");
        sceIoLseek32(pspavfd, 0, SEEK_SET);
        sceIoRead(pspavfd, pHeader, 2048);
    }
    else {
        retVal = -1;
        goto error;
    }

    m_iLastTimeStamp = *(int*)(pHeader + 80 + 12);
    m_iLastTimeStamp = SWAPINT(m_iLastTimeStamp);

    retVal = sceMpegQueryStreamOffset(&m_Mpeg, pHeader, &m_MpegStreamOffset);
    if (retVal != 0)
    {PSPAV_PadState
        m_MpegStreamOffset = 0;
        //printf("sceMpegQueryStreamOffset: %p\n", retVal);
        m_iLastTimeStamp = -1;
        //goto error;
    }

    retVal = sceMpegQueryStreamSize(pHeader, &m_MpegStreamSize);
    if (retVal != 0)
    {
        m_MpegStreamSize = MPEGsize;
        //printf("sceMpegQueryStreamSize: %p\n", retVal);
        //goto error;
    }

    free(pHeader);

    MPEGcounter = MPEGstart = m_MpegStreamOffset;
    if (pspavfd >= 0) sceIoLseek(pspavfd, MPEGcounter, PSP_SEEK_SET);
    return 0;

error:
    free(pHeader);
    return -1;
}


void pspavInit(sceMpegRingbufferCB RingbufferCallback) {

    m_RingbufferPackets = 0x3C0;

    int status = 0;
    status |= sceUtilityLoadModule(PSP_MODULE_AV_ATRAC3PLUS);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_MPEGBASE);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_VAUDIO);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_AAC);
    
    int res;

    res = sceMpegInit();
    //printf("sceMpegInit: %p\n", res);
    m_RingbufferSize = sceMpegRingbufferQueryMemSize(m_RingbufferPackets);
    
    m_MpegMemSize    = sceMpegQueryMemSize(0);
    m_RingbufferData = malloc(m_RingbufferSize);
    m_MpegMemData    = malloc(m_MpegMemSize);
    res = sceMpegRingbufferConstruct(&m_Ringbuffer, m_RingbufferPackets, m_RingbufferData, m_RingbufferSize, RingbufferCallback, MPEGdata);
    //printf("sceMpegRingbufferConstruct: %p\n", res);
    res = sceMpegCreate(&m_Mpeg, m_MpegMemData, m_MpegMemSize, &m_Ringbuffer, BUFFER_WIDTH, 0, 0);
    //printf("sceMpegCreate: %p\n", res);

    m_MpegAvcMode.iUnk0 = -1;
    m_MpegAvcMode.iPixelFormat = 3;
    res = sceMpegAvcDecodeMode(&m_Mpeg, &m_MpegAvcMode);
    //printf("sceMpegAvcDecodeMode: %p\n", res);
}

void pspavLoad() {
    MPEGcounter = 0;
    MPEGstart = 0;
    ParseHeader();
    m_MpegStreamAVC = sceMpegRegistStream(&m_Mpeg, 0, 0);
    m_MpegStreamAtrac = sceMpegRegistStream(&m_Mpeg, 1, 0);
    m_pEsBufferAVC = sceMpegMallocAvcEsBuf(&m_Mpeg);
    retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAVC, &m_MpegAuAVC);
    //printf("sceMpegInitAu: %p\n", retVal);
    retVal = sceMpegQueryAtracEsSize(&m_Mpeg, &m_MpegAtracEsSize, &m_MpegAtracOutSize);
    //printf("sceMpegQueryAtracEsSize: %p\n", retVal);
    m_pEsBufferAtrac = memalign(64, m_MpegAtracEsSize);
    retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAtrac, &m_MpegAuAtrac);
    //printf("sceMpegInitAu: %p\n", retVal);
}

int pspavPlay(){

    int retVal, fail = 0;
    ReaderThreadData * TDR = &Reader;
    DecoderThreadData* TDD = &Decoder;

    retVal = InitReader();
    if (retVal < 0)
    {
        fail++;
        goto exit_reader;
    }
    
    retVal = InitVideo();
    if (retVal < 0)
    {
        fail++;
        goto exit_video;
    }

    retVal = InitAudio();
    if (retVal < 0)
    {
        fail++;
        goto exit_audio;
    }

    retVal = InitDecoder();
    if (retVal < 0)
    {
        fail++;
        goto exit_decoder;
    }

    sceKernelStartThread(Reader.m_ThreadID,  sizeof(void*), &TDR);
    if (!playAT3 || (playAT3 && !at3_thread_started)){
        sceKernelStartThread(Audio.m_ThreadID,   sizeof(void*), &TDD);
        at3_thread_started = 1;
    }
    sceKernelStartThread(Video.m_ThreadID,   sizeof(void*), &TDD);
    sceKernelStartThread(Decoder.m_ThreadID, sizeof(void*), &TDD);

    sceKernelWaitThreadEnd(Decoder.m_ThreadID, 0);
    sceKernelWaitThreadEnd(Video.m_ThreadID, 0);
    if (!playAT3)
        sceKernelWaitThreadEnd(Audio.m_ThreadID, 0);
    sceKernelWaitThreadEnd(Reader.m_ThreadID, 0);

    ShutdownDecoder();
exit_decoder:
    ShutdownAudio();
exit_audio:
    ShutdownVideo();
exit_video:
    ShutdownReader();
exit_reader:

    if (fail > 0) return -1;

    return 0;
}

SceVoid pspavShutdown()
{

    if (m_pEsBufferAVC    != NULL) sceMpegFreeAvcEsBuf(&m_Mpeg, m_pEsBufferAVC);
    if (m_MpegStreamAVC   != NULL) sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAVC);
    if (m_MpegStreamAtrac != NULL) sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAtrac);

    sceMpegDelete(&m_Mpeg);
    sceMpegRingbufferDestruct(&m_Ringbuffer);
    sceMpegFinish();
    
    sceUtilityUnloadModule(PSP_MODULE_AV_AAC);
    sceUtilityUnloadModule(PSP_MODULE_AV_VAUDIO);
    sceUtilityUnloadModule(PSP_MODULE_AV_MPEGBASE);
    sceUtilityUnloadModule(PSP_MODULE_AV_ATRAC3PLUS);
    
    if (m_pEsBufferAtrac  != NULL) free(m_pEsBufferAtrac);
    if (m_RingbufferData  != NULL) free(m_RingbufferData); // This crashes....double free or corruption?
    if (m_MpegMemData     != NULL) free(m_MpegMemData);
    
}

void T_pspav(){
    work = 1;
    while (work){
        MPEGcounter = MPEGstart; // reset MPEG to play on loop
        pspavPlay(); // play MPEG
    }
}

unsigned char pspavPlayGamePSPAV(PSPAVEntry* e, PSPAVCallbacks* callbacks, int x, int y){

    if (!e || !callbacks) return 0;

    void* pspavData = e->icon1;
    int pspavSize = e->size_icon1;
    void* at3data = e->at3data;
    int at3size = e->size_at3data;
    
    playAT3 = at3data != NULL; // are we gonna play an at3 file?
    playAV = pspavData != NULL; // are we gonna play a pspav file too?
    playAudio = playAV;
    if (!playAT3 && !playAV)
        return 0; // we need to play something
    entry = e;
    av_callbacks = callbacks; 
    pspavfd = -1;
    MPEGdata = pspavData;
    MPEGsize = pspavSize;
    AT3->at3_data = (char*)at3data;
    AT3->at3_size = at3size;
    at3_started = 0;
    at3_thread_started = 0;
    dx = x;
    dy = y;

    // init and start MPEG
    pspavInit(RingbufferCallbackFromBuffer);
    pspavLoad();
    T_pspav(); // do play
    pspavShutdown(); // shutdown MPEG
    
    return run;
}

void pspavPlayVideoFile(const char* path, PSPAVCallbacks* callbacks){

    if (!callbacks) return;

    pspavfd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (pspavfd < 0) return;

    char* ext = strrchr(path, '.');
    if (ext && strcasecmp(ext, ".mps")){
        //printf("mps detected\n");
        is_mps = 1;
    }
    else {
        is_mps = 0;
    }

    //printf("play video file %s\n", path);
    playAT3 = 0; // are we gonna play an at3 file? nope
    playAV = 1; // are we gonna play a pspav file too? of course we are
    playAudio = 1;
    entry = NULL;
    av_callbacks = callbacks;
    MPEGdata = NULL;
    MPEGsize = sceIoLseek(pspavfd, 0, SEEK_END);
    AT3->at3_data = NULL;
    AT3->at3_size = 0;
    at3_started = 0;
    at3_thread_started = 0;
    dx = 0;
    dy = 0;
    MPEGcounter = MPEGstart = 0;
    mps_header_injected = 0;
    sceIoLseek(pspavfd, 0, PSP_SEEK_SET);

    // init and start MPEG
    //printf("pspav init\n");
    pspavInit(RingbufferCallbackFromFile);
    //printf("pspav loade\n");
    pspavLoad();
    //printf("pspav loop\n");
    T_pspav(); // do play
    //printf("pspav shutdown\n");
    pspavShutdown(); // shutdown MPEG
}