#include <pspsdk.h>
#include <string.h>
#include <ark.h>

#include "sysreg.h"
#include "kirk.h"
#include "syscon.h"
#include "gpio.h"

#ifdef DEBUG
#include "printf.h"
#include "uart.h"
#endif

#define JAL_OPCODE    0x0C000000
#define J_OPCODE    0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

#if IPL_01G

#define ERASE_RAM_START 0x40ec0d8
#define RESET_VECTOR_ADDRESS 0x40ec12c
#define PRE_STAGE2_ADDR 0x40ec2bc
#define ROM_HMAC_ADDRESS 0x40ed000

#define STAGE2_PATCH_INJECTION_ADDRESS 0x4000364
#define CLEAR_STRACHPAD_ADDRESS 0x4001120
#define ENTRY_POINT_ADDRESS 0x400cf18

#elif IPL_02G

#define ERASE_RAM_START 0x40ec0d8
#define RESET_VECTOR_ADDRESS 0x40ec12c
#define PRE_STAGE2_ADDR 0x40ec2bc
#define ROM_HMAC_ADDRESS 0x40ed000

#define STAGE2_PATCH_INJECTION_ADDRESS 0x4000364
#define CLEAR_STRACHPAD_ADDRESS 0x4001210
#define ENTRY_POINT_ADDRESS 0x400DC98
#define SET_SEED_ADDRESS 0x4001160

#elif IPL_03G

#define ERASE_RAM_START 0x40ec0d8
#define RESET_VECTOR_ADDRESS 0x40ec12c
#define PRE_STAGE2_ADDR 0x40ec2bc
#define ROM_HMAC_ADDRESS 0x40ed000

#define STAGE2_PATCH_INJECTION_ADDRESS 0x4000364
#define CLEAR_STRACHPAD_ADDRESS 0x4001244
#define ENTRY_POINT_ADDRESS 0x400DC98
#define SET_SEED_ADDRESS 0x4001194

#elif IPL_04G || IPL_07G || IPL_09G

#define ERASE_RAM_START 0x40ec0d8
#define RESET_VECTOR_ADDRESS 0x40ec12c
#define PRE_STAGE2_ADDR 0x40ec2bc
#define ROM_HMAC_ADDRESS 0x40ed000

#define STAGE2_PATCH_INJECTION_ADDRESS 0x4000364
#define CLEAR_STRACHPAD_ADDRESS 0x400122C
#define ENTRY_POINT_ADDRESS 0x400DC98
#define SET_SEED_ADDRESS 0x400117C

#elif IPL_05G

#define ERASE_RAM_START 0x40ec0d8
#define RESET_VECTOR_ADDRESS 0x40ec12c
#define PRE_STAGE2_ADDR 0x40ec2bc
#define ROM_HMAC_ADDRESS 0x40ed000

#define STAGE2_PATCH_INJECTION_ADDRESS 0x4000364
#define CLEAR_STRACHPAD_ADDRESS 0x400122C
#define ENTRY_POINT_ADDRESS 0x4018B08
#define SET_SEED_ADDRESS 0x400117C

#elif IPL_11G

#define ERASE_RAM_START 0x40ec0d8
#define RESET_VECTOR_ADDRESS 0x40ec12c
#define PRE_STAGE2_ADDR 0x40ec2bc
#define ROM_HMAC_ADDRESS 0x40ed000

#define STAGE2_PATCH_INJECTION_ADDRESS 0x4000364
#define CLEAR_STRACHPAD_ADDRESS 0x400122C
#define ENTRY_POINT_ADDRESS 0x400DD18
#define SET_SEED_ADDRESS 0x400117C

#else

#error Define IPL

#endif

#include "ipl_stage2_payload.h"

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

u8 rand_xor[] =
{
#if IPL_02G
    0x61, 0x7A, 0x56, 0x42, 0xF8, 0xED, 0xC5, 0xE4, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B
#elif IPL_03G
    0x61, 0x7A, 0x56, 0x42, 0xF8, 0xED, 0xC5, 0xE4, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25
#elif IPL_05G
    0x61, 0x7A, 0x56, 0x42, 0xF8, 0xED, 0xC5, 0xE4, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23
#elif IPL_04G || IPL_07G || IPL_09G || IPL_11G
    0x8D, 0x5D, 0xA6, 0x08, 0xF2, 0xBB, 0xC6, 0xCC, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23
#endif
};

