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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * This file contains functions that Sony used to have in the Kernel,
 * but removed for whatever reason they had...
 *
 * To make life easier for developers, we reimplemented them and export
 * them via our NID Resolver.
 *
 * `strncat` on ansi-c/strsafe.c
 */

// Search Needle in Haystack
int ownstrcspn(char * str1, char * str2)
{
    // Iterate Symbols from Haystack
    unsigned int i = 0; for(; i < strlen(str1); i++)
       {
        // Iterate Symbols from Needle
        unsigned int j = 0; for(; j < strlen(str2); j++)
           {
            // Match found
            if(str1[i] == str2[j]) break;
        }
    }

    // Return Offset
    return i;
}

// Inverse Needle Search in Haystack
int ownstrspn(char * str1, char * str2)
{
    // Iterate Symbols from Haystack
    unsigned int i = 0; for(; i < strlen(str1); i++)
       {
        // Iterate Symbols from Needle
        unsigned int j = 0; for(; j < strlen(str2); j++)
           {
            // Mismatch found
            if(str1[i] != str2[j]) break;
        }
    }

    // Return Offset
    return i;
}

char * ownstrtok_r(char * s, const char * delim, char ** last)
{
    // Required Variables
    char * spanp = NULL;
    int c = 0;
    int sc = 0;
    char * tok = NULL;

    // Invalid Parameters
    if(s == NULL && (s = *last) == NULL) return NULL;

cont:
    // Move Pointer
    c = *s++;

    // Scan for Delimiter
    for(spanp = (char *)delim; (sc = *spanp++) != 0;)
       {
        if (c == sc)
            goto cont;
    }

    // End of String
    if(c == 0)
       {
        *last = NULL;
        return NULL;
    }

    // Update Token
    tok = s - 1;

    // Processing Loop
    for(;;)
       {
           // Fetch Symbol
        c = *s++;

        // Fetch Delimtier
        spanp = (char *)delim;

        // Process Text
        do
           {
               // Character Match
            if ((sc = *spanp++) == c)
               {
                   // End of String
                if (c == 0) s = NULL;

                // Terminate String
                else s[-1] = 0;

                // Update Pointer
                *last = s;

                // Return Token
                return tok;
            }
        } while (sc != 0);
    }
}

// String Token Function
char * ownstrtok(char * s, const char * delim)
{
    // Last Token
    char* last = NULL;

    // Recursive Token Implementation
    return ownstrtok_r(s, delim, &last);
}

// Number Parser for Decimal Numbers
int ownstrtol(const char * str, int * res)
{
    // End Pointer
    char * endptr = NULL;

    // Forward Call (with Base 10)
    int result = strtol(str, &endptr, 10);

    // Return Result in Parameter
    if (res) *res = result;

    // Return Result
    return result;
}
