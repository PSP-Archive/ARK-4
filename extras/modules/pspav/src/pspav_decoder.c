#include "pspav_decoder.h"

SceInt32 IsRingbufferFull(ReaderThreadData* D)
{
    int size;
    if (D->m_Status == ReaderThreadData__READER_EOF) return 1;
    size = sceMpegRingbufferAvailableSize(D->m_Ringbuffer);
    if (size > 0) return 0;
    return 1;
}


int T_Decoder(SceSize _args, void *_argp)
{

    DecoderThreadData* D = *((DecoderThreadData**)_argp);

    if (!playAV){
        while (1){
            PSPAV_PadState pad = av_callbacks->getPadState();
            if (pad == PAD_USER_CANCEL)
            {
                run = 0;
                break;
            }
            else if (pad == PAD_USER_ACCEPT)
            {
                run = 1;
                break;
            }
            //else if (AT3->end)
            //    break;
        }
        D->Audio->m_iAbort = 1;
        D->Video->m_iAbort = 1;
        D->Reader->m_Status = ReaderThreadData__READER_ABORT;
        work = 0;
        sceKernelExitThread(0);
        return 0;
    }

    int retVal;

    int iInitAudio = 1;
    SceInt32 iVideoStatus = 0;
    int videoFrameCount = 0;
    int audioFrameCount = 0;

    SceInt32 unknown = 0;

    int iThreadsRunning = 0;

    SceInt32 m_iAudioCurrentTimeStamp = 0;
    SceInt32 m_iVideoCurrentTimeStamp = 0;
    SceInt32 m_iVideoLastTimeStamp = 0;

    SceUInt32 m_iLastPacketsWritten = D->Reader->m_Ringbuffer->iUnk1;
    SceInt32  m_iLastPacketsAvailable = sceMpegRingbufferAvailableSize(D->Reader->m_Ringbuffer);
    //printf("m_iLastPacketsAvailable: %p\n", m_iLastPacketsAvailable);

    //D->Connector->initConnector();

    for (;;)
    {
        PSPAV_PadState pad = av_callbacks->getPadState();
        if (pad == PAD_USER_CANCEL)
        {
            run = 0;
            work = 0;
        }
        else if (pad == PAD_USER_ACCEPT)
        {
            if (entry){
                run = 1;
                work = 0;
            }
            else{
                // TODO: figure out pause/resume
            }
        }

        if (!work) break;

        sceKernelDelayThread(1);
        scePowerTick(0);

        if( iThreadsRunning == 0 && IsRingbufferFull(D->Reader) && D->Video->m_iNumBuffers == D->Video->m_iFullBuffers)
        {
            iThreadsRunning = 1;
            sceKernelSignalSema(D->Video->m_SemaphoreStart, 1);
            sceKernelSignalSema(D->Audio->m_SemaphoreStart, 1);
        }

        if (D->Reader->m_Status == ReaderThreadData__READER_ABORT)
        {
            break;
        }
        else if (D->Reader->m_Status == ReaderThreadData__READER_EOF)
        {
            retVal = sceMpegRingbufferAvailableSize(D->Reader->m_Ringbuffer);
            ////printf("sceMpegRingbufferAvailableSize: %p\n", retVal);
            if(retVal == D->Reader->m_RingbufferPackets) break;
        }

        if (!IsRingbufferFull(D->Reader))
        {
            sceKernelWaitSema(D->Reader->m_Semaphore, 1, 0);
            if (D->Reader->m_Status == ReaderThreadData__READER_ABORT) break;
        }

        if (D->Audio->m_iFullBuffers < D->Audio->m_iNumBuffers)
        {
            retVal = sceMpegGetAtracAu(&D->m_Mpeg, D->m_MpegStreamAtrac, D->m_MpegAuAtrac, &unknown);
            //printf("sceMpegGetAtracAu: %p\n", retVal);
            if (retVal != 0)
            {
                playAudio = 0;
                if (!IsRingbufferFull(D->Reader))
                {
                    sceKernelWaitSema(D->Reader->m_Semaphore, 1, 0);

                    if(D->Reader->m_Status == ReaderThreadData__READER_ABORT) break;
                }
            }
            else
            {
                playAudio = 1;
                if (m_iAudioCurrentTimeStamp >= D->m_iLastTimeStamp - D->m_iVideoFrameDuration) break;

                memset(D->Audio->m_pAudioBuffer[D->Audio->m_iDecodeBuffer], 0, m_MpegAtracOutSize);

                retVal = sceMpegAtracDecode(&D->m_Mpeg, D->m_MpegAuAtrac, D->Audio->m_pAudioBuffer[D->Audio->m_iDecodeBuffer], iInitAudio);
                //printf("sceMpegAtracDecode: %p\n", retVal);
                if (retVal != 0)
                {
                    break;
                }

                if (D->m_MpegAuAtrac->iPts == 0xFFFFFFFF) {
                    m_iAudioCurrentTimeStamp += D->m_iAudioFrameDuration;
                } else {
                    m_iAudioCurrentTimeStamp = D->m_MpegAuAtrac->iPts;
                }

                if (m_iAudioCurrentTimeStamp <= 0x15F90 - D->m_iAudioFrameDuration) {
                    iInitAudio = 1;
                }

                D->Audio->m_iBufferTimeStamp[D->Audio->m_iDecodeBuffer] = m_iAudioCurrentTimeStamp;

                if (iInitAudio == 0)
                {
                    //D->Connector->sendAudioFrame(audioFrameCount, D->Audio->m_pAudioBuffer[D->Audio->m_iDecodeBuffer], D->m_MpegAtracOutSize, m_iAudioCurrentTimeStamp);
                    audioFrameCount++;

                    sceKernelWaitSema(D->Audio->m_SemaphoreLock, 1, 0);

                    D->Audio->m_iFullBuffers++;

                    sceKernelSignalSema(D->Audio->m_SemaphoreLock, 1);

                    D->Audio->m_iDecodeBuffer = (D->Audio->m_iDecodeBuffer + 1) % D->Audio->m_iNumBuffers;
                }

                iInitAudio = 0;
            }
        }

        if (!IsRingbufferFull(D->Reader))
        {
            sceKernelWaitSema(D->Reader->m_Semaphore, 1, 0);

            if (D->Reader->m_Status == ReaderThreadData__READER_ABORT) break;
        }

        if (D->Video->m_iFullBuffers < D->Video->m_iNumBuffers)
        {

            retVal = sceMpegGetAvcAu(&D->m_Mpeg, D->m_MpegStreamAVC, D->m_MpegAuAVC, &unknown);
            //printf("sceMpegGetAvcAu: %p\n", retVal);
            if ((SceUInt32)retVal == 0x80618001)
            {
                if (!IsRingbufferFull(D->Reader))
                {
                    sceKernelWaitSema(D->Reader->m_Semaphore, 1, 0);

                    if (D->Reader->m_Status == ReaderThreadData__READER_ABORT) break;
                }
            }
            else if (retVal != 0)
            {
                break;
            }
            else
            {
                if (m_iVideoCurrentTimeStamp >= D->m_iLastTimeStamp - D->m_iVideoFrameDuration) break;

                retVal = sceMpegAvcDecode(&D->m_Mpeg, D->m_MpegAuAVC, D->Video->m_iBufferWidth, &D->Video->m_pVideoBuffer[D->Video->m_iPlayBuffer], &iVideoStatus);
                //printf("sceMpegAvcDecode: %p\n", retVal);
                if (retVal != 0)
                {
                    break;
                }

                if (D->m_MpegAuAVC->iPts == 0xFFFFFFFF) {
                    m_iVideoCurrentTimeStamp += 0x0BBC;
                } else {
                    m_iVideoCurrentTimeStamp = D->m_MpegAuAVC->iPts;
                }

                if (iVideoStatus == 1)
                {
                    SceUInt32 m_iPacketsWritten = D->Reader->m_Ringbuffer->iUnk1;
                    SceInt32  m_iPacketsAvailable = sceMpegRingbufferAvailableSize(D->Reader->m_Ringbuffer);

                    SceInt32  m_iDeltaPacketsWritten = m_iPacketsWritten - m_iLastPacketsWritten;
                    if (m_iDeltaPacketsWritten < 0)
                    {
                        m_iDeltaPacketsWritten += D->Reader->m_Ringbuffer->iPackets;
                    }
                    //SceInt32 m_iDeltaPacketsAvailable = m_iPacketsAvailable - m_iLastPacketsAvailable;
                    //SceInt32 m_iConsumedPackets = m_iDeltaPacketsAvailable + m_iDeltaPacketsWritten;

                    m_iLastPacketsWritten = m_iPacketsWritten;
                    m_iLastPacketsAvailable = m_iPacketsAvailable;

                    //D->Connector->sendVideoFrame(videoFrameCount, D->Video->m_pVideoBuffer[D->Video->m_iPlayBuffer], m_iVideoCurrentTimeStamp, D->Reader, m_iConsumedPackets);
                    videoFrameCount++;

                    D->Video->m_iBufferTimeStamp[D->Video->m_iPlayBuffer] = m_iVideoLastTimeStamp;

                    sceKernelWaitSema(D->Video->m_SemaphoreLock, 1, 0);

                    D->Video->m_iFullBuffers++;

                    sceKernelSignalSema(D->Video->m_SemaphoreLock, 1);
                }

                m_iVideoLastTimeStamp = m_iVideoCurrentTimeStamp;
            }
        }

        if (!IsRingbufferFull(D->Reader))
        {
            sceKernelWaitSema(D->Reader->m_Semaphore, 1, 0);

            if(D->Reader->m_Status == ReaderThreadData__READER_ABORT) break;
        }

    }

    //D->Connector->exitConnector();

    sceKernelSignalSema(D->Audio->m_SemaphoreStart, 1);
    sceKernelSignalSema(D->Video->m_SemaphoreStart, 1);

    D->Reader->m_Status = ReaderThreadData__READER_ABORT;

    if (!playAT3 || !work)
        D->Audio->m_iAbort = 1;

    while (D->Video->m_iFullBuffers > 0)
    {
        sceKernelWaitSema(D->Video->m_SemaphoreWait, 1, 0);
        sceKernelSignalSema(D->Video->m_SemaphoreLock, 1);
    }

    sceMpegAvcDecodeStop(&D->m_Mpeg, BUFFER_WIDTH, D->Video->m_pVideoBuffer, &iVideoStatus);

    if (iVideoStatus > 0)
    {
        sceKernelWaitSema(D->Video->m_SemaphoreLock, 1, 0);

        D->Video->m_iFullBuffers++;

        sceKernelSignalSema(D->Video->m_SemaphoreLock, 1);
    }

    D->Video->m_iAbort = 1;

    sceMpegFlushAllStream(&D->m_Mpeg);

    sceKernelExitThread(0);

    return 0;
}

