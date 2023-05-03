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
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "globals.h"
#include "macros.h"

extern ARKConfig* ark_config;

char server[64];

void load_server_file(){
	char path[ARK_PATH_SIZE];
	strcpy(path, ark_config->arkpath);
	strcat(path, "UPDATER.TXT");

	memset(server, 0, sizeof(server));
	
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	sceIoRead(fd, server, sizeof(server)-1);
	sceIoClose(fd);

	int len = strlen(server);
	if (len && server[len-1] == '\n') server[--len] = 0;
	if (len && server[len-1] == '\r') server[--len] = 0;
	if (len && server[len-1] == '/') server[--len] = 0;
}

void patch_update_plugin_module(SceModule2* mod)
{

	if (server[0] == 0) return;

	int version;
	int i;
	u32 text_addr, text_size, top_addr;
	text_addr = mod->text_addr;
	text_size = mod->text_size;
	top_addr = text_addr+text_size;

	version = sctrlHENGetMinorVersion(); // ARK's full version number

	for (u32 addr=text_addr; addr<top_addr; addr+=4){
		if (_lw(addr) == 0x8FA40100){
			// ImageVersion
			// If it's lower than the one in updatelist.txt then the FW will update
			_sw((version >> 16) | 0x3C050000, addr - 8);
			_sw((version & 0xFFFF) | 0x34A40000, addr);

			//beql -> beq
			_sw( 0x10400002, addr - 12);
			break;
		}
	}

	// substitute all /UPDATE with /ARK_FW
	/*
	for(i = 0; i < text_size;) {
		u32 addr = text_addr + i;

		if(0 == strncmp((char *)addr, "/UPDATE", 7)) {
			memcpy((char *)addr, "/ARK_FW", 7);
			i += 7;
		} else {
			i++;
		}
	}
	*/
}

void patch_SceUpdateDL_Library(SceModule2* mod)
{
	if (server[0] == 0) return;

	if(NULL == sceKernelFindModuleByName("update_plugin_module")) {
		return;
	}

	u32 text_addr, text_size, top_addr;
	text_addr = mod->text_addr;
	text_size = mod->text_size;
	top_addr = text_addr+text_size;

	for (u32 addr = text_addr; addr<top_addr; addr++){
		if (strcmp(addr, "http://") == 0){
			memset(addr, 0, 84);
			sprintf(addr, "%s/psp-updatelist.txt?", server);
			break;
		}
	}
}
