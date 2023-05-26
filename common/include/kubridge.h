#ifndef __KULIBRARY__

#define __KULIBRARY__

#ifdef __cplusplus
extern "C"{
#endif

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspctrl.h>
#include <module2.h>

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
 * Find module by name
 *
 * @param modname - Name of Module
 * @param mod - module structure for output (actually treated as SceModule2)
 *
 * @return < 0 on error
 */
int kuKernelFindModuleByName(char *modname, SceModule2 *mod);

/**
 * Invalidate the entire instruction cache
 */
void kuKernelIcacheInvalidateAll(void);

/**
 * Read 4 bytes from memory (with kernel memory access)
 *
 * @param addr - Address to read, must have 4 bytes alignment
 */
u32 kuKernelPeekw(void *addr);

/**
 * Write 4 bytes to memory (with kernel memory access)
 *
 * @param addr - Address to write, must have 4 bytes alignment
 */
void kuKernelPokew(void *addr, u32 value);

/**
 * memcpy (with kernel memory access)
 *
 * @param dest - Destination address
 * @param src - Source address
 * @param num - copy bytes count
 *
 * @return Destination address
 */
void *kuKernelMemcpy(void *dest, const void *src, size_t num);

struct KernelCallArg {
    u32 arg1;
    u32 arg2;
    u32 arg3;
    u32 arg4;
    u32 arg5;
    u32 arg6;
    u32 arg7;
    u32 arg8;
    u32 arg9;
    u32 arg10;
    u32 arg11;
    u32 arg12;
    u32 ret1;
    u32 ret2;
};

/**
 * Call a kernel function with kernel privilege
 *
 * @param func_addr - kernel function address
 * @param args - kernel arguments and return values
 *
 * return < 0 on error
 */
int kuKernelCall(void *func_addr, struct KernelCallArg *args);

/**
 * Call a kernel function with kernel privilege and extended stack
 *
 * @param func_addr - kernel function address
 * @param args - kernel arguments and return values
 *
 * return < 0 on error
 */
int kuKernelCallExtendStack(void *func_addr, struct KernelCallArg *args, int stack_size);

void kuKernelGetUmdFile(char *umdfile, int size);

#ifdef __cplusplus
}
#endif

#endif
