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

#include <ansi_c_functions.h>

void *memmove(void * to_, const void * from_, unsigned int length)
{
    char *to = to_;
    const char *from = from_;

    if (to > from) {
        //back buffer
        char * tob = to + length;
        const char * fromb = from + length;

        //loop copy
        unsigned int pos = 0; for(; pos < length; pos++)
        {
            //copy byte
            *--tob = *--fromb;
        }

        return to_;
    }

    return memcpy(to, from, length);
}
