#define MD5_S11 7
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21 5
#define MD5_S22 9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31 4
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41 6
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21

void MD5_Transform (unsigned long *pulState,unsigned char *pucBlock);
void MD5_Encode(unsigned char *pucOutput,unsigned long *pulInput,unsigned int uiLen);
void MD5_Decode(unsigned long *pulOutput,unsigned char *pucInput,unsigned int uiLen);

#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

#define MD5_ROTATELEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define MD5_FF(a, b, c, d, x, s, ac) { \
    			(a) += MD5_F ((b), (c), (d)) + (x) + (unsigned long)(ac); \
    			(a) = MD5_ROTATELEFT ((a), (s)); \
    			(a) += (b); \
  			}
#define MD5_GG(a, b, c, d, x, s, ac) { \
    			(a) += MD5_G ((b), (c), (d)) + (x) + (unsigned long)(ac); \
    			(a) = MD5_ROTATELEFT ((a), (s)); \
   			 (a) += (b); \
  			}
#define MD5_HH(a, b, c, d, x, s, ac) { \
    			(a) += MD5_H ((b), (c), (d)) + (x) + (unsigned long)(ac); \
    			(a) = MD5_ROTATELEFT ((a), (s)); \
    			(a) += (b); \
  			}
#define MD5_II(a, b, c, d, x, s, ac) { \
    			(a) += MD5_I ((b), (c), (d)) + (x) + (unsigned long)(ac); \
   			(a) = MD5_ROTATELEFT ((a), (s)); \
    			(a) += (b); \
  			}

typedef struct
{
	unsigned long ulState[4];
	unsigned long ulCount[2];
	unsigned char aucBuf[64];
}MD5_CTX;

void MD5_Init(MD5_CTX *);
void MD5_Update(MD5_CTX *,unsigned char *,unsigned int);
void MD5_Final(unsigned char *, MD5_CTX *);

void MD5_Init(MD5_CTX *pCtx)
{
	pCtx->ulCount[0] = 0;
	pCtx->ulCount[1] = 0;
	pCtx->ulState[0] = 0x67452301;
	pCtx->ulState[1] = 0xefcdab89;
	pCtx->ulState[2] = 0x98badcfe;
	pCtx->ulState[3] = 0x10325476;
}

void MD5_Update(MD5_CTX * pCtx,unsigned char *pucInData,unsigned int uiInLen)
{
 	unsigned int uiI;
	unsigned int uiIndex;
	unsigned int uiLen;

 	uiIndex = (unsigned int)((pCtx->ulCount[0] >> 3) & 0x3F);

	if ((pCtx->ulCount[0] += ((unsigned long)uiInLen << 3))< ((unsigned long)uiInLen << 3))
    		pCtx->ulCount[1]++;
  	pCtx->ulCount[1] += uiInLen >> 29;

  	uiLen = 64 - uiIndex;

  	if (uiInLen >= uiLen)
  	{
	 	memcpy (&pCtx->aucBuf[uiIndex], pucInData, uiLen);
		MD5_Transform (pCtx->ulState, pCtx->aucBuf);
		for (uiI = uiLen; uiI + 63 < uiInLen; uiI += 64)
			MD5_Transform (pCtx->ulState, pucInData+uiI);
		uiIndex = 0;
	}else
		uiI = 0;

	memcpy(&pCtx->aucBuf[uiIndex],pucInData+uiI,uiInLen-uiI);
}

