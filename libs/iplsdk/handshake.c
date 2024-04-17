#include "comms.h"
#include "kirk.h"
#include "sysreg.h"
#include "syscon.h"

#include <stdint.h>
#include <string.h>

#define KIRK_WORK_AREA      ((void *)0xBFC00C00)
#define KIRK_WORK_AREA_SIZE (0x400)

#define HANDSHAKE_SEED_SIZE (0x10)

#define HANDSHAKE_RX_STEP1  (0x00)
#define HANDSHAKE_RX_STEP2  (0x01)
#define HANDSHAKE_RX_STEP3  (0x02)
#define HANDSHAKE_RX_STEP4  (0x03)
#define HANDSHAKE_RX_STEP5  (0x04)
#define HANDSHAKE_RX_STEP6  (0x05)

#define HANDSHAKE_TX_STEP1  (0x80)
#define HANDSHAKE_TX_STEP2  (0x81)
#define HANDSHAKE_TX_STEP3  (0x82)
#define HANDSHAKE_TX_STEP4  (0x83)
#define HANDSHAKE_TX_STEP5  (0x84)
#define HANDSHAKE_TX_STEP6  (0x85)
#define HANDSHAKE_TX_STEP7  (0x86)
#define HANDSHAKE_TX_STEP8  (0x87)

typedef struct
{
    const uint8_t secret1[16];
    const uint8_t secret2[16];
    const uint8_t expected1[8];
    const uint8_t expected2[8];
} HandshakeSecrets;

struct SysconHandshakePacket {
    unsigned char step;
    unsigned char data[8];
}  __attribute__ ((__packed__));

static void copy_from_work_area(void *dst, size_t len)
{
    memcpy(dst, KIRK_WORK_AREA, len);
}

static void copy_to_work_area(const void *src, size_t len)
{
    memcpy(KIRK_WORK_AREA, src, len);
}

static void clean_work_area(void)
{
    memset(KIRK_WORK_AREA, 0, KIRK_WORK_AREA_SIZE);
}

static void xor_seed(void *out_xor_seed, const void *src1, const void *src2)
{
    unsigned char *out = (unsigned char *)out_xor_seed;
    unsigned char *_src1 = (unsigned char *)src1;
    unsigned char *_src2 = (unsigned char *)src2;

    for (unsigned int i = 0; i < HANDSHAKE_SEED_SIZE; ++i)
    {
        out[i] = _src1[i] ^ _src2[i];
    }
}

static void rotate_seed_8(void *dst, const void *src, unsigned int rot)
{
    unsigned char *_dst = (unsigned char *)dst;
    unsigned char *_src = (unsigned char *)src;

    for (unsigned int i = 0; i < HANDSHAKE_SEED_SIZE; ++i)
    {
        _dst[i] = _src[i] + rot;
    }
}

static void rotate_seed_16(void *dst, const void *src, unsigned int rot)
{
    unsigned short *_dst = (unsigned short *)dst;
    unsigned short *_src = (unsigned short *)src;

    for (unsigned int i = 0; i < HANDSHAKE_SEED_SIZE/sizeof(unsigned short); ++i)
    {
        _dst[i] = _src[i] + rot;
    }
}

static int crypt_seed(void *crypt_seed_data, const void *seed_data, int key_id)
{
    // most of our operations require performing decryption ops
    // and sending the data to syscon. allocate a work buffer that
    // can accomodate the data plus the kirk header. since we cannot
    // issue kirk calls point into the EDRAM (where we are executing from)
    // we must copy this data to a work area accessible to KIRK.
    uint8_t work_buffer[sizeof(Kirk47Header) + HANDSHAKE_SEED_SIZE];
    Kirk47Header *k7_hdr = (Kirk47Header *)work_buffer;
    uint8_t *enc_data = &work_buffer[sizeof(Kirk47Header)];

    // perform a kirk7 decryption operation on the RNG
    // output using key 0x69. this requires preparation
    // and then copying to the work area
    k7_hdr->mode = 5;
    k7_hdr->unk_4 = 0;
    k7_hdr->unk_8 = 0;
    k7_hdr->keyslot = key_id;
    k7_hdr->size = HANDSHAKE_SEED_SIZE;
    memcpy(enc_data, seed_data, HANDSHAKE_SEED_SIZE);
    copy_to_work_area(work_buffer, sizeof(work_buffer));
    int res = kirk7(KIRK_WORK_AREA, KIRK_WORK_AREA);
    copy_from_work_area(crypt_seed_data, HANDSHAKE_SEED_SIZE);
    return res;
}

