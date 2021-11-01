#include "magma.h"
//#define DEBUG_MODE




static unsigned char Pi[8][16]= // Таблица перестановок ГОСТ 34.12—2015
{
    {12,4,6,2,10,5,11,9,14,8,13,7,0,3,15,1},
    {6,8,2,3,9,10,5,12,1,14,4,7,11,13,0,15},
    {11,3,5,8,2,15,10,13,14,1,7,4,12,9,6,0},
    {12,8,2,1,13,4,15,6,7,0,10,5,3,14,9,11},
    {7,15,5,10,8,1,6,13,0,9,3,14,11,4,2,12},
    {5,13,15,6,9,2,12,10,11,7,8,1,4,3,14,0},
    {8,14,2,5,6,9,1,12,15,4,11,0,13,10,3,7},
    {1,7,14,13,0,5,8,3,4,15,10,6,9,12,11,2}
};


vect    iter_key[32]; //  итерационные ключи шифрования


#ifdef DEBUG_MODE
#include <stdio.h>

static void
GOST_Magma_Blk_4_PrintDebug(uint8_t *state)
{
    int i;
    for (i = 0; i < 4; i++)
        printf("%02x", state[i]);
    printf("\n");
}

static void
GOST_Magma_Blk_8_PrintDebug(uint8_t *state)
{
    int i;
    for (i = 0; i < 8; i++)
        printf("%02x", state[i]);
    printf("\n");
}
#endif

static void
GOST_Magma_T(const uint8_t *in_data, uint8_t *out_data)
{
    uint8_t first_part_byte, sec_part_byte;
    int i;
    for (i = 0; i < 4; i++)
    {
        first_part_byte = (in_data[i] & 0x0f);
        sec_part_byte = (in_data[i] & 0xf0) >> 4;
        first_part_byte = Pi[i * 2] [first_part_byte];
        sec_part_byte = Pi[i * 2 + 1] [sec_part_byte];
        out_data[i] =  (sec_part_byte << 4) | first_part_byte;
    }
}

static void
GOST_Magma_Add(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
    int i;
    for (i = 0; i < 4; i++)
        c[i] = a[i]^b[i];
}

static void
GOST_Magma_Add_32(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
    int i;
    unsigned int internal = 0;
    for (i = 0; i < 4; i++)
    {
        internal = a[i] + b[i] + (internal >> 8);
        c[i] = internal & 0xff;
    }
}

static void
GOST_Magma_g(const uint8_t *k, const uint8_t *a, uint8_t *out_data)
{
    uint8_t internal[4];
    uint32_t out_data_32;
    GOST_Magma_Add_32(a, k, internal);
    GOST_Magma_T(internal, internal);
    out_data_32 = internal[3];
    out_data_32 = (out_data_32  << 8) + internal[2];
    out_data_32 = (out_data_32  << 8) + internal[1];
    out_data_32 = (out_data_32  << 8) + internal[0];
    out_data_32 = (out_data_32 << 11)|(out_data_32 >> 21);
    out_data[0] = out_data_32;
    out_data[1] = out_data_32 >> 8;
    out_data[2] = out_data_32 >> 16;
    out_data[3] = out_data_32 >> 24;
}

static void
GOST_Magma_G(const uint8_t *k, const uint8_t *a, uint8_t *out_data)
{
    uint8_t a_0[4];
    uint8_t a_1[4];
    uint8_t G[4];
    int i;
    for(i = 0; i < 4; i++)
    {
       a_1[i] = a[4 + i];
       a_0[i] = a[i];
    }
    GOST_Magma_g(k, a_0, G);
    GOST_Magma_Add(a_1, G, G);
    for(i = 0; i < 4; i++)
    {
        a_1[i] = a_0[i];
        a_0[i] = G[i];
    }
    for(i = 0; i < 4; i++)
    {
        out_data[i] = a_0[i];
        out_data[4 + i] = a_1[i];
    }

}

