#include <pspsdk.h>
#include <string.h>

#include "sysreg.h"
#include "kirk.h"
#include "syscon.h"

#include "printf.h"
#include "uart.h"
#include "ms.h"

void Dcache();
void Icache();

void *memset(void *dest, int value, size_t size)
{
	u8 *d = (u8 *) dest;
	
	while (size--)
		*(d++) = value;
	
	return dest;
}

void *memcpy(void *dest, const void *src, size_t size)
{
	u8 *d = (u8 *) dest;
	u8 *s = (u8 *) src;
	
	while (size--)
		*(d++) = *(s++);

	return dest;
}

void MsLoadBlock(u32 idx, void *addr)
{
	for (int i = 0; i < 8; i++)
		while (pspMsReadSector(0x10 + (8 * idx) + i, &((u8 *) addr)[512 * i]) < 0);
}

typedef struct 
{
	void *dest;
	u32 size;
	void (* jumpTo)();
	u32 lastSum;
	uint8_t data[0xF30];
} IPLBlockHeader;

int main()
{
	// Turn off the lcd
	_sw(_lw(0xBC800110) & ~3, 0xBC800110);
	_sw(0, 0xBC800100);
	_sw(_lw(0xBE140000) & ~1, 0xBE140000);
	_sw(_lw(0xbc100078) & ~0x2040, 0xbc100078);
	_sw(_lw(0xbc100058) & ~0xc0000, 0xbc100058);
	_sw(_lw(0xbc100050) & ~0x10, 0xbc100050);
	_sw(_lw(0xbc10003c) & ~1, 0xbc10003c);

	sceSysconInit();

	// power off devices
	sceSysconCtrlHRPower(0);
	sceSysconCtrlNandPower(0);
	sceSysconResetDevice(0x82);
	sceSysconCtrlLeptonPower(0);
	sceSysconCtrlMsPower(0);

	// reset memory permissions
	_sw(0xffffffff, 0xbc000000);
	_sw(0xffffffff, 0xbc000004);
	_sw(0xffffffff, 0xbc000008);
	_sw(0xffffffff, 0xbc00000c);
	_sw(0x003f3fff, 0xbc000030);
	_sw(0xf003ffff, 0xbc000034);
	_sw(0x0f3f0f0f, 0xbc000038);
	_sw(0xffffffff, 0xbc00003c);
	_sw(0x000003ff, 0xbc000040);
	_sw(0x01ff00ff, 0xbc000044);
	_sw(0x01000244, 0xbc000050);

	// SYSCON SPI enable
	SYSREG_CLK2_ENABLE_REG |= 0x02;
	REG32(0xbc10007c) |= 0xc8;
	__asm("sync");
	
	sceSysconCtrlHRPower(1);
	uart_init();
	
	_putchar('H');
	_putchar('i');
	_putchar('\n');
	
	sceSysconCtrlMsPower(1);
	pspMsInit();
	KirkReset();

	// The first block is the reset exploit, so we jump over that
	for (int i = 1; ; i++)
	{
		MsLoadBlock(i, 0xBFC00000);
		if (KirkCmd1(0xBFC00000, 0xBFC00000) > 0)
			break;

		IPLBlockHeader *ipl = (IPLBlockHeader *) 0xBFC00000;
		
		if (ipl->dest)
			memcpy(ipl->dest, ipl->data, ipl->size);
		
		if (ipl->jumpTo)
		{
			Dcache();
			Icache();
			_putchar('G');
			_putchar('o');
			_putchar('\n');

			ipl->jumpTo();
		}
	}
	
	while(1);
}
