#include "lflash.h"

#include <emcsm.h>
#include <model.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
    size_t unk0;
    size_t num_blocks;
    size_t num_segments;
    size_t num_physical_block_per_segment;
    size_t num_logical_block_per_segment;
    size_t num_logical_blocks;
} SmartMediaProperties;

// if you change ANY of the values here then please ensure to update
// the constants below.
static const SmartMediaProperties g_sm_flash_properties[] = {
    { 0x0800, 0x7C0, 0x4, 0x1F0, 0x1E0, 0x780 },
    { 0x1000, 0xFC0, 0x8, 0x1F8, 0x1E0, 0xF00 },
    { 0x1000, 0xF80, 0x8, 0x1F0, 0x1E0, 0xF00 }
};

#define MAX_NUM_SEGMENTS        			(8)
#define MAX_NUM_LOGICAL_BLOCKS_PER_SEGMENT    (0x1E0)
#define MAX_NUM_UNUSED_BLOCKS_PER_SEGMENT    (0x18)

#define INVALID_PBN        					(0xFFFFFFFF)
#define INVALID_LBN        					(0xFFFFFFFF)
#define FLASH_AREA_START_BLOCK        		(0x40)

// TODO: these dont belong here!
#define EMCSM_NUM_PAGE_PER_BLOCK        	(0x20)
#define EMCSM_BLOCK_SIZE        			(0x20 * 0x200)

typedef struct
{
    uint8_t status;
    uint8_t chs_first_addr[3];
    uint8_t type;
    uint8_t chs_last_addr[3];
    uint32_t lba;
    uint32_t num_sectors;
} __attribute__((packed)) PartitionEntry;


#define MAX_PARTITIONS    (16)

typedef struct UnusedBlockNode
{
    struct UnusedBlockNode *next;
    size_t pbn;
} UnusedBlockNode;

typedef struct
{
    UnusedBlockNode *top;
    UnusedBlockNode *bottom;
} UnusedBlockList;

typedef struct
{
    size_t lbn_start, lbn_end;
    size_t pbn_start, pbn_end;
    size_t pbn_lookup[MAX_NUM_LOGICAL_BLOCKS_PER_SEGMENT];
    UnusedBlockList unused_blocks;
    UnusedBlockList unallocated_list;
    UnusedBlockNode unused_block_nodes[MAX_NUM_UNUSED_BLOCKS_PER_SEGMENT];
} Segment;

typedef struct
{
    int type;
    size_t lba;
    size_t num_sectors;
    uint32_t scrambles[2];
} PartitionInfo;

typedef struct
{
    size_t page_size;
    size_t pages_per_block;
    const SmartMediaProperties *properties;
    Segment segments[MAX_NUM_SEGMENTS];
    PartitionInfo part_info[MAX_PARTITIONS];
    size_t num_partitions;
    size_t cache_lbn, cache_pbn;
    uint8_t block_buffer[EMCSM_BLOCK_SIZE] __attribute__( ( aligned ( 64 ) ) ) ;
    EmcSmBlockMetadata spare_buffer[EMCSM_NUM_PAGE_PER_BLOCK] __attribute__( ( aligned ( 64 ) ) ) ;
} FlashTranslationLayer;

enum LflashError read_sector(FlashTranslationLayer *ftl, size_t sector, void *);
enum LflashError sync_write(FlashTranslationLayer *ftl);

FlashTranslationLayer g_ftl;

static size_t read_lbn(const EmcSmBlockMetadata *metadata)
{
    uint16_t lbn = ((size_t)metadata->lbn[0] << 8) | (metadata->lbn[1]);
    return lbn != 0xFFFF ? (lbn) : (INVALID_LBN);
}

enum LflashError map_emcsm_err(enum EmcSmError res)
{
    switch (res) {
        case EMCSM_ERR_NONE:
        	return LFLASH_ERR_NONE;
        case EMCSM_ERR_DATA_ECC:
        	return LFLASH_ERR_DATA_ECC;
        case EMCSM_ERR_SPARE_ECC:
        	return LFLASH_ERR_SPARE_ECC;
        default:
        	return LFLASH_ERR_EMCSM_ERR;
    }

