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
#include "vshctrl_patch_offset.h"

#if !defined(CONFIG_635) && !defined(CONFIG_620) && !defined(CONFIG_639) && !defined(CONFIG_660) && !defined(CONFIG_661)
#error You have to define CONFIG_620 or CONFIG_635 or CONFIG_639 or CONFIG_660 or CONFIG_661
#endif

#if defined(CONFIG_660) || defined(CONFIG_661)
#ifdef CONFIG_660
PatchOffset g_660_offsets = {
	.fw_version = FW_660,
#endif
#ifdef CONFIG_661
PatchOffset g_661_offsets = {
	.fw_version = FW_661,
#endif
	.vshbridge_patch = {
		.sceDisplaySetHoldMode = 0x00005630,
		.sceDisplaySetHoldModeCall = 0x00001A34,
		.HibBlockCheck = 0x000051C8,
		.sceCtrlReadBufferPositiveNID = 0xBE30CED0,
	},
	.sysconf_plugin_patch = {
		.SystemVersionStr = 0x0002A62C,
#ifdef CONFIG_660
		.SystemVersionMessage = "6.60 PRO-%c",
#endif
#ifdef CONFIG_661
		.SystemVersionMessage = "6.61 PRO-%c",
#endif
		.SystemVersion = 0x000192E0,
		.MacAddressStr = 0x0002E9A0,
		.SlimColor = 0x000076EC,
	},
	.game_plugin_patch = {
		.HomebrewCheck = 0x00020528,
		.PopsCheck = 0x00020E6C,
		.MultiDiscPopsCheck = 0x00014850,
		.HidePicCheck1 = 0x0001D858,
		.HidePicCheck2 = 0x0001D864,
		.SkipGameBootSubroute = 0x000194B0,
		.SkipGameBoot = 0x00019130,
		.RifFileCheck = 0x0002062C,
		.RifCompareCheck = 0x00020654,
		.RifTypeCheck = 0x00020668,
		.RifNpDRMCheck = 0x000206D0,
	},
	.htmlviewer_plugin_patch = {
		.htmlviewer_save_location = 0x0001C7FC,
		.htmlviewer_manual_location = 0x0001C2B0,
	},
	.htmlviewer_utility_patch = {
		.htmlviewer_manual_location = 0x0000CDBC,
	},
	.msvideo_main_plugin_patch = {
		.checks = {
			0x0003AF24,
			0x0003AFAC,
			0x0003D7EC,
			0x0003DA48,
			0x000441A0,
			0x000745A0,
			0x00088BF0,
			0x0003D764,
			0x0003D7AC,
			0x00043248,
		},
	},
	.vsh_module_patch = {
		.checks = {
			0x000122B0,
			0x00012058,
			0x00012060,
		},
		.loadexecNID1 = 0x21D4D038,
		.loadexecNID2 = 0xE533E98C,
		.loadexecDisc = 0x63E69956,
		.loadexecDiscUpdater = 0x81682A40,
		.PBPFWCheck = {
			0x000119C0,
			0x000121A4,
			0x00012BA4,
			0x00013288,
		},
		.vshbridge_get_model_call = {
			0x0000670C,
			0x0002068C,
			0x0002D240,
		},
	},
	.update_plugin_patch = {
		.UpdatePluginImageVersion1 = 0x000082A4,
		.UpdatePluginImageVersion2 = 0x000082AC,
		.UpdatePluginImageVersion3 = 0x000082A0,
	},
	.SceUpdateDL_library_patch = {
		.SceUpdateDL_UpdateListStr = 0x000032BC,
		.SceUpdateDL_UpdateListCall1 = 0x00002044,
		.SceUpdateDL_UpdateListCall2 = 0x00002054,
		.SceUpdateDL_UpdateListCall3 = 0x00002080,
		.SceUpdateDL_UpdateListCall4 = 0x0000209C,
	},
};
#endif

#ifdef CONFIG_639
PatchOffset g_639_offsets = {
	.fw_version = FW_639,
	.vshbridge_patch = {
		.sceDisplaySetHoldMode = 0x00005618,
		.sceDisplaySetHoldModeCall = 0x00001A14,
		.HibBlockCheck = 0x000051A8,
		.sceCtrlReadBufferPositiveNID = 0x9F3038AC,
	},
	.sysconf_plugin_patch = {
		.SystemVersionStr = 0x0002A1FC,
		.SystemVersionMessage = "6.39 PRO-%c",
		.SystemVersion = 0x00018F3C,
		.MacAddressStr = 0x0002E4D8,
		.SlimColor = 0x00007660,
	},
	.game_plugin_patch = {
		.HomebrewCheck = 0x000202A8,
		.PopsCheck = 0x00020BC8,
		.MultiDiscPopsCheck = 0x00014634,
		.HidePicCheck1 = 0x0001D5DC,
		.HidePicCheck2 = 0x0001D5E8,
		.SkipGameBootSubroute = 0x00019294,
		.SkipGameBoot = 0x00018F14,
		.RifFileCheck = 0x000203AC,
		.RifCompareCheck = 0x000203D4,
		.RifTypeCheck = 0x000203E8,
		.RifNpDRMCheck = 0x0002042C,
	},
	.htmlviewer_plugin_patch = {
		.htmlviewer_save_location = 0x0001C7FC,
		.htmlviewer_manual_location = 0x0001C2B0,
	},
	.htmlviewer_utility_patch = {
		.htmlviewer_manual_location = 0x0000CD3C,
	},
	.msvideo_main_plugin_patch = {
		.checks = {
			0x0003AED4,
			0x0003AF5C,		
			0x0003D79C,
			0x0003D9F8,
			0x00044150,
			0x00074550,
			0x00088BA0,
			0x0003D714,
			0x0003D75C,
			0x000431F8,
		},
	},
	.vsh_module_patch = {
		.checks = {
			0x00012230,
			0x00011FD8,
			0x00011FE0,
		},
		.loadexecNID1 = 0x59BBA567,
		.loadexecNID2 = 0xD4BA5699,
		.loadexecDisc = 0xA74228D4,
		.loadexecDiscUpdater = 0x5CA71F45,
		.PBPFWCheck = {
			0x00011940,
			0x00012124,
			0x00012B08,
			0x000131EC,
		},
		.vshbridge_get_model_call = {
			0x0000670C,
			0x00020674,
			0x0002D1B4,
		},
	},
	.update_plugin_patch = {
		.UpdatePluginImageVersion1 = 0x000081B4,
		.UpdatePluginImageVersion2 = 0x000081BC,
		.UpdatePluginImageVersion3 = 0x000081B0,
	},
	.SceUpdateDL_library_patch = {
		.SceUpdateDL_UpdateListStr = 0x000032BC,
		.SceUpdateDL_UpdateListCall1 = 0x00002044,
		.SceUpdateDL_UpdateListCall2 = 0x00002054,
		.SceUpdateDL_UpdateListCall3 = 0x00002080,
		.SceUpdateDL_UpdateListCall4 = 0x0000209C,
	},
};
#endif

#ifdef CONFIG_635
PatchOffset g_635_offsets = {
	.fw_version = FW_635,
	.vshbridge_patch = {
		.sceDisplaySetHoldMode = 0x00005618,
		.sceDisplaySetHoldModeCall = 0x00001A14,
		.HibBlockCheck = 0x000051A8,
		.sceCtrlReadBufferPositiveNID = 0x9F3038AC,
	},
	.sysconf_plugin_patch = {
		.SystemVersionStr = 0x0002A1FC,
		.SystemVersionMessage = "6.35 PRO-%c",
		.SystemVersion = 0x00018F3C,
		.MacAddressStr = 0x0002E4D8,
		.SlimColor = 0x00007660,
	},
	.game_plugin_patch = {
		.HomebrewCheck = 0x000202A8,
		.PopsCheck = 0x00020BC8,
		.MultiDiscPopsCheck = 0x00014634,
		.HidePicCheck1 = 0x0001D5DC,
		.HidePicCheck2 = 0x0001D5E8,
		.SkipGameBootSubroute = 0x00019294,
		.SkipGameBoot = 0x00018F14,
		.RifFileCheck = 0x000203AC,
		.RifCompareCheck = 0x000203D4,
		.RifTypeCheck = 0x000203E8,
		.RifNpDRMCheck = 0x0002042C,
	},
	.htmlviewer_plugin_patch = {
		.htmlviewer_save_location = 0x0001C7FC,
		.htmlviewer_manual_location = 0x0001C2B0,
	},
	.htmlviewer_utility_patch = {
		.htmlviewer_manual_location = 0x0000CD3C,
	},
	.msvideo_main_plugin_patch = {
		.checks = {
			0x0003AED4,
			0x0003AF5C,		
			0x0003D79C,
			0x0003D9F8,
			0x00044150,
			0x00074550,
			0x00088BA0,
			0x0003D714,
			0x0003D75C,
			0x000431F8,
		},
	},
	.vsh_module_patch = {
		.checks = {
			0x00012230,
			0x00011FD8,
			0x00011FE0,
		},
		.loadexecNID1 = 0x59BBA567,
		.loadexecNID2 = 0xD4BA5699,
		.loadexecDisc = 0xA74228D4,
		.loadexecDiscUpdater = 0x5CA71F45,
		.PBPFWCheck = {
			0x00011940,
			0x00012124,
			0x00012B08,
			0x000131EC,
		},
		.vshbridge_get_model_call = {
			0x0000670C,
			0x00020674,
			0x0002D1B4,
		},
	},
	.update_plugin_patch = {
		.UpdatePluginImageVersion1 = 0x000081B4,
		.UpdatePluginImageVersion2 = 0x000081BC,
		.UpdatePluginImageVersion3 = 0x000081B0,
	},
	.SceUpdateDL_library_patch = {
		.SceUpdateDL_UpdateListStr = 0x000032BC,
		.SceUpdateDL_UpdateListCall1 = 0x00002044,
		.SceUpdateDL_UpdateListCall2 = 0x00002054,
		.SceUpdateDL_UpdateListCall3 = 0x00002080,
		.SceUpdateDL_UpdateListCall4 = 0x0000209C,
	},
};
#endif

#ifdef CONFIG_620
PatchOffset g_620_offsets = {
	.fw_version = FW_620,
	.vshbridge_patch = {
		.sceDisplaySetHoldMode = 0x00005570,
		.sceDisplaySetHoldModeCall = 0x00001A14,
		.HibBlockCheck = 0x000050F8,
		.sceCtrlReadBufferPositiveNID = 0xD073ECA4,
	},
	.sysconf_plugin_patch = {
		.SystemVersionStr = 0x000298AC,
		.SystemVersionMessage = "6.20 PRO-%c",
		.SystemVersion = 0x00018920,
		.MacAddressStr = 0x0002DB90,
		.SlimColor = 0x00007494,
	},
	.game_plugin_patch = {
		.HomebrewCheck = 0x0001EB08,
		.PopsCheck = 0x0001F41C,
		.MultiDiscPopsCheck = 0x00013850,
		.HidePicCheck1 = 0x0001C098,
		.HidePicCheck2 = 0x0001C0A4,
		.SkipGameBootSubroute = 0x000181BC,
		.SkipGameBoot = 0x00017E5C,
		.RifFileCheck = 0x0001EC0C,
		.RifCompareCheck = 0x0001EC34,
		.RifTypeCheck = 0x0001EC48,
		.RifNpDRMCheck = 0x0001EC8C,
	},
	.htmlviewer_plugin_patch = {
		.htmlviewer_save_location = 0x0001C7C0,
		.htmlviewer_manual_location = 0x0001C274,
	},
	.htmlviewer_utility_patch = {
		.htmlviewer_manual_location = 0x0000CCCC,
	},
	.msvideo_main_plugin_patch = {
		.checks = {
			0x0003AB2C,
			0x0003ABB4,
			0x0003D3AC,
			0x0003D608,
			0x00043B98,
            0x00073A84,
            0x000880A0,
			0x0003D324,
            0x0003D36C,
            0x00042C40,
		},
	},
	.vsh_module_patch = {
		.checks = {
			0x00011D84,
			0x00011A70,
			0x00011A78,
		},
		.loadexecNID1 = 0x4ECCCDBC,
		.loadexecNID2 = 0x2D5C9178,
		.loadexecDisc = 0x2B24AEAC,
		.loadexecDiscUpdater = 0x8959D61E,
		.PBPFWCheck = {
			0x0001136C,
			0x00011BA0,
			0x0001256C,
			0x00012D8C,
		},
		.vshbridge_get_model_call = {
			0x000065A0,
			0x0001FEC0,
			0x0002C944,
		},
	},
	.update_plugin_patch = {
		.UpdatePluginImageVersion1 = 0x0000819C,
		.UpdatePluginImageVersion2 = 0x000081A4,
		.UpdatePluginImageVersion3 = 0x00008198,
	},
	.SceUpdateDL_library_patch = {
		.SceUpdateDL_UpdateListStr = 0x000032BC,
		.SceUpdateDL_UpdateListCall1 = 0x00002044,
		.SceUpdateDL_UpdateListCall2 = 0x00002054,
		.SceUpdateDL_UpdateListCall3 = 0x00002080,
		.SceUpdateDL_UpdateListCall4 = 0x0000209C,
	},
};
#endif

PatchOffset *g_offs = NULL;

void setup_patch_offset_table(u32 fw_version)
{
#ifdef CONFIG_661
	if(fw_version == g_661_offsets.fw_version) {
		g_offs = &g_661_offsets;
	}
#endif

#ifdef CONFIG_660
	if(fw_version == g_660_offsets.fw_version) {
		g_offs = &g_660_offsets;
	}
#endif

#ifdef CONFIG_639
	if(fw_version == g_639_offsets.fw_version) {
		g_offs = &g_639_offsets;
	}
#endif

#ifdef CONFIG_635
	if(fw_version == g_635_offsets.fw_version) {
		g_offs = &g_635_offsets;
	}
#endif

#ifdef CONFIG_620
   	if(fw_version == g_620_offsets.fw_version) {
		g_offs = &g_620_offsets;
	}
#endif
}
