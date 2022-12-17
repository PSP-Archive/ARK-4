#include <pspsdk.h>
#include <globals.h>
#include <macros.h>

#include "seedkey.h"

#include "../payloadex/nand_payloadex.h"

void patch_main_bin(void);
void iplDcachePatched();
void seed_patch();
void (*seed_patch_return)(void) = NULL;

void (*iplDcache)(void) = NULL;
int (*pspSyscon_rx_dword)(u32 *param, u8 cmd) = NULL;

void ClearCache() __attribute__((noinline));
void ClearCache()
{
	Dcache();
	Icache();
}

int memset(unsigned char * buffer, unsigned char value, unsigned int length)
{
	//result
	int result = 0;

	//loop set
	unsigned int pos = 0; for(; pos < length; pos++)
	{
		//set byte
		buffer[pos] = value;

		//increment result
		result++;
	}

	return result;
}

void Patch_Entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2) __attribute__ ((section (".text.startup")));
void Patch_Entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2)
{
	MAKE_JUMP(0x040EC2C8, patch_main_bin);
	_sw(0, 0x040EC0D8);

	ClearCache();

	void (* entry)() = (void *)0x040EC000;

	return entry(a0, a1, a2, a3, t0, t1, t2);
}

void patch_main_bin(void)
{
	u32 mainbin_start = MAINBIN_TEXT;
	u32 mainbin_end = mainbin_start + 0xE000;

	int patches = 3;
	for (u32 addr = mainbin_start; addr < mainbin_end && patches; addr+=4){
		u32 data = _lw(addr);
		if (data == 0x34E6C000) {
			addr += 4;
			iplDcache = K_EXTRACT_CALL(addr);
			MAKE_CALL(addr, iplDcachePatched);
			patches--;
		}
		else if (data == 0x3C198860) {
			_sw(0x3C1988FC, addr);
			patches--;
		}
		else if (data == 0x24050046) {
			pspSyscon_rx_dword = K_EXTRACT_CALL(addr-4);
			patches--;
		}
		else if (data == 0x2DCD2290) {
			u32 a = addr;
			seed_patch_return = K_EXTRACT_BRANCH(addr+4);
			do { a+=4; } while (_lw(a) != 0xAFA00030);
			MAKE_JUMP(a, seed_patch);
			_sw(NOP, a+4);
		}
	}

	ClearCache();

	__asm("lui		$25, 0x0400");
	__asm("lui		$sp, 0x040F");
	__asm("jr		$25");
	__asm("ori		$sp,$sp, 0xFF00");
}

void seed_patch()
{
	memcpy(0xBFC00200, seed_key_66x , sizeof(seed_key_66x));
	return seed_patch_return();
}

#define SYSCON_CTRL_HOME      0x00001000

void iplDcachePatched()
{
	u32 *key_buff = (u32 *)BOOT_KEY_BUFFER;

	key_buff[0] = -1;
	pspSyscon_rx_dword(key_buff , 7);

	if(key_buff[0] & SYSCON_CTRL_HOME)
	{
		memcpy(REBOOTEX_TEXT , payloadex , size_payloadex);
	}
	else
	{
		REDIRECT_FUNCTION(REBOOTEX_TEXT, REBOOT_TEXT);
	}

	return iplDcache();
}

void Icache(void)
{
	__asm__ __volatile__ ("\
	.word 0x40088000;\
	.word 0x24091000;\
	.word 0x7D081240;\
	.word 0x01094804;\
	.word 0x4080E000;\
	.word 0x4080E800;\
	.word 0x00004021;\
	.word 0xBD010000;\
	.word 0xBD030000;\
	.word 0x25080040;\
	.word 0x1509FFFC;\
	.word 0x00000000;\
	"::);

	return;
}

void Dcache(void)
{
	__asm__ __volatile__ ("\
	.word 0x40088000;\
	.word 0x24090800;\
	.word 0x7D081180;\
	.word 0x01094804;\
	.word 0x00004021;\
	.word 0xBD100000;\
	.word 0x400AE000;\
	.word 0x400BE800;\
	.word 0x7D4C0500;\
	.word 0x11800004;\
	.word 0x7D6C0500;\
	.word 0x000A5340;\
	.word 0x01485025;\
	.word 0xBD5A0000;\
	.word 0x11800003;\
	.word 0x000B5B40;\
	.word 0x01685825;\
	.word 0xBD7A0000;\
	.word 0x25080040;\
	.word 0x1509FFF1;\
	.word 0x00000000;\
	"::);

	return;
}

int memcpy(u8 *dst,u8 *src,int size)
{
	u8 *p1 = (u8 *)dst;
	u8 *p2 = (u8 *)src;
	while(size--)
	{
		*p1++ = *p2++;
	}
	return p1;
}
