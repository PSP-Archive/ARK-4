#ifndef PMF_VIDEO_H
#define PMF_VIDEO_H

#include <pspsdk.h>
#include <psptypes.h>
#include "pmf_common.h"

int RenderFrame(int width, int height, void* Buffer);
int T_Video(SceSize _args, void *_argp);
SceInt32 InitVideo();
SceInt32 ShutdownVideo();


#endif
