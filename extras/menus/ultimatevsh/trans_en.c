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

#include "common.h"

const char *g_messages_en[] = {
	"Default",
	"Disable",
	"Enable",
	"Normal",
	"M33 driver",
	"Sony NP9660",
	"Inferno",
	"CPU CLOCK XMB  ",
	"CPU CLOCK GAME ",
	"USB DEVICE     ",
	"UMD ISO MODE   ",
	"ISO VIDEO MOUNT",
	"RECOVERY MENU  ->",
	"SHUTDOWN DEVICE",
	"SUSPEND DEVICE",
	"RESET DEVICE",
	"RESET VSH",
	"EXIT",
	"PRO VSH MENU",
	"Flash",
	"UMD Disc",
	"Memory Stick",
	"None",
};

u8 message_test_en[NELEMS(g_messages_en) == MSG_END ? 0 : -1];
