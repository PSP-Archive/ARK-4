#ifndef PSPAV_AUDIO_H
#define PSPAV_AUDIO_H

#include <pspsdk.h>
#include <psptypes.h>
#include "pspav_common.h"

extern int at3_started;

int sceAtracGetSecondBufferInfo(int atracID, u32 *puiPosition, u32 *puiDataByte);
int sceAtracGetNextDecodePosition(int atracID, u32 *puiSamplePosition);
int sceAtracGetChannel(int, u32*);

SceInt32 AudioSyncStatus(DecoderThreadData* D);
int T_Audio(SceSize _args, void *_argp);
SceInt32 InitAudio();
SceInt32 ShutdownAudio();

#endif
