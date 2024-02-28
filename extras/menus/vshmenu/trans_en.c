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

#include <psptypes.h>

#include "macros.h"

const char *g_messages_en[] = {
	"Default",
	"Japan",
	"America",
	"Europe",
	"Korea",
	"United Kingdom",
	"Latin America",
	"Australia",
	"Hong Kong",
	"Taiwan",
	"Russia",
	"China",
	"Debug I",
	"Debug II",
	"Disabled",
	"Enabled",
	"Normal",
	"Toggle",
	"X -> O",
	"O -> X",
	"USB DEVICE",
	"USB READONLY",
	"ISO VIDEO MOUNT",
	"FG COLORS",
	"BG COLORS",
	"FONT",
	"MENU DESIGN",
	"MAIN MENU",
	"CONVERT BATTERY",
	"SWAP X/O BTNS",
	"VSH REGION",
	"UMD REGION",
	"IMPORT CLASSIC PLUGINS",
	"ACTIVATE FLASH AND WMA",
	"DELETE HIBERNATION",
	"BOOT RANDOM ISO",
	"GO BACK",
	"NO HIBERNATION SUPPORT",
	"Normal -> Pandora",
	"Pandora -> Normal",
	"VSH MENU",
	"ADVANCED VSH MENU",
	"RECOVERY MENU ->",
	"SHUTDOWN DEVICE",
	"SUSPEND DEVICE",
	"RESET DEVICE",
	"RESET VSH",
	"EXIT",
	"Unsupported",
	"Adrenaline USB",
	"Flash",
	"UMD Disc",
	"Internal Storage",
	"Memory Stick",
	"None",
	"Classic",
	"Simple",
	"Advanced",
	"New",
	"Random",
	"Red",
	"Light Red",
	"Orange",
	"Light Orange",
	"Yellow",
	"Light Yellow",
	"Green",
	"Light Green",
	"Blue",
	"Light Blue",
	"Indigo",
	"Light Indigo",
	"Violet",
	"Light Violet",
	"Pink",
	"Light Pink",
	"Purple",
	"Light Purple",
	"Teal",
	"Light Teal",
	"Aqua",
	"Light Aqua",
	"Grey",
	"Light Grey",
	"Black",
	"Light Black",
	"White",
	"Light White"
};
u8 message_test_en[NELEMS(g_messages_en) == MSG_END ? 0 : -1];
