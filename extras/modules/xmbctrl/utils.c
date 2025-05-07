/*
    6.39 TN-A, XmbControl
    Copyright (C) 2011, Total_Noob
    Copyright (C) 2011, Frostegater

    main.h: XmbControl main header file
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <pspsdk.h>
#include <pspkernel.h>

#include "include/main.h"

#define MASKBITS 0x3F
#define MASKBYTE 0x80
#define MASK2BYTES 0xC0
#define MASK3BYTES 0xE0
#define MASK4BYTES 0xF0
#define MASK5BYTES 0xF8
#define MASK6BYTES 0xFC

int utf8_to_unicode(wchar_t *dest, char *src)
{
    int i, x;
    unsigned char *usrc = (unsigned char *)src;

    for(i = 0, x = 0; usrc[i];)
    {
        wchar_t ch;

        if((usrc[i] & MASK3BYTES) == MASK3BYTES)
        {
            ch = ((usrc[i] & 0x0F) << 12) | (
                (usrc[i+1] & MASKBITS) << 6)
                | (usrc[i+2] & MASKBITS);

            i += 3;
        }

        else if((usrc[i] & MASK2BYTES) == MASK2BYTES)
        {
            ch = ((usrc[i] & 0x1F) << 6) | (usrc[i+1] & MASKBITS);
            i += 2;
        }

        else/* if(usrc[i] < MASKBYTE)*/
        {
            ch = usrc[i];
            i += 1;
        }

        dest[x++] = ch;
    }
    
    dest[x++] = '\0';

    return x;
}

int atoi(const char* txt){
    return sce_paf_private_strtoul(txt, NULL, 10);
}