    // cant reach here
    return LFLASH_ERR_EMCSM_ERR;
}

enum LflashError read_extra_meta(size_t ppn, EmcSmBlockMetadata *meta, size_t num_pages)
{
    EmcSmBlockMetadataWithEcc meta_with_ecc;
    int res = emcsm_read_extra_only(ppn, &meta_with_ecc, num_pages);
    memcpy(meta, &meta_with_ecc.metadata, sizeof(*meta));
    return map_emcsm_err(emcsm_int_to_err(res));
}

size_t lpn_to_lbn(FlashTranslationLayer *ftl, size_t lpn)
{
    return lpn / ftl->pages_per_block;
}

size_t lbn_to_pbn(FlashTranslationLayer *ftl, size_t lbn)
{
    size_t seg_num = lbn / ftl->properties->num_logical_block_per_segment;

    if (seg_num >= ftl->properties->num_segments) {
        return INVALID_PBN;
    }

    return ftl->segments[seg_num].pbn_lookup[lbn - ftl->segments[seg_num].lbn_start];
}

static inline void map_lbn_to_pbn(Segment *segment, size_t lbn, size_t pbn)
{
    segment->pbn_lookup[lbn - segment->lbn_start] = pbn;
}

static inline size_t pbn_to_flash_ppn(FlashTranslationLayer *ftl, size_t pbn)
{
    size_t ppn = pbn * ftl->pages_per_block;
    size_t ppn_base = FLASH_AREA_START_BLOCK * ftl->pages_per_block;
    return ppn_base + ppn;
}

void add_block_to_unused_list(FlashTranslationLayer *ftl, Segment *segment, size_t pbn)
{
    if (pbn < segment->pbn_start || pbn >= segment->pbn_end) {
        return;
    }

    if (emcsm_erase_block(pbn_to_flash_ppn(ftl, pbn)) < 0) {
        return;
    }

    UnusedBlockNode *node = segment->unallocated_list.top;

    if (!node) {
        // can this happen?
        return;
    }

    // detach from the first chain, and update the new top of the list
    segment->unallocated_list.top = node->next;
    node->pbn = pbn;
    node->next = NULL;

    // if we exhausted this unallocated list then set the bottom to the same
    if (!segment->unallocated_list.top) {
        segment->unallocated_list.bottom = NULL;
    }

    // add to the bottom of the unused_blocks list
    UnusedBlockNode *list_bot = segment->unused_blocks.bottom;

    if (list_bot) {
        // we're essentially making a queue. add to the bottom of the
        // list
        list_bot->next = node;
        segment->unused_blocks.bottom = node;
    }
    else {
        // if there is no bottom of the list then set it to the root node
        segment->unused_blocks.top = node;
        segment->unused_blocks.bottom = node;
    }
}

size_t pop_unused_block(FlashTranslationLayer *ftl, Segment *segment)
{
    if (!segment->unused_blocks.top) {
        return INVALID_PBN;
    }

    UnusedBlockNode *node = segment->unused_blocks.top;

    // unlink node
    size_t pbn = node->pbn;
    segment->unused_blocks.top = node->next;

    // if there are no more blocks then we must also update
    // our bottom pointer, else it will be dangling (they were
    // pointing to the same node)
    if (!segment->unused_blocks.top) {
        segment->unused_blocks.bottom = NULL;
    }

    // clean node, and add it to BOTTOM of the free list. we
    // want to maximise the amount of different nodes used to prevent
    // deterioration of the NAND
    node->next = NULL;
    node->pbn = INVALID_PBN;

    UnusedBlockNode *last_free = segment->unallocated_list.bottom;

    if (last_free) {
        last_free->next = node;
        segment->unallocated_list.bottom = node;
    }
    else {
        segment->unallocated_list.top = node;
        segment->unallocated_list.bottom = node;
    }

    return pbn;
}

