/*
	KIRK ENGINE CODE
	Thx for coyotebean, Davee, hitchhikr, kgsws, Mathieulh, SilverSpring
*/

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include "types.h"
#include "kirk_engine.h"
#include "crypto.h"


/* ------------------------- KEY VAULT ------------------------- */

u8 kirk1_key[] =   {0x98, 0xC9, 0x40, 0x97, 0x5C, 0x1D, 0x10, 0xE8, 0x7F, 0xE6, 0x0E, 0xA3, 0xFD, 0x03, 0xA8, 0xBA};

u8 kirk7_key03[] = {0x98, 0x02, 0xC4, 0xE6, 0xEC, 0x9E, 0x9E, 0x2F, 0xFC, 0x63, 0x4C, 0xE4, 0x2F, 0xBB, 0x46, 0x68};
u8 kirk7_key04[] = {0x99, 0x24, 0x4C, 0xD2, 0x58, 0xF5, 0x1B, 0xCB, 0xB0, 0x61, 0x9C, 0xA7, 0x38, 0x30, 0x07, 0x5F};
u8 kirk7_key05[] = {0x02, 0x25, 0xD7, 0xBA, 0x63, 0xEC, 0xB9, 0x4A, 0x9D, 0x23, 0x76, 0x01, 0xB3, 0xF6, 0xAC, 0x17};
u8 kirk7_key0C[] = {0x84, 0x85, 0xC8, 0x48, 0x75, 0x08, 0x43, 0xBC, 0x9B, 0x9A, 0xEC, 0xA7, 0x9C, 0x7F, 0x60, 0x18};
u8 kirk7_key0D[] = {0xB5, 0xB1, 0x6E, 0xDE, 0x23, 0xA9, 0x7B, 0x0E, 0xA1, 0x7C, 0xDB, 0xA2, 0xDC, 0xDE, 0xC4, 0x6E};
u8 kirk7_key0E[] = {0xC8, 0x71, 0xFD, 0xB3, 0xBC, 0xC5, 0xD2, 0xF2, 0xE2, 0xD7, 0x72, 0x9D, 0xDF, 0x82, 0x68, 0x82};
u8 kirk7_key0F[] = {0x0A, 0xBB, 0x33, 0x6C, 0x96, 0xD4, 0xCD, 0xD8, 0xCB, 0x5F, 0x4B, 0xE0, 0xBA, 0xDB, 0x9E, 0x03};
u8 kirk7_key10[] = {0x32, 0x29, 0x5B, 0xD5, 0xEA, 0xF7, 0xA3, 0x42, 0x16, 0xC8, 0x8E, 0x48, 0xFF, 0x50, 0xD3, 0x71};
u8 kirk7_key11[] = {0x46, 0xF2, 0x5E, 0x8E, 0x4D, 0x2A, 0xA5, 0x40, 0x73, 0x0B, 0xC4, 0x6E, 0x47, 0xEE, 0x6F, 0x0A};
u8 kirk7_key12[] = {0x5D, 0xC7, 0x11, 0x39, 0xD0, 0x19, 0x38, 0xBC, 0x02, 0x7F, 0xDD, 0xDC, 0xB0, 0x83, 0x7D, 0x9D};
u8 kirk7_key38[] = {0x12, 0x46, 0x8D, 0x7E, 0x1C, 0x42, 0x20, 0x9B, 0xBA, 0x54, 0x26, 0x83, 0x5E, 0xB0, 0x33, 0x03};
u8 kirk7_key39[] = {0xC4, 0x3B, 0xB6, 0xD6, 0x53, 0xEE, 0x67, 0x49, 0x3E, 0xA9, 0x5F, 0xBC, 0x0C, 0xED, 0x6F, 0x8A};
u8 kirk7_key3A[] = {0x2C, 0xC3, 0xCF, 0x8C, 0x28, 0x78, 0xA5, 0xA6, 0x63, 0xE2, 0xAF, 0x2D, 0x71, 0x5E, 0x86, 0xBA};
u8 kirk7_key4B[] = {0x0C, 0xFD, 0x67, 0x9A, 0xF9, 0xB4, 0x72, 0x4F, 0xD7, 0x8D, 0xD6, 0xE9, 0x96, 0x42, 0x28, 0x8B}; //1.xx game eboot.bin
u8 kirk7_key53[] = {0xAF, 0xFE, 0x8E, 0xB1, 0x3D, 0xD1, 0x7E, 0xD8, 0x0A, 0x61, 0x24, 0x1C, 0x95, 0x92, 0x56, 0xB6};
u8 kirk7_key57[] = {0x1C, 0x9B, 0xC4, 0x90, 0xE3, 0x06, 0x64, 0x81, 0xFA, 0x59, 0xFD, 0xB6, 0x00, 0xBB, 0x28, 0x70};
u8 kirk7_key5D[] = {0x11, 0x5A, 0x5D, 0x20, 0xD5, 0x3A, 0x8D, 0xD3, 0x9C, 0xC5, 0xAF, 0x41, 0x0F, 0x0F, 0x18, 0x6F};
u8 kirk7_key63[] = {0x9C, 0x9B, 0x13, 0x72, 0xF8, 0xC6, 0x40, 0xCF, 0x1C, 0x62, 0xF5, 0xD5, 0x92, 0xDD, 0xB5, 0x82};
u8 kirk7_key64[] = {0x03, 0xB3, 0x02, 0xE8, 0x5F, 0xF3, 0x81, 0xB1, 0x3B, 0x8D, 0xAA, 0x2A, 0x90, 0xFF, 0x5E, 0x61};

