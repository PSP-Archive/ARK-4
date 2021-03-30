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
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <string.h>
#include <rebootconfig.h>
#include <systemctrl.h>
#include "imports.h"
#include "rebootex.h"

extern ARKConfig* ark_config;

// Original Load Reboot Buffer Function
int (* _LoadReboot)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4) = NULL;

// Reboot Buffer Backup
u8 reboot_backup[REBOOTEX_MAX_SIZE];
u8 rebootex_config[REBOOTEX_CONFIG_MAXSIZE];
RebootConfigFunctions* reboot_funcs = NULL;
static RebootConfigFunctions _reboot_funcs;
// Reboot ISO Path
char* reboot_config_isopath = NULL;

static void ARKSetBootConfFileIndex(int index)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    reboot_config->iso_mode = index;
}

static unsigned int ARKGetBootConfFileIndex(void)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    return reboot_config->iso_mode;
}

static void ARKSetDiscType(int type)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    reboot_config->iso_disc_type = type;
}

static int ARKGetDiscType(void)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    reboot_config->iso_disc_type;
}

static void PROSetBootConfFileIndex(int index)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    reboot_config->iso_mode = index;
}

static unsigned int PROGetBootConfFileIndex(void)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    return reboot_config->iso_mode;
}

static void PROSetDiscType(int type)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    reboot_config->iso_disc_type = type;
}

static int PROGetDiscType(void)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    reboot_config->iso_disc_type;
}

static void AdrenalineSetBootConfFileIndex(int index)
{
    RebootConfigAdrenaline* reboot_config = (RebootConfigAdrenaline*)rebootex_config;
    reboot_config->bootfileindex = index;
}

static unsigned int AdrenalineGetBootConfFileIndex(void)
{
    RebootConfigAdrenaline* reboot_config = (RebootConfigAdrenaline*)rebootex_config;
    return reboot_config->bootfileindex;
}

static void AdrenalineSetDiscType(int type)
{
    RebootConfigAdrenaline* reboot_config = (RebootConfigAdrenaline*)rebootex_config;
}

static int AdrenalineGetDiscType(void)
{
    RebootConfigAdrenaline* reboot_config = (RebootConfigAdrenaline*)rebootex_config;
    return PSP_UMD_TYPE_GAME;
}

static void setRebootConfigAdrenaline(){
    RebootConfigAdrenaline* reboot_config = (RebootConfigAdrenaline*)rebootex_config;
    _reboot_funcs.SetBootConfFileIndex = &AdrenalineSetBootConfFileIndex;
    _reboot_funcs.GetBootConfFileIndex = &AdrenalineGetBootConfFileIndex;
    _reboot_funcs.SetDiscType = &AdrenalineSetDiscType;
    _reboot_funcs.GetDiscType = &AdrenalineGetDiscType;
    reboot_funcs = &_reboot_funcs;
    reboot_config_isopath = reboot_config->umdfilename;
}

static void setRebootConfigPRO(){
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    _reboot_funcs.SetBootConfFileIndex = &PROSetBootConfFileIndex;
    _reboot_funcs.GetBootConfFileIndex = &PROGetBootConfFileIndex;
    _reboot_funcs.SetDiscType = &PROSetDiscType;
    _reboot_funcs.GetDiscType = &PROGetDiscType;
    reboot_funcs = &_reboot_funcs;
    reboot_config_isopath = (char*)&(rebootex_config[0x100]);
}

static void setRebootConfigARK(){
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    _reboot_funcs.SetBootConfFileIndex = &ARKSetBootConfFileIndex;
    _reboot_funcs.GetBootConfFileIndex = &ARKGetBootConfFileIndex;
    _reboot_funcs.SetDiscType = &ARKSetDiscType;
    _reboot_funcs.GetDiscType = &ARKGetDiscType;
    reboot_funcs = &_reboot_funcs;
    reboot_config_isopath = reboot_config->iso_path;
}

static int findRebootISOPath(){
    // Find Reboot ISO Path
    for (int i=0; i<REBOOTEX_CONFIG_MAXSIZE; i++){
        char* iso_path = (char*)&(rebootex_config[i]);
        if ( (iso_path[0] == 'm' && iso_path[1] == 's' && iso_path[2] == '0' && iso_path[3] == ':')
          || (iso_path[0] == 'e' && iso_path[1] == 'f' && iso_path[2] == '0' && iso_path[3] == ':') ){
            reboot_config_isopath = iso_path;
            return 1;
        }
    }
    return 0;
}

// Backup Reboot Buffer
void backupRebootBuffer(void)
{
    // Copy Reboot Buffer Payload
    memcpy(reboot_backup, (void *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE);
    
    // Copy Reboot Buffer Configuration
    memcpy(rebootex_config, (void *)REBOOTEX_CONFIG, REBOOTEX_CONFIG_MAXSIZE);
    
    // Copy ARK runtime Config
    if (*(u32*)ARK_CONFIG == ARK_CONFIG_MAGIC)
        memcpy(ark_config, ARK_CONFIG, sizeof(ARKConfig));
    
    // Figure out how to handle rebootex config
    if (((RebootConfigARK*)rebootex_config)->magic == ARK_CONFIG_MAGIC){
        setRebootConfigARK(); // ARK
    }
    else if (((RebootConfigPRO*)rebootex_config)->magic == PRO_CONFIG_MAGIC){
        setRebootConfigPRO(); // PRO
        // Load PSP Compat layer since PRO's rebootex won't
        int uid = sceKernelLoadModule("flash0:/kd/ark_pspcompat.prx", 0, NULL);
        int status; sceKernelStartModule(uid, 0, NULL, &status, NULL);
    }
    else {
        setRebootConfigAdrenaline(); // Assume Adrenaline
    }
    
    // Flush Cache
    flushCache();
}

// Restore Reboot Buffer
void restoreRebootBuffer(void)
{
    // Restore Reboot Buffer Payload
    memcpy((void *)REBOOTEX_TEXT, reboot_backup, REBOOTEX_MAX_SIZE);
    
    // Restore Reboot Buffer Configuration
    memcpy((void *)REBOOTEX_CONFIG, rebootex_config, REBOOTEX_CONFIG_MAXSIZE);
    
    // Restore ARK Config
    ark_config->recovery = 0; // reset recovery mode for next reboot
    memcpy(ARK_CONFIG, ark_config, sizeof(ARKConfig));
}

// Reboot Buffer Loader
int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
    // Restore Reboot Buffer Configuration
    restoreRebootBuffer();
    
    // Load Sony Reboot Buffer
    return _LoadReboot(arg1, arg2, arg3, arg4);
}

// Patch loadexec
void patchLoadExec(SceModule2* loadexec)
{
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;
    patchLoadExecCommon(loadexec, (u32)LoadReboot, 2);
    // Flush Cache
    flushCache();
}
