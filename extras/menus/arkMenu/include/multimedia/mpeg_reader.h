#ifndef MPEG_READER_H
#define MPEG_READER_H

#include <pspsdk.h>
#include <psptypes.h>
#include "mpeg_common.h"


int T_Reader(SceSize _args, void *_argp);
SceInt32 InitReader();
SceInt32 ShutdownReader();


#endif