SceInt32 InitDecoder()
{

    Decoder.m_ThreadID = sceKernelCreateThread("decoder_thread", T_Decoder, 0x10, 0x10000, PSP_THREAD_ATTR_USER, NULL);

    if (Decoder.m_ThreadID < 0)
    {
        return -1;
    }

    Decoder.Reader                = &Reader;
    Decoder.Video                 = &Video;
    Decoder.Audio                 = &Audio;
    Decoder.m_Mpeg                = m_Mpeg;
    Decoder.m_MpegStreamAVC       = m_MpegStreamAVC;
    Decoder.m_MpegAuAVC           = &m_MpegAuAVC;
    Decoder.m_MpegStreamAtrac     = m_MpegStreamAtrac;
    Decoder.m_MpegAuAtrac         = &m_MpegAuAtrac;
    Decoder.m_MpegAtracOutSize    = m_MpegAtracOutSize;

    Decoder.m_iAudioFrameDuration = 4180; // ??
    Decoder.m_iVideoFrameDuration = (int)(90000 / 29.97);
    Decoder.m_iLastTimeStamp      = m_iLastTimeStamp;

    return 0;
}

SceInt32 ShutdownDecoder()
{
    sceKernelDeleteThread(Decoder.m_ThreadID);
    return 0;
}
