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

#include "registry.h"

#include <pspsdk.h>
#include <pspdebug.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psputility.h>
#include <pspreg.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include "vpl.h"
#include "scepaf.h"

void delete_hibernation(vsh_Menu *vsh) {
	if (vsh->psp_model == PSP_GO) {
		vshCtrlDeleteHibernation();
		vsh->status.reset_vsh = 1;
	}
}

int codecs_activated() {
	u32 flash_activated = 0;
	u32 flash_play = 0;
	u32 wma_play = 0;

	vctrlGetRegistryValue("/CONFIG/BROWSER", "flash_activated", &flash_activated);
	vctrlGetRegistryValue("/CONFIG/BROWSER", "flash_play", &flash_play);
	vctrlGetRegistryValue("/CONFIG/MUSIC", "wma_play", &wma_play);

	if (!flash_activated || !flash_play || !wma_play){
		return 0;
	}
	
	return 1;
}

int activate_codecs(vsh_Menu *vsh) {

	if (!codecs_activated()){
		vctrlSetRegistryValue("/CONFIG/BROWSER", "flash_activated", 1);
		vctrlSetRegistryValue("/CONFIG/BROWSER", "flash_play", 1);
		vctrlSetRegistryValue("/CONFIG/MUSIC", "wma_play", 1);
		vsh->status.reset_vsh = 1;
	}
	
	return 0;
}

int swap_buttons(vsh_Menu *vsh) {
	u32 value;
	vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &value);
	value = !value;
	vctrlSetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", value);
	
	vsh->status.reset_vsh = 1;
	return 0;
}
