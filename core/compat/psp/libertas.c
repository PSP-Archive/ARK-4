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

#include <string.h>
#include <pspiofilemgr.h>
#include "libertas.h"
#include "globals.h"
#include "macros.h"

extern int psp_model;

// MAC Backup Flag
int isRealMACSafe = 0;

// Original MAC
static uint8_t realMAC[6];

// MAC Read Flag
int isFakeMACAvailable = 0;

// Fake MAC
static uint8_t fakeMAC[6];

// Original ARM to MIPS Endian Translator
int (* ReverseCommandEndianess)(LIBERTAS_COMMAND * cmd, int size) = NULL;

// Replacement ARM to MIPS Endian Translator
int ReverseCommandEndianessPatched(LIBERTAS_COMMAND *cmd, int size)
{
	// Hardware Specification Read Command
	if(cmd->CmdCode == LIBERTAS_CMD_GET_HW_SPEC)
	{
		// Verify Size (is this really required?)
		if(cmd->Size == (sizeof(LIBERTAS_GET_HW_SPEC_COMMAND) + 8))
		{
			// MAC Backup required
			if(!isRealMACSafe)
			{
				// Copy MAC
				memcpy(realMAC, cmd->data.hwspec.MacAddr, sizeof(realMAC));
				
				// Set Flag
				isRealMACSafe = 1;
			}
			
			// Overwrite MAC with Fake MAC
			if(isFakeMACAvailable)
			{
				// Copy Fake MAC into Field
				memcpy(cmd->data.hwspec.MacAddr, fakeMAC, sizeof(fakeMAC));
			}
		}		
	}
	
	// Forward Call
	return ReverseCommandEndianess(cmd, size);
}

// Switch Lower to Upper Case Characters
void toUpperCase(char * path)
{
	// Iterate Characters
	uint32_t i = 0; for(; i < strlen(path); i++)
	{
		// Lower Case Character
		if(path[i] >= 'a' && path[i] <= 'z')
		{
			// Replace with Upper Case
			path[i] = path[i] - 'a' + 'A';
		}
	}
}

// Read Fake MAC Config
int read_MAC_config(char * path)
{
	// Read Buffer
	char buffer[18];
	
	// Read Size in Bytes
	int read = 0;
	
	// Terminate Buffer (in advance)
	buffer[17] = 0;
	
	// Open MAC Configuration File
	SceUID fd = sceIoOpen((path != NULL) ? (path) : ("ms0:/seplugins/mac.txt"), PSP_O_RDONLY, 0);
	
	// Opened MAC Configuration File
	if(fd >= 0)
	{
		// Read Buffer
		read = sceIoRead(fd, buffer, sizeof(buffer) - 1);
		
		// Close MAC Configuration File
		sceIoClose(fd);
	}
	
	// MAC Configuration File not found
	else return -1;
	
	// Valid MAC Configuration Size
	if(read == (sizeof(buffer) - 1))
	{
		// Change Buffer to Upper Case
		toUpperCase(buffer);
		
		// Valid Delimiters
		if(buffer[2] == ':' && buffer[5] == ':' && buffer[8] == ':' && buffer[11] == ':' && buffer[14] == ':')
		{
			// MAC Validicity
			int isValidMAC = 1;
			
			// Process MAC Bytes
			uint32_t i = 0; for(; i < 6; i++)
			{
				// Nibble Characters
				char nibblechar[2];
				nibblechar[0] = buffer[i * 3];
				nibblechar[1] = buffer[i * 3 + 1];
				
				// High Nibble Value
				uint8_t highnibble = 0;
				if(nibblechar[0] >= '0' && nibblechar[0] <= '9') highnibble = nibblechar[0] - '0';
				else if(nibblechar[0] >= 'A' && nibblechar[0] <= 'F') highnibble = nibblechar[0] - 'A' + 10;
				else isValidMAC = 0;
				
				// Low Nibble Value
				uint8_t lownibble = 0;
				if(nibblechar[1] >= '0' && nibblechar[1] <= '9') lownibble = nibblechar[1] - '0';
				else if(nibblechar[1] >= 'A' && nibblechar[1] <= 'F') lownibble = nibblechar[1] - 'A' + 10;
				else isValidMAC = 0;
				
				// Invalid MAC Byte
				if(!isValidMAC) break;
				
				// Calculate Hex Byte Value
				fakeMAC[i] = (highnibble << 4) | lownibble;
			}
			
			// Successfully parsed Fake MAC
			if(isValidMAC)
			{
				// Set Flag
				isFakeMACAvailable = 1;
				
				// Return Success
				return 0;
			}
			
			// Invalid MAC Bytes encountered
			return -4;
		}
		
		// Invalid Byte Delimiters encountered
		return -3;
	}
	
	// Invalid MAC Configuration Size encountered
	return -2;
}

// Hook MAC Getter
void patch_Libertas_MAC(SceModule2 * mod)
{
	// MAC Spoofer activated
		
	// Read Fake MAC Configuration
	if(read_MAC_config(NULL) != 0)
	{
		// Retry for PSP Go Models
		if(psp_model == PSP_GO)
		{
			if (read_MAC_config("ef0:/seplugins/mac.txt") != 0){
				return;
			}
		}
	}
	
	// Cast Text Segment
	uint32_t * text = (uint32_t *)mod->text_addr;
	
	// Scan Instruction
	uint32_t scan = 0;
	
	// Find ARM to MIPS Endian Translator
	uint32_t i = 0; for(; i < mod->text_size / 4 - 1; i++)
	{
		// Found Function
		if(text[i] == 0x00053083 && text[i + 1] == 0x00061880)
		{
			// Save Original Function Reference
			ReverseCommandEndianess = (void *)&text[i];
			
			// Craft JAL Scan Instruction
			scan = JAL((uint32_t)&text[i]);
			
			// Stop Search
			break;
		}
	}
	
	// Found Function
	if(scan != 0)
	{
		// Find Referencing JAL Calls
		for(i = 0; i < mod->text_size / 4; i++)
		{
			// Found matching JAL Call
			if(text[i] == scan)
			{
				// Overwrite JAL Call
				text[i] = JAL(ReverseCommandEndianessPatched);
			}
		}
	}
}

// Original MAC Getter
int sctrlGetRealEthernetAddress(uint8_t * mac)
{
	// Valid Argument
	if(mac != NULL)
	{
		// Clear Memory
		memset(mac, 0, 6);
		
		// Real MAC available
		if(isRealMACSafe)
		{
			// Copy Real MAC
			memcpy(mac, realMAC, sizeof(realMAC));
			
			// Success
			return 0;
		}
		
		// Real MAC unavailable
		return -2;
	}
	
	// NULL Pointer Argument
	return -1;
}

