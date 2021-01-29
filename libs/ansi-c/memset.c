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

// Memset Accelerator
void fast_set(unsigned int * buffer, unsigned int value, unsigned int size)
{
    // Divide Size into Dword Chunks
    size /= 4;
    value = value & 0xFF;
    value = value << 24 | value << 16 | value << 8 | value;
    
    // Set Data
    while(size > 0)
    {
        // Set Dword
        *buffer++ = value;
        
        // Reduce remaining Size
        size--;
    }
}

// Minimal Memset Implementation
void *memset(void * buffer_, int value, unsigned int size)
{
    char *buffer = buffer_;
    
    // Valid Arguments
    if(buffer != NULL && size > 0)
    {
        // Align Dword Size
        unsigned int dwordsize = (size >> 2) << 2;
        
        // Check Alignment
        if((((unsigned int)buffer) % 4) != 0)
        {
            // Invalid Alignment for Fast Copy
            dwordsize = 0;
        }
        
        // Copy Dwords
        if(dwordsize > 0)
        {
            // Use Dword Setter
            fast_set((unsigned int *)buffer, (unsigned int)value, dwordsize);
        }
        
        // Set Bytes
        if(size > dwordsize)
        {
            // Copy Bytes
            unsigned int i = dwordsize; for(; i < size; i++)
            {
                // Set Byte
                buffer[i] = value;
            }
        }
    }
    
    // Return Result
    return buffer_;
}
