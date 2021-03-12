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

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <pspiofilemgr.h>
#include <pspiofilemgr_kernel.h>

// sceIOFileManager Patch
void initFileSystem();

// Directory IO Patch for PSP-like Behaviour
void patchFileManagerImports(SceModule2 * mod);
void patchFileSystemDirSyscall(void);

// FileIO patch
extern SceModule2* patchFileIO();

#endif

