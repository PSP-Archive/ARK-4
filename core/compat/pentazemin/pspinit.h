#ifndef __PSPINIT_H__

#define __PSPINIT_H__

enum PSPBootFrom
{
	PSP_BOOT_FLASH = 0, /* ? */
	PSP_BOOT_DISC = 0x20,
	PSP_BOOT_USBWLAN = 0x30,
	PSP_BOOT_MS = 0x40,
	PSP_BOOT_EF = 0x50,
	PSP_BOOT_FLASH3 = 0x80,
};

enum PSPInitApitype
{
	PSP_INIT_APITYPE_DISC = 0x120,
	PSP_INIT_APITYPE_DISC_UPDATER = 0x121,
	PSP_INIT_APITYPE_UMDEMU_MS1 = 0x123, /* np9660 game , x-reader */
	PSP_INIT_APITYPE_UMDEMU_MS2 = 0x124,
	PSP_INIT_APITYPE_UMDEMU_EF1 = 0x125,
	PSP_INIT_APITYPE_UMDEMU_EF2 = 0x126,
	PSP_INIT_APITYPE_USBWLAN = 0x130, /* Game Shareing */
	PSP_INIT_APITYPE_MS1 = 0x140,
	PSP_INIT_APITYPE_MS2 = 0x141,
	PSP_INIT_APITYPE_MS3 = 0x142,
	PSP_INIT_APITYPE_MS4 = 0x143, /* comic reader */
	PSP_INIT_APITYPE_MS5 = 0x144, /* pops */
	PSP_INIT_APITYPE_MS6 = 0x145,
	PSP_INIT_APITYPE_EF1 = 0x151,
	PSP_INIT_APITYPE_EF2 = 0x152,
	PSP_INIT_APITYPE_EF3 = 0x153,
	PSP_INIT_APITYPE_EF4 = 0x154,
	PSP_INIT_APITYPE_EF5 = 0x155,
	PSP_INIT_APITYPE_EF6 = 0x156,
	PSP_INIT_APITYPE_MLNAPP_MS = 0x170,
	PSP_INIT_APITYPE_MLNAPP_EF = 0x171,
	PSP_INIT_APITYPE_VSH1 = 0x210, /* ExitGame */
	PSP_INIT_APITYPE_VSH2 = 0x220, /* ExitVSH */
};

enum PSPKeyConfig
{
	PSP_INIT_KEYCONFIG_VSH		= 0x100,
	PSP_INIT_KEYCONFIG_UPDATER	= 0x110,
	PSP_INIT_KEYCONFIG_GAME		= 0x200,
	PSP_INIT_KEYCONFIG_POPS		= 0x300,
	PSP_INIT_KEYCONFIG_APP		= 0x400,
};

/**
 * Gets the api type 
 *
 * @returns the api type in which the system has booted
*/
int sceKernelInitApitype();

/**
 * Gets the filename of the executable to be launched after all modules of the api.
 *
 * @returns filename of executable or NULL if no executable found.
*/
char *sceKernelInitFileName();

/**
 *
 * Gets the device in which the application was launched.
 *
 * @returns the device code, one of PSPBootFrom values.
*/
int sceKernelBootFrom();

/**
 * Get the key configuration in which the system has booted.
 *
 * @returns the key configuration code, one of PSPKeyConfig values 
*/
int InitForKernel_7233B5BC();

#define sceKernelInitKeyConfig InitForKernel_7233B5BC

#endif

