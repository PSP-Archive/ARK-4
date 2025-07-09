#ifndef __VSHCTRL_H__
#define __VSHCTRL_H__

/**
 * This api is for vsh menu, xmb control or any other vsh/xmb plugin.
 * 
 * Note: when refering to CSO, other compressed formats (ZSO, JSO, DAX) also apply.
*/




/**
 * Registers the vsh menu.
 * When HOME is pressed, vshctrl will load the satelite module.
 * In module_start, call this function to register the vsh menu.
 *
 * @param ctrl - The function that will be executed each time 
 * the system calls ReadBufferPositive. Despite satelite.prx being
 * an user module, this function will be executed in kernel mode.
 *
 * @returns 0 on success, < 0 on error.
*/
int vctrlVSHRegisterVshMenu(int (* ctrl)(SceCtrlData *, int));


/**
 * Exits the vsh menu. 
 * vshmenu module must call this module after destroying vsh menu display and 
 * freeing resources.
 *
 * vshmenu module doesn't need to stop-unload itself, as that is vshctrl job. 
 *
 * @param conf - Indicates the new config. vshctrl will update the internal
 * vshctrl and systemctrl variables with the new configuration given by this param.
 * However is job of satelite.prx to save those settings to the configuration file.
 * using sctrlSESetConfig.
 *
 * @param videoiso - Indicates the videoiso to mount, or NULL if none.
 * @param disctype - The disctype of the videoiso (0x20=video, 0x40=audio)
 *
 * @returns 0 on success, < 0 on error.
*/
int vctrlVSHExitVSHMenu(SEConfig *conf, char *videoiso, int disctype);


/**
 * Sets SE configuration
 * 
 * @param conf - Indicates the new config. vshctrl will update the internal
 * vshctrl and systemctrl variables with the new configuration given by this param.
 * However is job of satelite.prx to save those settings to the configuration file.
 * using sctrlSESetConfig.
 * 
 * @returns 0 on success, < 0 on error.
 */
int vctrlVSHUpdateConfig(SEConfig *config);


/**
 * Detect the UMD type of an ISO.
 * 
 * @param path - full path to the ISO (or CSO) file.
 * 
 * @returns disc type on success, < 0 on error.
 */
int vshDetectDiscType(const char *path);


/**
 * Delete PSP Go hibernation (paused game).
 * 
 * @returns 0 on success, < 0 on error.
 */
int vshCtrlDeleteHibernation();


/**
 * Open an ISO (or CSO) for reading.
 * 
 * @param path - full path to the ISO (or CSO) file.
 * 
 * @returns 0 on success, < 0 on error.
 */
int isoOpen(const char *path);


/**
 * Raw sector read of currently opened ISO.
 * 
 * @param buffer - pointer to where data will be stored.
 * @param lba - Logical Block Address of the ISO. Each ISO block is 2048 bytes in size.
 * @param offset - offset within the LBA to start reading data from.
 * @param size - amount of bytes to read into provided buffer.
 * 
 * @returns amount of actual bytes read into buffer, < 0 on error.
 */
int isoRead(void *buffer, u32 lba, int offset, u32 size);


/**
 * Closes the currently opened ISO, if there was one.
 */
void isoClose();


/**
 * Get information about a file inside the currently opened ISO.
 * Information retrieved is the file size and LBA where the file starts
 *  (all files in an ISO are aligned by ISO block size - a file does not start in the middle of a block).
 * 
 * @param path - path of the file inside the ISO that you want to scan.
 * @param filesize - pointer to an unsigned 32 bit integer where the size of the file will be stored.
 * @param lba - pointer to an unsigned 32 bit integer where the starting LBA of the file will be stored.
 * 
 * @returns 0 on success, < 0 on error.
 */
int isoGetFileInfo(char * path, u32 *filesize, u32 *lba);


/**
 * Get total amount of sectors/blocks in the currently opened ISO.
 * Each standard ISO sector/block is 2048 bytes in size.
 * Not to be confused with CSO sectors/blocks, which can be of any arbitrary size.
 * 
 * @returns total number of sectors in the currently opened ISO file.
 */
int isoGetTotalSectorSize();


/**
 * Helper function to detect if an ISO (or CSO) has been patched with Prometheus Patches.
 * 
 * @param isopath - full path of the ISO (or CSO) file.
 * 
 * @returns boolean - 1 if patched, 0 if unpatched.
 */
int has_prometheus_module(const char *isopath);


/**
 * Helper function to detect if an ISO (or CSO) has an update (PBOOT.PBP) available to use.
 * 
 * @param isopath - full path of the ISO (or CSO) file.
 * 
 * @returns boolean - 1 if update available, 0 if unavailable.
 */
int has_update_file(const char *isopath);



/**
 * Obtain the value of a registry variable.
 * 
 * @param dir - registry directory.
 * @param name - variable name.
 * @param val - pointer to an unsigned 32 bit integer where the value will be stored.
 */
int vctrlGetRegistryValue(const char *dir, const char *name, u32 *val);


/**
 * Set the value of a registry variable.
 * 
 * @param dir - registry directory.
 * @param name - variable name.
 * @param val - new value of the variable.
 */
int vctrlSetRegistryValue(const char *dir, const char *name, u32 val);

#endif