void init_segment(FlashTranslationLayer *ftl, Segment *segment, size_t num)
{
    segment->lbn_start = ftl->properties->num_logical_block_per_segment * num;
    segment->lbn_end = segment->lbn_start + ftl->properties->num_logical_block_per_segment;
    segment->pbn_start = ftl->properties->num_physical_block_per_segment * num;
    segment->pbn_end = segment->pbn_start + ftl->properties->num_physical_block_per_segment;

    for (size_t i = 0; i < ftl->properties->num_logical_block_per_segment; ++i) {
        segment->pbn_lookup[i] = INVALID_PBN;
    }

    // populate our dumb allocator. have every node point to the next
    // one (as in a linked-list) and add to the unallocated node lists
    UnusedBlockNode *last_unallocated_node = NULL;
    for (size_t i = 0; i < MAX_NUM_UNUSED_BLOCKS_PER_SEGMENT; ++i) {
        segment->unused_block_nodes[i].next = NULL;
        segment->unused_block_nodes[i].pbn = INVALID_PBN;

        if (last_unallocated_node) {
        	last_unallocated_node->next = &segment->unused_block_nodes[i];
        }

        last_unallocated_node = &segment->unused_block_nodes[i];
    }

    // we currently have no unused blocks and a full unallocated list
    segment->unallocated_list.top = &segment->unused_block_nodes[0];
    segment->unallocated_list.bottom = last_unallocated_node;
    segment->unused_blocks.top = NULL;
    segment->unused_blocks.bottom = NULL;

    // don't use a scramble for these operations
    emcsm_set_scramble(0);

    for (size_t pbn = segment->pbn_start; pbn < segment->pbn_end; ++pbn) {
        if (emcsm_is_bad_block((FLASH_AREA_START_BLOCK + pbn) * ftl->pages_per_block)) {
        	continue;
        }

        EmcSmBlockMetadata meta;
        if (read_extra_meta((FLASH_AREA_START_BLOCK + pbn) * ftl->pages_per_block, &meta, 1) < 0) {
        	continue;
        }

        if ((meta.flags & 0x80) == 0) {
        	continue;
        }

        if ((meta.flags & 0x60) != 0x60) {
        	continue;
        }

        size_t lbn = read_lbn(&meta);

        if (lbn == INVALID_LBN || lbn < segment->lbn_start || lbn >= segment->lbn_end) {
        	add_block_to_unused_list(ftl, segment, pbn);
        	continue;
        }

        size_t other_pbn = segment->pbn_lookup[lbn - segment->lbn_start];

        if (other_pbn != INVALID_PBN) {
        	// TODO: should we ignore errors here?
        	EmcSmBlockMetadata other_meta;
        	read_extra_meta(pbn_to_flash_ppn(ftl, other_pbn), &other_meta, 1);

        	if ((meta.flags & 0x10) == (other_meta.flags & 0x10))
        	{
        		if ((meta.flags & 0x10) == 0) {
        			add_block_to_unused_list(ftl, segment, other_pbn);
        		}
        		else {
        			add_block_to_unused_list(ftl, segment, pbn);
        			continue;
        		}
        	}

        	else if (other_pbn <= pbn) {
        		add_block_to_unused_list(ftl, segment, other_pbn);
        	}
        	else {
        		add_block_to_unused_list(ftl, segment, pbn);
        		continue;
        	}
        }

        // success, map the lbn to this pbn
        map_lbn_to_pbn(segment, lbn, pbn);
    }
}

static const SmartMediaProperties *get_properties(void)
{
    if (model_get_identity()->model == PSP_MODEL_01G) {
        return &g_sm_flash_properties[0];
    }
    else {
        return &g_sm_flash_properties[1];
    }
}

void init_ftl(FlashTranslationLayer *ftl)
{
    // TODO: derive this from somewhere? held assumption?
    ftl->page_size = 0x200;
    ftl->pages_per_block = 32;
    ftl->properties = get_properties();
    ftl->cache_lbn = INVALID_LBN;
    ftl->cache_pbn = INVALID_PBN;

    for (size_t i = 0; i < ftl->properties->num_segments; ++i) {
        init_segment(ftl, &ftl->segments[i], i);
    }
}

static uint32_t bitwise_rotate32(uint32_t value, int shift)
{
    uint32_t mask = 0x1F;
    shift &= mask;
    return (value >> shift) | (value << (32 - shift));
}

