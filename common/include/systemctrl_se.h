#ifndef __SCTRLLIBRARY_SE_H__
#define __SCTRLLIBRARY_SE_H__

#ifdef __cplusplus
extern "C"{
#endif

/**
 * These functions are only available in SE-C and later, 
 * and they are not in HEN 
*/

enum fakeregion
{
    FAKE_REGION_DISABLED = 0,
    FAKE_REGION_JAPAN = 1,
    FAKE_REGION_AMERICA = 2,
    FAKE_REGION_EUROPE = 3,
    FAKE_REGION_KOREA = 4, 
    FAKE_REGION_UNITED_KINGDOM = 5,
    FAKE_REGION_LATIN_AMERICA = 6,
    FAKE_REGION_AUSTRALIA = 7,
    FAKE_REGION_HONGKONG = 8,
    FAKE_REGION_TAIWAN = 9,
    FAKE_REGION_RUSSIA = 10,
    FAKE_REGION_CHINA = 11,
    FAKE_REGION_DEBUG_TYPE_I = 12,
    FAKE_REGION_DEBUG_TYPE_II = 13,
};

// No MODE_OE_LEGACY any more
enum SEUmdModes
{
    MODE_UMD = 0,
    MODE_MARCH33 = 1, // not available anymore, will default to inferno
    MODE_NP9660 = 2, // (Galaxy) not available anymore, will default to inferno
    MODE_INFERNO = 3,
    MODE_VSHUMD = 4,
    MODE_UPDATERUMD = 5,
};

enum swap_xo
{
	XO_CURRENT_O_PRIMARY = 0,
	XO_CURRENT_X_PRIMARY = 1
};

enum vsh_bg_colors
{
	BG_RANDOM		= 0,
	BG_RED 			= 1,
	BG_LITE_RED 	= 2,
	BG_ORANGE 		= 3,
	BG_LITE_ORANGE 	= 4,
	BG_YELLOW 		= 5,
	BG_LITE_YELLOW 	= 6,
	BG_GREEN 		= 7,
	BG_LITE_GREEN 	= 8,
	BG_BLUE 		= 9,
	BG_LITE_BLUE 	= 10,
	BG_INDIGO 		= 11,
	BG_LITE_INDIGO 	= 12,
	BG_VIOLET 		= 13,
	BG_LITE_VIOLET 	= 14,
	BG_PINK 		= 15,
	BG_LITE_PINK 	= 16,
	BG_PURPLE 		= 17,
	BG_LITE_PURPLE 	= 18,
	BG_TEAL 		= 19,
	BG_LITE_TEAL 	= 20,
	BG_AQUA 		= 21,
	BG_LITE_AQUA 	= 22,
	BG_GREY 		= 23,
	BG_LITE_GREY 	= 24,
	BG_BLACK 		= 25,
	BG_LITE_BLACK 	= 26,
	BG_WHITE		= 27,
	BG_LITE_WHITE	= 28,

};

enum vsh_fg_colors
{
	FG_RANDOM		= 0,
	FG_WHITE 		= 1,
	FG_ORANGE 		= 2,
	FG_LITE_ORANGE 	= 3,
	FG_YELLOW 		= 4,
	FG_LITE_YELLOW 	= 5,
	FG_GREEN 		= 6,
	FG_LITE_GREEN 	= 7,
	FG_BLUE 		= 8,
	FG_LITE_BLUE 	= 9,
	FG_INDIGO 		= 10,
	FG_LITE_INDIGO 	= 11,
	FG_VIOLET 		= 12,
	FG_LITE_VIOLET 	= 13,
	FG_PINK 		= 14,
	FG_LITE_PINK 	= 15,
	FG_PURPLE 		= 16,
	FG_LITE_PURPLE 	= 17,
	FG_TEAL 		= 18,
	FG_LITE_TEAL 	= 19,
	FG_AQUA 		= 20,
	FG_LITE_AQUA 	= 21,
	FG_GREY 		= 22,
	FG_LITE_GREY 	= 23,
	FG_BLACK 		= 24,
	FG_LITE_BLACK 	= 25,
	FG_LITE_RED 	= 26,
	FG_RED 			= 27,
	FG_LITE_WHITE 	= 28,
};

enum convert_battery
{
	NORMAL_TO_PANDORA	= 0,
	PANDORA_TO_NORMAL	= 1,
	UNSUPPORTED			= 2,
};

enum MsSpeedFlag
{
    MSSPEED_NONE     = 0,
    MSSPEED_POP      = 1,
    MSSPEED_GAME     = 2,
    MSSPEED_VSH      = 3,
    MSSPEED_POP_GAME = 4,
    MSSPEED_GAME_VSH = 5,
    MSSPEED_VSH_POP  = 6,
    MSSPEED_ALWAYS   = 7,
};

enum InfernoCachePolicy
{
    CACHE_POLICY_LRU = 0,
    CACHE_POLICY_RR = 1,
};

enum umdregion
{
    // UMD regions
    UMD_REGION_DEFAULT  = 0,
    UMD_REGION_JAPAN    = 1,
    UMD_REGION_AMERICA  = 2,
    UMD_REGION_EUROPE   = 3,
};

typedef struct _SEConfig
{
    u32 magic;
    u8 language;
    u8 umdmode;
    u8 clock;
    u8 vshregion;
    u8 umdregion;
    s8 usbdevice;
    u8 usbcharge;
    u8 usbdevice_rdonly;
    u8 hidemac;
    u8 hidedlc;
    u8 skiplogos;
    u8 hidepics;
    u8 useownupdate;
    u8 usenodrm;
    u8 hibblock;
    u8 noanalog;
    u8 oldplugin;
    u8 hide_cfw_dirs;
    u8 chn_iso;
    u8 msspeed;
    u8 slimcolor;
    u8 iso_cache;
    u8 force_high_memory;
    u8 launcher_mode;
    u8 disable_pause;
    u8 noled;
} SEConfig;

/**
 * Gets the SE/OE version
 *
 * @returns the SE version
 *
 * 3.03 OE-A: 0x00000500
*/
int sctrlSEGetVersion();

/**
 * Gets the SE configuration.
 * Avoid using this function, it may corrupt your program.
 * Use sctrlSEGetCongiEx function instead.
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @returns pointer to original SEConfig structure in SystemControl
*/
SEConfig* sctrlSEGetConfig(SEConfig *config);

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns pointer to original SEConfig structure in SystemControl
*/
SEConfig* sctrlSEGetConfigEx(SEConfig *config, int size);

/**
 * Sets the SE configuration
 * This function can corrupt the configuration in flash, use
 * sctrlSESetConfigEx instead.
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @returns 0 on success
*/
int sctrlSESetConfig(SEConfig *config);

/**
 * Sets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @param size - the size of the structure
 * @returns 0 on success
*/
int sctrlSESetConfigEx(SEConfig *config, int size);

/**
 * Initiates the emulation of a disc from an ISO9660/CSO file.
 *
 * @param file - The path of the 
 * @param noumd - Wether use noumd or not
 * @param isofs - Wether use the custom SE isofs driver or not
 * 
 * @returns 0 on success
 *
 * @Note - When setting noumd to 1, isofs should also be set to 1,
 * otherwise the umd would be still required.
 *
 * @Note 2 - The function doesn't check if the file is valid or even if it exists
 * and it may return success on those cases
 *
 * @Note 3 - This function is not available in SE for devhook
 * @Example:
 *
 * SEConfig config;
 *
 * sctrlSEGetConfig(&config);
 *
 * if (config.usenoumd)
 * {
 *        sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 1, 1);
 * }
 * else
 * {
 *        sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 0, config.useisofsonumdinserted);
 * }
*/
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs);

