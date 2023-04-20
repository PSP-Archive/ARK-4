#ifndef __LIBPSARDUMPER_H__
#define __LIBPSARDUMPER_H__

/**
 * Init psar decoding 
 *
 * @param dataPSAR - The buffer with the psar data
 * @param dataOut - Buffer that will receive data 1
 * @param dataOut2 - Buffer that will receive data 2
 *
 * @returns 0 on success, < 0 on error
*/
int pspPSARInit(u8 *dataPSAR, u8 *dataOut, u8 *dataOut2);

/**
 * Gets the next psar file
 *
 * @param dataPSAR - The buffer with the psar data
 * @param cbFile - The size of the psar
 * @param dataOut - Buffer for internal temporal use
 * @param dataOut2 - Buffer that receives the file data
 * @param name - Buffer that receives file name
 * @param retSize - Pointer to an integer that receives the file size
 * @param retPos - Pointer to an integer that receives the current position within the psar
 * @param signcheck - Pointer to an integer that receives wether the file should be sign checked or not
 *
 * @returns 0 if there is no more files, 1 if there are more files, < 0 on error
*/
int pspPSARGetNextFile(u8 *dataPSAR, int cbFile, u8 *dataOut, u8 *dataOut2, char *name, int *retSize, int *retPos, int *signcheck);

/**
 * Sets the buffer position
 * 
 * @param position - The position to set
 *
 * @returns always 0
*/
int pspPSARSetBufferPosition(int position);

#endif
