#include "pmf_video.h"
#include "pmf_common.h"
#include "common.h"

SceInt32 AVSyncStatus(DecoderThreadData* D)
{

	if(D->Audio->m_iFullBuffers == 0 || D->Video->m_iFullBuffers == 0 || !playPMFAudio) return 1;

	int iAudioTS = D->Audio->m_iBufferTimeStamp[D->Audio->m_iPlayBuffer];
	int iVideoTS = D->Video->m_iBufferTimeStamp[D->Video->m_iPlayBuffer];

	// if video ahead of audio, do nothing
	if (iVideoTS - iAudioTS > 2 * D->m_iVideoFrameDuration) return 0;

	// if audio ahead of video, skip frame
	if (iAudioTS - iVideoTS > 2 * D->m_iVideoFrameDuration) return 2;

	return 1;
}

static ya2d_texture* image = NULL;
static int debug = 0;

int RenderFrame(int width, int height, void* Buffer)
{

	common::clearScreen(CLEAR_COLOR);
	entry->drawBG();

    int x, y;
    for (y = 0; y < IMAGE_H; y++)
        for (x = 0; x < IMAGE_W; x++)
            ((unsigned int*)image->data)[x+ y * TEXTURE_W] |= 0xFF000000;
            
    ya2d_flush_texture(image);

	ya2d_draw_texture(image, dx, dy);

	if (playAT3 || !playPMFAudio)
		sceKernelDelayThread(10000);
	else
		sceKernelDelayThread(0);

	common::flipScreen();
	
	return 0;
}

int T_Video(SceSize _args, void *_argp)
{

	DecoderThreadData* D = *((DecoderThreadData**)_argp);

	if (!playPMF){
		while (!D->Video->m_iAbort){
			common::clearScreen(CLEAR_COLOR);
			entry->drawBG();
			entry->getIcon()->draw(10, 98);
			common::flipScreen();
		}
		sceKernelExitThread(0);
		return 0;
	}

	int iSyncStatus = 1;

	sceKernelWaitSema(D->Video->m_SemaphoreStart, 1, 0);

	for (;;)
	{
		if (D->Video->m_iAbort != 0) break;

		if (D->Video->m_iFullBuffers > 0)
		{
			iSyncStatus = AVSyncStatus(D);

			if (iSyncStatus > 0)
			{
				if(iSyncStatus == 1) RenderFrame(D->Video->m_iWidth, D->Video->m_iHeight, D->Video->m_pVideoBuffer[D->Video->m_iPlayBuffer]);
				sceKernelWaitSema(D->Video->m_SemaphoreLock, 1, 0);

				D->Video->m_iFullBuffers--;
				sceKernelSignalSema(D->Video->m_SemaphoreLock, 1);
			}
		}
		else
		{
			if (playAT3 || !playPMFAudio)
				sceKernelDelayThread(10000);
			else
				sceKernelDelayThread(0);
		}

		sceKernelSignalSema(D->Video->m_SemaphoreWait, 1);

		if (playAT3 || !playPMFAudio)
			sceKernelDelayThread(10000);
		else
			sceKernelDelayThread(0);
	}

	while (D->Video->m_iFullBuffers > 0)
	{
		RenderFrame(D->Video->m_iWidth, D->Video->m_iHeight, D->Video->m_pVideoBuffer[D->Video->m_iPlayBuffer]);

		sceKernelWaitSema(D->Video->m_SemaphoreLock, 1, 0);

		D->Video->m_iFullBuffers--;

		sceKernelSignalSema(D->Video->m_SemaphoreLock, 1);

		if (playAT3 || !playPMFAudio)
			sceKernelDelayThread(10000);
		else
			sceKernelDelayThread(0);
	}

	sceKernelExitThread(0);

	return 0;
}

SceInt32 InitVideo()
{

	Video.m_ThreadID = sceKernelCreateThread("video_thread", T_Video, 0x3F, 0x10000, PSP_THREAD_ATTR_USER, NULL);
	if (Video.m_ThreadID < 0)
	{
		return -1;
	}

	Video.m_SemaphoreStart = sceKernelCreateSema("video_start_sema", 0, 0, 1, NULL);
	if (Video.m_SemaphoreStart < 0)
	{
		goto exit0;
	}

	Video.m_SemaphoreWait = sceKernelCreateSema("video_wait_sema", 0, 1, 1, NULL);
	if (Video.m_SemaphoreWait < 0)
	{
		goto exit1;
	}

	Video.m_SemaphoreLock = sceKernelCreateSema("video_lock_sema", 0, 1, 1, NULL);
	if (Video.m_SemaphoreLock < 0)
	{
		goto exit2;
	}

	Video.m_iBufferTimeStamp[0]  = 0;
	Video.m_iNumBuffers = 1;
	Video.m_iFullBuffers         = 0;
	Video.m_iPlayBuffer          = 0;
	Video.m_iAbort               = 0;

	// not sure how to get these
	Video.m_iWidth               = IMAGE_W;
	Video.m_iHeight              = IMAGE_H;

    image = ya2d_create_texture(IMAGE_W, IMAGE_H, GU_PSM_8888, YA2D_PLACE_VRAM);
    //image->has_alpha = 0;
    Video.m_pVideoBuffer[0] = image->data;

	return 0;

exit2:
	sceKernelDeleteSema(Video.m_SemaphoreWait);
exit1:
	sceKernelDeleteSema(Video.m_SemaphoreStart);
exit0:
	sceKernelDeleteThread(Video.m_ThreadID);

	return -1;
}

SceInt32 ShutdownVideo()
{
	sceKernelDeleteThread(Video.m_ThreadID);
	sceKernelDeleteSema(Video.m_SemaphoreStart);
	sceKernelDeleteSema(Video.m_SemaphoreWait);
	sceKernelDeleteSema(Video.m_SemaphoreLock);
    
    ya2d_free_texture(image);
    image = NULL;
    Video.m_pVideoBuffer[0] = NULL;
    
	return 0;
}
