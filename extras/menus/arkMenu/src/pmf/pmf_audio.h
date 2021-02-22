#ifndef PMF_AUDIO_H
#define PMF_AUDIO_H

#include <pspsdk.h>
#include <psptypes.h>
#include "pmf_common.h"


extern "C" {
int sceAtracGetSecondBufferInfo(int atracID, u32 *puiPosition, u32 *puiDataByte);
int sceAtracGetNextDecodePosition(int atracID, u32 *puiSamplePosition);
int sceAtracGetChannel(int, u32*);
}

extern int at3_started;

SceInt32 AudioSyncStatus(DecoderThreadData* D);
int T_Audio(SceSize _args, void *_argp);
SceInt32 InitAudio();
SceInt32 ShutdownAudio();

#endif