void MD5_Final(unsigned char *pucDigest, MD5_CTX *pCtx)
{
  	unsigned char aucBit[8];
  	unsigned int uiIndex;
	unsigned int uiLen;
    unsigned char MD5_PADDING[64] =
    {
    	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

  	MD5_Encode (aucBit, pCtx->ulCount, 8);
  	uiIndex = (unsigned int)((pCtx->ulCount[0] >> 3) & 0x3f);
  	uiLen = (uiIndex < 56) ? (56 - uiIndex) : (120 - uiIndex);
  	MD5_Update (pCtx, MD5_PADDING, uiLen);
  	MD5_Update (pCtx, aucBit, 8);
  	MD5_Encode (pucDigest, pCtx->ulState, 16);

	memset (pCtx, 0, sizeof (MD5_CTX));
}

void MD5_Compute(unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucDigest)
{
	MD5_CTX Context;

    MD5_Init(&Context);
    MD5_Update(&Context,pucInData,uiInLen);
    MD5_Final(pucDigest,&Context);
}

void MD5_Transform (unsigned long *pulState,unsigned char *pucBlock)
{
	unsigned long ulA;
	unsigned long ulB;
	unsigned long ulC;
	unsigned long ulD;
	unsigned long aulX[16];

  	ulA = *(pulState);
	ulB = *(pulState+1);
	ulC = *(pulState+2);
	ulD = *(pulState+3);
	MD5_Decode (aulX, pucBlock, 64);

	/* Round 1 */
	MD5_FF (ulA, ulB, ulC, ulD, aulX[ 0], MD5_S11, 0xd76aa478); /* 1 */
	MD5_FF (ulD, ulA, ulB, ulC, aulX[ 1], MD5_S12, 0xe8c7b756); /* 2 */
	MD5_FF (ulC, ulD, ulA, ulB, aulX[ 2], MD5_S13, 0x242070db); /* 3 */
	MD5_FF (ulB, ulC, ulD, ulA, aulX[ 3], MD5_S14, 0xc1bdceee); /* 4 */
	MD5_FF (ulA, ulB, ulC, ulD, aulX[ 4], MD5_S11, 0xf57c0faf); /* 5 */
	MD5_FF (ulD, ulA, ulB, ulC, aulX[ 5], MD5_S12, 0x4787c62a); /* 6 */
  	MD5_FF (ulC, ulD, ulA, ulB, aulX[ 6], MD5_S13, 0xa8304613); /* 7 */
  	MD5_FF (ulB, ulC, ulD, ulA, aulX[ 7], MD5_S14, 0xfd469501); /* 8 */
  	MD5_FF (ulA, ulB, ulC, ulD, aulX[ 8], MD5_S11, 0x698098d8); /* 9 */
  	MD5_FF (ulD, ulA, ulB, ulC, aulX[ 9], MD5_S12, 0x8b44f7af); /* 10 */
  	MD5_FF (ulC, ulD, ulA, ulB, aulX[10], MD5_S13, 0xffff5bb1); /* 11 */
  	MD5_FF (ulB, ulC, ulD, ulA, aulX[11], MD5_S14, 0x895cd7be); /* 12 */
  	MD5_FF (ulA, ulB, ulC, ulD, aulX[12], MD5_S11, 0x6b901122); /* 13 */
  	MD5_FF (ulD, ulA, ulB, ulC, aulX[13], MD5_S12, 0xfd987193); /* 14 */
  	MD5_FF (ulC, ulD, ulA, ulB, aulX[14], MD5_S13, 0xa679438e); /* 15 */
  	MD5_FF (ulB, ulC, ulD, ulA, aulX[15], MD5_S14, 0x49b40821); /* 16 */

  	/* Round 2 */
  	MD5_GG (ulA, ulB, ulC, ulD, aulX[ 1], MD5_S21, 0xf61e2562); /* 17 */
  	MD5_GG (ulD, ulA, ulB, ulC, aulX[ 6], MD5_S22, 0xc040b340); /* 18 */
  	MD5_GG (ulC, ulD, ulA, ulB, aulX[11], MD5_S23, 0x265e5a51); /* 19 */
  	MD5_GG (ulB, ulC, ulD, ulA, aulX[ 0], MD5_S24, 0xe9b6c7aa); /* 20 */
  	MD5_GG (ulA, ulB, ulC, ulD, aulX[ 5], MD5_S21, 0xd62f105d); /* 21 */
  	MD5_GG (ulD, ulA, ulB, ulC, aulX[10], MD5_S22,  0x2441453); /* 22 */
  	MD5_GG (ulC, ulD, ulA, ulB, aulX[15], MD5_S23, 0xd8a1e681); /* 23 */
  	MD5_GG (ulB, ulC, ulD, ulA, aulX[ 4], MD5_S24, 0xe7d3fbc8); /* 24 */
  	MD5_GG (ulA, ulB, ulC, ulD, aulX[ 9], MD5_S21, 0x21e1cde6); /* 25 */
  	MD5_GG (ulD, ulA, ulB, ulC, aulX[14], MD5_S22, 0xc33707d6); /* 26 */
  	MD5_GG (ulC, ulD, ulA, ulB, aulX[ 3], MD5_S23, 0xf4d50d87); /* 27 */
  	MD5_GG (ulB, ulC, ulD, ulA, aulX[ 8], MD5_S24, 0x455a14ed); /* 28 */
  	MD5_GG (ulA, ulB, ulC, ulD, aulX[13], MD5_S21, 0xa9e3e905); /* 29 */
  	MD5_GG (ulD, ulA, ulB, ulC, aulX[ 2], MD5_S22, 0xfcefa3f8); /* 30 */
  	MD5_GG (ulC, ulD, ulA, ulB, aulX[ 7], MD5_S23, 0x676f02d9); /* 31 */
  	MD5_GG (ulB, ulC, ulD, ulA, aulX[12], MD5_S24, 0x8d2a4c8a); /* 32 */

  	/* Round 3 */
  	MD5_HH (ulA, ulB, ulC, ulD, aulX[ 5], MD5_S31, 0xfffa3942); /* 33 */
  	MD5_HH (ulD, ulA, ulB, ulC, aulX[ 8], MD5_S32, 0x8771f681); /* 34 */
  	MD5_HH (ulC, ulD, ulA, ulB, aulX[11], MD5_S33, 0x6d9d6122); /* 35 */
  	MD5_HH (ulB, ulC, ulD, ulA, aulX[14], MD5_S34, 0xfde5380c); /* 36 */
  	MD5_HH (ulA, ulB, ulC, ulD, aulX[ 1], MD5_S31, 0xa4beea44); /* 37 */
  	MD5_HH (ulD, ulA, ulB, ulC, aulX[ 4], MD5_S32, 0x4bdecfa9); /* 38 */
  	MD5_HH (ulC, ulD, ulA, ulB, aulX[ 7], MD5_S33, 0xf6bb4b60); /* 39 */
  	MD5_HH (ulB, ulC, ulD, ulA, aulX[10], MD5_S34, 0xbebfbc70); /* 40 */
  	MD5_HH (ulA, ulB, ulC, ulD, aulX[13], MD5_S31, 0x289b7ec6); /* 41 */
  	MD5_HH (ulD, ulA, ulB, ulC, aulX[ 0], MD5_S32, 0xeaa127fa); /* 42 */
  	MD5_HH (ulC, ulD, ulA, ulB, aulX[ 3], MD5_S33, 0xd4ef3085); /* 43 */
  	MD5_HH (ulB, ulC, ulD, ulA, aulX[ 6], MD5_S34,  0x4881d05); /* 44 */
  	MD5_HH (ulA, ulB, ulC, ulD, aulX[ 9], MD5_S31, 0xd9d4d039); /* 45 */
  	MD5_HH (ulD, ulA, ulB, ulC, aulX[12], MD5_S32, 0xe6db99e5); /* 46 */
  	MD5_HH (ulC, ulD, ulA, ulB, aulX[15], MD5_S33, 0x1fa27cf8); /* 47 */
  	MD5_HH (ulB, ulC, ulD, ulA, aulX[ 2], MD5_S34, 0xc4ac5665); /* 48 */

  	/* Round 4 */
  	MD5_II (ulA, ulB, ulC, ulD, aulX[ 0], MD5_S41, 0xf4292244); /* 49 */
  	MD5_II (ulD, ulA, ulB, ulC, aulX[ 7], MD5_S42, 0x432aff97); /* 50 */
  	MD5_II (ulC, ulD, ulA, ulB, aulX[14], MD5_S43, 0xab9423a7); /* 51 */
  	MD5_II (ulB, ulC, ulD, ulA, aulX[ 5], MD5_S44, 0xfc93a039); /* 52 */
  	MD5_II (ulA, ulB, ulC, ulD, aulX[12], MD5_S41, 0x655b59c3); /* 53 */
  	MD5_II (ulD, ulA, ulB, ulC, aulX[ 3], MD5_S42, 0x8f0ccc92); /* 54 */
  	MD5_II (ulC, ulD, ulA, ulB, aulX[10], MD5_S43, 0xffeff47d); /* 55 */
  	MD5_II (ulB, ulC, ulD, ulA, aulX[ 1], MD5_S44, 0x85845dd1); /* 56 */
  	MD5_II (ulA, ulB, ulC, ulD, aulX[ 8], MD5_S41, 0x6fa87e4f); /* 57 */
  	MD5_II (ulD, ulA, ulB, ulC, aulX[15], MD5_S42, 0xfe2ce6e0); /* 58 */
  	MD5_II (ulC, ulD, ulA, ulB, aulX[ 6], MD5_S43, 0xa3014314); /* 59 */
  	MD5_II (ulB, ulC, ulD, ulA, aulX[13], MD5_S44, 0x4e0811a1); /* 60 */
  	MD5_II (ulA, ulB, ulC, ulD, aulX[ 4], MD5_S41, 0xf7537e82); /* 61 */
  	MD5_II (ulD, ulA, ulB, ulC, aulX[11], MD5_S42, 0xbd3af235); /* 62 */
  	MD5_II (ulC, ulD, ulA, ulB, aulX[ 2], MD5_S43, 0x2ad7d2bb); /* 63 */
  	MD5_II (ulB, ulC, ulD, ulA, aulX[ 9], MD5_S44, 0xeb86d391); /* 64 */

  	*(pulState) += ulA;
  	*(pulState+1) += ulB;
  	*(pulState+2) += ulC;
  	*(pulState+3) += ulD;

  	memset (aulX, 0, sizeof (aulX));
}

void MD5_Encode(unsigned char *pucOutput,unsigned long *pulInput,unsigned int uiLen)
{
	unsigned int uiI;
	unsigned int uiJ;

  	for (uiI = 0, uiJ = 0; uiJ < uiLen; uiI++, uiJ += 4)
  	{
    		pucOutput[uiJ] = (unsigned char)(pulInput[uiI] & 0xff);
    		pucOutput[uiJ+1] = (unsigned char)((pulInput[uiI] >> 8) & 0xff);
    		pucOutput[uiJ+2] = (unsigned char)((pulInput[uiI] >> 16) & 0xff);
    		pucOutput[uiJ+3] = (unsigned char)((pulInput[uiI] >> 24) & 0xff);
  	}
}

void MD5_Decode(unsigned long *pulOutput,unsigned char *pucInput,unsigned int uiLen)
{
	unsigned int uiI;
	unsigned int uiJ;

  	for (uiI = 0, uiJ = 0; uiJ < uiLen; uiI++, uiJ += 4)
    		pulOutput[uiI] = ((unsigned long)pucInput[uiJ]) | (((unsigned long)pucInput[uiJ+1]) << 8) |
      			(((unsigned long)pucInput[uiJ+2]) << 16) | (((unsigned long)pucInput[uiJ+3]) << 24);
}


