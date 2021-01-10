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

#ifndef _ANSI_C_FUNCTIONS_H_
#define _ANSI_C_FUNCTIONS_H_

/*
 * This is the main function prototype header of the ANSI-C Functions.
 * Use it if your lowlevel binary consists of multiple source files.
 */

// Minimal Memcpy Implementation
void *memcpy(void *to_, const void *from_, unsigned int size);

// Minimal Memset Implementation
void *memset(void * buffer_, int value, unsigned int size);

// Check String Prefix
int strbeginswith(char * base, char * prefix);

// Copy String Buffer
char *strcpy(char *to, const char *from);

// Concatenate append into string s
char *strcat(char *s, const char *append);

// Return Length of String in Bytes (without terminator)
unsigned int strlen(const char * text);

// Replace Character in String
unsigned int strreplaceall(char * base, char from, char to);

// Framebuffer Painter (for debugging)
void colorDebug(unsigned int color);

// Framebuffer Color Freeze Loop (for debugging)
void colorLoop(void);

// Lightweight String Conversion for Debug Screen Output (not threadsafe)
char * hex32(unsigned int v);

// memmove
void *memmove(void * to, const void * from, unsigned int length);

// compare string a with string b
int strcmp(const char *a, const char *b);

// compare string a with string b for a certain amount of characters
int strncmp(const char *a, const char *b, unsigned int count);

char* memfindsz(const char* s1, char* start, unsigned int size);

unsigned int* memfindu32(const unsigned int val, unsigned int* start, unsigned int size);

// Find the first character ch in string cp
char * strchr(const char *cp, int ch);

// Find the last character ch in string cp
char * strrchr(const char *cp, int ch);

#endif

