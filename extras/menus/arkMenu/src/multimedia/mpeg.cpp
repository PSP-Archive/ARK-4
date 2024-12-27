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
bool is_mps = false;

int at3_thread_started = 0;

void* MPEGdata = NULL;
SceUID mpegfd = -1;
SceOff MPEGsize = 0;
SceOff MPEGcounter = 0;
int MPEGstart = 0;

Entry* entry = NULL;
u8* ringbuf = NULL;

bool run = 0;

int dx;
int dy;

bool mps_header_injected = false;
u8 mps_header[] = {
    0x50, 0x53, 0x4D, 0x46, 0x30, 0x30, 0x30, 0x34, 0x00, 0x00, 0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x01, 0x5F, 0x90, 0x00, 0x00, 0x00, 0x69, 0x6F, 0x75,
    0x00, 0x00, 0x61, 0xA8, 0x00, 0x01, 0x5F, 0x90, 0x02, 0x01, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00,
    0x00, 0x01, 0x5F, 0x90, 0x00, 0x00, 0x00, 0x69, 0x6F, 0x75, 0x00, 0x01, 0x00, 0x00, 0x00, 0x22,
    0x00, 0x02, 0xE0, 0x00, 0x21, 0xEF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x11,
    0x00, 0x00, 0xBD, 0x00, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

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

    unsigned char* mpegData = (unsigned char*)pParam+MPEGcounter;

    int toRead = iNumPackets*2048;
    if (MPEGcounter + toRead > MPEGsize)
        toRead = MPEGsize-MPEGcounter;

    memcpy(pData, mpegData, toRead);

    MPEGcounter += toRead;

    return toRead/2048;
}

SceInt32 RingbufferCallbackFromFile(ScePVoid pData, SceInt32 iNumPackets, ScePVoid pParam)
{

    int res = 0;

    if (is_mps && MPEGcounter == 0 && !mps_header_injected){
        printf("faking header for mps\n");
        copyHeader(pData);
        mps_header_injected = true;
        if (iNumPackets == 1) return 1;
        res = 1;
        iNumPackets--;
        pData = (u8*)pData+2048;
    }

    if (MPEGcounter >= MPEGsize){
        printf("reset MPEGcounter (%d, %d)\n", MPEGcounter, MPEGsize);
        MPEGcounter = 0;
        sceIoLseek(mpegfd, 0, PSP_SEEK_SET);
    }

    int toRead = iNumPackets*2048;
    if (MPEGcounter + toRead > MPEGsize)
        toRead = MPEGsize-MPEGcounter;

    printf("reading %d bytes at %d\n", toRead, MPEGcounter);

    sceIoRead(mpegfd, pData, toRead);

    MPEGcounter += toRead;

    printf("MPEGcounter: %d\n");

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
        m_iLastTimeStamp = -1;
        m_MpegStreamSize = MPEGsize;
        MPEGcounter = 0;
        MPEGstart = 0;
        copyHeader(pHeader);
    }
    else if (MPEGdata){
        memcpy(pHeader, MPEGdata, 2048);
    }
    else if (mpegfd >= 0){
        printf("reading header from file\n");
        sceIoLseek32(mpegfd, 0, SEEK_SET);
        sceIoRead(mpegfd, pHeader, 2048);
    }
    else {
        retVal = -1;
        goto error;
    }

    m_iLastTimeStamp = *(int*)(pHeader + 80 + 12);
    m_iLastTimeStamp = SWAPINT(m_iLastTimeStamp);

    retVal = sceMpegQueryStreamOffset(&m_Mpeg, pHeader, &m_MpegStreamOffset);
    if (retVal != 0)
    {
        m_MpegStreamOffset = 0;
        printf("sceMpegQueryStreamOffset: %p\n", retVal);
        m_iLastTimeStamp = -1;
        //goto error;
    }

    retVal = sceMpegQueryStreamSize(pHeader, &m_MpegStreamSize);
    if (retVal != 0)
    {
        m_MpegStreamSize = MPEGsize;
        printf("sceMpegQueryStreamSize: %p\n", retVal);
        //goto error;
    }

    free(pHeader);

    MPEGcounter = MPEGstart = m_MpegStreamOffset;
    if (mpegfd >= 0) sceIoLseek(mpegfd, MPEGcounter, PSP_SEEK_SET);
    return 0;

error:
    free(pHeader);
    return -1;
}


