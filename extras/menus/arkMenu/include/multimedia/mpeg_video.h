#ifndef MPEG_VIDEO_H
#define MPEG_VIDEO_H

#include <pspsdk.h>
#include <psptypes.h>
#include "mpeg_common.h"

int RenderFrame(int width, int height, void* Buffer);
int T_Video(SceSize _args, void *_argp);
SceInt32 InitVideo();
SceInt32 ShutdownVideo();


#endif
