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
#include "loader/rebootex/payload.h"
#include "core/compat/psp/pro_rebootex.h"

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

static void SetBootConfFileIndexARK(int index)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    reboot_config->iso_mode = index;
}

static unsigned int GetBootConfFileIndexARK(void)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    return reboot_config->iso_mode;
}

static void SetDiscTypeARK(int type)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    reboot_config->iso_disc_type = type;
}

static int GetDiscTypeARK(void)
{
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    reboot_config->iso_disc_type;
}

static void SetBootConfFileIndexPRO(int index)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    reboot_config->iso_mode = index;
}

static unsigned int GetBootConfFileIndexPRO(void)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    return reboot_config->iso_mode;
}

static void SetDiscTypePRO(int type)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    reboot_config->iso_disc_type = type;
}

static int GetDiscTypePRO(void)
{
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    reboot_config->iso_disc_type;
}

static void setRebootConfigPRO(){
    RebootConfigPRO* reboot_config = (RebootConfigPRO*)rebootex_config;
    _reboot_funcs.SetBootConfFileIndex = &SetBootConfFileIndexPRO;
    _reboot_funcs.GetBootConfFileIndex = &GetBootConfFileIndexPRO;
    _reboot_funcs.SetDiscType = &SetDiscTypePRO;
    _reboot_funcs.GetDiscType = &GetDiscTypePRO;
    reboot_funcs = &_reboot_funcs;
    reboot_config_isopath = (char*)&(rebootex_config[0x100]);
    if (reboot_config->rebootex_size == 0){ // Infinity setup, must inject PRO rebootex
        memcpy(reboot_backup, pro_rebootex, size_pro_rebootex);
        reboot_config->rebootex_size = size_pro_rebootex;
    }
}

static void setRebootConfigARK(){
    RebootConfigARK* reboot_config = (RebootConfigARK*)rebootex_config;
    _reboot_funcs.SetBootConfFileIndex = &SetBootConfFileIndexARK;
    _reboot_funcs.GetBootConfFileIndex = &GetBootConfFileIndexARK;
    _reboot_funcs.SetDiscType = &SetDiscTypeARK;
    _reboot_funcs.GetDiscType = &GetDiscTypeARK;
    reboot_funcs = &_reboot_funcs;
    reboot_config_isopath = reboot_config->iso_path;
    if (reboot_config->reboot_buffer_size == 0){ // Infinity setup, must inject ARK rebootex
        memcpy(reboot_backup, rebootbuffer, size_rebootbuffer);
        reboot_config->reboot_buffer_size = size_rebootbuffer;
    }
}

char* findRebootISOPath(){
    // Find Reboot ISO Path
    for (int i=0; i<REBOOTEX_CONFIG_MAXSIZE; i++){
        char* iso_path = (char*)&(rebootex_config[i]);
        if ( (iso_path[0] == 'm' && iso_path[1] == 's' && iso_path[2] == '0' && iso_path[3] == ':')
          || (iso_path[0] == 'e' && iso_path[1] == 'f' && iso_path[2] == '0' && iso_path[3] == ':') ){
            return iso_path;
        }
    }
    return NULL;
}

// Backup Reboot Buffer
void backupRebootBuffer(void)
{
    // Copy Reboot Buffer Payload
    memcpy(reboot_backup, (void *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE);
    
    // Copy Reboot Buffer Configuration
    memcpy(rebootex_config, (void *)REBOOTEX_CONFIG, REBOOTEX_CONFIG_MAXSIZE);
    
    // Copy ARK runtime Config
    if (IS_ARK_CONFIG(ARK_CONFIG))
        memcpy(ark_config, ARK_CONFIG, sizeof(ARKConfig));
    
    // Figure out how to handle rebootex config
    if (IS_ARK_CONFIG(rebootex_config)){
        setRebootConfigARK(); // ARK
    }
    else if (IS_PRO_CONFIG(rebootex_config)){
        setRebootConfigPRO(); // PRO
        // set ARK Config
        ark_config->exec_mode = PSP_ORIG;
        if (sceKernelGetModel() == PSP_GO){
            ark_config->arkpath[0] = 'e';
            ark_config->arkpath[1] = 'f';
        }
    }
    else{
        // can't handle it :P
    }
    
    // Flush Cache
    flushCache();
}

// Restore Reboot Buffer
void restoreRebootBuffer(void)
{
    // Restore Reboot Buffer Payload
    if (reboot_backup[0] == 0x1F && reboot_backup[1] == 0x8B) // gzip packed rebootex
        sceKernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, reboot_backup, NULL);
    else // plain payload
        memcpy((void *)REBOOTEX_TEXT, reboot_backup, REBOOTEX_MAX_SIZE);
        
    // Restore Reboot Buffer Configuration
    memcpy((void *)REBOOTEX_CONFIG, rebootex_config, REBOOTEX_CONFIG_MAXSIZE);

    // Restore ARK Configuration
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
