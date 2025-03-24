#include <pspsdk.h>
#include <syscon.h>
#include "gpio.h"
#ifdef DEBUG
#include <uart.h>
#include <printf.h>
#endif

#ifdef MSIPL
#include "ms_payloadex/ms_payloadex.h"
#else
#include "nand_payloadex/nand_payloadex.h"
#endif

void Dcache();
void Icache();

void *memcpy(void *dest, void *src, uint32_t size)
{
    uint32_t *d = (uint32_t *) dest;
    uint32_t *s = (uint32_t *) src;
    
    size /= 4;
    
    while (size--)
        *(d++) = *(s++);

    return dest;
}

int main()
{
#ifdef DEBUG
    uart_init();
    printf("stage2 starting...\n");
#endif

    memcpy((u8 *) 0x8FC0000, &payloadex, size_payloadex);
    
#ifdef MSIPL
    syscon_ctrl_ms_power(1);
#endif
    
    *(u32 *) 0x8FB0000 = -1;
    syscon_issue_command_read(0x07, (u32 *) 0x8FB0000);

    Dcache();
    Icache();

}
