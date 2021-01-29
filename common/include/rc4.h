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

#ifndef RC4_H
#define RC4_H

typedef unsigned char RC4BYTE;
typedef unsigned int RC4DWORD;

struct rc4_state
{
    RC4BYTE mstate[256];
    RC4BYTE x;
    RC4BYTE y;
};

struct rc4_state g_state;

/*
 * Transparent rc4 implementation
 * Based upon sample in crypto++ library,
 * which was based upon an anonymous usenet posting.
 * Implemented by Lucas Madar <lucas@dal.net>
 *
 * Remember that it is IMPERITAVE to generate a new key
 * for each state. DO NOT USE THE SAME KEY FOR ANY TWO STATES.
 */

void *rc4_initstate(unsigned char *key, int keylen);

void rc4_process_stream(void *rc4_context, unsigned char *istring,
			unsigned int stringlen);

void rc4_destroystate(void *a);

#endif
