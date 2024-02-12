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

/*
	PSP VSH
*/

#include "clock.h"
#include "common.h"

#include "vsh.h"
#include "macros.h"


#define ROLL_OVER(val, min, max) ( ((val) < (min)) ? (max): ((val) > (max)) ? (min) : (val) )


const int cpu_list[] = {0, 20, 75, 100, 133, 166, 222, 266, 300, 333};
const int bus_list[] = {0, 10, 37, 50, 66, 83, 111, 133, 150, 166};


int cpu2no(int cpu)
{
	int i;

	for(i=0; i<NELEMS(cpu_list); i++) {
		if(cpu==cpu_list[i])
			return i;
	}

	return 0;
}

int bus2no(int cpu)
{
	int i;

	for(i=0; i<NELEMS(bus_list); i++) {
		if(cpu==bus_list[i])
			return i;
	}

	return 0;
}

void swap_readonly(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.se.usbdevice_rdonly;
	sel = ROLL_OVER(sel+dir, 0, 1);
	vsh->config.se.usbdevice_rdonly=sel;
}

void change_bg_color(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.ark_menu.vsh_bg_color;
	sel = ROLL_OVER(sel+dir, 0, 28);
	vsh->config.ark_menu.vsh_bg_color=sel;
}

void change_fg_color(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.ark_menu.vsh_fg_color;
	sel = ROLL_OVER(sel+dir, 0, 28);
	vsh->config.ark_menu.vsh_fg_color=sel;
}

void change_font(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.ark_menu.vsh_font;
	sel = ROLL_OVER(sel+dir, 0, 55);
	vsh->config.ark_menu.vsh_font=sel;
}

void change_design(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.ark_menu.window_mode;
	sel = ROLL_OVER(sel+dir, 0, 1);
	vsh->config.ark_menu.window_mode = sel;
}
void change_menu(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.ark_menu.advanced_vsh;
	sel = ROLL_OVER(sel+dir, 0, 1);
	vsh->config.ark_menu.advanced_vsh = sel;
}

void change_usb(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.se.usbdevice;
	int top = (vsh->psp_model == PSP_GO) ? 4 : 5;

	// select new
	sel = ROLL_OVER(sel + dir, 0, top);
	
	vsh->config.se.usbdevice = sel;
	
	// Enable Read Only by default for Flash files and UMD Disc (on Non GO model)
	if(sel>0)
		vsh->config.se.usbdevice_rdonly = 1;
	if(sel==0)
		vsh->config.se.usbdevice_rdonly = 0;
}

void change_umd_mode(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.se.umdmode;

	// select new
	sel = ROLL_OVER(sel+dir, 2, 3);
	vsh->config.se.umdmode=sel;
}

void change_umd_mount_idx(int dir) {
	vsh_Menu *vsh = vsh_menu_pointer();
	vsh->status.umdvideo_idx = ROLL_OVER(vsh->status.umdvideo_idx + dir, 0, umdvideolist_count(&vsh->umdlist));
}

void change_umd_region(int dir, int max) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.se.umdregion;
			
	// select new
	sel = ROLL_OVER(sel+dir, 0, max);
	vsh->config.se.umdregion=sel;
}

void change_region(int dir, int max) {
	vsh_Menu *vsh = vsh_menu_pointer();
	
	int sel = vsh->config.se.vshregion;

	// select new
	sel = ROLL_OVER(sel+dir, 0, max);
	vsh->config.se.vshregion=sel;
}

void change_bool_option(int *p, int direction) {
	int sel = *p;

	sel = ROLL_OVER(sel+direction, 0, 1);
	*p=sel;
}
