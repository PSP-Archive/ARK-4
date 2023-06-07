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
#include "common.h"

extern SEConfig cnf;
extern t_conf config;

const int cpu_list[]={0, 20, 75, 100, 133, 166, 222, 266, 300, 333};
const int bus_list[]={0, 10, 37, 50, 66, 83, 111, 133, 150, 166};

/** 0 - None, 1~count - umdvideo entry */
int umdvideo_idx = 0;

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
	int sel = cnf.usbdevice_rdonly;
	sel = limit(sel+dir, 0, 1);
	cnf.usbdevice_rdonly=sel;
}

void change_bg_color(int dir) {
	int sel = config.vsh_bg_color;
	sel = limit(sel+dir, 0, 28);
	config.vsh_bg_color=sel;
}

void change_fg_color(int dir) {
	int sel = config.vsh_fg_color;
	sel = limit(sel+dir, 0, 28);
	config.vsh_fg_color=sel;
}

void change_usb(int dir)
{
	int sel = cnf.usbdevice;
	int top = (psp_model==PSP_GO)?4:5;

	// select new
	sel = limit(sel+dir, 0, top);
	
	cnf.usbdevice=sel;
}

void change_umd_mode(int dir)
{
	int sel = cnf.umdmode;

	// select new
	sel = limit(sel+dir, 2, 3);
	cnf.umdmode=sel;
}

void change_umd_mount_idx(int dir)
{
	umdvideo_idx = limit(umdvideo_idx+dir, 0, umdvideolist_count(&g_umdlist));
}

void change_umd_region(int dir, int max)
{
	int sel = cnf.umdregion;
			
	// select new
	sel = limit(sel+dir, 0, max);
	cnf.umdregion=sel;
}

void change_region(int dir, int max)
{
	int sel = cnf.vshregion;

	// select new
	sel = limit(sel+dir, 0, max);
	cnf.vshregion=sel;
}

void change_bool_option(int *p, int direction)
{
	int sel = *p;

	sel = limit(sel+direction, 0, 1);
	*p=sel;
}
