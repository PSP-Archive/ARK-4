# Use this in your project, or as reference
	.set noreorder

#include "pspimport.s"

IMPORT_START "unziprar", 0x40090000
IMPORT_FUNC "unziprar", 0x0E063607, unzipToDir
IMPORT_FUNC "unziprar", 0x0528D2EE, DoExtractRAR
