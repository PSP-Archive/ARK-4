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
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <systemctrl_private.h>
#include <globals.h>

// Patch mediasync.prx
void patchMediaSync(SceModule2* mod)
{
    colorDebug(0xff0000);
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr+mod->text_size;
    
    int disc_patches = 2;
    int patches = 3+disc_patches;

    for (u32 addr=text_addr; addr<top_addr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x00600008 && _lw(addr+4) == NOP && _lw(addr-4) == 0x8D030000){
            u32 ms_check_media = K_EXTRACT_CALL(addr+8);
            _sw(JR_RA, ms_check_media);
            _sw(LI_V0(1), ms_check_media + 4);
            patches--;
        }
        else if (data == 0x2406000A && _lw(addr-16) == 0x24510008 && disc_patches){
            _sw(0x1000001D, addr+4); // MEDIASYNC_DISC_MEDIA_CHECK
            disc_patches--;
            patches--;
        }
        else if (data == 0x2406000D && _lw(addr+8) == 0x00008021){
            _sw(0x1000FFDB, addr + 4); // MEDIASYNC_MS_SYSTEM_FILE_CHECK
            patches--;
        }
        else if (data == 0x27A60014){ // MEDIASYNC_DISC_ID_CHECK
            _sw(NOP, addr+12);
            u32 a = addr+16;
            do {a+=4;} while (_lw(a)&0xFFFF0000 != 0x14400000);
            _sw(NOP, a);
            patches--;
        }
        else if (data == 0x24040034){
            _sw(0x00001021, addr-8); // MEDIASYNC_KD_FOLDER_PATCH
            patches--;
        } 
    }
}

