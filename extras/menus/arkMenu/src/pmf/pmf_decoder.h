#ifndef PMF_DECODER_H
#define PMF_DECODER_H

#include <pspsdk.h>
#include <psptypes.h>
#include "pmf_common.h"

SceInt32 IsRingbufferFull(ReaderThreadData* D);
int T_Decoder(SceSize _args, void *_argp);
SceInt32 InitDecoder();
SceInt32 ShutdownDecoder();

#endif
