#include "pspav_video.h"
#include "pspav_common.h"

SceInt32 AVSyncStatus(DecoderThreadData* D)
{

    if(D->Audio->m_iFullBuffers == 0 || D->Video->m_iFullBuffers == 0 || !playAudio) return 1;

    int iAudioTS = D->Audio->m_iBufferTimeStamp[D->Audio->m_iPlayBuffer];
    int iVideoTS = D->Video->m_iBufferTimeStamp[D->Video->m_iPlayBuffer];

    // if video ahead of audio, do nothing
    if (iVideoTS - iAudioTS > 2 * D->m_iVideoFrameDuration) return 0;

    // if audio ahead of video, skip frame
    if (iAudioTS - iVideoTS > 2 * D->m_iVideoFrameDuration) return 2;

    return 1;
}

static void* image[N_VIDEO_BUFFERS] = {NULL};

int RenderFrame(DecoderThreadData* D)
{

    av_callbacks->clearScreen(0);
    if (entry){
        entry->drawBG(entry);
    }

    void* tex = image[D->Video->m_iPlayBuffer];

    av_callbacks->flushTexture(tex);
    av_callbacks->drawTexture(tex, dx, dy);

    if (playAT3 || !playAudio)
        sceKernelDelayThread(10000);
    else
        sceKernelDelayThread(1);

    av_callbacks->flipScreen();
    
    return 0;
}

int T_Video(SceSize _args, void *_argp)
{

    DecoderThreadData* D = *((DecoderThreadData**)_argp);

    if (!playAV){
        while (!D->Video->m_iAbort){
            av_callbacks->clearScreen(0);
            if (entry){
                entry->drawBG(entry);
                entry->drawIcon(entry, dx, dy);
            }
            av_callbacks->flipScreen();
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
                if(iSyncStatus == 1) RenderFrame(D);
                sceKernelWaitSema(D->Video->m_SemaphoreLock, 1, 0);

                D->Video->m_iFullBuffers--;
                sceKernelSignalSema(D->Video->m_SemaphoreLock, 1);
            }
        }
        else
        {
            if (playAT3 || !playAudio)
                sceKernelDelayThread(10000);
            else
                sceKernelDelayThread(0);
        }

        sceKernelSignalSema(D->Video->m_SemaphoreWait, 1);

        if (playAT3 || !playAudio)
            sceKernelDelayThread(10000);
        else
            sceKernelDelayThread(0);
    }

    while (D->Video->m_iFullBuffers > 0)
    {
        RenderFrame(D);

        sceKernelWaitSema(D->Video->m_SemaphoreLock, 1, 0);

        D->Video->m_iFullBuffers--;

        sceKernelSignalSema(D->Video->m_SemaphoreLock, 1);

        if (playAT3 || !playAudio)
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
    Video.m_iBufferTimeStamp[1]  = 0;
    Video.m_iNumBuffers = N_VIDEO_BUFFERS;
    Video.m_iFullBuffers         = 0;
    Video.m_iPlayBuffer          = 0;
    Video.m_iAbort               = 0;

    // not sure how to get these, hardcoded for now
    if (entry){
        Video.m_iWidth               = IMAGE_W;
        Video.m_iHeight              = IMAGE_H;
        Video.m_iBufferWidth         = BUFFER_WIDTH;
    }
    else {
        Video.m_iWidth               = 480;
        Video.m_iHeight              = 272;
        Video.m_iBufferWidth         = 512;
    }

    //image = ya2d_create_texture(Video.m_iWidth, Video.m_iHeight, GU_PSM_8888, YA2D_PLACE_VRAM);
    for (int i=0; i<N_VIDEO_BUFFERS; i++){
        image[i] = av_callbacks->createTexture((entry)?Video.m_iWidth:768, (entry)?Video.m_iHeight:480);
        av_callbacks->setTextureAlpha(image[i], 0);
        Video.m_pVideoBuffer[i] = av_callbacks->getRawTexture(image[i]);
    }

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
    
    for (int i=0; i<N_VIDEO_BUFFERS; i++){
        av_callbacks->freeTexture(image[i]);
        image[i] = NULL;
        Video.m_pVideoBuffer[i] = NULL;
    }
    
    return 0;
}
