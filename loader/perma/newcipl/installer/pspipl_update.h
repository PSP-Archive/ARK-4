#ifndef __PSPIPL_UPDATE_H__
#define __PSPIPL_UPDATE_H__

/**
 * Gets the ipl from nand
 *
 * @param buf - The buffer where the ipl will be stored
 *
 * @returns - the size of ipl on success, < 0 on error
*/
int pspIplUpdateGetIpl(u8 *buf);

/**
 * Clears the nand blocks of the ipl area
 *
 * @returns 0 on success, < 0 on error
*/
int pspIplUpdateClearIpl(void);

/**
 * Sets the nand ipl
 *
 * @param buf - The buffer containing the ipl data
 * @param size - The size of the buffer
 *
 * @returns 0 on success, < 0 on error
*/
int pspIplUpdateSetIpl(u8 *buf, u32 size, u16 key);

#endif