u8 key_86[] =
{ // scidx
#if IPL_02G
    0x51, 0x00, 0x7C, 0x1F, 0xA4, 0x89, 0x27, 0xBA, 0x9D, 0x71, 0xD9, 0x6B, 0xDB, 0xE7, 0x5B, 0xAB
#elif IPL_03G
    0xFB, 0xC6, 0x64, 0xCB, 0xEC, 0xD6, 0xB1, 0x1C, 0xC2, 0xFE, 0x0D, 0x6C, 0x43, 0xC5, 0x6D, 0x39
#elif IPL_04G || IPL_07G || IPL_09G || IPL_11G
    0xD5, 0x96, 0x55, 0x56, 0xB9, 0x39, 0xD8, 0x9D, 0x6E, 0x79, 0xD3, 0x8C, 0x88, 0x7B, 0xF3, 0x0A
#elif IPL_05G
    0xF3, 0x58, 0x22, 0xBA, 0x99, 0x4F, 0x86, 0x93, 0x6A, 0xFF, 0xB7, 0x05, 0x5D, 0xDD, 0x14, 0xFC
#endif
};

void xor(u8 *dest, u8 *src_a, u8 *src_b)
{
    for (int i = 0; i < 16; i++)
        dest[i] = src_a[i] ^ src_b[i];
}

int seed_gen1(u8 *random_key, u8 *random_key_dec_resp_dec)
{
    memset(random_key, 0xAA, 16);
    
    u8 random_key_dec[16];
    int ret = kirk_decrypt_aes(random_key_dec, random_key, 16, 0x69);
    if (ret)
        return ret;
    
    ret = syscon_send_auth(0x80, random_key_dec);
    if (ret)
        return ret;
    
    u8 random_key_dec_resp[16];
    ret = syscon_recv_auth(0, random_key_dec_resp);
    if (ret)
        return ret;
    
    ret = kirk_decrypt_aes(random_key_dec_resp_dec, random_key_dec_resp, 16, 0x14);
    if (ret)
        return ret;
        
    u8 random_key_dec_resp_dec_swapped[16];
    memcpy(random_key_dec_resp_dec_swapped, &random_key_dec_resp_dec[8], 8);
    memcpy(&random_key_dec_resp_dec_swapped[8], random_key_dec_resp_dec, 8);
    
    u8 seed_dec_resp_dec_hi_low_swapped_dec[16];
    ret = kirk_decrypt_aes(seed_dec_resp_dec_hi_low_swapped_dec, random_key_dec_resp_dec_swapped, 16, 0x69);
    if (ret)
        return ret;

    ret = syscon_send_auth(0x82, seed_dec_resp_dec_hi_low_swapped_dec);
    if (ret)
        return ret;

    return 0;
}

int seed_gen2(u8 *rand_xor, u8 *key_86, u8 *random_key, u8 *random_key_dec_resp_dec)
{
    u8 random_key_xored[16];
    xor(random_key_xored, random_key, rand_xor);
    
    int ret = kirk_decrypt_aes(random_key_xored, random_key_xored, 16, 0x15);
    if (ret)
        return ret;
        
    u8 random_key_dec_resp_dec_xored[16];
    xor(random_key_dec_resp_dec_xored, random_key_dec_resp_dec, random_key_xored);
    
    ret = syscon_send_auth(0x84, random_key_dec_resp_dec_xored);
    if (ret)
        return ret;
        
    ret = syscon_send_auth(0x86, key_86);
    if (ret)
        return ret;

    u8 resp_2[16];
    ret = syscon_recv_auth(2, resp_2);
    if (ret)
        return ret;

    u8 resp_4[16];
    ret = syscon_recv_auth(4, resp_4);
    if (ret)
        return ret;
    
    return 0;
}

int unlockSyscon()
{
    kirk_hwreset();

    kirkF(0xBFC00C00);
    
    u8 random_key[16];
    u8 random_key_dec_resp_dec[16];
    
    int ret = seed_gen1(random_key, random_key_dec_resp_dec);
    if (ret)
        return ret;
    
    ret = seed_gen2(rand_xor, key_86, random_key, random_key_dec_resp_dec);
    if (ret)
        return ret;
    
    kirk_hwreset();

    return 0;
}

