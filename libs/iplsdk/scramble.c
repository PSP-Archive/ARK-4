#include "scramble.h"
#include "state.h"

#include <cpu.h>

void scramble(volatile uint32_t *dst, const uint32_t *src, size_t len, size_t ppn)
{
    uint32_t scramble_code = g_emcsm_state.scramble_code;
    uint32_t code = (scramble_code >> 21) | (scramble_code << 11);
    uint32_t key = ((ppn >> 17) | (ppn << 15)) ^ (code * 7);
    size_t offset = ((ppn ^ code) % 32) * 16;

    for (size_t i = 0; i < len; i += 16) {
        const uint32_t *src_32 = (const uint32_t *)((uintptr_t)src + i);
        volatile uint32_t *dst_32 = (volatile uint32_t *)((uintptr_t)dst + offset);

        uint32_t w0 = src_32[0];
        uint32_t w1 = src_32[1];
        uint32_t w2 = src_32[2];
        uint32_t w3 = src_32[3];

        uint32_t sw0 = w0 + key;
        uint32_t sw1 = w1 + sw0;
        uint32_t sw2 = w2 + (sw0 ^ w1);
        uint32_t sw3 = w3 + (sw0 ^ w1) - w2;
    
        dst_32[0] = sw0;
        dst_32[1] = sw1;
        dst_32[2] = sw2;
        dst_32[3] = sw3;
        
        key = cpu_bitrev(sw3 + code);
        offset = (offset + 16) % len;
    }
}

void descramble(void *dst, uintptr_t src, size_t len, size_t ppn)
{
    uint32_t scramble_code = g_emcsm_state.scramble_code;
    uint32_t code = (scramble_code >> 21) | (scramble_code << 11);
    uint32_t key = ((ppn >> 17) | (ppn << 15)) ^ (code * 7);
    size_t offset = ((ppn ^ code) % 32) * 16;

    for (size_t i = 0; i < len; i += 16) {
        uint32_t *dst_32 = (uint32_t *)((uintptr_t)dst + i);
        uint32_t *src_32 = (uint32_t *)((uintptr_t)src + offset);

        uint32_t iVar2 = src_32[1];
        uint32_t iVar5 = src_32[2];
        uint32_t iVar6 = src_32[3];
        uint32_t uVar4 = key + (*src_32 - key);
        *dst_32 = *src_32 - key;
        key = iVar2 - uVar4;
        uVar4 = uVar4 ^ key;
        dst_32[1] = key;
        iVar5 = iVar5 - uVar4;
        iVar2 = uVar4 - iVar5;
        dst_32[2] = iVar5;
        iVar6 = iVar6 - iVar2;
        dst_32[3] = iVar6;
        key = iVar2 + iVar6 + code;
        key = cpu_bitrev(key);
        offset = (offset + 16) % len;
    }
}

void emcsm_set_scramble(uint32_t scramble)
{
    g_emcsm_state.scramble_code = scramble;
}
