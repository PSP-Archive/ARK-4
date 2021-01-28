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

#ifndef _XMBISO_H_
#define _XMBISO_H_

#include <pspiofilemgr.h>
#include <psploadexec_kernel.h>

//------ stage 1 - fake directory existence ------

//open directory
SceUID gamedopen(const char * dirname);

//read directory
int gamedread(SceUID fd, SceIoDirent * dir);

//close directory
int gamedclose(SceUID fd);

//------ stage 2 - fake file existence ------

//create and open temporary eboot
SceUID opentemp(const char * file, int flags, SceMode mode);

//open file
SceUID gameopen(const char * file, int flags, SceMode mode);

//read file
int gameread(SceUID fd, void * data, SceSize size);

//close file
int gameclose(SceUID fd);

//seek file
SceOff gamelseek(SceUID fd, SceOff offset, int whence);

//get file status
int gamegetstat(const char * file, SceIoStat * stat);

//remove file
int gameremove(const char * file);

//remove folder
int gamermdir(const char * path);

//load and execute file
int gameloadexec(char * file, struct SceKernelLoadExecVSHParam * param);

//rename file
int gamerename(const char *oldname, const char *newfile);

int gamechstat(const char *file, SceIoStat *stat, int bits);

#endif
