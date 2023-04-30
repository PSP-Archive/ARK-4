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

void patch_update_plugin_module(SceModule *mod_)
{
	int version;
	int i;
	u32 text_addr, text_size;
	SceModule2 *mod = (SceModule2*)mod_;

	text_addr = mod->text_addr;
	text_size = mod->text_size;
	// ImageVersion
	// If it's lower than the one in updatelist.txt then the FW will update
	version = sctrlHENGetMinorVersion(); // ARK's full version number

	_sw((version >> 16) | 0x3C050000, text_addr + 0x000082A4);
	_sw((version & 0xFFFF) | 0x34A40000, text_addr + 0x000082AC);

	//beql -> beq
	_sw( 0x10400002, text_addr + 0x000082A0);

	// substitute all /UPDATE with /ARK_FW
	for(i = 0; i < text_size;) {
		u32 addr = text_addr + i;

		if(0 == strncmp((char *)addr, "/UPDATE", 7)) {
			memcpy((char *)addr, "/ARK_FW", 7);
			i += 7;
		} else {
			i++;
		}
	}
}

void patch_SceUpdateDL_Library(u32 text_addr)
{
	char *p;
	char* server = "http://pro.coldbird.uk.to";

	if(NULL == sceKernelFindModuleByName("update_plugin_module")) {
		return;
	}

	p = (char*)(text_addr + 0x000032BC);
	_sw(NOP, text_addr + 0x00002044);
	_sw(NOP, text_addr + 0x00002054);
	_sw(NOP, text_addr + 0x00002080);
	_sw(NOP, text_addr + 0x0000209C);

	sprintf(p, "%s/psp-updatelist.txt?console=0x%08X", server, ark_config->exec_mode);
}