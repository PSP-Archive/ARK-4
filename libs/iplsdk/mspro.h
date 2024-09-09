#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define MS_SECTOR_SIZE  (0x200)

int mspro_init(void);
int mspro_read_sector(uint32_t sector, void *data);
int mspro_write_sector(uint32_t sector, const void *data);

#ifdef __cplusplus
}
#endif //__cplusplus
