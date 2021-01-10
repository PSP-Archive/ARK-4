#include <pspkernel.h>
#include <pspsdk.h>
#include <psptypes.h>
#include <psppower.h>

#define DISPLAY_VIDEO 1

//#include "pmfplayer.h"
#include <psputilsforkernel.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <pspgu.h>
//#include <pspctrl.h>
#include <pspaudio.h>
#include <psputility.h>
#include <pspatrac3.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <pspmpeg.h>

#include "pmf.h"
#include "pmf_common.h"
#include "pmf_video.h"
#include "pmf_decoder.h"
#include "pmf_reader.h"
#include "pmf_audio.h"

#include "common.h"

#define SWAPINT(x) (((x)<<24) | (((uint)(x)) >> 24) | (((x) & 0x0000FF00) << 8) | (((x) & 0x00FF0000) >> 8))

int retVal;
SceMpegAvcMode m_MpegAvcMode;

ReaderThreadData                    Reader;
VideoThreadData                     Video;
AudioThreadData                     Audio;
DecoderThreadData                   Decoder;
AT3ThreadData						Atrac3;
AT3ThreadData*						AT3 = &Atrac3;

SceInt32                            m_MpegStreamOffset;
SceInt32                            m_MpegStreamSize;

SceMpeg                             m_Mpeg;
SceInt32                            m_MpegMemSize;
ScePVoid                            m_MpegMemData;

SceInt32                            m_RingbufferPackets;
SceInt32                            m_RingbufferSize;
ScePVoid                            m_RingbufferData;
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

bool work = true;

bool playAT3;
bool playPMF;
bool playPMFAudio = true;

int at3_thread_started = 0;

void* PMFdata = NULL;
int PMFsize = 0;
int PMFcounter = 0;

Entry* entry = NULL;

bool run = 0;

int dx;
int dy;

SceInt32 RingbufferCallback(ScePVoid pData, SceInt32 iNumPackets, ScePVoid pParam)
{

	if (PMFcounter == PMFsize)
		return -1;

	unsigned char* pmfData = (unsigned char*)pParam+PMFcounter;

	int toRead = iNumPackets*2048;
	if (PMFcounter + toRead > PMFsize)
		toRead = PMFsize-PMFcounter;

	memcpy(pData, pmfData, toRead);

	PMFcounter += toRead;

	return toRead/2048;
}

SceInt32 ParseHeader()
{
	int retVal;
	char * pHeader = (char *)malloc(2048);

	//sceIoLseek(m_FileHandle, 0, SEEK_SET);

	//retVal = sceIoRead(m_FileHandle, pHeader, 2048);
	if (PMFsize < 2048)
	{
		goto error;
	}
	
	memcpy(pHeader, PMFdata, 2048);

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

	//sceIoLseek(m_FileHandle, m_MpegStreamOffset, SEEK_SET);
	PMFcounter = m_MpegStreamOffset;

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


void pmfInit() {

	m_RingbufferPackets = 0x3C0;

	int status = 0;
	status |= sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	status |= sceUtilityLoadModule(PSP_MODULE_AV_ATRAC3PLUS);
	status |= sceUtilityLoadModule(PSP_MODULE_AV_MP3);
	status |= sceUtilityLoadModule(PSP_MODULE_AV_MPEGBASE);
	status |= sceUtilityLoadModule(PSP_MODULE_AV_VAUDIO);
	
	sceMpegInit();
	m_RingbufferSize = sceMpegRingbufferQueryMemSize(m_RingbufferPackets);
	
	m_MpegMemSize    = sceMpegQueryMemSize(0);
	m_RingbufferData = malloc(m_RingbufferSize);
	m_MpegMemData    = malloc(m_MpegMemSize);
	sceMpegRingbufferConstruct(&m_Ringbuffer, m_RingbufferPackets, m_RingbufferData, m_RingbufferSize, &RingbufferCallback, PMFdata);
	sceMpegCreate(&m_Mpeg, m_MpegMemData, m_MpegMemSize, &m_Ringbuffer, BUFFER_WIDTH, 0, 0);
	
	m_MpegAvcMode.iUnk0 = -1;
	m_MpegAvcMode.iPixelFormat = 3;
	sceMpegAvcDecodeMode(&m_Mpeg, &m_MpegAvcMode);
}

void pmfLoad() {
	PMFcounter = 0;
	ParseHeader();
	m_MpegStreamAVC = sceMpegRegistStream(&m_Mpeg, 0, 0);
	m_MpegStreamAtrac = sceMpegRegistStream(&m_Mpeg, 1, 0);
	m_pEsBufferAVC = sceMpegMallocAvcEsBuf(&m_Mpeg);
	retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAVC, &m_MpegAuAVC);
	retVal = sceMpegQueryAtracEsSize(&m_Mpeg, &m_MpegAtracEsSize, &m_MpegAtracOutSize);
	m_pEsBufferAtrac = memalign(64, m_MpegAtracEsSize);
	retVal = sceMpegInitAu(&m_Mpeg, m_pEsBufferAtrac, &m_MpegAuAtrac);
}

int pmfPlay(){

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

SceVoid pmfShutdown()
{

	if (m_pEsBufferAtrac  != NULL) free(m_pEsBufferAtrac);
	if (m_pEsBufferAVC    != NULL) sceMpegFreeAvcEsBuf(&m_Mpeg, m_pEsBufferAVC);
	if (m_MpegStreamAVC   != NULL) sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAVC);
	if (m_MpegStreamAtrac != NULL) sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAtrac);

	sceMpegDelete(&m_Mpeg);
	sceMpegRingbufferDestruct(&m_Ringbuffer);
	sceMpegFinish();

	if (m_RingbufferData != NULL) free(m_RingbufferData);
	if (m_MpegMemData    != NULL) free(m_MpegMemData);
}

void T_pmf(){

	work = true;

	while (work){
		pmfInit();
		pmfLoad();
		pmfPlay();
		pmfShutdown();
	}
}

bool pmfStart(Entry* e, void* pmfData, int pmfSize, void* at3data, int at3size, int x, int y){
	playAT3 = at3data != NULL; // are we gonna play an at3 file?
	playPMF = pmfData != NULL; // are we gonna play a pmf file too?
	playPMFAudio = playPMF;
	if (!playAT3 && !playPMF)
		return false; // we need to play something
	entry = e;
	PMFdata = pmfData;
	PMFsize = pmfSize;
	AT3->at3_data = (char*)at3data;
	AT3->at3_size = at3size;
	at3_started = 0;
	at3_thread_started = 0;
	dx = x;
	dy = y;
	T_pmf();

	sceUtilityUnloadModule(PSP_MODULE_AV_VAUDIO);
	sceUtilityUnloadModule(PSP_MODULE_AV_MPEGBASE);
	sceUtilityUnloadModule(PSP_MODULE_AV_MP3);
	sceUtilityUnloadModule(PSP_MODULE_AV_ATRAC3PLUS);
	sceUtilityUnloadModule(PSP_MODULE_AV_AVCODEC);

	return run;
}