static int handshake_tx_step(int step, const void *seed)
{
    struct SysconHandshakePacket packet;

    packet.step = step;
    memcpy(packet.data, seed, sizeof(packet.data));

    return syscon_issue_command_write(SYSCON_HANDSHAKE_AUTH_UNLOCK, (void *)&packet, sizeof(packet));
}

static int handshake_rx_step(int step, void *dst)
{
    struct SysconHandshakePacket packet;
    unsigned char step_byte = step;
    int res = syscon_issue_command_read_write(SYSCON_HANDSHAKE_AUTH_UNLOCK, (void *)&step_byte, sizeof(step_byte), (void *)&packet);

    if (res < 0) {
        return res;
    }

    memcpy(dst, packet.data, sizeof(packet.data));
    return res;
}

static const HandshakeSecrets *get_handshake_secrets(unsigned int version)
{
    // not every model has the handshake, and some models use different
    // secret. return NULL when there is no handshake, and either type1
    // (04g/07g/09g/11g) or type2 (05g) secrets
    unsigned int model_code = BARYON_VERSION_MODEL_CODE(version);

    // type1 is 04g/07g/09g/11g
    if (BARYON_MODEL_CODE_IS_HANDSHAKE_TYPE1(model_code)) {
        static const HandshakeSecrets type1_secrets = {
            .secret1 = {      
                0x8D, 0x5D, 0xA6, 0x08, 0xF2, 0xBB, 0xC6, 0xCC,
                0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23
            },
            .secret2 = {
                0xD5, 0x96, 0x55, 0x56, 0xB9, 0x39, 0xD8, 0x9D,
                0x6E, 0x79, 0xD3, 0x8C, 0x88, 0x7B, 0xF3, 0x0A
            },
            .expected1 = {    
                0x34, 0xDB, 0x81, 0x24, 0x1D, 0x6F, 0x40, 0x57
            },
            .expected2 = {
                0xE0, 0xDC, 0x41, 0xAF, 0xC2, 0xCD, 0x1C, 0x2D
            }
        };

        return &type1_secrets;
    }

    // type2 is 05g
    else if (BARYON_MODEL_CODE_IS_HANDSHAKE_TYPE2(model_code)) {
        static const HandshakeSecrets type2_secret = {
            .secret1 = {      
                0x61, 0x7A, 0x56, 0x42, 0xF8, 0xED, 0xC5, 0xE4,
                0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23
            },
            .secret2 = {
                0xF3, 0x58, 0x22, 0xBA, 0x99, 0x4F, 0x86, 0x93,
                0x6A, 0xFF, 0xB7, 0x05, 0x5D, 0xDD, 0x14, 0xFC,
            },
            .expected1 = {
                0xDB, 0xB1, 0x1E, 0x20, 0x48, 0x83, 0xB1, 0x6F
            },
            .expected2 = {
                0x04, 0xF4, 0x69, 0x8A, 0x8C, 0xAA, 0x95, 0x30
            }
        };

        return &type2_secret;
    }

    return NULL;
}

