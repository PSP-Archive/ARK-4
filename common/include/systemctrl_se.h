#ifndef __SCTRLLIBRARY_SE_H__
#define __SCTRLLIBRARY_SE_H__

/**
 * These functions are only available in SE-C and later, 
 * and they are not in HEN 
*/

enum 
{
	FAKE_REGION_DISABLED = 0,
	FAKE_REGION_JAPAN = 1,
	FAKE_REGION_AMERICA = 2,
	FAKE_REGION_EUROPE = 3,
	FAKE_REGION_KOREA = 4, 
	FAKE_REGION_UNK = 5,
	FAKE_REGION_UNK2 = 6,
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
	MODE_MARCH33 = 1,
	MODE_NP9660 = 2,
	MODE_INFERNO = 3,
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

typedef struct _SEConfig
{
	int magic;
	s16 umdmode;
	s16 vshcpuspeed;
	s16 vshbusspeed;
	s16 umdisocpuspeed;
	s16 umdisobusspeed;
	s16 fakeregion;
	s16 usbdevice;
	s16 usbcharge;
	s16 machidden;
	s16 skipgameboot;
	s16 hidepic;
	s16 plugvsh; 
	s16 pluggame;
	s16 plugpop;
	s16 flashprot;
	s16 skiplogo;
	s16 useversion;
	s16 useownupdate;
	s16 usenodrm;
	s16 hibblock;
	s16 noanalog;
	s16 oldplugin;
	s16 htmlviewer_custom_save_location;
	s16 hide_cfw_dirs;
	s16 chn_iso;
	s16 msspeed;
	s16 slimcolor;
	s16 iso_cache;
	s16 iso_cache_total_size; // in MB
	s16 iso_cache_num;
	s16 iso_cache_policy;
	s16 usbversion;
	s16 language; /* -1 as autodetect */
	s16 retail_high_memory;
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
 * @returns 0 on success
*/
int sctrlSEGetConfig(SEConfig *config);

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns 0 on success
*/
int sctrlSEGetConfigEx(SEConfig *config, int size);

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
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 1, 1);
 * }
 * else
 * {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 0, config.useisofsonumdinserted);
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

#endif
