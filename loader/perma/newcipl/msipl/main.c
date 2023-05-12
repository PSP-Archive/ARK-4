#include <pspsdk.h>
#include <string.h>
#include <sysreg.h>
#include "syscon.h"
#include "fat.h"

#include "printf.h"
#ifdef DEBUG
#include "uart.h"
#endif

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a)
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a)

void DcacheClear();
void IcacheClear();

void ClearCaches()
{
	DcacheClear();
	IcacheClear();
}

uint32_t GetTachyonVersion()
{
	uint32_t ver = _lw(0xbc100040);
	
	if (ver & 0xFF000000)
		return (ver >> 8);

	return 0x100000;
}

int entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2)
{
	// SYSCON SPI enable
	SYSREG_CLK2_ENABLE_REG |= 0x02;
	REG32(0xbc10007c) |= 0xc8;
	
	__asm("sync"::);

	sceSysconInit();
	
#ifdef DEBUG
	sceSysconCtrlHRPower(1);
	
	uart_init();

	printf("msipl starting...\n");
#endif
	uint32_t baryon_version = 0;
	sceSysconGetBaryonVersion(&baryon_version);
	uint32_t tachyon_version = GetTachyonVersion();
	
#ifdef DEBUG
	printf("Tachyon: %x\n", tachyon_version);
	printf("Baryon: %x\n", baryon_version);
#endif
	
	if (tachyon_version >= 0x600000)
		_sw(0x20070910, 0xbfc00ffc);
	else if (tachyon_version >= 0x400000)
		_sw(0x20050104, 0xbfc00ffc);
	else
		_sw(0x20040420, 0xbfc00ffc);
	
	int gen = 11;
	if (tachyon_version <= 0x400000) {
		gen = 1;
	} else if (tachyon_version == 0x500000 || (tachyon_version == 0x600000 && baryon_version == 0x243000)) {
		gen = 2;
	} else if (tachyon_version <= 0x600000) {
		gen = 3;
	} else if (tachyon_version == 0x810000 && baryon_version == 0x2C4000) {
		gen = 4;
	} else if (tachyon_version <= 0x800000) {
		gen = 5;
	} else if (tachyon_version == 0x810000 && baryon_version == 0x2E4000) {
		gen = 7;
	} else if (tachyon_version == 0x820000 && baryon_version == 0x2E4000) {
		gen = 9;
	}
		
	MsFatMount();
		
	char path[60];
	
	sprintf(path, "/TM/DC10/ipl_%02dg.bin", gen);
	
#ifdef DEBUG
	printf("ipl: %s\n", path);
#endif

	MsFatOpen(path);
	
	MsFatRead((void *) 0x40e0000, 0xC000);
	MsFatRead((void *) 0x40ec000, 0xe0000);

	ClearCaches();
	
	return ((int (*)()) 0x40e0000)();
}

