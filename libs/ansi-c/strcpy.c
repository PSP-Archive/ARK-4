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

#include "ansi_c_functions.h"

// Copy String Buffer
char *strcpy(char *to, const char *from)
{
    char *oto = to;
    
    // Position
    unsigned int position = 0;
    
    // Copy Bytes
    while(from[position] != 0)
    {
        // Copy Byte
        to[position] = from[position];
        
        // Change Position
        position++;
    }
    
    // Terminate String
    to[position] = 0;
    
    return oto;
}

// Concatenates string s + append
char *strcat(char *s, const char *append)
{
    char *pRet = s;
    while(*s)
    {
        s++;
    }

    while(*append)
    {
        *s++ = *append++;
    }

    *s = 0;

    return pRet;
}