/* ------------------------- KEY VAULT END ------------------------- */

/* ------------------------- INTERNAL STUFF ------------------------- */

typedef struct header_keys
{
    u8 AES[16];
    u8 CMAC[16];
}header_keys;  //small struct for temporary keeping AES & CMAC key from CMD1 header

u8 fuseID[16]; //Emulate FUSEID

AES_ctx aes_kirk1; //global

char is_kirk_initialized; //"init" emulation

/* ------------------------- INTERNAL STUFF END ------------------------- */


/* ------------------------- IMPLEMENTATION ------------------------- */

int kirk_CMD0(void* outbuff, void* inbuff, int size, int generate_trash)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;

    KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)outbuff;

    memcpy(outbuff, inbuff, size);

	if(header->mode != KIRK_MODE_CMD1) return KIRK_INVALID_MODE;

	header_keys *keys = (header_keys *)outbuff; //0-15 AES key, 16-31 CMAC key

	//FILL PREDATA WITH RANDOM DATA
	if(generate_trash) kirk_CMD14(outbuff+sizeof(KIRK_CMD1_HEADER), header->data_offset);

	//Make sure data is 16 aligned
	int chk_size = header->data_size;
	if(chk_size % 16) chk_size += 16 - (chk_size % 16);

	//ENCRYPT DATA
	AES_ctx k1;
	AES_set_key(&k1, keys->AES, 128);

	AES_cbc_encrypt(&k1, inbuff+sizeof(KIRK_CMD1_HEADER)+header->data_offset, outbuff+sizeof(KIRK_CMD1_HEADER)+header->data_offset, chk_size);

	//CMAC HASHES
	AES_ctx cmac_key;
	AES_set_key(&cmac_key, keys->CMAC, 128);

	u8 cmac_header_hash[16];
	u8 cmac_data_hash[16];

	AES_CMAC(&cmac_key, outbuff+0x60, 0x30, cmac_header_hash);

	AES_CMAC(&cmac_key, outbuff+0x60, 0x30 + chk_size + header->data_offset, cmac_data_hash);

	memcpy(header->CMAC_header_hash, cmac_header_hash, 16);
	memcpy(header->CMAC_data_hash, cmac_data_hash, 16);

	//ENCRYPT KEYS

	AES_cbc_encrypt(&aes_kirk1, inbuff, outbuff, 16*2);
	return KIRK_OPERATION_SUCCESS;
}

int kirk_decrypt_keys(u8 *keys, void *inbuff)
{
	AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)keys, 16*2); //decrypt AES & CMAC key to temp buffer
	return 0;
}

