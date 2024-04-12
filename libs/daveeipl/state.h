#pragma once

#include <stdint.h>
#include <stddef.h>

typedef void (* Descrambler)(void *dst, uintptr_t src, size_t len, size_t ppn);

typedef struct
{
    size_t len;
    size_t ppn;
    void *user_dst;
    void *spare_dst;
    unsigned int flags;
    unsigned int status;
} AccessState;

enum ProcessState
{
    PROCESS_STATE_INACTIVE = -1,
    PROCESS_STATE_INACTIVE0 = 0,
    PROCESS_STATE_READ_BUSY,
    PROCESS_STATE_WRITE_BUSY,
    PROCESS_STATE_READ_READY,
    PROCESS_STATE_WRITE_READY,
};

typedef struct
{
    int is_inited;
    size_t num_manufacturer_blocks;
    size_t page_size;
    size_t num_pages_per_block;
    size_t num_total_blocks;
    Descrambler descrambler;
    AccessState access_state;
    enum ProcessState state;
    uint32_t scramble_code;
} EmcsmState;

extern EmcsmState g_emcsm_state;
