
#ifndef DES_H
#define DES_H

void DesEncrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8);
void DesDecrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8);
void Des3Key2Encrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8);
void Des3Key2Decrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8);
#endif