static void
GOST_Magma_G_Fin(const uint8_t *k, const uint8_t *a, uint8_t *out_data)
{
    uint8_t a_0[4];
    uint8_t a_1[4];
    uint8_t G[4];
    int i;
    for(i = 0; i < 4; i++)
    {
       a_1[i] = a[4 + i];
       a_0[i] = a[i];
    }
    GOST_Magma_g(k, a_0, G);
    GOST_Magma_Add(a_1, G, G);
    for(i = 0; i < 4; i++)
        a_1[i] = G[i];
    for(i = 0; i < 4; i++)
    {
        out_data[i] = a_0[i];
        out_data[4 + i] = a_1[i];
    }
}

void
GOST_Magma_Expand_Key(const uint8_t *key)
{
    memcpy(iter_key[7], key, 4);
    memcpy(iter_key[6], key + 4, 4);
    memcpy(iter_key[5], key + 8, 4);
    memcpy(iter_key[4], key + 12, 4);
    memcpy(iter_key[3], key + 16, 4);
    memcpy(iter_key[2], key + 20, 4);
    memcpy(iter_key[1], key + 24, 4);
    memcpy(iter_key[0], key + 28, 4);
    memcpy(iter_key[15], key, 4);
    memcpy(iter_key[14], key + 4, 4);
    memcpy(iter_key[13], key + 8, 4);
    memcpy(iter_key[12], key + 12, 4);
    memcpy(iter_key[11], key + 16, 4);
    memcpy(iter_key[10], key + 20, 4);
    memcpy(iter_key[9], key + 24, 4);
    memcpy(iter_key[8], key + 28, 4);
    memcpy(iter_key[23], key, 4);
    memcpy(iter_key[22], key + 4, 4);
    memcpy(iter_key[21], key + 8, 4);
    memcpy(iter_key[20], key + 12, 4);
    memcpy(iter_key[19], key + 16, 4);
    memcpy(iter_key[18], key + 20, 4);
    memcpy(iter_key[17], key + 24, 4);
    memcpy(iter_key[16], key + 28, 4);
    memcpy(iter_key[31], key + 28, 4);
    memcpy(iter_key[30], key + 24, 4);
    memcpy(iter_key[29], key + 20, 4);
    memcpy(iter_key[28], key + 16, 4);
    memcpy(iter_key[27], key + 12, 4);
    memcpy(iter_key[26], key + 8, 4);
    memcpy(iter_key[25], key + 4, 4);
    memcpy(iter_key[24], key, 4);

#ifdef DEBUG_MODE
    printf("Iteration cipher keys:\n");
    int i;
    for (i = 0; i < 32; i++)
    {
        printf("K%d=", i+1);
        GOST_Magma_Blk_4_PrintDebug(iter_key[i]);
    }
#endif
}

void
GOST_Magma_Destroy_Key()
{
    int i;
    for (i = 0; i < 32; i++)
        memset(iter_key[i], 0x00, 4);
}


void
GOST_Magma_Encrypt(const uint8_t *blk, uint8_t *out_blk)
{
#ifdef DEBUG_MODE
    printf("Text:\n");
    GOST_Magma_Blk_8_PrintDebug(blk);
#endif

    int i;
    GOST_Magma_G(iter_key[0], blk, out_blk);
    for(i = 1; i < 31; i++)
        GOST_Magma_G(iter_key[i], out_blk, out_blk);
    GOST_Magma_G_Fin(iter_key[31], out_blk, out_blk);

#ifdef DEBUG_MODE
    printf("Encrypted text:\n");
    GOST_Magma_Blk_8_PrintDebug(out_blk);
#endif
}

void
GOST_Magma_Decrypt(const uint8_t *blk, uint8_t *out_blk)
{
#ifdef DEBUG_MODE
    printf("Gipher text:\n");
    GOST_Magma_Blk_8_PrintDebug(blk);
#endif

    int i;
    GOST_Magma_G(iter_key[31], blk, out_blk);
    for(i = 30; i > 0; i--)
        GOST_Magma_G(iter_key[i], out_blk, out_blk);
    GOST_Magma_G_Fin(iter_key[0], out_blk, out_blk);

#ifdef DEBUG_MODE
    printf("Decrypted text:\n");
    GOST_Magma_Blk_8_PrintDebug(out_blk);
#endif
}



