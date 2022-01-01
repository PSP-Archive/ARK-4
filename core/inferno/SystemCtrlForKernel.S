	.set noreorder

#include "pspimport.s"

	IMPORT_START "SystemCtrlForKernel",0x00090000
	IMPORT_FUNC  "SystemCtrlForKernel",0x16100529,LZ4_decompress_fast
