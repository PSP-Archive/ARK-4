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
void patchFileManager(void);

// Directory IO Patch for PSP-like Behaviour
void patchFileManagerImports(SceModule2 * mod);

void patchFileSystemDirSyscall(void);
int patchKermitPeripheral(SceModule2 * kermit_peripheral);

SceUID sceIoOpenPSX(const char *file, int flags, SceMode mode);
int sceIoIoctlPSX(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
int sceIoReadPSX(SceUID fd, void *data, SceSize size);

#endif

