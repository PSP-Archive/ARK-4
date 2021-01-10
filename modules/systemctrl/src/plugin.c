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
#include <pspinit.h>
#include <pspmodulemgr.h>
#include <pspiofilemgr.h>
#include <globals.h>
#include <systemctrl_private.h>
#include "plugin.h"

#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

// Missing Libc Function strcasecmp (needed for stricmp to work)
int strcasecmp(const char * a, const char * b)
{
	// Pointer Equality
	if(a == b) return 0;
	
	// NULL Pointer
	if(a == NULL || b == NULL) return -1;
	
	// Comparison Position
	unsigned int i = 0;
	
	// Compare Character
	while(1)
	{
		// Calculate Difference
		int diff = tolower(a[i]) - tolower(b[i]);
		
		// Difference Detected
		if(diff != 0) return diff;
		
		// End of String Detected
		if(a[i] == 0) break;
		
		// Move Position
		i++;
	}
	
	// Equal Strings
	return 0;
}

// Runlevel Check
static int matchingRunlevel(char * runlevel)
{
	// Fetch Apitype
	int apitype = sceKernelInitApitype();
	
	// "game" Runlevel
	if((apitype == 0x141 || apitype == 0x123) && stricmp(runlevel, "game") == 0) return 1;
	
	// "umdemu" Runlevel
	if(apitype == 0x123 && stricmp(runlevel, "umdemu") == 0) return 1;
	
	// "pops" Runlevel
	if(apitype == 0x144 && stricmp(runlevel, "pops") == 0) return 1;
	
	// Unsupported Runlevel (we don't touch those to keep stability up)
	return 0;
}

// Boolean String Parser
static int booleanOn(char * text)
{
	// Different Variations of "true"
	if(stricmp(text, "true") == 0 || stricmp(text, "on") == 0 ||
		strcmp(text, "1") == 0 || stricmp(text, "enabled") == 0)
			return 1;
	
	// Default to False
	return 0;
}

// Load and Start Plugin Module
static void startPlugin(char * path)
{
	// Load Module
	int uid = sceKernelLoadModule(path, 0, NULL);
	
	// Loaded Module
	if(uid >= 0)
	{
		// Start Module
		sceKernelStartModule(uid, strlen(path) + 1, path, NULL, NULL);
	}
}

// Whitespace Detection
int isspace(int c)
{
	// Whitespaces
	if(c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f' || c == '\n')
		return 1;
	
	// Normal Character
	return 0;
}

// Trim Leading and Trailing Whitespaces
static char * strtrim(char * text)
{
	// Invalid Argument
	if(text == NULL) return NULL;
	
	// Remove Leading Whitespaces
	while(isspace(text[0])) text++;
	
	// Scan Position
	unsigned int pos = strlen(text)-1;
	
	// Find Trailing Whitespaces
	while(isspace(text[pos])) pos--;
	
	// Terminate String
	text[pos+1] = (char)0;
	
	// Return Trimmed String
	return text;
}

// Read Line from File Descriptor
static char * readLine(int fd, char * buf, unsigned int buflen)
{
	// Valid Arguments
	if(fd >= 0 && buf != NULL && buflen > 0)
	{
		// Clean Memory
		memset(buf, 0, buflen);
		
		// Buffer Position
		unsigned int pos = 0;
		
		// Read Text
		while(pos < buflen - 1 && sceIoRead(fd, buf + pos, 1) == 1)
		{
			// Carriage Return (Windows)
			if(buf[pos] == '\r')
			{
				// Next Symbol
				char c = 0;
				
				// Read Next Symbol (to prevent double tapping)
				if(sceIoRead(fd, &c, 1) == 1)
				{
					// Newline
					if(c == '\n') break;
					
					// Rewind File
					sceIoLseek32(fd, -1, PSP_SEEK_CUR);
				}
				
				// Handle as Newline
				break;
			}
			
			// Newline
			if(buf[pos] == '\n') break;
			
			// Move Position
			pos++;
		}
		
		// End of File
		if(pos == 0) return NULL;
		
		// Remove \r\n
		if(buf[pos] == '\r' || buf[pos] == '\n') buf[pos] = 0;
		
		// Return Line Buffer
		return buf;
	}
	
	// Invalid Arguments
	return NULL;
}

// Parse and Process Line
void processLine(char * line)
{
	// Skip Comment Lines
	if(strncmp(line, "//", 2) == 0 || line[0] == ';' || line[0] == '#')
		return;
	
	// String Token
	char * runlevel = line;
	char * path = NULL;
	char * enabled = NULL;
	
	// Original String Length
	unsigned int length = strlen(line);
	
	// Fetch String Token
	unsigned int i = 0; for(; i < length; i++)
	{
		// Got all required Token
		if(enabled != NULL)
		{
			// Handle Trailing Comments as Terminators
			if(strncmp(line + i, "//", 2) == 0 || line[i] == ';' || line[i] == '#')
			{
				// Terminate String
				line[i] = 0;
				
				// Stop Token Scan
				break;
			}
		}
		
		// Found Delimiter
		if(line[i] == LINE_TOKEN_DELIMITER)
		{
			// Terminate String
			line[i] = 0;
			
			// Path Start
			if(path == NULL) path = line + i + 1;
			
			// Enabled Start
			else if(enabled == NULL) enabled = line + i + 1;
			
			// Got all Data
			else break;
		}
	}
	
	// Unsufficient Plugin Information
	if(enabled == NULL) return;
	
	// Trim Whitespaces
	runlevel = strtrim(runlevel);
	path = strtrim(path);
	enabled = strtrim(enabled);
	
	// Matching Plugin Runlevel
	if(matchingRunlevel(runlevel))
	{
		// Enabled Plugin
		if(booleanOn(enabled))
		{
			// Start Plugin
			startPlugin(path);
		}
	}
}

// Load Plugins
void LoadPlugins(void)
{

	if (IS_VITA_POPS)
		return;

	// start the mandatory qsplink plugin
	startPlugin("flash0:/kd/ark_qsplink.prx");

	// Open Plugin Config
	
	char path[SAVE_PATH_SIZE+20];
	strcpy(path, ARKPATH);
	strcat(path, "PLUGINS.TXT");
	
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	
	// Opened Plugin Config
	if(fd >= 0)
	{
		// Allocate Line Buffer
		char * line = (char *)oe_malloc(LINE_BUFFER_SIZE);
		
		// Buffer Allocation Success
		if(line != NULL)
		{
			// Read Lines
			while(readLine(fd, line, LINE_BUFFER_SIZE) != NULL)
			{
				// Process Line
				processLine(strtrim(line));
			}
			
			// Free Buffer
			oe_free(line);
		}
		
		// Close Plugin Config
		sceIoClose(fd);
	}
}

