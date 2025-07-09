#ifndef PSPAV_COMMON_H
#define PSPAV_COMMON_H

#include <pspkernel.h>
#include <pspsdk.h>
#include <psptypes.h>
#include <psppower.h>
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
#include "pspav_entry.h"

enum
{
    ReaderThreadData__READER_OK = 0,
    ReaderThreadData__READER_EOF,
    ReaderThreadData__READER_ABORT
};

#define PIXEL_SIZE          4

#define SCREEN_W            480
#define SCREEN_H            272
#define DRAW_BUFFER_SIZE    512 * SCREEN_H * PIXEL_SIZE
#define DISP_BUFFER_SIZE    512 * SCREEN_H * PIXEL_SIZE

#define TEXTURE_W           256
#define TEXTURE_H           128
#define IMAGE_W                144
#define IMAGE_H                80
#define TEXTURE_SIZE        TEXTURE_W * TEXTURE_H * PIXEL_SIZE

#define BUFFER_WIDTH            256

#define N_VIDEO_BUFFERS 1

typedef struct VideoThreadData
{
    SceUID                          m_SemaphoreStart;
    SceUID                          m_SemaphoreWait;
    SceUID                          m_SemaphoreLock;
    SceUID                          m_ThreadID;

    ScePVoid                        m_pVideoBuffer[N_VIDEO_BUFFERS];
    SceInt32                        m_iBufferTimeStamp[N_VIDEO_BUFFERS];

    SceInt32                        m_iNumBuffers;
    SceInt32                        m_iFullBuffers;
    SceInt32                        m_iPlayBuffer;

    SceInt32                        m_iAbort;

    SceInt32                        m_iWidth;
    SceInt32                        m_iHeight;
    SceInt32                        m_iBufferWidth;

} VideoThreadData;

typedef struct {
    SceUID                          m_Semaphore;
    SceUID                          m_ThreadID;

    SceInt32                        m_StreamSize;
    SceMpegRingbuffer*              m_Ringbuffer;
    SceInt32                        m_RingbufferPackets;
    SceInt32                        m_Status;
    SceInt32                        m_TotalBytes;
} ReaderThreadData;

typedef struct
{
    SceUID                          m_SemaphoreStart;
    SceUID                          m_SemaphoreLock;
    SceUID                          m_ThreadID;

    SceInt32                        m_AudioChannel;

    ScePVoid                        m_pAudioBuffer[4];
    SceInt32                        m_iBufferTimeStamp[4];

    SceInt32                        m_iNumBuffers;
    SceInt32                        m_iFullBuffers;
    SceInt32                        m_iPlayBuffer;
    SceInt32                        m_iDecodeBuffer;

    SceInt32                        m_iAbort;
} AudioThreadData;

typedef struct {
    char* at3_data;
    int at3_size;
    int end;
} AT3ThreadData;

typedef struct {
    SceUID                          m_ThreadID;

    ReaderThreadData*               Reader;
    VideoThreadData*                Video;
    AudioThreadData*                Audio;

    SceMpeg                         m_Mpeg;

    SceMpegStream*                  m_MpegStreamAVC;
    SceMpegAu*                      m_MpegAuAVC;
    SceMpegStream*                  m_MpegStreamAtrac;
    SceMpegAu*                      m_MpegAuAtrac;
    SceInt32                        m_MpegAtracOutSize;

    SceInt32                        m_iAudioFrameDuration;
    SceInt32                        m_iVideoFrameDuration;
    SceInt32                        m_iLastTimeStamp;
} DecoderThreadData;

extern int retVal;
extern SceMpegAvcMode m_MpegAvcMode;

extern ReaderThreadData                    Reader;
extern VideoThreadData                     Video;
extern AudioThreadData                     Audio;
extern DecoderThreadData                   Decoder;
extern AT3ThreadData                       Atrac3;
extern AT3ThreadData*                      AT3;

extern SceInt32                            m_MpegStreamOffset;
extern SceInt32                            m_MpegStreamSize;

extern SceMpeg                             m_Mpeg;
extern SceInt32                            m_MpegMemSize;
extern ScePVoid                            m_MpegMemData;

extern SceInt32                            m_RingbufferPackets;
extern SceInt32                            m_RingbufferSize;
extern ScePVoid                            m_RingbufferData;
extern SceMpegRingbuffer                   m_Ringbuffer;

extern SceMpegStream*                      m_MpegStreamAVC;
extern ScePVoid                            m_pEsBufferAVC;
extern SceMpegAu                           m_MpegAuAVC;

extern SceMpegStream*                      m_MpegStreamAtrac;
extern ScePVoid                            m_pEsBufferAtrac;
extern SceMpegAu                           m_MpegAuAtrac;

extern SceInt32                            m_MpegAtracEsSize;
extern SceInt32                            m_MpegAtracOutSize;

extern SceInt32                            m_iLastTimeStamp;

extern int work;

extern unsigned char playAT3;
extern unsigned char playAV;
extern unsigned char playAudio;

extern void* MPEGdataL;
extern SceOff MPEGsize;
extern SceOff MPEGcounter;

extern PSPAVEntry* entry;
extern PSPAVCallbacks* av_callbacks;

extern unsigned char run;

extern int dx;
extern int dy;


#endif
