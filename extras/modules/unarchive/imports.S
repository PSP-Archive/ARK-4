# Use this in your project, or as reference
	.set noreorder

#include "pspimport.s"

IMPORT_START "unarchive", 0x40090000
IMPORT_FUNC "unarchive", 0xF6EC2A3A, unarchiveFile
