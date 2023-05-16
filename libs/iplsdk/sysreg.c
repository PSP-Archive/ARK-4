#include "sysreg.h"

#define REG32(ADDR) (*(vu32*)(ADDR))
#define SYNC() __asm(" sync; nop"::)

u32 sceSysregSpiClkSelect(int a1,int a2)
{
	u32 shift;
	u32 in,out;

	shift = a1<<2;

	in = REG32(0xbc100064);
	out  = in & ~(7<<shift);
	out |= a2<<shift;
	REG32(0xbc100064) = out;
	return (in>shift) & 7;
}

u32 sceSysregSpiClkEnable(u32 bit)
{
	u32 in , out;
	u32 mask = (1<<bit);
	u32 enable = 1;

	in = SYSREG_CLK2_ENABLE_REG;

	out = (in & (~mask));
	if(enable)
		out |= mask;

	SYSREG_CLK2_ENABLE_REG = out;

	return in & mask;
}

void SysregReset(u32 mask, u32 enable)
{
	if (enable)
		REG32(0xBC10004C) |= mask;
	else
		REG32(0xBC10004C) &= ~mask;
}

void SysregBusclk(u32 mask, u32 enable)
{
	if (enable)
		SYSREG_BUSCLK_ENABLE_REG |= mask;
	else
		SYSREG_BUSCLK_ENABLE_REG &= ~mask;
}

void SysregResetKirkEnable()
{
	SysregReset(0x400, 1);
}

void SysregBusclkKirkEnable()
{
	SysregBusclk(0x80, 1);
}

void SysregBusclkKirkDisable()
{
	SysregBusclk(0x80, 0);
}

void SysregResetKirkDisable()
{
	SysregReset(0x400, 0);
}
