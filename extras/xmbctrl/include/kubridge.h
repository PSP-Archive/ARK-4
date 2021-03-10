#ifndef __KULIBRARY__

#define __KULIBRARY__

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspctrl.h>

/**
 * Functions to let user mode access certain functions only available in
 * kernel mode
*/

/**
  * Load a module using ModuleMgrForKernel.
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 */
SceUID kuKernelLoadModule(const char *path, int flags, SceKernelLMOption *option);


/**
  * Load a module with a specific apitype
  * 
  * @param apìtype - The apitype
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID kuKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option);

/**
 * Gets the api type 
 *
 * @returns the api type in which the system has booted
*/
int kuKernelInitApitype();

/**
 * Gets the filename of the executable to be launched after all modules of the api.
 *
 * @param initfilename - String where copy the initfilename
 * @returns 0 on success
*/
int kuKernelInitFileName(char *initfilename);

/**
 *
 * Gets the device in which the application was launched.
 *
 * @returns the device code, one of PSPBootFrom values.
*/
int kuKernelBootFrom();

/**
 * Get the key configuration in which the system has booted.
 *
 * @returns the key configuration code, one of PSPKeyConfig values 
*/
int kuKernelInitKeyConfig();

/**
 * Get the user level of the current thread
 *
 * @return The user level, < 0 on error
 */
int kuKernelGetUserLevel(void);

/**
 * Set the protection of a block of ddr memory
 *
 * @param addr - Address to set protection on
 * @param size - Size of block
 * @param prot - Protection bitmask
 *
 * @return < 0 on error
 */
int kuKernelSetDdrMemoryProtection(void *addr, int size, int prot);

/**
 * Gets the model of the PSP from user mode.
 * This function is available since 3.60 M33.
 * In previous version, use the kernel function sceKernelGetModel
 *
 * @return one of PspModel values
*/
int kuKernelGetModel(void);

/**
 * Find a module by it's name.
 *
 * @param modname - The name of the module.
 *
 * @returns Pointer to the ::SceModule structure if found, otherwise NULL.
 */
SceModule2 *kuKernelFindModuleByName(const char *modname);

/**
 * Invalidate the entire instruction cache
 */
void kuKernelIcacheInvalidateAll(void);

#endif

