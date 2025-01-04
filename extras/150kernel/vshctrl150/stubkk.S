	.set noreorder

#include "pspstub.s"

STUB_START "scePaf",0x00090011,0x00120005
STUB_FUNC  0x9E9FFBFB,sce_paf_private_malloc
STUB_FUNC  0x809A4F83,sce_paf_private_snprintf
STUB_END

