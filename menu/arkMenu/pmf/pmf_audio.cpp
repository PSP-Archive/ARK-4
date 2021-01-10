#include "pmf_audio.h"
#include <pspaudio.h>

#include "at3.h"

int at3_end = 0;
int at3_started = 0;


SceInt32 AudioSyncStatus(DecoderThreadData* D)
{
	if (D->Audio->m_iFullBuffers == 0 || D->Video->m_iFullBuffers == 0) return 0;

	int iAudioTS = D->Audio->m_iBufferTimeStamp[D->Audio->m_iPlayBuffer];
	int iVideoTS = D->Video->m_iBufferTimeStamp[D->Video->m_iPlayBuffer];

	// if audio ahead of video, wait for video
	if (iAudioTS - iVideoTS > 1 * D->m_iVideoFrameDuration) return 0;

	return 1;
}

int T_Audio(SceSize _args, void *_argp)
{

	DecoderThreadData* D = *((DecoderThreadData**)_argp);

	if (playAT3){

		AT3_T(_args, _argp); // handle the AT3 playback to the library

		at3_end = 1;
		at3_started = 0;
	}
		
	else{

		sceKernelWaitSema(D->Audio->m_SemaphoreStart, 1, 0);

		if (!playPMFAudio){
			while (!D->Audio->m_iAbort)
				sceKernelDelayThread(0);
			sceKernelExitThread(0);
			return 0;
		}

		for (;;)
		{
			if (D->Audio->m_iAbort != 0) break;

			if (D->Audio->m_iFullBuffers > 0 && AudioSyncStatus(D))
			{
				sceAudioOutputBlocking(D->Audio->m_AudioChannel, PSP_AUDIO_VOLUME_MAX, D->Audio->m_pAudioBuffer[D->Audio->m_iPlayBuffer]);
				D->Audio->m_iPlayBuffer = (D->Audio->m_iPlayBuffer + 1) % D->Audio->m_iNumBuffers;

				sceKernelWaitSema(D->Audio->m_SemaphoreLock, 1, 0);
				D->Audio->m_iFullBuffers--;

				sceKernelSignalSema(D->Audio->m_SemaphoreLock, 1);
			}
			else
			{
				sceKernelDelayThread(0);
			}
		}

		while (D->Audio->m_iFullBuffers > 0)
		{
			sceAudioOutputBlocking(D->Audio->m_AudioChannel, PSP_AUDIO_VOLUME_MAX, D->Audio->m_pAudioBuffer[D->Audio->m_iPlayBuffer]);
			D->Audio->m_iPlayBuffer = (D->Audio->m_iPlayBuffer + 1) % D->Audio->m_iNumBuffers;

			sceKernelWaitSema(D->Audio->m_SemaphoreLock, 1, 0);
			D->Audio->m_iFullBuffers--;

			sceKernelSignalSema(D->Audio->m_SemaphoreLock, 1);
		}
	}

	sceKernelExitThread(0);

	return 0;
}

SceInt32 InitAudio()
{

	if (playAT3){
		if (!at3_started){
			Audio.m_ThreadID = sceKernelCreateThread("audio_thread", T_Audio, 0x3D, 0x10000, PSP_THREAD_ATTR_USER, NULL);
			
			Audio.m_iAbort = 0;
			at3_started = 1;
			at3_end = 0;
			
			setAt3Data(AT3->at3_data, AT3->at3_size, &Audio.m_iAbort, 0);
		}
		return 0;
	}

	else {

		int i = 0, fail = 0;

		Audio.m_AudioChannel = sceAudioChReserve(-1, 512, PSP_AUDIO_FORMAT_STEREO);
		if (Audio.m_AudioChannel < 0)
		{
			return -1;
		}

		sceAudioSetChannelDataLen(Audio.m_AudioChannel, m_MpegAtracOutSize / 4);

		Audio.m_ThreadID = sceKernelCreateThread("audio_thread", T_Audio, 0x3D, 0x10000, PSP_THREAD_ATTR_USER, NULL);
		if (Audio.m_ThreadID < 0)
		{
			goto exit0;
		}

		Audio.m_SemaphoreStart = sceKernelCreateSema("audio_start_sema",  0, 0, 1, NULL);
		if (Audio.m_SemaphoreStart < 0)
		{
			goto exit1;
		}

		Audio.m_SemaphoreLock = sceKernelCreateSema("audio_lock_sema",  0, 1, 1, NULL);
		if (Audio.m_SemaphoreLock < 0)
		{
			goto exit2;
		}

		Audio.m_iNumBuffers    = 4;
		Audio.m_iFullBuffers   = 0;
		Audio.m_iPlayBuffer    = 1;
		Audio.m_iDecodeBuffer  = 0;
		Audio.m_iAbort         = 0;

		for (i = 0; i < Audio.m_iNumBuffers; i++)
		{
			Audio.m_pAudioBuffer[i] = NULL;
			Audio.m_iBufferTimeStamp[i] = 0;
		}

		for (i = 0; i < Audio.m_iNumBuffers; i++)
		{
			Audio.m_pAudioBuffer[i] = memalign(64, m_MpegAtracOutSize);
			if (Audio.m_pAudioBuffer[i] < 0) fail++;
		}

		if (fail > 0)
		{
			for (i = 0; i < Audio.m_iNumBuffers; i++)
			{
				if(Audio.m_pAudioBuffer[i] != NULL) free(Audio.m_pAudioBuffer[i]);
			}
			goto exit3;
		}

		return 0;

	exit3:
		sceKernelDeleteSema(Audio.m_SemaphoreLock);
	exit2:
		sceKernelDeleteSema(Audio.m_SemaphoreStart);
	exit1:
		sceKernelDeleteThread(Audio.m_ThreadID);
	exit0:
		sceAudioChRelease(Audio.m_AudioChannel);

		return -1;
	}
}

SceInt32 ShutdownAudio()
{

	if (playAT3){
		if (at3_end){
			
			sceKernelDeleteThread(Audio.m_ThreadID);
			at3_started = 0;
		}
		return 0;
	}

	else {
	
		int i;
	
		sceAudioChRelease(Audio.m_AudioChannel);
	
		sceKernelDeleteThread(Audio.m_ThreadID);

		sceKernelDeleteSema(Audio.m_SemaphoreStart);
		sceKernelDeleteSema(Audio.m_SemaphoreLock);

		for (i = 0; i < Audio.m_iNumBuffers; i++) free(Audio.m_pAudioBuffer[i]);
	}

	return 0;
}
