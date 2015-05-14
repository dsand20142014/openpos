
#ifndef HEADER_AES_H
#define HEADER_AES_H

#include <stddef.h>


# define GETU32(pt) (((unsigned int)(pt)[0] << 24) ^ ((unsigned int)(pt)[1] << 16) ^ ((unsigned int)(pt)[2] <<  8) ^ ((unsigned int)(pt)[3]))
# define PUTU32(ct, st) { (ct)[0] = (unsigned char)((st) >> 24); (ct)[1] = (unsigned char)((st) >> 16); (ct)[2] = (unsigned char)((st) >>  8); (ct)[3] = (unsigned char)(st); }


/* Because array size can't be a const in C, the following two are macros.
   Both sizes are in bytes. */
#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16

#ifdef  __cplusplus
extern "C" {
#endif

/* This should be a hidden type, but EVP requires that the size be known */
struct aes_key_st {
#ifdef AES_LONG
    unsigned long rd_key[4 *(AES_MAXNR + 1)];
#else
    unsigned int rd_key[4 *(AES_MAXNR + 1)];
#endif
    int rounds;
};
typedef struct aes_key_st AES_KEY;

const char *AES_options(void);

int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);
int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);

void AES_encrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);
void AES_decrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);

void AES_ecb_encrypt(const unsigned char *in, unsigned char *out,
	unsigned char *key);

void AES_ecb_decrypt(const unsigned char *in, unsigned char *out,
	unsigned char *key);


#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_AES_H */
