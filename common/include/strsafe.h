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

#ifndef STRSAFE_H
#define STRSAFE_H

#define STRCAT_S(d, s) do { strcat_s((d), (sizeof(d) / sizeof(d[0])), (s));} while ( 0 )
#define STRCPY_S(d, s) strcpy_s((d), (sizeof(d) / sizeof(d[0])), (s))

size_t strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count);
size_t strncat_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count);
int strncasecmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);

static inline size_t strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
	return strncpy_s(strDestination, numberOfElements, strSource, -1);
}

static inline size_t strcat_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
	return strncat_s(strDestination, numberOfElements, strSource, -1);
}

#endif
