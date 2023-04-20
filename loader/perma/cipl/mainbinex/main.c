#include <pspsdk.h>
#include <globals.h>
#include <macros.h>
#include "ansi_c_functions.h"

#include "cache.h"
#include "seedkey.h"
#ifndef MS_IPL
#include "../payloadex/nand_payloadex.h"
#else
#include <ms_payloadex.h>
#endif
#define SYSCON_CTRL_HOME 0x00001000

void (*iplDcache)(void) = NULL;
int (*pspSysconRxDword)(u32 *param, u8 cmd) = NULL;
void (*seedPatchReturn)(void) = NULL;

void seedPatch()
{
	memcpy((void *)0xBFC00200, seed_key_66x , sizeof(seed_key_66x));
	return seedPatchReturn();
}

void iplDcachePatched()
{
	u32 *keyBuf = (u32 *)BOOT_KEY_BUFFER;

	keyBuf[0] = -1;
	pspSysconRxDword(keyBuf , 7);

	if (keyBuf[0] & SYSCON_CTRL_HOME)
		memcpy((void *)REBOOTEX_TEXT , payloadex , size_payloadex);
	else
		REDIRECT_FUNCTION(REBOOTEX_TEXT, REBOOT_TEXT);

	return iplDcache();
}

void patchMainBin(void)
{
	u32 mainbin_start = MAINBIN_TEXT;
	u32 mainbin_end = mainbin_start + 0xE000;

	int patches = 3;
	for (u32 addr = mainbin_start; addr < mainbin_end && patches; addr+=4){
		u32 data = _lw(addr);
		if (data == 0x34E6C000) {
			addr += 4;
			iplDcache = (void *)K_EXTRACT_CALL(addr);
			MAKE_CALL(addr, iplDcachePatched);
			patches--;
		}
		else if (data == 0x3C198860) {
			_sw(0x3C1988FC, addr);
			patches--;
		}
		else if (data == 0x24050046) {
			pspSysconRxDword = (void *)K_EXTRACT_CALL(addr-4);
			patches--;
		}
		else if (data == 0x2DCD2290) {
			u32 a = addr;
			seedPatchReturn = (void *)K_EXTRACT_BRANCH(addr+4);
			do { a+=4; } while (_lw(a) != 0xAFA00030);
			MAKE_JUMP(a, seedPatch);
			_sw(NOP, a+4);
		}
	}

	ClearCache();

	__asm("lui		$25, 0x0400");
	__asm("lui		$sp, 0x040F");
	__asm("jr		$25");
	__asm("ori		$sp,$sp, 0xFF00");
}

void _arkBoot() __attribute__ ((section (".text.startup")));
void _arkBoot()
{
#ifdef MS_IPL
	patchMainBin();
#else
	MAKE_JUMP(0x040EC2C8, patchMainBin); // Redirect jump to main.bin
	
	_sw(NOP, 0x040EC0D8); // NOP out call to memset 0x040E0000-0x040EC000

	ClearCache();

	void (* entry)() = (void *)0x040EC000;

	return entry();
#endif
}