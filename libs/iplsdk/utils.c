#include "utils.h"

#define REG32(ADDR) (*(volatile unsigned int*)(ADDR))

// void clear_dcache(void)
// {
// }

void clear_dcache(void)
{
	__asm("\
	.word 0x40088000;\
	.word 0x24090800;\
	.word 0x7D081180;\
	.word 0x01094804;\
	.word 0x00004021;\
	.word 0xBD140000;\
	.word 0xBD140000;\
	.word 0x25080040;\
	.word 0x1509FFFC;\
	.word 0x00000000;\
	.word 0x0000000F;\
	.word 0x00000000;\
	"::);
}

void clear_icache(void)
{
    __asm("\
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
}

unsigned int delay_us(unsigned int usec)
{
	unsigned int i = 0;
	volatile unsigned int dmy = 0;

	while(usec--)
	{
		for(i = 0; i < 10; ++i)
		{
			dmy ^= REG32(0xbe240000);
		}
	}
	return dmy;
}