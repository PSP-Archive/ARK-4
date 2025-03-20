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

#include <string.h>
#include <pspkernel.h>
#include <psputility.h>
#include <systemctrl.h>
#include <ark.h>
#include <module2.h>

extern ARKConfig config;

/*
  This code was used back when savedata exploits were the entry point for ARK
  Since the savedata was crafted to load ARK, it meant the game that was used for the savedata could not be played anymore.
  This patch redirects the savedata to a different location while using ARK.
  We no longer offer support for actual game saves exploits so we move this code this this file for historical purposes.
*/

// Return Game Product ID of currently running Game
int sctrlARKGetGameID(char gameid[GAME_ID_MINIMUM_BUFFER_SIZE])
{
    // Invalid Arguments
    if(gameid == NULL) return -1;
    
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Fetch Game Information Structure
    void * gameinfo = getUMDDataFixed();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Game Information unavailable
    if(gameinfo == NULL) return -3;
    
    // Copy Product Code
    memcpy(gameid, gameinfo + 0x44, GAME_ID_MINIMUM_BUFFER_SIZE - 1);
    
    // Terminate Product Code
    gameid[GAME_ID_MINIMUM_BUFFER_SIZE - 1] = 0;
    
    // Return Success
    return 0;
}

// Fix Exploit Game Save
void fixExploitGameModule(SceModule2 * mod)
{
    // Game ID Buffer
    char gameid[GAME_ID_MINIMUM_BUFFER_SIZE];
    
    // Exploit Game was set as LoadExecute Target
    if(sctrlKernelGetGameID(gameid) == 0 && strcmp(gameid, config.exploit_id) == 0)
    {
        // User Module
        if((mod->text_addr & 0x80000000) == 0)
        {
            // Iterate Segments
            unsigned int i = 0; for(; i < mod->nsegment; i++)
            {
                // Module Scan Start Address
                char * base = (char *)mod->segmentaddr[i];
                
                // Scan Module for Game ID
                for(; base < (char *)mod->segmentaddr[i] + mod->segmentsize[i] - 9; base++)
                {
                    // Found Game ID
                    if(strncmp(base, config.exploit_id, 9) == 0)
                    {
                        // Patch Game ID
                        memcpy(base, "CB", 2);
                        
                        // Skip Remainder String
                        i += GAME_ID_MINIMUM_BUFFER_SIZE - 2;
                    }
                }
            }
        }
    }
}

