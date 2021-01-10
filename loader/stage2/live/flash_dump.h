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


#ifndef FLASH_DUMP_H
#define FLASH_DUMP_H

#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <pspumd.h>
#include <pspctrl.h>
#include <module2.h>
#include <lflash0.h>
#include <macros.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include "libs/graphics/graphics.h"

struct minZipHeader {
	char pk[2];
	unsigned nb;
	char space[12];
	unsigned fileSize;
	unsigned fileSizeClone;
	unsigned pathLen;
	/*
	path
	data
	*/
};


int kthread(SceSize args, void *argp);
void initKernelThread(void);

unsigned addWriteFile( SceUID packFileID, void *data, unsigned size, char *name, u8 found_nb );
int findFlashIndex( const VitaFlashBufferFile *f0, void *origContent );
void flashVitaDump( char *packName );



#endif

