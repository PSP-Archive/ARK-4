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

// Check String Prefix
int strbeginswith(char * base, char * prefix)
{
    // Result
    int result = 1;
    
    // Position
    unsigned int position = 0;
    
    // Compare
    while(prefix[position] != 0)
    {
        // Invalid Match
        if(base[position] != prefix[position])
        {
            // Set Result
            result = 0;
            
            // Stop Comparison
            break;
        }
        
        // Change Position
        position++;
    }
    
    // Return Result
    return result;
}

