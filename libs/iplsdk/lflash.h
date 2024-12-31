#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdint.h>
#include <stddef.h>

enum LflashError
{
    LFLASH_ERR_NONE = 0,
    LFLASH_ERR_INVALID_PBN,
    LFLASH_ERR_DATA_ECC,
    LFLASH_ERR_SPARE_ECC,
    LFLASH_ERR_EMCSM_ERR,
    LFLASH_ERR_INVALID_SEGMENT,
    LFLASH_ERR_BLOCK_EXHAUSTION
};

enum LflashError lflash_init(void);
enum LflashError lflash_read_sector(size_t sector, void *buff);
enum LflashError lflash_write_sectors(size_t sector, const void *buff, size_t num_sectors, size_t *num_written);
size_t lflash_get_sector_count(void);
size_t lflash_get_block_size(void);
enum LflashError lflash_sync(void);
size_t lflash_get_size(void);

#ifdef __cplusplus
}
#endif //__cplusplus
