/* RSAREF.H - header file for RSAREF cryptographic toolkit
 */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
     Inc., created 1991. All rights reserved.
 */

#ifndef _RSA_H_
#define _RSA_H_ 1

#ifdef __cplusplus
extern "C" {
#endif


/* RSA key lengths.
 */
#define RSA_MINMODULUSBITS 		508
#define RSA_MAXMODULUSBITS 		4096
#define RSA_MAXMODULUSLEN 		((RSA_MAXMODULUSBITS + 7) / 8)
#define RSA_MAXPRIMEBITS 		((RSA_MAXMODULUSBITS + 1) / 2)
#define RSA_MAXPRIMELEN 		((RSA_MAXPRIMEBITS + 7) / 8)


/* Error codes.
 */
#define RE_CONTENT_ENCODING 	0x01
#define RE_DATA 				0x02
#define RE_DIGEST_ALGORITHM 	0x03
#define RE_ENCODING 			0x04
#define RE_KEY 					0x05
#define RE_KEY_ENCODING 		0x06
#define RE_LEN 					0x07
#define RE_MODULUS_LEN 			0x08
#define RE_NEED_RANDOM 			0x09
#define RE_PRIVATE_KEY 			0x0A
#define RE_PUBLIC_KEY 			0x0B
#define RE_SIGNATURE 			0x0C
#define RE_SIGNATURE_ENCODING 	0x0D
#define RE_ENCRYPTION_ALGORITHM	0x0E

/* Random structure.
 */
typedef struct
{
	unsigned int bytesNeeded;
  	unsigned char state[16];
  	unsigned int outputAvailable;
  	unsigned char output[16];
}RANDOMSTRUCT;

/* RSA public and private key.
 */
typedef struct
{
  	unsigned int bits;                          			 	/* length in bits of modulus */
  	unsigned char modulus[RSA_MAXMODULUSLEN];  	/* modulus */
  	unsigned char exponent[RSA_MAXMODULUSLEN];  	/* public exponent */
}RSAPUBLICKEY;

typedef struct
{
  	unsigned int bits;                           					/* length in bits of modulus */
  	unsigned char modulus[RSA_MAXMODULUSLEN];          	/* modulus */
  	unsigned char publicExponent[RSA_MAXMODULUSLEN]; 	/* public exponent */
  	unsigned char exponent[RSA_MAXMODULUSLEN];          	/* private exponent */
  	unsigned char prime[2][RSA_MAXPRIMELEN];               	/* prime factors */
  	unsigned char primeExponent[2][RSA_MAXPRIMELEN];   	/* exponents for CRT */
  	unsigned char coefficient[RSA_MAXPRIMELEN];          	/* CRT coefficient */
}RSAPRIVATEKEY;

/* RSA prototype key.
 */
typedef struct
{
  	unsigned int bits;                  	/* length in bits of modulus */
  	int useFermat4;                        	/* public exponent (1 = F4, 0 = 3) */
}RSAPROTOKEY;



unsigned char RSA_PublicDecrypt(unsigned char *pucPublicKey,unsigned int uiPublicKeyLen,
					unsigned char *pucPublicKeyExp,unsigned int uiPublicKeyExpLen,
					unsigned char *pucIn,unsigned int uiInLen,
					unsigned char *pucOut);
unsigned char  RSA_recovery_check(unsigned char *pucBegin, unsigned long ulFilesize);

#ifdef __cplusplus
}
#endif

#endif

