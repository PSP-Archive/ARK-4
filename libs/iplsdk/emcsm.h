#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t id; // 0xFF for IPL, 0x00 for FAT
    uint8_t validity; // 0xFF for valid
    uint8_t lbn[2]; // logical block number for FAT, special designation for other envs. big endian
    uint32_t unk;
    uint16_t meta_ecc;
    uint8_t flags;
    uint8_t unkb;
} __attribute__((packed)) EmcSmBlockMetadata;

typedef struct
{
    uint32_t user_ecc;
    EmcSmBlockMetadata metadata;
} __attribute__((packed)) EmcSmBlockMetadataWithEcc;


enum EmcSmWriteProtect
{
    EMCSM_DISABLE_WRITE_PROTECT,
    EMCSM_ENABLE_WRITE_PROTECT
};

enum EmcSmError
{
    EMCSM_ERR_NONE = 0,
    EMCSM_ERR_TOO_MANY_PAGES,
    EMCSM_ERR_MULTIPLE_BLOCKS,
    EMCSM_ERR_UNALIGNED,
    EMCSM_ERR_DATA_ECC,
    EMCSM_ERR_SPARE_ECC,
};

static inline int emcsm_err_to_int(enum EmcSmError err)
{
    return -err;
}

static inline enum EmcSmError emcsm_int_to_err(int res)
{
    return -res;
}

void emcsm_init(void);
int emcsm_read_pages(size_t ppn, void *user, void *spare, size_t num_pages);
int emcsm_read_pages_raw_all(size_t ppn, void *user, void *spare, size_t num_pages);
int emcsm_read_extra_only(size_t ppn, void *spare, size_t num_pages);
int emcsm_is_bad_block(size_t pbn);
void emcsm_set_scramble(uint32_t scramble);
int emcsm_read_block_with_retry(size_t ppn, void *user, void *spare);
int emcsm_erase_block(size_t ppn);
int emcsm_write_pages_raw_extra(size_t ppn, const void *user, const void *spare, size_t num_pages);
int emcsm_write_block_with_verify(size_t ppn, const void *user, const void *spare);
void emcsm_set_write_protect(enum EmcSmWriteProtect wp);

void read_id(uint8_t *dst, size_t len);

#ifdef __cplusplus
}
#endif //__cplusplus
