#ifndef KIRK_ENGINE
#define KIRK_ENGINE


//Kirk return values
#define KIRK_OPERATION_SUCCESS 0
#define KIRK_NOT_ENABLED 1
#define KIRK_INVALID_MODE 2
#define KIRK_HEADER_HASH_INVALID 3
#define KIRK_DATA_HASH_INVALID 4
#define KIRK_SIG_CHECK_INVALID 5
#define KIRK_UNK_1 6
#define KIRK_UNK_2 7
#define KIRK_UNK_3 8
#define KIRK_UNK_4 9
#define KIRK_UNK_5 0xA
#define KIRK_UNK_6 0xB
#define KIRK_NOT_INITIALIZED 0xC
#define KIRK_INVALID_OPERATION 0xD
#define KIRK_INVALID_SEED_CODE 0xE
#define KIRK_INVALID_SIZE 0xF
#define KIRK_DATA_SIZE_ZERO 0x10

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
	u8 AES_key[16];            //0
	u8 CMAC_key[16];           //10
	u8 CMAC_header_hash[16];   //20
	u8 CMAC_data_hash[16];     //30
	u8 unused[32];             //40
	u32 mode;                  //60
	u8 unk3[12];               //64
	u32 data_size;             //70
	u32 data_offset;           //74
	u8 unk4[8];                //78
	u8 unk5[16];               //80
} KIRK_CMD1_HEADER; //0x90

typedef struct
{
    u32 data_size;             //0
} KIRK_SHA1_HEADER;            //4

//mode passed to sceUtilsBufferCopyWithRange
#define KIRK_CMD_DECRYPT_PRIVATE 1
#define KIRK_CMD_ENCRYPT_IV_0 4
#define KIRK_CMD_ENCRYPT_IV_FUSE 5
#define KIRK_CMD_ENCRYPT_IV_USER 6
#define KIRK_CMD_DECRYPT_IV_0 7
#define KIRK_CMD_DECRYPT_IV_FUSE 8
#define KIRK_CMD_DECRYPT_IV_USER 9
#define KIRK_CMD_PRIV_SIG_CHECK 10
#define KIRK_CMD_SHA1_HASH 11

//"mode" in header
#define KIRK_MODE_CMD1 1
#define KIRK_MODE_CMD2 2
#define KIRK_MODE_CMD3 3
#define KIRK_MODE_ENCRYPT_CBC 4
#define KIRK_MODE_DECRYPT_CBC 5

//sceUtilsBufferCopyWithRange errors
#define SUBCWR_NOT_16_ALGINED 0x90A
#define SUBCWR_HEADER_HASH_INVALID 0x920
#define SUBCWR_BUFFER_TOO_SMALL 0x1000

/*
      // Private Sig + Cipher
      0x01: Super-Duper decryption (no inverse)
      0x02: Encrypt Operation (inverse of 0x03)
      0x03: Decrypt Operation (inverse of 0x02)

      // Cipher
      0x04: Encrypt Operation (inverse of 0x07) (IV=0)
      0x05: Encrypt Operation (inverse of 0x08) (IV=FuseID)
      0x06: Encrypt Operation (inverse of 0x09) (IV=UserDefined)
      0x07: Decrypt Operation (inverse of 0x04)
      0x08: Decrypt Operation (inverse of 0x05)
      0x09: Decrypt Operation (inverse of 0x06)

      // Sig Gens
      0x0A: Private Signature Check (checks for private SCE sig)
      0x0B: SHA1 Hash
      0x0C: Mul1
      0x0D: Mul2
      0x0E: Random Number Gen
      0x0F: (absolutely no idea – could be KIRK initialization)
      0x10: Signature Gen
      // Sig Checks
      0x11: Signature Check (checks for generated sigs)
      0x12: Certificate Check (idstorage signatures)
*/

//kirk-like funcs
int kirk_CMD0(void* outbuff, void* inbuff, int size, int generate_trash);
int kirk_CMD1(void* outbuff, void* inbuff, int size, int do_check);
int kirk_CMD4(void* outbuff, void* inbuff, int size);
int kirk_CMD7(void* outbuff, void* inbuff, int size);
int kirk_CMD10(void* inbuff, int insize);
int kirk_CMD11(void* outbuff, void* inbuff, int size);
int kirk_CMD14(void* outbuff, int size);
int kirk_init(); //CMD 0xF?

//helper funcs
u8* kirk_4_7_get_key(int key_type);

//kirk "ex" functions
int kirk_CMD1_ex(void* outbuff, void* inbuff, int size, KIRK_CMD1_HEADER* header);

//sce-like funcs
int sceUtilsSetFuseID(void*fuse);
int sceUtilsBufferCopyWithRange(void* outbuff, int outsize, void* inbuff, int insize, int cmd);

int kirk_forge(u8* inbuff, int insize);

int kirk_decrypt_keys(u8 *keys, void *inbuff);

#endif