static void gen_partition_scrambles(FlashTranslationLayer *ftl, PartitionInfo *part_info, size_t part_num)
{
    // TODO: implement
    uint32_t fuse_id[2] = { *(volatile uint32_t *)0xBC100090, *(volatile uint32_t *)0xBC100094 };
    uint32_t rotate1 = (part_num * 3) % 32;
    uint32_t rotate2 = (32 - (part_num * 2)) % 32;

    uint32_t scrambles[2];
    scrambles[0] = fuse_id[0] ^ bitwise_rotate32(fuse_id[1], rotate2);
    scrambles[1] = 0;

    if (!scrambles[0]) {
        scrambles[0] = bitwise_rotate32(0xC4536DE6, 32 - part_num);
    }

    // TODO: this is some global setting passed into the lflash driver.
    // investigate
    int g_TODO_global_flags = 0b1101;
    if (g_TODO_global_flags & (1 << (part_num % 32))) {
        if (part_num != 3) {	
        	// TODO: constants
        	scrambles[1] = fuse_id[0] ^ bitwise_rotate32(fuse_id[1], rotate1) ^ 0x556D81FE;

        	if (!scrambles[0]) {
        		scrambles[0] = bitwise_rotate32(0xC4536DE6, rotate2) + bitwise_rotate32(0xC543DE42, 32 - rotate2);
        	}

        	if (!scrambles[1]) {
        		scrambles[1] = bitwise_rotate32(0x556D81FE, rotate1);
        	}
        }
        else {
        	scrambles[1] = 0x3C22812A;
        }
    }

    if (scrambles[1]) {
        part_info->scrambles[0] = scrambles[0];
        part_info->scrambles[1] = scrambles[1];
    }
    else {
        part_info->scrambles[0] = 0;
        part_info->scrambles[1] = 0;
    }
}

void read_partitions(FlashTranslationLayer *ftl)
{
    uint8_t sector[0x200];
    enum LflashError res = read_sector(ftl, 0, sector);
    ftl->num_partitions = 0;

    if (res != LFLASH_ERR_NONE) {
        return;
    }

    uint16_t signature = ((uint16_t)sector[0x1FF] << 8) | sector[0x1FE];

    if (signature != 0xAA55) {
        return;
    }

    // TODO: check if FAT partition
    PartitionEntry entries[4];
    memcpy(entries, &sector[0x1BE], sizeof(entries));
    size_t part_num = 0;

    for (size_t i = 0; i < sizeof(entries)/sizeof(*entries) && part_num < MAX_PARTITIONS; ++i) {
        PartitionEntry *part = &entries[i];

        if (!part->type) {
        	continue;
        }

        // check if EBR
        if (part->type == 5 || part->type == 0xF) {
        	size_t ebr_lba = 0;
        	while (read_sector(ftl, part->lba + ebr_lba, sector) == LFLASH_ERR_NONE) {
        		signature = ((uint16_t)sector[0x1FF] << 8) | sector[0x1FE];

        		if (signature != 0xAA55) {
        			break;
        		}

        		PartitionEntry *ebr_parts = (PartitionEntry *)&sector[0x1BE];

        		if (ebr_parts[0].type == 0 || ebr_parts[0].type == 0x5 || ebr_parts[0].type == 0xF) {
        			break;
        		}

        		ftl->part_info[part_num].type = ebr_parts[0].type;
        		ftl->part_info[part_num].lba = part->lba + ebr_lba + ebr_parts[0].lba;
        		ftl->part_info[part_num].num_sectors = ebr_parts[0].num_sectors;
        		gen_partition_scrambles(ftl, &ftl->part_info[part_num], part_num);

        		part_num += 1;

        		if (part_num >= MAX_PARTITIONS) {
        			break;
        		}

        		if (ebr_parts[1].type != 5 && ebr_parts[1].type != 0xF) {
        			break;
        		}

        		ebr_lba = ebr_parts[1].lba;
        	}
        }
        else {
        	ftl->part_info[part_num].type = part->type;
        	ftl->part_info[part_num].lba = part->lba;
        	ftl->part_info[part_num].num_sectors = part->num_sectors;
        	gen_partition_scrambles(ftl, &ftl->part_info[part_num], part_num);
        	part_num += 1;
        }
    }

    ftl->num_partitions = part_num;
}

