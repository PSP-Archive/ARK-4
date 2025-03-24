/*
    Custom Emulator Firmware
    Copyright (C) 2012-2014, ColdBird/Total_Noob/Acid_Snake

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    (at your option) any later version.
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>

#include "spu/stdafx.h"
#include "spu/externals.h"

#define BUFFER_SIZE    22050
#define PSP_NUM_AUDIO_SAMPLES 1024

short *pSndBuffer = NULL;
int iBufSize = 0;
volatile int iReadPos = 0, iWritePos = 0;

SceUID audio_thid = -1;
int channel = -1;

static void FillAudio(unsigned char *stream, int len)
{
    short *p = (short *)stream;

    len /= sizeof(short);

    while(iReadPos != iWritePos && len > 0)
    {
        *p++ = pSndBuffer[iReadPos++];
        if(iReadPos >= iBufSize) iReadPos = 0;
        --len;
    }

    // Fill remaining space with zero
    while(len > 0)
    {
        *p++ = 0;
        --len;
    }
}

int audio_thread(SceSize args, void *argp)
{
    static unsigned char buf[PSP_NUM_AUDIO_SAMPLES * 4] __attribute__((aligned(64)));

    while(1)
    {
        FillAudio(buf, sizeof(buf));
        sceAudioOutputPannedBlocking(channel, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, buf);
    }

    return 0;
}

void SetupSound(void)
{
    if(pSndBuffer != NULL) return;

    channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_NUM_AUDIO_SAMPLES, PSP_AUDIO_FORMAT_STEREO);

    audio_thid = sceKernelCreateThread("audio_thread", audio_thread, 0x10, 0x1000, 0, NULL);
    if(audio_thid >= 0) sceKernelStartThread(audio_thid, 0, NULL);

    iBufSize = BUFFER_SIZE;

    pSndBuffer = (short *)malloc(iBufSize * sizeof(short));
    if(pSndBuffer == NULL)
    {
        return;
    }

    iReadPos = 0;
    iWritePos = 0;
}

void RemoveSound(void)
{
    if(pSndBuffer == NULL) return;

    if(audio_thid >= 0)
    {
        sceKernelTerminateDeleteThread(audio_thid);
    }

    if(channel >= 0)
    {
        sceAudioChRelease(channel);
    }

    free(pSndBuffer);
    pSndBuffer = NULL;
}

unsigned long SoundGetBytesBuffered(void)
{
    int size;

    if(pSndBuffer == NULL) return SOUNDSIZE;

    size = iReadPos - iWritePos;
    if(size <= 0) size += iBufSize;

    if(size < iBufSize / 2) return SOUNDSIZE;

    return 0;
}

void SoundFeedStreamData(unsigned char *pSound, long lBytes)
{
    short *p = (short *)pSound;

    if(pSndBuffer == NULL) return;

    while(lBytes > 0)
    {
        if(((iWritePos + 1) % iBufSize) == iReadPos) break;

        pSndBuffer[iWritePos] = *p++;

        ++iWritePos;
        if(iWritePos >= iBufSize) iWritePos = 0;

        lBytes -= sizeof(short);
    }
}