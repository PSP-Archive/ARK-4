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

#ifndef _MISSINGFUNC_H_
#define _MISSINGFUNC_H_

/*
 * Missing Function Header (mostly for nidresolver.c)
 * Contains all reimplemented Function Prototypes that Sony removed in Updates.
 */

int ownsetjmp(void);
int ownlongjmp(void);
int ownstrcspn(char * str1, char * str2);
int ownstrspn(char * str1, char * str2);
char * ownstrtok(char * s, const char * delim);
char * ownstrtok_r(char * s, const char * delim, char ** last);
char * strncat(char * dst, const char * src, size_t n);
int ownstrtol(const char * str, int * res);

#endif

