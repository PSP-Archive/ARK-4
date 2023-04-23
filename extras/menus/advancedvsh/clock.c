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

int swap_readonly(int dir) {
	int sel = cnf.usbdevice_rdonly;
	sel = limit(sel+dir, 0, 1);
	cnf.usbdevice_rdonly=sel;
}

void change_bg_colors(int dir) {
	int sel = cnf.vsh_bg_colors;
	sel = limit(sel+dir, 0, 27);
	cnf.vsh_bg_colors=sel;
}

void change_fg_colors(int dir) {
	int sel = cnf.vsh_fg_colors;
	sel = limit(sel+dir, 0, 27);
	cnf.vsh_fg_colors=sel;
}

void change_clock(int dir, int flag)
{
	int sel;
	s16 *cpu[2];

	if(flag) {
		cpu[0]=&(cnf.umdisocpuspeed);
		cpu[1]=&(cnf.umdisobusspeed);
	} else {
		cpu[0]=&(cnf.vshcpuspeed);
		cpu[1]=&(cnf.vshbusspeed);
	}

	sel = cpu2no(*cpu[0]);

	// select new
	sel = limit(sel+dir, 0, NELEMS(cpu_list)-1);

	*cpu[0] = cpu_list[sel];
	*cpu[1] = bus_list[sel];
}

void change_usb(int dir)
{
	int sel = cnf.usbdevice;

	// select new
	sel = limit(sel+dir, 0, 5);
	
	cnf.usbdevice=sel;
}

void change_umd_mode(int dir)
{
	int sel = cnf.umdmode;

	// select new
	sel = limit(sel+dir, 1, 2);
	cnf.umdmode=sel;
}

void change_umd_mount_idx(int dir)
{
	umdvideo_idx = limit(umdvideo_idx+dir, 0, umdvideolist_count(&g_umdlist));
}

void change_region(int dir, int max)
{
	int sel = cnf.fakeregion;

	// select new
	sel = limit(sel+dir, 0, max);
	cnf.fakeregion=sel;
}

void change_plugins(int dir, int flag)
{
	int sel;
	s16 *plugins;

	if(flag == 0) {
		plugins=&(cnf.plugvsh);
	} else if(flag == 1) {
		plugins=&(cnf.pluggame);
	} else {
		plugins=&(cnf.plugpop);
	}

	sel = *plugins;
	sel = !sel;
	*plugins = sel;
}

void change_bool_option(int *p, int direction)
{
	int sel = *p;

	sel = limit(sel+direction, 0, 1);
	*p=sel;
}