int kirk_CMD1(void* outbuff, void* inbuff, int size, int do_check)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;

    KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)inbuff;
	if(header->mode != KIRK_MODE_CMD1) return KIRK_INVALID_MODE;

	header_keys keys; //0-15 AES key, 16-31 CMAC key

	AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)&keys, 16*2); //decrypt AES & CMAC key to temp buffer

	// HOAX WARRING! I have no idea why the hash check on last IPL block fails, so there is an option to disable checking
	if(do_check)
	{
       int ret = kirk_CMD10(inbuff, size);
       if(ret != KIRK_OPERATION_SUCCESS) return ret;
    }

	AES_ctx k1;
	AES_set_key(&k1, keys.AES, 128);

	AES_cbc_decrypt(&k1, inbuff+sizeof(KIRK_CMD1_HEADER)+header->data_offset, outbuff, header->data_size);

	return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD4(void* outbuff, void* inbuff, int size)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;

	KIRK_AES128CBC_HEADER *header = (KIRK_AES128CBC_HEADER*)inbuff;
	if(header->mode != KIRK_MODE_ENCRYPT_CBC) return KIRK_INVALID_MODE;
	if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;

	u8* key = kirk_4_7_get_key(header->keyseed);
	if(key == (u8*)KIRK_INVALID_SIZE) return KIRK_INVALID_SIZE;

	//Set the key
	AES_ctx aesKey;
	AES_set_key(&aesKey, key, 128);
 	AES_cbc_encrypt(&aesKey, inbuff+sizeof(KIRK_AES128CBC_HEADER), outbuff, size);

	return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD7(void* outbuff, void* inbuff, int size)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;

	KIRK_AES128CBC_HEADER *header = (KIRK_AES128CBC_HEADER*)inbuff;
	if(header->mode != KIRK_MODE_DECRYPT_CBC) return KIRK_INVALID_MODE;
	if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;

	u8* key = kirk_4_7_get_key(header->keyseed);
	if(key == (u8*)KIRK_INVALID_SIZE) return KIRK_INVALID_SIZE;

	//Set the key
	AES_ctx aesKey;
	AES_set_key(&aesKey, key, 128);

 	AES_cbc_decrypt(&aesKey, inbuff+sizeof(KIRK_AES128CBC_HEADER), outbuff, size);

	return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD10(void* inbuff, int insize)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;

    KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)inbuff;

	if(!(header->mode == KIRK_MODE_CMD1 || header->mode == KIRK_MODE_CMD2 || header->mode == KIRK_MODE_CMD3)) return KIRK_INVALID_MODE;
	if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;

	if(header->mode == KIRK_MODE_CMD1)
	{
        header_keys keys; //0-15 AES key, 16-31 CMAC key

        AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)&keys, 32); //decrypt AES & CMAC key to temp buffer

	    AES_ctx cmac_key;
	    AES_set_key(&cmac_key, keys.CMAC, 128);

		u8 cmac_header_hash[16];
		u8 cmac_data_hash[16];

		AES_CMAC(&cmac_key, inbuff+0x60, 0x30, cmac_header_hash);

		//Make sure data is 16 aligned
		int chk_size = header->data_size;
		if(chk_size % 16) chk_size += 16 - (chk_size % 16);
		AES_CMAC(&cmac_key, inbuff+0x60, 0x30 + chk_size + header->data_offset, cmac_data_hash);

		if(memcmp(cmac_header_hash, header->CMAC_header_hash, 16) != 0)
        {
            printf("header hash invalid\n");
            return KIRK_HEADER_HASH_INVALID;
        }
		if(memcmp(cmac_data_hash, header->CMAC_data_hash, 16) != 0)
        {
            printf("data hash invalid\n");
            return KIRK_DATA_HASH_INVALID;
        }

		return KIRK_OPERATION_SUCCESS;
	}
	return KIRK_SIG_CHECK_INVALID; //Checks for cmd 2 & 3 not included right now
}