static void
inc_ctr(uint8_t *ctr)
{
    int i;
    unsigned int internal = 0;
    uint8_t bit[BLCK_SIZE];
    memset(bit, 0x00, BLCK_SIZE);
    bit[0] = 0x01;
    for (i = 0; i < BLCK_SIZE; i++)
    {
        internal = ctr[i] + bit[i] + (internal >> 8);
        ctr[i] = internal & 0xff;
    }
}

static void
add_xor(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
    int i;
    for (i = 0; i < BLCK_SIZE; i++)
        c[i] = a[i]^b[i];
}

void
ECB_Encrypt(uint8_t *in_buf, uint8_t *out_buf, uint8_t *key, uint64_t size)
{
    uint64_t num_blocks = size / BLCK_SIZE;
    uint8_t internal[BLCK_SIZE];
    uint64_t i;
    GOST_Magma_Expand_Key(key);
    for (i = 0; i < num_blocks; i++)
    {
        memcpy(internal, in_buf+i*BLCK_SIZE, BLCK_SIZE);
        GOST_Magma_Encrypt(internal, internal);
        memcpy(out_buf + i*BLCK_SIZE, internal, BLCK_SIZE);
    }
    GOST_Magma_Destroy_Key();
}

void
ECB_Decrypt(uint8_t *in_buf, uint8_t *out_buf, uint8_t *key, uint64_t size)
{
    uint64_t num_blocks = size / BLCK_SIZE;
    uint8_t internal[BLCK_SIZE];
    uint64_t i;
    GOST_Magma_Expand_Key(key);
    for (i = 0; i < num_blocks; i++)
    {
        memcpy(internal, in_buf + i*BLCK_SIZE, BLCK_SIZE);
        GOST_Magma_Decrypt(internal, internal);
        memcpy(out_buf + i*BLCK_SIZE, internal, BLCK_SIZE);
    }
    GOST_Magma_Destroy_Key();
}

void
CTR_Crypt(uint8_t *ctr, uint8_t *in_buf, uint8_t *out_buf, uint8_t *key, uint64_t size)
{
    uint64_t num_blocks = size / BLCK_SIZE;
    uint8_t gamma[BLCK_SIZE];
    uint8_t internal[BLCK_SIZE];

    uint64_t i;
    GOST_Magma_Expand_Key(key);
    for (i = 0; i < num_blocks; i++)
    {
        GOST_Magma_Encrypt(ctr, gamma);
        inc_ctr(ctr);
        memcpy(internal, in_buf + i*BLCK_SIZE, BLCK_SIZE);
        add_xor(internal, gamma, internal);
        memcpy(out_buf + i*BLCK_SIZE, internal, BLCK_SIZE);
        size = size - BLCK_SIZE;
    }
    if (size > 0)
    {
        GOST_Magma_Encrypt(ctr, gamma);
        inc_ctr(ctr);
        memcpy(internal, in_buf + i*BLCK_SIZE, size);
        add_xor(internal, gamma, internal);
        memcpy(out_buf + num_blocks*BLCK_SIZE, internal, size);
        size = 0;
    }
    GOST_Magma_Destroy_Key();
}


#ifdef DEBUG_MODE

static const unsigned char test_key[32] = {
    0xff, 0xfe, 0xfd, 0xfc,
    0xfb, 0xfa, 0xf9, 0xf8,
    0xf7, 0xf6, 0xf5, 0xf4,
    0xf3, 0xf2, 0xf1, 0xf0,
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb,
    0xcc, 0xdd, 0xee, 0xff
};

static const unsigned char encrypt_test_string[8] = {
    0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe
};

static const unsigned char decrypt_test_string[8] = {
    0x3d, 0xca, 0xd8, 0xc2, 0xe5, 0x01, 0xe9, 0x4e
};


int magma_test(int argc, char *argv[])
{
    uint8_t out_blk[8];
    GOST_Magma_Expand_Key(test_key);
    GOST_Magma_Encrypt(encrypt_test_string, out_blk);
        GOST_Magma_Encrypt(encrypt_test_string, out_blk);
            GOST_Magma_Encrypt(encrypt_test_string, out_blk);
    GOST_Magma_Decrypt(decrypt_test_string, out_blk);
        GOST_Magma_Decrypt(decrypt_test_string, out_blk);
            GOST_Magma_Decrypt(decrypt_test_string, out_blk);
    return 0;
}

#endif

