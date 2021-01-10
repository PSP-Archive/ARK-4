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
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <globals.h>

// Patch mediasync.prx
void patchMediaSync(unsigned int textAddr)
{
	if (IS_VITA_POPS){
		MAKE_DUMMY_FUNCTION(textAddr + 0x000006A8, 0);

		// Avoid SCE_MEDIASYNC_ERROR_INVALID_MEDIA
		_sh(0x5000, textAddr + 0x00000328 + 2);
		_sh(0x1000, textAddr + 0x00000D2C + 2);

		return;
	}

	// Patch MsCheckMedia to always succeed
	_sw(JR_RA, textAddr + MEDIASYNC_MS_CHECK_MEDIA);
	_sw(LI_V0(1), textAddr + MEDIASYNC_MS_CHECK_MEDIA + 4);
	
	// Patch DiscCheckMedia
	_sw(0x1000001D, textAddr + MEDIASYNC_DISC_MEDIA_CHECK_1);
	_sw(0x1000001D, textAddr + MEDIASYNC_DISC_MEDIA_CHECK_2);
	
	// Patch MsSystemFile
	_sw(0x1000FFDB, textAddr + MEDIASYNC_MS_SYSTEM_FILE_CHECK);
	
	// Patch DISC_ID Check (to make homebrews without one work)
	_sw(NOP, textAddr + MEDIASYNC_DISC_ID_CHECK_1);
	_sw(NOP, textAddr + MEDIASYNC_DISC_ID_CHECK_2);
}