uint32_t read_scramble(FlashTranslationLayer *ftl, size_t sector)
{
    for (size_t i = 0; i < ftl->num_partitions; ++i) {
        if (ftl->part_info[i].lba > sector) {
        	continue;
        }

        if (sector < ftl->part_info[i].lba + ftl->pages_per_block) {
        	return ftl->part_info[i].scrambles[0];
        }

        if (sector < ftl->part_info[i].lba + ftl->part_info[i].num_sectors) {
        	return ftl->part_info[i].scrambles[1];
        }
    }

    return 0;
}

enum LflashError read_sector(FlashTranslationLayer *ftl, size_t sector, void *buff)
{
    enum LflashError err = sync_write(ftl);

    if (err != LFLASH_ERR_NONE) {
        return err;
    }

    size_t pbn = lbn_to_pbn(ftl, lpn_to_lbn(ftl, sector));
    size_t ppn = pbn_to_flash_ppn(ftl, pbn) + (sector % ftl->pages_per_block);

    uint32_t scramble = read_scramble(ftl, sector);
    emcsm_set_scramble(scramble);

    int res = emcsm_read_pages(ppn, buff, NULL, 1);

    if (res < 0) {
        return map_emcsm_err(emcsm_int_to_err(res));
    }

    return LFLASH_ERR_NONE;
}

enum LflashError read_pbn_spare_data(FlashTranslationLayer *ftl, size_t pbn, uint32_t scramble)
{
    if (scramble) {
        emcsm_set_scramble(scramble);
    }

    int res = emcsm_read_block_with_retry(pbn_to_flash_ppn(ftl, pbn), ftl->block_buffer, ftl->spare_buffer);
    return map_emcsm_err(emcsm_int_to_err(res));
}

void spare_set_active_flag(EmcSmBlockMetadata *spare)
{
    spare->flags |= 0x80;
}

void spare_set_mapped_flag(EmcSmBlockMetadata *spare)
{
    spare->flags |= 0x60;
}

void spare_set_inuse_flag(EmcSmBlockMetadata *spare)
{
    spare->flags |= 0x10;
}

void spare_mark_block_valid(EmcSmBlockMetadata *spare)
{
    spare->unkb = 0xFF;
}

void spare_set_lbn(EmcSmBlockMetadata *spare, size_t lbn)
{
    spare->lbn[0] = (uint8_t)(lbn >> 8);
    spare->lbn[1] = (uint8_t)(lbn);
}

enum LflashError write_new_block(FlashTranslationLayer *ftl, size_t pbn, size_t lbn)
{
    for (size_t page = 0; page < ftl->pages_per_block; ++page) {
        spare_set_active_flag(&ftl->spare_buffer[page]);
        spare_set_mapped_flag(&ftl->spare_buffer[page]);
        spare_set_inuse_flag(&ftl->spare_buffer[page]);
        spare_set_lbn(&ftl->spare_buffer[page], lbn);
    }

    int res = emcsm_write_block_with_verify(pbn_to_flash_ppn(ftl, pbn), ftl->block_buffer, ftl->spare_buffer);

    if (res < 0) {
        return map_emcsm_err(emcsm_int_to_err(res));
    }

    return LFLASH_ERR_NONE;
}

enum LflashError sync_write(FlashTranslationLayer *ftl)
{
    if (ftl->cache_lbn == INVALID_LBN) {
        return LFLASH_ERR_NONE;
    }

    size_t lbn = ftl->cache_lbn;
    size_t pbn = ftl->cache_pbn;

    uint32_t scramble = read_scramble(ftl, lbn * ftl->pages_per_block);
    emcsm_set_scramble(scramble);

    EmcSmBlockMetadata spare;
    memset(&spare, 0xFF, sizeof(spare));
    spare.flags &= ~0x10;

    int res = emcsm_write_pages_raw_extra(pbn_to_flash_ppn(ftl, pbn), NULL, &spare, 1);

