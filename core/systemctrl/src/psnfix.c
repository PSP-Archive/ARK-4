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
#include <module2.h>

//fix playstation network account registration
void patch_npsignup(SceModule2* mod)
{
    //ImageVersion = 0x10000000
    //_sw(0x3C041000, mod->text_addr + 0x00038CBC);
    for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size; addr+=4){
        if (_lw(addr) == 0xE7B800A0){
        	_sw(0x3C041000, addr-40);
        	break;
        }
    }
}

//fix playstation network login
void patch_npsignin(SceModule2* mod)
{
    //kill connection error
    //_sw(0x10000008, mod->text_addr + 0x00006CF4);

    //ImageVersion = 0x10000000
    //_sw(0x3C041000, mod->text_addr + 0x000096C4);

    int patches = 2;
    for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x24510004 && _lw(addr+4) == 0x00002021){
        	_sw(0x10000008, addr + 108);
        	patches--;
        }
        else if (data == 0x8FA40100){
        	_sw(0x3C041000, addr);
        	patches--;
        }
    }
}

//fake hardcoded np version for npmatching library (psp2, fat princess, etc.)
void patch_np(SceModule2* mod, u8 major, u8 minor)
{
    //np firmware version spoof
    //_sb(mayor, mod->text_addr + 0x00004604);
    //_sb(minor, mod->text_addr + 0x0000460C);
    for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size; addr+=4){
        if (_lw(addr) == 0x34620003){
        	_sb(major, addr + 24);
        	_sb(minor, addr + 32);
        	break;
        }
    }
}
