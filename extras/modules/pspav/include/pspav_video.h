#ifndef PSPAV_VIDEO_H
#define PSPAV_VIDEO_H

#include <pspsdk.h>
#include <psptypes.h>
#include "pspav_common.h"

int RenderFrame(DecoderThreadData* D);
int T_Video(SceSize _args, void *_argp);
SceInt32 InitVideo();
SceInt32 ShutdownVideo();


#endif
