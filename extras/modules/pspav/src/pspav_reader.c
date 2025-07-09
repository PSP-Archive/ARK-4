#include "pspav_reader.h"

int T_Reader(SceSize _args, void *_argp)
{

    ReaderThreadData* D   = *((ReaderThreadData**)_argp);

    if (!playAV){
        //while (D->m_Status != ReaderThreadData__READER_ABORT){}
        D->m_Status = ReaderThreadData__READER_EOF;
        sceKernelExitThread(0);
        return 0;
    }

    SceInt32 iFreePackets = 0;
    SceInt32 iFreeLast    = -1;
    SceInt32 iReadPackets = 0;
    SceInt32 iPackets     = 0;

    for (;;)
    {
    
        iPackets = 0;

        if (D->m_Status == ReaderThreadData__READER_ABORT) break;

        iFreePackets = sceMpegRingbufferAvailableSize(D->m_Ringbuffer);
        ////printf("iFreePackets: %p\n", iFreePackets);
        if (iFreeLast != iFreePackets)
        {
            iFreeLast = iFreePackets;
        }

        if (iFreePackets > 0)
        {
            iReadPackets = iFreePackets;

            if (D->m_TotalBytes < D->m_StreamSize)
            {
                if (iReadPackets > 32) iReadPackets = 32;

                int iPacketsLeft = (D->m_StreamSize - D->m_TotalBytes) / 2048;

                if (iPacketsLeft < iReadPackets) iReadPackets = iPacketsLeft;

                iPackets = sceMpegRingbufferPut(D->m_Ringbuffer, iReadPackets, iFreePackets);
                ////printf("iPackets: %p\n", iPackets);
                if (iPackets < 0)
                {
                    D->m_Status = ReaderThreadData__READER_ABORT;
                    break;
                }
            }
        }

        sceKernelSignalSema(D->m_Semaphore, 1);

        if (iPackets > 0) D->m_TotalBytes += iPackets * 2048;

        if (D->m_TotalBytes >= D->m_StreamSize && D->m_Status != ReaderThreadData__READER_ABORT)
        {
            D->m_Status = ReaderThreadData__READER_EOF;
        }
    }

    sceKernelSignalSema(D->m_Semaphore, 1);

    sceKernelExitThread(0);

    return 0;
}

SceInt32 InitReader()
{

    Reader.m_ThreadID = sceKernelCreateThread("reader_thread", T_Reader, 0x41, 0x10000, PSP_THREAD_ATTR_USER, NULL);
    if (Reader.m_ThreadID    < 0)
    {
        return -1;
    }

    Reader.m_Semaphore = sceKernelCreateSema("reader_sema", 0, 0, 1, NULL);
    if (Reader.m_Semaphore < 0)
    {
        sceKernelDeleteSema(Reader.m_Semaphore);
        return -1;
    }

    Reader.m_StreamSize                     = m_MpegStreamSize;
    Reader.m_Ringbuffer                     = &m_Ringbuffer;
    Reader.m_RingbufferPackets              = m_RingbufferPackets;
    Reader.m_Status                         = 0;
    Reader.m_TotalBytes                     = 0;

    return 0;
}

SceInt32 ShutdownReader()
{
    sceKernelDeleteThread(Reader.m_ThreadID);
    sceKernelDeleteSema(Reader.m_Semaphore);
    return 0;
}