#ifdef SET_SEED_ADDRESS
u8 seed_xor[] =
{ // scxor
#if IPL_02G
    0x28, 0x0E, 0xF4, 0x16, 0x45, 0x59, 0xFE, 0x8C, 0xA6, 0x58, 0x25, 0x51, 0xD7, 0x3B, 0x31, 0x4B
#elif IPL_03G
    0x42, 0x75, 0x4C, 0xC6, 0xB6, 0xEA, 0xFE, 0xA1, 0xEB, 0x60, 0x77, 0xDE, 0xA6, 0xC0, 0x37, 0xA0
#elif IPL_04G || IPL_07G || IPL_09G || IPL_11G
    0x5E, 0xEA, 0x7E, 0x3F, 0xA5, 0x23, 0x15, 0xB7, 0xD1, 0x92, 0xFD, 0x3C, 0xF8, 0x2F, 0xC7, 0x52
#elif IPL_05G
    0x35, 0x85, 0x74, 0x2E, 0x7A, 0xDF, 0xF2, 0x01, 0x64, 0x00, 0x3C, 0xF5, 0xFA, 0x38, 0x89, 0x4A
#endif
};

int set_seed(u8 *xor_key, u8 *random_key, u8 *random_key_dec_resp_dec)
{
    for (int i = 0; i < sizeof(seed_xor); i++)
        *(u8 *) (0xBFC00210 + i) ^= seed_xor[i];

    return 0;
}
#endif


u8 rom_hmac[] =
{
#if IPL_01G
    0x27, 0x85, 0xcf, 0x5c, 0xbb, 0xad, 0xf4, 0x2f, 0xff, 0xa5, 0xc4, 0x90, 0xb3, 0xa0, 0xa5, 0x64,
    0xd9, 0x29, 0xdb, 0xe2, 0xdb, 0x07, 0x35, 0x4a, 0xe7, 0x76, 0xcf, 0x51, 0x00, 0x00, 0x00, 0x00
#elif IPL_02G
    0xee, 0x38, 0xdb, 0x54, 0xa2, 0xae, 0xcf, 0x84, 0xfb, 0x25, 0x59, 0xf8, 0xa5, 0x99, 0xa0, 0xcb,
    0xab, 0xdf, 0x92, 0x92, 0x39, 0x9b, 0xa5, 0xff, 0x07, 0x15, 0xb3, 0x5d, 0x00, 0x00, 0x00, 0x00
#elif IPL_03G
    0x48, 0xf5, 0x2f, 0x88, 0x5f, 0xbe, 0x24, 0x3a, 0xd3, 0x19, 0x01, 0x85, 0x20, 0x42, 0xb0, 0x0b,
    0x63, 0x2a, 0x8f, 0x55, 0x17, 0xa5, 0x9d, 0x61, 0xf1, 0xac, 0x0e, 0x15, 0x00, 0x00, 0x00, 0x00
#elif IPL_04G
    0x9e, 0x36, 0xa4, 0x65, 0x02, 0x37, 0xd3, 0x48, 0xf6, 0x97, 0xd9, 0x4a, 0xa5, 0x1f, 0xd7, 0xfb,
    0x75, 0x9b, 0xcd, 0x47, 0xbe, 0x4d, 0x9e, 0xf3, 0x25, 0xee, 0x53, 0xae, 0x00, 0x00, 0x00, 0x00
#elif IPL_05G
    0x81, 0x1e, 0xe1, 0x6b, 0xac, 0x16, 0xe6, 0x2b, 0xd3, 0xc7, 0xb3, 0xd1, 0x1e, 0xb0, 0xc7, 0xd1,
    0xa6, 0x57, 0xa6, 0x28, 0x9f, 0x26, 0x67, 0x16, 0xba, 0xc4, 0x03, 0xda, 0x00, 0x00, 0x00, 0x00
#elif IPL_07G
    0x33, 0xba, 0xeb, 0x9e, 0xcf, 0xef, 0xe3, 0x7d, 0x9c, 0x18, 0x8c, 0xe3, 0x26, 0x47, 0x97, 0x61,
    0x35, 0x30, 0xca, 0x75, 0xb9, 0x66, 0xca, 0xcd, 0x35, 0x4e, 0x09, 0xfc, 0x00, 0x00, 0x00, 0x00
#elif IPL_09G
    0x22, 0x31, 0x19, 0x87, 0x88, 0xc3, 0xe8, 0x39, 0xab, 0x9c, 0xfa, 0x9b, 0x58, 0x90, 0x9a, 0x97,
    0x4c, 0x70, 0x12, 0x3d, 0xd9, 0x8b, 0x52, 0x4d, 0x31, 0x2e, 0x7d, 0x13, 0x00, 0x00, 0x00, 0x00
#elif IPL_11G
    0xd4, 0x3a, 0x09, 0x7f, 0x02, 0x2e, 0xb5, 0x75, 0x3d, 0xbe, 0xc1, 0x20, 0x93, 0x4b, 0x20, 0xc3,
    0xd0, 0xfc, 0x88, 0x97, 0xdc, 0xac, 0x5c, 0xae, 0x56, 0x89, 0xef, 0x1e, 0x00, 0x00, 0x00, 0x00
#endif
};