void mpegInit(sceMpegRingbufferCB RingbufferCallback) {

    m_RingbufferPackets = 0x3C0;

    int status = 0;
    status |= sceUtilityLoadModule(PSP_MODULE_AV_ATRAC3PLUS);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_MPEGBASE);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_VAUDIO);
    status |= sceUtilityLoadModule(PSP_MODULE_AV_AAC);
    
    int res;

    res = sceMpegInit();
    printf("sceMpegInit: %p\n", res);
    m_RingbufferSize = sceMpegRingbufferQueryMemSize(m_RingbufferPackets);
    
    m_MpegMemSize    = sceMpegQueryMemSize(0);
    m_RingbufferData = malloc(m_RingbufferSize);
    m_MpegMemData    = malloc(m_MpegMemSize);
    res = sceMpegRingbufferConstruct(&m_Ringbuffer, m_RingbufferPackets, m_RingbufferData, m_RingbufferSize, RingbufferCallback, MPEGdata);
    printf("sceMpegRingbufferConstruct: %p\n", res);
    res = sceMpegCreate(&m_Mpeg, m_MpegMemData, m_MpegMemSize, &m_Ringbuffer, BUFFER_WIDTH, 0, 0);
    printf("sceMpegCreate: %p\n", res);

    m_MpegAvcMode.iUnk0 = -1;
    m_MpegAvcMode.iPixelFormat = 3;
    res = sceMpegAvcDecodeMode(&m_Mpeg, &m_MpegAvcMode);
    printf("sceMpegAvcDecodeMode: %p\n", res);
}

void mpegLoad() {
    MPEGcounter = 0;
    MPEGstart = 0;
    ParseHeader();
    m_MpegStreamAVC = sceMpegRegistStream(&m_Mpeg, 0, 0);
    m_MpegStreamAtrac = sceMpegRegistStream(&m_Mpeg, 1, 0);
    m_pEsBufferAVC = sceMpegMallocAvcEsBuf(&m_Mpeg);
    retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAVC, &m_MpegAuAVC);
    printf("sceMpegInitAu: %p\n", retVal);
    retVal = sceMpegQueryAtracEsSize(&m_Mpeg, &m_MpegAtracEsSize, &m_MpegAtracOutSize);
    printf("sceMpegQueryAtracEsSize: %p\n", retVal);
    m_pEsBufferAtrac = memalign(64, m_MpegAtracEsSize);
    retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAtrac, &m_MpegAuAtrac);
    printf("sceMpegInitAu: %p\n", retVal);
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
    
    sceUtilityUnloadModule(PSP_MODULE_AV_AAC);
    sceUtilityUnloadModule(PSP_MODULE_AV_VAUDIO);
    sceUtilityUnloadModule(PSP_MODULE_AV_MPEGBASE);
    sceUtilityUnloadModule(PSP_MODULE_AV_ATRAC3PLUS);
    
    if (m_pEsBufferAtrac  != NULL) free(m_pEsBufferAtrac);
    if (m_RingbufferData  != NULL) free(m_RingbufferData); // This crashes....double free or corruption?
    if (m_MpegMemData     != NULL) free(m_MpegMemData);
    
}

void T_mpeg(){
    work = 1;
    while (work){
        MPEGcounter = MPEGstart; // reset MPEG to play on loop
        mpegPlay(); // play MPEG
    }
}

bool mpegPlayGamePMF(Entry* e, int x, int y){
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
    mpegfd = -1;
    MPEGdata = mpegData;
    MPEGsize = mpegSize;
    AT3->at3_data = (char*)at3data;
    AT3->at3_size = at3size;
    at3_started = 0;
    at3_thread_started = 0;
    dx = x;
    dy = y;

    // init and start MPEG
    mpegInit(RingbufferCallbackFromBuffer);
    mpegLoad();
    T_mpeg(); // do play
    mpegShutdown(); // shutdown MPEG
    
    return run;
}

void mpegPlayVideoFile(const char* path){

    mpegfd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (mpegfd < 0) return;

    string ext = common::getExtension(path);
    if (ext == "mps"){
        printf("mps detected\n");
        is_mps = true;
    }
    else {
        is_mps = false;
    }

    printf("play video file %s\n", path);
    playAT3 = false; // are we gonna play an at3 file? nope
    playMPEG = true; // are we gonna play a mpeg file too? of course we are
    playMPEGAudio = true;
    entry = NULL;
    MPEGdata = NULL;
    MPEGsize = sceIoLseek(mpegfd, 0, SEEK_END);
    AT3->at3_data = NULL;
    AT3->at3_size = 0;
    at3_started = 0;
    at3_thread_started = 0;
    dx = 0;
    dy = 0;
    MPEGcounter = MPEGstart = 0;
    mps_header_injected = false;
    sceIoLseek(mpegfd, 0, PSP_SEEK_SET);

    // init and start MPEG
    printf("mpeg init\n");
    mpegInit(RingbufferCallbackFromFile);
    printf("mpeg loade\n");
    mpegLoad();
    printf("mpeg loop\n");
    T_mpeg(); // do play
    printf("mpeg shutdown\n");
    mpegShutdown(); // shutdown MPEG
}