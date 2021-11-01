#ifndef MAGMA_H
#define MAGMA_H
#include <stdint.h>
#include <string.h>

typedef uint8_t vect[4]; // блок размером 32 бита
//extern vect    iter_key[32]; //  итерационные ключи шифрования

void GOST_Magma_Expand_Key(const uint8_t *key);
void GOST_Magma_Encrypt(const uint8_t *blk, uint8_t *out_blk);
void GOST_Magma_Decrypt(const uint8_t *blk, uint8_t *out_blk);

//


#define BLCK_SIZE 8

extern  unsigned char init_vect_ctr_string[BLCK_SIZE / 2]; // init vector string

void ECB_Encrypt(uint8_t *in_buf, uint8_t *out_buf, uint8_t *key, uint64_t size);

void ECB_Decrypt(uint8_t *in_buf, uint8_t *out_buf, uint8_t *key, uint64_t size);

void CTR_Crypt(uint8_t *init_vec, uint8_t *in_buf, uint8_t *out_buf, uint8_t *key, uint64_t size);



#endif // MAGMA_H