int kirk_CMD11(void* outbuff, void* inbuff, int size)
{
    if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
	KIRK_SHA1_HEADER *header = (KIRK_SHA1_HEADER *)inbuff;
	if(header->data_size == 0 || size == 0) return KIRK_DATA_SIZE_ZERO;

    SHA1Context sha;
    SHA1Reset(&sha);
    size <<= 4;
    size >>= 4;
	size = size < header->data_size ? size : header->data_size;
    SHA1Input(&sha, inbuff+sizeof(KIRK_SHA1_HEADER), size);
    memcpy(outbuff, sha.Message_Digest, 16);
    return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD14(void* outbuff, int size)
{
    if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
    int i;
    u8* buf = (u8*)outbuff;
    for(i = 0; i < size; i++)
    {
          buf[i] = rand()%255;
    }
    return KIRK_OPERATION_SUCCESS;
}

int kirk_init()
{
    AES_set_key(&aes_kirk1, kirk1_key, 128);
	is_kirk_initialized = 1;
	srand(time(0));
    return KIRK_OPERATION_SUCCESS;
}

u8* kirk_4_7_get_key(int key_type)
{
    switch(key_type)
	{
		case(0x03): return kirk7_key03; break;
		case(0x04): return kirk7_key04; break;
		case(0x05): return kirk7_key05; break;
		case(0x0C): return kirk7_key0C; break;
		case(0x0D): return kirk7_key0D; break;
		case(0x0E): return kirk7_key0E; break;
		case(0x0F): return kirk7_key0F; break;
		case(0x10): return kirk7_key10; break;
		case(0x11): return kirk7_key11; break;
		case(0x12): return kirk7_key12; break;
		case(0x38): return kirk7_key38; break;
		case(0x39): return kirk7_key39; break;
		case(0x3A): return kirk7_key3A; break;
		case(0x4B): return kirk7_key4B; break;
		case(0x53): return kirk7_key53; break;
		case(0x57): return kirk7_key57; break;
		case(0x5D): return kirk7_key5D; break;
		case(0x63): return kirk7_key63; break;
		case(0x64): return kirk7_key64; break;
		default: return (u8*)KIRK_INVALID_SIZE; break; //need to get the real error code for that, placeholder now :)
	}
}

int kirk_CMD1_ex(void* outbuff, void* inbuff, int size, KIRK_CMD1_HEADER* header)
{
    u8* buffer = (u8*)malloc(size);
    memcpy(buffer, header, sizeof(KIRK_CMD1_HEADER));
    memcpy(buffer+sizeof(KIRK_CMD1_HEADER), inbuff, header->data_size);
    int ret = kirk_CMD1(outbuff, buffer, size, 1);
    free(buffer);
    return ret;
}

int sceUtilsSetFuseID(void*fuse)
{
	memcpy(fuseID, fuse, 16);
	return 0;
}

int sceUtilsBufferCopyWithRange(void* outbuff, int outsize, void* inbuff, int insize, int cmd)
{
    switch(cmd)
    {
		case KIRK_CMD_DECRYPT_PRIVATE:
             if(insize % 16) return SUBCWR_NOT_16_ALGINED;
             int ret = kirk_CMD1(outbuff, inbuff, insize, 1);
             if(ret == KIRK_HEADER_HASH_INVALID) return SUBCWR_HEADER_HASH_INVALID;
             return ret;
             break;
		case KIRK_CMD_ENCRYPT_IV_0: return kirk_CMD4(outbuff, inbuff, insize); break;
		case KIRK_CMD_DECRYPT_IV_0: return kirk_CMD7(outbuff, inbuff, insize); break;
		case KIRK_CMD_PRIV_SIG_CHECK: return kirk_CMD10(inbuff, insize); break;
		case KIRK_CMD_SHA1_HASH: return kirk_CMD11(outbuff, inbuff, insize); break;
	}
	return -1;
}


int kirk_forge(u8* inbuff, int insize)
{
   KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)inbuff;
   AES_ctx cmac_key;
   u8 cmac_header_hash[16];
   u8 cmac_data_hash[16];
   int chk_size;

   if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
   if(!(header->mode == KIRK_MODE_CMD1 || header->mode == KIRK_MODE_CMD2 || header->mode == KIRK_MODE_CMD3)) return KIRK_INVALID_MODE;
   if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;

   if(header->mode == KIRK_MODE_CMD1){
      header_keys keys; //0-15 AES key, 16-31 CMAC key

      AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)&keys, 32); //decrypt AES & CMAC key to temp buffer
      AES_set_key(&cmac_key, keys.CMAC, 128);
      AES_CMAC(&cmac_key, inbuff+0x60, 0x30, cmac_header_hash);
      if(memcmp(cmac_header_hash, header->CMAC_header_hash, 16) != 0) return KIRK_HEADER_HASH_INVALID;

      //Make sure data is 16 aligned
      chk_size = header->data_size;
      if(chk_size % 16) chk_size += 16 - (chk_size % 16);
      AES_CMAC(&cmac_key, inbuff+0x60, 0x30 + chk_size + header->data_offset, cmac_data_hash);

      if(memcmp(cmac_data_hash, header->CMAC_data_hash, 16) != 0) {
      //printf("data hash invalid, correcting...\n");
    } else {
         printf("data hash is already valid!\n");
         return 100;
      }
      // Forge collision for data hash
    memcpy(cmac_data_hash,header->CMAC_data_hash,0x10);
    AES_CMAC_forge(&cmac_key, inbuff+0x60, 0x30+ chk_size + header->data_offset, cmac_data_hash);
      //printf("Last row in bad file should be :\n"); for(i=0;i<0x10;i++) printf("%02x", cmac_data_hash[i]);
      //printf("\n\n");

      return KIRK_OPERATION_SUCCESS;
   }
   return KIRK_SIG_CHECK_INVALID; //Checks for cmd 2 & 3 not included right now
}