void sha256hmacPatched(u8 *key, u32 keylen, u8 *data, u32 datalen, u8 *out)
{
    memcpy(out, rom_hmac, sizeof(rom_hmac));
}

void prestage2()
{
    // Copy stage 2 to scratchpad
    memcpy((u8 *) 0x10000, &payload, size_payload);
    
    // Replace call to Dcache with jump to patch2
    MAKE_CALL(STAGE2_PATCH_INJECTION_ADDRESS, 0x10000);

    // Nullify memset to scratch pad
    _sw(0, CLEAR_STRACHPAD_ADDRESS);
    
    // Change payload entrypoint to 0x88fc0000
    _sw(0x3C1988FC, ENTRY_POINT_ADDRESS); // lui t9, 0x88fc

#ifdef SET_SEED_ADDRESS
    MAKE_CALL(SET_SEED_ADDRESS, set_seed);
#endif
    
    Dcache();
    Icache();
    
    // Execute main.bin
    ((void (*)()) 0x4000000)();
}

#if IPL_01G
void boot150()
{
#ifndef MSIPL
    syscon_ctrl_ms_power(1);
#endif

    MsFatMount();

    int res = MsFatOpen("/TM/DCARK/150/msipl.raw");

    if (res == 0){
        MsFatRead(0x40c0000, 0x4000);
        MsFatClose();
        Dcache();
        Icache();
        return ((int (*)()) 0x40c0000)();
    }
}
#endif

int main()
{

#ifndef MSIPL
    syscon_init();
#ifdef SET_SEED_ADDRESS
    unlockSyscon();
    syscon_handshake_unlock();
#endif
#endif

    u32 tachyon_version = syscon_get_tachyon_version();

    if (tachyon_version >= 0x600000)
        _sw(0x20070910, 0xbfc00ffc);
    else if (tachyon_version >= 0x400000)
        _sw(0x20050104, 0xbfc00ffc);
    else
        _sw(0x20040420, 0xbfc00ffc);

#ifndef MSIPL
    uint32_t keys = -1;
    syscon_get_digital_key(&keys);
    if ((keys & SYSCON_CTRL_LTRIGGER) == 0)
    {
        syscon_ctrl_ms_power(1);
    
        MsFatMount();

        int res = MsFatOpen("/TM/DCARK/msipl.raw");

        if (res == 0){
        	MsFatRead(0x40c0000, 0x4000);
        	MsFatClose();
        	Dcache();
        	Icache();
        	return ((int (*)()) 0x40c0000)();
        }
    }
#endif

#if IPL_01G
    int wakeup_factor;
    syscon_get_wakeup_factor(&wakeup_factor);

    if ((wakeup_factor & 0x80) == 0) {

        // If we are not resuming from suspend wipe suspend FW version
        unsigned int data = 0;
        syscon_write_scratchpad(SYSCON_SCRATCHPAD_RESUME_FW_ADDR, &data);
    }
    else {
        unsigned int fw_ver;
        syscon_read_scratchpad(SYSCON_SCRATCHPAD_RESUME_FW_ADDR, &fw_ver);

        if (fw_ver == FW_150)
            boot150();
    }
#endif

    _sw(0, ERASE_RAM_START);

    _sw(0x3c08bfc0, RESET_VECTOR_ADDRESS);
    
    MAKE_JUMP(PRE_STAGE2_ADDR, prestage2);
    _sw(0, PRE_STAGE2_ADDR + 4);

    MAKE_CALL(ROM_HMAC_ADDRESS, sha256hmacPatched);

    Dcache();
    Icache();
    
    ((void (*)())0x40ec000)();
}
