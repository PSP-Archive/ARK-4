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

#include <stdlib.h>
#include "ansi_c_functions.h"

// Memcpy Accelerator
void fast_copy(unsigned int * dst, unsigned int * src, unsigned int size)
{
    // Divide Size into Dword Chunks
    size /= 4;
    
    // Copy Data
    while(size > 0)
    {
        // Copy Dword
        *dst++ = *src++;
        
        // Reduce remaining Size
        size--;
    }
}

// Minimal Memcpy Implementation
void *memcpy(void *to_, const void *from_, unsigned int size)
{
    char *to = to_;
    const char *from = from_;
    
    // Valid Arguments
    if(to != NULL && from != NULL && size > 0)
    {
        // Align Dword Size
        unsigned int dwordsize = (size >> 2) << 2;
        
        // Check Alignment
        if((((unsigned int)to) % 4) != 0 || (((unsigned int)from) % 4) != 0)
        {
            // Invalid Alignment for Fast Copy
            dwordsize = 0;
        }
        
        // Copy Dwords
        if(dwordsize > 0) fast_copy((unsigned int *)to, (unsigned int *)from, dwordsize);
        
        // Copy Bytes
        if(size > dwordsize)
        {
            // Copy Bytes
            unsigned int i = dwordsize; for(; i < size; i++)
            {
                // Copy Byte
                to[i] = from[i];
            }
        }
    }
    
    // Return Result
    return to_;
}
