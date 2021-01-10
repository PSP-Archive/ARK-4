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

#ifndef VSHCTRL_PATCH_OFFSET_H
#define VSHCTRL_PATCH_OFFSET_H

#include "utils.h"

struct VshBridgePatch {
	u32 sceDisplaySetHoldMode;
	u32 sceDisplaySetHoldModeCall;
	u32 HibBlockCheck;
	u32 sceCtrlReadBufferPositiveNID;
};

struct SysConfPluginPatch {
	u32 SystemVersionStr;
	const char *SystemVersionMessage;
	u32 SystemVersion;
	u32 MacAddressStr;
	u32 SlimColor;
};

struct GamePluginPatch {
	u32 HomebrewCheck;
	u32 PopsCheck;
	u32 MultiDiscPopsCheck;
	u32 HidePicCheck1;
	u32 HidePicCheck2;
	u32 SkipGameBootSubroute;
	u32 SkipGameBoot;
	u32 RifFileCheck;
	u32 RifCompareCheck;
	u32 RifTypeCheck;
	u32 RifNpDRMCheck;
};

struct HtmlViewerPluginPatch {
	u32 htmlviewer_save_location;
	u32 htmlviewer_manual_location;
};

struct HtmlUtilityPatch {
	u32 htmlviewer_manual_location;
};

struct MsVideoMainPluginPatch {
	u32 checks[10];
};

struct VshModulePatch {
	u32 checks[3];
	u32 loadexecNID1;
	u32 loadexecNID2;
	u32 loadexecDisc;
	u32 loadexecDiscUpdater;
	u32 PBPFWCheck[4];
	u32 vshbridge_get_model_call[3];
};

struct UpdatePluginPatch {
	u32 UpdatePluginImageVersion1;
	u32 UpdatePluginImageVersion2;
	u32 UpdatePluginImageVersion3;
};

struct SceUpdateDLLibraryPatch {
	u32 SceUpdateDL_UpdateListStr;
	u32 SceUpdateDL_UpdateListCall1;
	u32 SceUpdateDL_UpdateListCall2;
	u32 SceUpdateDL_UpdateListCall3;
	u32 SceUpdateDL_UpdateListCall4;
};

typedef struct _PatchOffset {
	u32 fw_version;
	struct VshBridgePatch vshbridge_patch;
	struct SysConfPluginPatch sysconf_plugin_patch;
	struct GamePluginPatch game_plugin_patch;
	struct HtmlViewerPluginPatch htmlviewer_plugin_patch;
	struct HtmlUtilityPatch htmlviewer_utility_patch;
	struct MsVideoMainPluginPatch msvideo_main_plugin_patch;
	struct VshModulePatch vsh_module_patch;
	struct UpdatePluginPatch update_plugin_patch;
	struct SceUpdateDLLibraryPatch SceUpdateDL_library_patch;
} PatchOffset;

extern PatchOffset *g_offs;

void setup_patch_offset_table(u32 fw_version);

#endif
