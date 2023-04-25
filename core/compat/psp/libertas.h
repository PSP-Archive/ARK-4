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

#ifndef LIBERTAS_H_
#define LIBERTAS_H_

#include <pspsdk.h>
#include "module2.h"

#define LIBERTAS_CMD_GET_HW_SPEC 0x0003
#define LIBERTAS_ACTION_GET 0
#define LIBERTAS_ACTION_SET 1

typedef struct LIBERTAS_GET_HW_SPEC_COMMAND
{
	u16 HwIfVersion;
	u16 HwVersion;
	u16 NumOfWCB;
	u16 NumOfMCastAddr;
	u8  MacAddr[6];
	u16 RegionCode;
	u16 NumberOfAntenna;
	u32 FWReleaseNumber;
	u32 WcbBase;
	u32 RxPdRdPtr;
	u32 RxPdWrPtr;
	u32 FwCapInfo;
} __attribute__((packed)) LIBERTAS_GET_HW_SPEC_COMMAND;

typedef struct LIBERTAS_MAC_CONTROL_COMMAND
{
	u16 Action; // Get or Set
	u16 Reserved;
} __attribute__((packed)) LIBERTAS_MAC_CONTROL_COMMAND;

typedef struct LIBERTAS_COMMAND
{
	u16 CmdCode;
	u16 Size;
	u16 SeqNum;
	u16 Result;

	union
	{
		LIBERTAS_GET_HW_SPEC_COMMAND	hwspec;
		LIBERTAS_MAC_CONTROL_COMMAND	maccontrol;
	} data;
} __attribute__((packed)) LIBERTAS_COMMAND;

// Read Fake MAC Config
int read_MAC_config(char * path);

// Hook MAC Getter
void patch_Libertas_MAC(SceModule2 * mod);

#endif