    if (res < 0) {
        return map_emcsm_err(emcsm_int_to_err(res));
    }

    size_t seg_num = pbn / ftl->properties->num_physical_block_per_segment;

    if (seg_num >= ftl->properties->num_segments) {
        return LFLASH_ERR_INVALID_SEGMENT;
    }
    
    Segment *segment = &ftl->segments[seg_num];

    size_t new_pbn = INVALID_PBN;

    while (1) {
        // get a new block to write all the data to. we will erase and add the old
        // one back into the pool.
        new_pbn = pop_unused_block(ftl, segment);

        if (new_pbn == INVALID_PBN) {
        	return LFLASH_ERR_BLOCK_EXHAUSTION;
        }

        // attempt to erase this new block. if we fail just try a new block
        if (emcsm_erase_block(pbn_to_flash_ppn(ftl, new_pbn)) < 0) {
        	continue;
        }

        // success, this block is probably good. try to write the data. if
        // we fail then we try another block
        if (write_new_block(ftl, new_pbn, lbn) == LFLASH_ERR_NONE) {
        	break;
        }
    }

    map_lbn_to_pbn(segment, lbn, new_pbn);
    add_block_to_unused_list(ftl, segment, pbn);
    ftl->cache_lbn = INVALID_LBN;
    ftl->cache_pbn = INVALID_PBN;
    return LFLASH_ERR_NONE;
}

enum LflashError write_sectors(FlashTranslationLayer *ftl, size_t sector, const void *buff, size_t num_sectors, size_t *num_written_sectors)
{
    uint32_t scramble = read_scramble(ftl, sector);
    emcsm_set_scramble(scramble);

    size_t lbn = lpn_to_lbn(ftl, sector);
    size_t pbn = lbn_to_pbn(ftl, lbn);

    if (ftl->cache_lbn != lbn) {
        sync_write(ftl);

        if (pbn == INVALID_PBN) {
        	return LFLASH_ERR_INVALID_PBN;
        }

        enum LflashError err = read_pbn_spare_data(ftl, pbn, scramble);

        // ignore all ECC errors
        if (err == LFLASH_ERR_DATA_ECC || err == LFLASH_ERR_SPARE_ECC) {
        	err = LFLASH_ERR_NONE;
        }

        if (err != LFLASH_ERR_NONE) {
        	return err;
        }

        ftl->cache_lbn = lbn;
        ftl->cache_pbn = pbn;
    }

    size_t max_possible_copy = ftl->pages_per_block - (sector % ftl->pages_per_block);
    size_t copylen = num_sectors > max_possible_copy ? (max_possible_copy) : (num_sectors);

    memcpy(ftl->block_buffer + (sector % ftl->pages_per_block) * ftl->page_size, buff, copylen * ftl->page_size);

    if (num_written_sectors) {
        *num_written_sectors = copylen;
    }

    return LFLASH_ERR_NONE;
}

enum LflashError lflash_init(void)
{
    emcsm_init();
    emcsm_set_write_protect(EMCSM_DISABLE_WRITE_PROTECT);
    init_ftl(&g_ftl);
    read_partitions(&g_ftl);
    return LFLASH_ERR_NONE;
}

enum LflashError lflash_read_sector(size_t sector, void *buff)
{
    return read_sector(&g_ftl, sector, buff);
}

enum LflashError lflash_write_sectors(size_t sector, const void *buff, size_t num_sectors, size_t *num_written)
{
    return write_sectors(&g_ftl, sector, buff, num_sectors, num_written);
}

size_t lflash_get_sector_count(void)
{
    // TODO: remove this constant. its pages per block...
    const SmartMediaProperties *properties = get_properties();
    return properties->num_blocks * 32;
}

size_t lflash_get_block_size(void)
{
    // TODO: remove this constant!
    return 32;
}

enum LflashError lflash_sync(void)
{
    return sync_write(&g_ftl);
}

size_t lflash_get_size(void)
{
    // TODO: remove this constant. its pages per block...
    const SmartMediaProperties *properties = get_properties();
    return (properties->num_logical_blocks - 2) * 32;
}
