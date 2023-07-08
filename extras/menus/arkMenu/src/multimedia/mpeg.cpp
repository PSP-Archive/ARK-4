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

#include <sstream>

#include "mpeg.h"
#include "mpeg_common.h"
#include "mpeg_video.h"
#include "mpeg_decoder.h"
#include "mpeg_reader.h"
#include "mpeg_audio.h"

#include "common.h"

#define SWAPINT(x) (((x)<<24) | (((uint)(x)) >> 24) | (((x) & 0x0000FF00) << 8) | (((x) & 0x00FF0000) >> 8))

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

bool playAT3;
bool playMPEG;
bool playMPEGAudio = true;

int at3_thread_started = 0;

void* MPEGdata = NULL;
int MPEGsize = 0;
int MPEGcounter = 0;
int MPEGstart = 0;

Entry* entry = NULL;

bool run = 0;

int dx;
int dy;

SceInt32 RingbufferCallback(ScePVoid pData, SceInt32 iNumPackets, ScePVoid pParam)
{

    if (MPEGcounter >= MPEGsize)
        MPEGcounter = MPEGstart;

    unsigned char* mpegData = (unsigned char*)pParam+MPEGcounter;

    int toRead = iNumPackets*2048;
    if (MPEGcounter + toRead > MPEGsize)
        toRead = MPEGsize-MPEGcounter;

    memcpy(pData, mpegData, toRead);

    MPEGcounter += toRead;

    return toRead/2048;
}

SceInt32 ParseHeader()
{
    int retVal;
    char * pHeader = (char *)malloc(2048);

    if (MPEGsize < 2048)
    {
        goto error;
    }
    
    memcpy(pHeader, MPEGdata, 2048);

    retVal = sceMpegQueryStreamOffset(&m_Mpeg, pHeader, &m_MpegStreamOffset);
    if (retVal != 0)
    {
        goto error;
    }

    retVal = sceMpegQueryStreamSize(pHeader, &m_MpegStreamSize);
    if (retVal != 0)
    {
        goto error;
    }

    m_iLastTimeStamp = *(int*)(pHeader + 80 + 12);
    m_iLastTimeStamp = SWAPINT(m_iLastTimeStamp);

    free(pHeader);

    MPEGcounter = MPEGstart = m_MpegStreamOffset;
    return 0;

error:
    free(pHeader);
    return -1;
}

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
    SceMpeg* mpeg;
} _SceMpegRingbuffer;

typedef struct {
    uint   magic1;
    uint   magic2;
    uint   magic3;
    uint   unk_m1;
    void*  ringbuffer_start;
    void*  ringbuffer_end;
} _SceMpeg;


void mpegInit() {

    m_RingbufferPackets = 100; //0x3C0;
    // 0x3C0 -> 2065920 bytes
    // 100 -> 215200 bytes
    
    static u8 ringbuf[215200]; // use static buffer

    int status = 0;
    status |= sceUtilityLoadModule(PSP_MODULE_AV_ATRAC3PLUS);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_MPEGBASE);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_VAUDIO);
    
    sceMpegInit();
    m_RingbufferSize = sceMpegRingbufferQueryMemSize(m_RingbufferPackets);
    
    m_MpegMemSize    = sceMpegQueryMemSize(0);
    m_RingbufferData = ringbuf; //malloc(m_RingbufferSize);
    m_MpegMemData    = malloc(m_MpegMemSize);
    sceMpegRingbufferConstruct(&m_Ringbuffer, m_RingbufferPackets, m_RingbufferData, m_RingbufferSize, &RingbufferCallback, MPEGdata);
    sceMpegCreate(&m_Mpeg, m_MpegMemData, m_MpegMemSize, &m_Ringbuffer, BUFFER_WIDTH, 0, 0);
    
    m_MpegAvcMode.iUnk0 = -1;
    m_MpegAvcMode.iPixelFormat = 3;
    sceMpegAvcDecodeMode(&m_Mpeg, &m_MpegAvcMode);
}

void mpegLoad() {
    MPEGcounter = 0;
    MPEGstart = 0;
    ParseHeader();
    m_MpegStreamAVC = sceMpegRegistStream(&m_Mpeg, 0, 0);
    m_MpegStreamAtrac = sceMpegRegistStream(&m_Mpeg, 1, 0);
    m_pEsBufferAVC = sceMpegMallocAvcEsBuf(&m_Mpeg);
    retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAVC, &m_MpegAuAVC);
    retVal = sceMpegQueryAtracEsSize(&m_Mpeg, &m_MpegAtracEsSize, &m_MpegAtracOutSize);
    m_pEsBufferAtrac = memalign(64, m_MpegAtracEsSize);
    retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAtrac, &m_MpegAuAtrac);
}

int mpegPlay(){

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

SceVoid mpegShutdown()
{

    if (m_pEsBufferAVC    != NULL) sceMpegFreeAvcEsBuf(&m_Mpeg, m_pEsBufferAVC);
    if (m_MpegStreamAVC   != NULL) sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAVC);
    if (m_MpegStreamAtrac != NULL) sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAtrac);

    sceMpegDelete(&m_Mpeg);
    sceMpegRingbufferDestruct(&m_Ringbuffer);
    sceMpegFinish();
    
    sceUtilityUnloadModule(PSP_MODULE_AV_VAUDIO);
    sceUtilityUnloadModule(PSP_MODULE_AV_MPEGBASE);
    sceUtilityUnloadModule(PSP_MODULE_AV_ATRAC3PLUS);
    
    if (m_pEsBufferAtrac  != NULL) free(m_pEsBufferAtrac);
    //if (m_RingbufferData  != NULL) free(m_RingbufferData); // This crashes....double free or corruption?
    if (m_MpegMemData     != NULL) free(m_MpegMemData);
    
}

void T_mpeg(){
    work = 1;
    // init and start MPEG
    mpegInit();
    mpegLoad();
    while (work){
        MPEGcounter = MPEGstart; // reset MPEG to play on loop
        mpegPlay(); // play MPEG
    }
    mpegShutdown(); // shutdown MPEG
}

bool mpegStart(Entry* e, int x, int y){
    void* mpegData = e->getIcon1();
    int mpegSize = e->getIcon1Size();
    void* at3data = e->getSnd();
    int at3size = e->getSndSize();
    
    playAT3 = at3data != NULL; // are we gonna play an at3 file?
    playMPEG = mpegData != NULL; // are we gonna play a mpeg file too?
    playMPEGAudio = playMPEG;
    if (!playAT3 && !playMPEG)
        return false; // we need to play something
    entry = e;
    MPEGdata = mpegData;
    MPEGsize = mpegSize;
    AT3->at3_data = (char*)at3data;
    AT3->at3_size = at3size;
    at3_started = 0;
    at3_thread_started = 0;
    dx = x;
    dy = y;
    T_mpeg();
    
    return run;
}