int syscon_handshake_unlock(void)
{
    // we have some buffers for storing results
    uint8_t crypt_buffer0[HANDSHAKE_SEED_SIZE];
    uint8_t crypt_buffer1[HANDSHAKE_SEED_SIZE];
    uint8_t crypt_buffer2[HANDSHAKE_SEED_SIZE];

    // this result is used for XOR in later stages
    uint8_t xor_crypt_buffer[HANDSHAKE_SEED_SIZE];

    // the handshake uses a random number source repeatedly. we will
    // bank it specifically here
    uint8_t rng_data[HANDSHAKE_SEED_SIZE];

    // the handshake uses secrets to provide a challenge response
    // across to syscon. annoyingly, these secrets aren't consistent
    // and we need to determine what syscon version we are talking to
    // and provide it the correct secrets
    const HandshakeSecrets *secrets = get_handshake_secrets(syscon_get_baryon_version());

    // a NULL value means that there is no handshake for the device
    // that is currently running.
    if (!secrets) {
        return 1;
    }

    // enable kirk hardware or reset it if it is
    // already enabled
    kirk_hwreset();

    // initialise kirk by using a temporary work area
    kirkF(KIRK_WORK_AREA);
    clean_work_area();

    // output random data into the kirk work area and
    // copy it to our own buffer. we will be using this
    // as the "unique-ness" for the challenge/response
    kirkE(KIRK_WORK_AREA);
    copy_from_work_area(rng_data, sizeof(rng_data));

    // crypt the random data seed
    crypt_seed(crypt_buffer0, rng_data, 0x69);

    // send the data to syscon
    handshake_tx_step(HANDSHAKE_TX_STEP1, crypt_buffer0);
    handshake_tx_step(HANDSHAKE_TX_STEP2, crypt_buffer0 + 8);

    // receive the response from syscon
    handshake_rx_step(HANDSHAKE_RX_STEP1, crypt_buffer1);
    handshake_rx_step(HANDSHAKE_RX_STEP2, crypt_buffer1 + 8);

    // crypt the syscon response from STEP 1&2 store the result
    // from the first crypt operation for use later
    crypt_seed(xor_crypt_buffer, crypt_buffer1, 0x14);
    memcpy(crypt_buffer1 + 0, xor_crypt_buffer + 8, 8);
    memcpy(crypt_buffer1 + 8, xor_crypt_buffer + 0, 8);
    crypt_seed(crypt_buffer0, crypt_buffer1, 0x69);

    // send the data to syscon
    handshake_tx_step(HANDSHAKE_TX_STEP3, crypt_buffer0);
    handshake_tx_step(HANDSHAKE_TX_STEP4, crypt_buffer0 + 8);

    // xor the RNG data with a shared secret
    xor_seed(crypt_buffer1, rng_data, secrets->secret1);
    crypt_seed(crypt_buffer0, crypt_buffer1, 0x15);

    // xor it again with the data from a previous crypt
    xor_seed(crypt_buffer0, crypt_buffer0, xor_crypt_buffer);

    // transmit the result to syscon
    handshake_tx_step(HANDSHAKE_TX_STEP5, crypt_buffer0);
    handshake_tx_step(HANDSHAKE_TX_STEP6, crypt_buffer0 + 8);

    // xor the random data with the second secret handshake
    xor_seed(crypt_buffer0, rng_data, secrets->secret2);

    // transmit the result to syscon
    handshake_tx_step(HANDSHAKE_TX_STEP7, crypt_buffer0);
    handshake_tx_step(HANDSHAKE_TX_STEP8, crypt_buffer0 + 8);

    // receive the response from syscon
    handshake_rx_step(HANDSHAKE_RX_STEP3, crypt_buffer0);
    handshake_rx_step(HANDSHAKE_RX_STEP4, crypt_buffer0 + 8);
    handshake_rx_step(HANDSHAKE_RX_STEP5, crypt_buffer1);
    handshake_rx_step(HANDSHAKE_RX_STEP6, crypt_buffer1 + 8);

    // xor the syscon data with the saved response from earlier
    // and crypt it
    xor_seed(crypt_buffer0, crypt_buffer0, xor_crypt_buffer);
    crypt_seed(crypt_buffer2, crypt_buffer0, 0x15);
    rotate_seed_8(crypt_buffer0, rng_data, 1);
    xor_seed(crypt_buffer0, crypt_buffer0, crypt_buffer2);

    // xor the second response from syscon with the earlier
    // response and crypt it
    xor_seed(crypt_buffer1, crypt_buffer1, xor_crypt_buffer);
    crypt_seed(crypt_buffer1, crypt_buffer1, 0x15);

    // rotate the random seed by a u16
    rotate_seed_16(crypt_buffer2, rng_data, 0xCAB9);
    xor_seed(crypt_buffer1, crypt_buffer1, crypt_buffer2);

    // we should have a predictable output in the first 8 bytes of each
    // response in buffer0 and buffer1
    if (memcmp(crypt_buffer0, secrets->expected1, sizeof(secrets->expected1)) != 0)
    {
        return -1;
    }
    else if (memcmp(crypt_buffer1, secrets->expected2, sizeof(secrets->expected2)) != 0)
    {
        return -2;
    }

    return 0;
}
