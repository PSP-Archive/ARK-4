/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <psploadexec_kernel.h>
#include <string.h>
#include "elf.h"

// Check Executable Type
int IsStaticElf(void * buf)
{
	// Cast Header
	Elf32_Ehdr * header = (Elf32_Ehdr *)buf;
	
	// Plain ELF and Static
	if (header->e_magic == 0x464C457F && header->e_type == 2)
	{
		return 1;
	}
	
	// Other Executable
	return 0;
}

// Get String Table
char * GetStrTab(unsigned char * buf)
{
	// Cast Header
	Elf32_Ehdr * header = (Elf32_Ehdr *)buf;
	
	// Not Plain ELF
	if(header->e_magic != 0x464C457F) return NULL;
	
	// Move to Section Header
	unsigned char * pData = buf + header->e_shoff;
	
	// Iterate Sections
	int i = 0; for(; i < header->e_shnum; i++)
	{
		// Found String Table Section Index
		if (header->e_shstrndx == i)
		{
			// Cast Section Header
			Elf32_Shdr * section = (Elf32_Shdr *)pData;
			
			// Valid Section Type
			if(section->sh_type == 3)
			{
				// Return Section Pointer
				return (char *)buf + section->sh_offset;
			}
		}
		
		// Move to next Section
		pData += header->e_shentsize;
	}
	
	// String Table not found
	return NULL;
}

