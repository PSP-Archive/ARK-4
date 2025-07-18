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

// Return Length of String in Bytes (without terminator)
unsigned int strlen(const char * text)
{
    // String Length
    unsigned int length = 0;
    
    // Count Characters
    while(text[length] != 0) length++;
    
    // Return String Length
    return length;
}

unsigned int strnlen(const char * text, unsigned int maxlen)
{
    // String Length
    unsigned int length = 0;
    
    // Count Characters
    while (text[length] != 0 && length<maxlen) length++;
    
    // Return String Length
    return length;
}