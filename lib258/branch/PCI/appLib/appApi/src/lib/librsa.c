/*
	Library Function
--------------------------------------------------------------------------
	FILE  librsa.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2005-07-27		Xiaoxi Jiang
    Last modified :	2005-07-27		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <librsa.h>
#include <libnn.h>

static int RSA_PublicBlock(unsigned char *pucPublicKey,unsigned int uiPublicKeyLen,
					unsigned char *pucPublicKeyExp,unsigned int uiPublicKeyExpLen,
					unsigned  char *pucIn,unsigned int uiInLen,
					unsigned char *pucOut);
unsigned char RSA_PublicDecrypt(unsigned char *pucPublicKey,unsigned int uiPublicKeyLen,
					unsigned char *pucPublicKeyExp,unsigned int uiPublicKeyExpLen,
					unsigned char *pucIn,unsigned int uiInLen,
					unsigned char *pucOut)
{
  	int status;

  	if (uiInLen > uiPublicKeyLen)
    		return (RE_LEN);

	status = RSA_PublicBlock(pucPublicKey,uiPublicKeyLen,
					pucPublicKeyExp,uiPublicKeyExpLen,
					pucIn,uiInLen,pucOut);

  	return (status);
}

static int RSA_PublicBlock(unsigned char *pucPublicKey,unsigned int uiPublicKeyLen,
					unsigned char *pucPublicKeyExp,unsigned int uiPublicKeyExpLen,
					unsigned  char *pucIn,unsigned int uiInLen,
					unsigned char *pucOut)
{
  	NN_DIGIT c[NN_MAXDIGITS];
	NN_DIGIT e[NN_MAXDIGITS];
	NN_DIGIT m[NN_MAXDIGITS];
    NN_DIGIT n[NN_MAXDIGITS];
  	unsigned int eDigits, nDigits;

  	NN_Decode (m, NN_MAXDIGITS, pucIn, uiInLen);
  	NN_Decode (n, NN_MAXDIGITS, pucPublicKey, uiPublicKeyLen);
  	NN_Decode (e, NN_MAXDIGITS, pucPublicKeyExp, uiPublicKeyExpLen);
  	nDigits = NN_Digits (n, NN_MAXDIGITS);
  	eDigits = NN_Digits (e, NN_MAXDIGITS);

  	if (NN_Cmp (m, n, nDigits) >= 0)
    		return (RE_DATA);

  	/* Compute c = m^e mod n.
   	*/
  	NN_ModExp (c, m, e, eDigits, n, nDigits);

  	NN_Encode (pucOut, uiInLen, c, nDigits);

  	/* Zeroize sensitive information.
   	*/
  	memset (c, 0, sizeof (c));
  	memset (m, 0, sizeof (m));

  	return (0);
}


