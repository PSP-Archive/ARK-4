/**
 * File for native encrypt/decrypt functions
*/

#include <pspsdk.h>

typedef struct
{
    int mode;    //0
    int unk_4;   //4
    int unk_8;   //8
    int keyseed; //C
    int data_size;   //10
} KIRK_AES128CBC_HEADER; //0x14

typedef struct
{
    u8  AES_key[16];            //0
    u8  CMAC_key[16];           //10
    u8  CMAC_header_hash[16];   //20
    u8  CMAC_data_hash[16];     //30
    u8  unused[32];             //40
    u32 mode;                   //60
    u8  ecdsa_hash;             //64
    u8  unk3[11];               //65
    u32 data_size;              //70
    u32 data_offset;            //74  
    u8  unk4[8];                //78
    u8  unk5[16];               //80
} KIRK_CMD1_HEADER; //0x90

#define KIRK_CMD_DECRYPT_PRIVATE 1
#define KIRK_CMD_2 2
#define KIRK_CMD_3 3
#define KIRK_CMD_ENCRYPT_IV_0 4
#define KIRK_CMD_ENCRYPT_IV_FUSE 5
#define KIRK_CMD_ENCRYPT_IV_USER 6
#define KIRK_CMD_DECRYPT_IV_0 7
#define KIRK_CMD_DECRYPT_IV_FUSE 8
#define KIRK_CMD_DECRYPT_IV_USER 9
#define KIRK_CMD_PRIV_SIGN_CHECK 10
#define KIRK_CMD_SHA1_HASH 11
#define KIRK_CMD_ECDSA_GEN_KEYS 12
#define KIRK_CMD_ECDSA_MULTIPLY_POINT 13
#define KIRK_CMD_PRNG 14
#define KIRK_CMD_15 15
#define KIRK_CMD_ECDSA_SIGN 16
#define KIRK_CMD_ECDSA_VERIFY 17

#define KIRK_MODE_CMD1 1
#define KIRK_MODE_CMD2 2
#define KIRK_MODE_CMD3 3
#define KIRK_MODE_ENCRYPT_CBC 4
#define KIRK_MODE_DECRYPT_CBC 5

/**
 * Sends a command to the KIRK encryption/decryption engine.
 *
 * @param inbuf - The input buffer
 * @param insize - The size of input buffer
 * @param outbuf - The output buffer
 * @param outsize - The size of output buffer
 * @param cmd - The commands to send to KIRK engine.
 *
 * @returns < 0 on error
 */
int sceUtilsBufferCopyWithRange(void *inbuf, SceSize insize, void *outbuf, int outsize, int cmd);

/**
 * Used for PSAR decoding (1.00 bogus)
 *
 * @param buf - The in/out buffer to decode.
 * @param bufsize - The size of the buffer pointed by buf
 * @param retSize - Pointer to an integer that receives the size of 
 * the decoded data.
 * 
 * @returns < 0 on error
*/
int sceNwman_driver_9555D68D(void* buf, SceSize bufsize, int* retSize);

/**
 * Used for PSAR decoding 
 *
 * @param buf - The in/out buffer to decode.
 * @param bufsize - The size of the buffer pointed by buf
 * @param retSize - Pointer to an integer that receives the size of 
 * the decoded data.
 * 
 * @returns < 0 on error
*/
int sceMesgd_driver_102DC8AF(void* buf, SceSize bufsize, int* retSize);
