#ifndef __SCTRLLIBRARY_H__
#define __SCTRLLIBRARY_H__

#include <pspsdk.h>
#include <pspkernel.h>
#include <psploadexec_kernel.h>
#include <pspinit.h>

/**
 * Restart the vsh.
 *
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL
 *
 * @returns < 0 on some errors.
 *
*/
int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute the EBOOT.BIN from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute an updater from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute an updater from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute games (and homebrew :P) from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHEf2(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute ... ?
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute psx games
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param);


/**
 * Executes a new executable with the specified apitype
 *
 * @param apitype - The apitype
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Sets the api type 
 *
 * @param apitype - The apitype to set
 * @returns the previous apitype
 *
 * @Note - this will modify also the value of sceKernelBootFrom, since the value of
 * bootfrom is calculated from the apitype
*/
int sctrlKernelSetInitApitype(int apitype);

/**
 * Sets the filename of the launched executable.
 *
 * @param filename - The filename to set
 * @returns 0 on success
*/
int sctrlKernelSetInitFileName(char *filename);

/**
 * Sets the init key config
 *
 * @param key - The key code
 * @returns the previous key config
*/
int sctrlKernelSetInitKeyConfig(int key);

/**
 * Sets the user level of the current thread
 *
 * @param level - The user level
 * @return the previous user level on success
 */
int sctrlKernelSetUserLevel(int level);

/**
 * Sets the devkit version
 * 
 * @param version - The devkit version to set
 * @return the previous devkit version
 * 
*/
int sctrlKernelSetDevkitVersion(int version);

/**
 * Checks if we are in SE.
 *
 * @returns 1 if we are in SE-C or later, 0 if we are in HEN-D or later,
 * and < 0 (a kernel error code) in any other case
*/
int	sctrlHENIsSE();

/**
 * Checks if we are in Devhook.
 *
 * @returns 1 if we are in SE-C/HEN-D for devhook  or later, 0 if we are in normal SE-C/HEN-D or later,
 * and < 0 (a kernel error code) in any other case
*/
int	sctrlHENIsDevhook();

/**
 * Gets the HEN version
 *
 * @returns - The HEN version
 *
 * HEN D / SE-C :  0x00000400
 */
int sctrlHENGetVersion();

/**
 * Gets the HEN minor version
 *
 * @returns - The HEN minor version
 */
int sctrlHENGetMinorVersion();

/**
 * Finds a driver
 *
 * @param drvname - The name of the driver (without ":" or numbers)
 *
 * @returns the driver if found, NULL otherwise
 *
 */
PspIoDrv *sctrlHENFindDriver(char *drvname);

/** 
 * Finds a function.
 *
 * @param modname - The module where to search the function
 * @param libname - The library name
 * @nid - The nid of the function
 *
 * @returns - The function address or 0 if not found
 *
*/
u32 sctrlHENFindFunction(char *modname, char *libname, u32 nid);

typedef struct SceModule2 {
    struct SceModule2   *next;
    unsigned short      attribute;
    unsigned char       version[2];
    char                modname[27];
    char                terminal;
    unsigned int        unknown1;
    unsigned int        unknown2;
    SceUID              modid;
    unsigned int        unknown3[2];
    u32         mpid_text;  // 0x38
    u32         mpid_data; // 0x3C
    void *              ent_top;
    unsigned int        ent_size;
    void *              stub_top;
    unsigned int        stub_size;
    unsigned int        unknown4[5];
    unsigned int        entry_addr;
    unsigned int        gp_value;
    unsigned int        text_addr;
    unsigned int        text_size;
    unsigned int        data_size;
    unsigned int        bss_size;
    unsigned int        nsegment;
    unsigned int        segmentaddr[4];
    unsigned int        segmentsize[4];
} SceModule2;

typedef int (* STMOD_HANDLER)(SceModule2 *);

/**
 * Sets a function to be called just before module_start of a module is gonna be called (useful for patching purposes)
 *
 * @param handler - The function, that will receive the module structure before the module is started.
 *
 * @returns - The previous set function (NULL if none);
 * @Note: because only one handler function is handled by HEN, you should
 * call the previous function in your code.
 *
 * @Example: 
 *
 * STMOD_HANDLER previous = NULL;
 *
 * int OnModuleStart(SceModule2 *mod);
 *
 * void somepointofmycode()
 * {
 *		previous = sctrlHENSetStartModuleHandler(OnModuleStart);
 * }
 *
 * int OnModuleStart(SceModule2 *mod)
 * {
 *		if (strcmp(mod->modname, "vsh_module") == 0)
 *		{
 *			// Do something with vsh module here
 *		}
 *
 *		if (!previous)
 *			return 0;
 *
 *		// Call previous handler
 *
 *		return previous(mod);
 * }
 *
 * @Note2: The above example should be compiled with the flag -fno-pic
 *			in order to avoid problems with gp register that may lead to a crash.
 *
*/
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);

typedef int (* KDEC_HANDLER)(u32 *buf, int size, int *retSize, int m);
typedef int (* MDEC_HANDLER)(u32 *tag, u8 *keys, u32 code, u32 *buf, int size, int *retSize, int m, void *unk0, int unk1, int unk2, int unk3, int unk4);

/**
 * Sets the speed (only for kernel usage)
 *
 * @param cpu - The cpu speed
 * @param bus - The bus speed
*/
void sctrlHENSetSpeed(int cpu, int bus);

/**
 * Sets the partition 2 and 8  memory for next loadexec.
 *
 * @param p2 - The size in MB for the user partition. Must be > 0
 * @param p8 - The size in MB for partition 8. Can be 0.
 *
 * @returns 0 on success, < 0 on error.
 * This function is only available in the slim. The function will fail
 * if p2+p8 > 52 or p2 == 0
*/
int sctrlHENSetMemory(u32 p2, u32 p8);

void sctrlHENPatchSyscall(void *addr, void *newaddr);

int sctrlKernelQuerySystemCall(void *func_addr);

int sctrlKernelBootFrom(void);

/**
 * Patch module by offset
 *
 * @param modname - module name
 * @param inst  - instruction
 * @param offset - module patch offset
 *
 * @return < 0 on error
 */
int sctrlPatchModule(char *modname, u32 inst, u32 offset);

/**
 * Get module text address
 *
 * @param modname - module name
 * 
 * @return text address, or 0 if not found
 */
u32 sctrlModuleTextAddr(char *modname);

#endif
