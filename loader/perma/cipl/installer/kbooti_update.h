#ifndef __KBOOTI_UPDATE_H__
#define __KBOOTI_UPDATE_H__

#include <psptypes.h>

/**
 * Gets flash3 kbooti_01g.bin size
 *
 * @returns - the size of kbooti_01g.bin on success, < 0 on error
*/
int pspKbootiUpdateGetKbootiSize();

/**
 * Creates a custom kbooti_01g.bin. The factory kbooti_01g.bin is backed up to flash3:/kbooti_01g.bin.bak
 *
 * @param cIplBlock - buffer with the custom kbooti loader and patcher. Must be 0x5000 bytes.
 *
 * @returns - 1 on success, < 0 on error
*/
int pspKbootiUpdateKbooti(u8 *cIplBlock, u32 cIplBlockSize);

/**
 * Restores original kbooti_01g.bin.
 *
 * @returns - 1 on success, < 0 on error
*/
int pspKbootiUpdateRestoreKbooti();

#endif


