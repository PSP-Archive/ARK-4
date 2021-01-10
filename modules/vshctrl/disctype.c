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

#include "isoreader.h"
#include <string.h>
#include <pspiofilemgr.h>
#include <psputilsforkernel.h>
#include <pspumd.h>
#include "printk.h"
#include "utils.h"
#include "systemctrl_private.h"

static int has_file(char *file)
{
	int ret;
	u32 size, lba;
	
	ret = isoGetFileInfo(file, &size, &lba);
	ret = (ret >= 0) ? 1 : 0;

	return ret;
}

int vshDetectDiscType(const char *path)
{
	int result, ret;
	u32 k1;

	result = -1;
	k1 = pspSdkSetK1(0);
	ret = isoOpen(path);

	if (ret < 0) {
		pspSdkSetK1(k1);
		return result;
	}

	result = 0;
	
	if(has_file("/PSP_GAME/SYSDIR/EBOOT.BIN")) {
		result |= PSP_UMD_TYPE_GAME;
	} 

	if(has_file("/UMD_VIDEO/PLAYLIST.UMD")) {
		result |= PSP_UMD_TYPE_VIDEO;
	} 

	if(has_file("/UMD_AUDIO/PLAYLIST.UMD")) {
		result |= PSP_UMD_TYPE_AUDIO;
	}

	if(result == 0) {
		result = -2;
	}

	isoClose();
	pspSdkSetK1(k1);
	
	return result;
}