/**
 * Umounts an iso.
 *
 * @returns 0 on success
*/
int sctrlSEUmountUmd(void);

/**
 * Forces the umd disc out state
 *
 * @param out - non-zero for disc out, 0 otherwise
 *
*/
void sctrlSESetDiscOut(int out);

/**
 * Sets the disctype.
 *
 * @param type - the disctype (0x10=game, 0x20=video, 0x40=audio)
 * @note: Currently only inferno available, needs reset to take effect
*/
void sctrlSESetDiscType(int type);

/**
 * Get the disctype.
*/
int sctrlSEGetDiscType(void);

/**
 * Sets the current umd file (kernel only)
*/
char *sctrlSEGetUmdFile(void);

/**
 * Gets the current umd file (kernel only)
*/
void sctrlSESetUmdFile(char *file);

/**
 * Sets the current umd file (kernel only)
 *
 * @param file - The umd file
*/
void sctrlSESetUmdFile(char *file);

void SetUmdFile(char *file);

/** 
 * Sets the boot config file for next reboot
 *
 * @param index - The index identifying the file (0 -> normal bootconf, 1 -> march33 driver bootconf, 2 -> np9660 bootcnf, 3 -> inferno bootconf), 4 -> inferno vsh mount
*/
void sctrlSESetBootConfFileIndex(int index);

/**
 * Get the boot config index
 */
unsigned int sctrlSEGetBootConfFileIndex(void);

#ifdef __cplusplus
}
#endif

#endif
