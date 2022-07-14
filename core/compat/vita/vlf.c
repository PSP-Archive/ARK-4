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
#include <systemctrl.h>
#include <macros.h>

// Fix VLF Module
void patchVLF(SceModule2 * mod)
{
    // Flawed Function NIDs
    unsigned int patches[5] =
    {
        0x2A245FE6,
        0x7B08EAAB,
        0x22050FC0,
        0x158BE61A,
        0xD495179F,
    };
    
    // Iterate NIDs
    for(int i = 0; i < NELEMS(patches); i++)
    {
        // Get Function Address
        unsigned int funcAddr = (unsigned int)sctrlHENFindFunction("VLF_Module", "VlfGui", patches[i]);
        
        // Found Function
        if(funcAddr)
        {
            // Dummy Function
            _sw(JR_RA, funcAddr);
            _sw(LI_V0(0), funcAddr + 4);
        }
    }
}

