/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>

typedef struct {
	int pll;
	int mode;
} SceDdrPllMode;

/*
SceDdrPllMode pll_modes[] = {
	{ 19,   8 },
	{ 37,   0 },
	{ 74,   9 },
	{ 95,  10 },
	{ 111, 11 },
	{ 133, 12 },
	{ 148,  1 },
	{ 166, 13 },
	{ 190,  2 },
	{ 222,  3 },
	{ 266,  4 },
	{ 333,  5 },
};
*/

int scePowerRequestColdResetPatched(int a0) {
	sctrlSESetBootConfFileIndex(MODE_UMD);
	return sctrlKernelExitVSH(NULL);
}

__attribute__((noinline)) int scePowerGetBatteryLifeTimePatched() {
	while(*(volatile u32 *)0xBFC0017C != *(volatile u32 *)0xBFC00180);
	short lifetime = *(volatile short *)0xBFC00184;
	return (lifetime < 0) ? 0 : (int)lifetime;
}

int power_online() {
	return (scePowerGetBatteryLifeTimePatched() == 0) ? 1 : 0;
}

float (* sceClkcGetCpuFrequency)();
float (* sceClkcGetBusFrequency)();
/*
333/166: 0x43A68000/0x43268000
300/150: 0x4395E1F1/0x4315E1F1
266/133: 0x4384F078/0x43053333
222/111: 0x435E0000/0x42DE0000
133/66: 0x4304F078/0x42853333
100/50: 0x42C7D7EC/0x4247D7EC
75/37: 0x4295B247/0x4213D5A2
20/10: 0x419FDFF0/0x41940000
*/

/*
333/166: 0x43A68000/0x43268000
300/150: 0x4395E1F1/0x4315E1F1
266/133: 0x4384F078/0x43268000
222/111: 0x435D90C8/0x43268000
133/66: 0x4304F078/0x43268000
100/50: 0x42C768B4/0x42A585C3
75/37: 0x4295E1F1/0x42A585C3
20/10: 0x419C6633/0x43268000
*/
float sceClkcGetCpuFrequencyPatched() {
	float res = sceClkcGetCpuFrequency();

	u32 res_hex;
	memcpy(&res_hex, &res, sizeof(u32));

	// 221.566 -> 222
	if (res_hex == 0x435D90C8) {
		res_hex = 0x435E0000;
		memcpy(&res, &res_hex, sizeof(u32));
	}

	return res;
}

float sceClkcGetBusFrequencyPatched() {
	float res = sceClkcGetBusFrequency();

	u32 res_hex;
	memcpy(&res_hex, &res, sizeof(u32));

	// 166.5 -> 111
	if (res_hex == 0x43268000) {
		res_hex = 0x42DE0000;
		memcpy(&res, &res_hex, sizeof(u32));
	}

	return res;
}

float sceSysregPllGetFrequencyPatched() {
	return 333.0f;
}

void PatchPowerService(u32 text_addr) {
	// Redirect to similar functions
	REDIRECT_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0x9B44CFD9), sctrlHENFindFunction("scePower_Service", "scePower_driver", 0x5C1333B7));
	REDIRECT_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0x9DAF25A0), scePowerRequestColdResetPatched);

	// Patch to fix charging status
	REDIRECT_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0xC79F9157), scePowerGetBatteryLifeTimePatched);
	REDIRECT_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0x872F4ECE), power_online);
	REDIRECT_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0x7A9EA6DE), power_online);
	MAKE_DUMMY_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0x67492C52), 1);

	// Dummy not working functions
	MAKE_DUMMY_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0x40870DAC), 0);
	MAKE_DUMMY_FUNCTION(sctrlHENFindFunction("scePower_Service", "scePower_driver", 0xF7DE0E81), 0);

	// Allow all frequencies for scePowerSetCpuClockFrequency
	_sh(0x1000, text_addr + 0x3182);
	_sh(0x1000, text_addr + 0x319A);

	// Allow all frequencies for scePowerSetClockFrequency
	_sh(0x1000, text_addr + 0x339A);

	// Patch
	sceClkcGetCpuFrequency = (void *)K_EXTRACT_IMPORT(text_addr + 0x4810);
	MAKE_JUMP(text_addr + 0x4810, sceClkcGetCpuFrequencyPatched);

	// sceClkcGetBusFrequency = (void *)K_EXTRACT_IMPORT(text_addr + 0x4830);
	// MAKE_JUMP(text_addr + 0x4830, sceClkcGetBusFrequencyPatched);
/*
	// Change ddr pll modes
	// By doing this, the speed is equivalent to the psp
	_sw(5, text_addr + 0x5488); // 19
	_sw(5, text_addr + 0x5490); // 37
	_sw(5, text_addr + 0x5498); // 74
	_sw(5, text_addr + 0x54A0); // 95
	_sw(5, text_addr + 0x54A8); // 111
	_sw(5, text_addr + 0x54B0); // 133
	_sw(5, text_addr + 0x54B8); // 148
	_sw(5, text_addr + 0x54C0); // 166
	_sw(5, text_addr + 0x54C8); // 190
	_sw(5, text_addr + 0x54D0); // 222
	_sw(5, text_addr + 0x54D8); // 266
	_sw(5, text_addr + 0x54E0); // 333
*/
	SceModule2 *mod = sceKernelFindModuleByName("sceLowIO_Driver");

	MAKE_CALL(mod->text_addr + 0x2B60, sceSysregPllGetFrequencyPatched);
	MAKE_CALL(mod->text_addr + 0x2BC4, sceSysregPllGetFrequencyPatched);
}
