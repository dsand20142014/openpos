#include <string.h>
#include <stdio.h>

typedef struct
{
    unsigned long aulState[5];
    unsigned long aulCount[2];
    unsigned char aucBuf[64];
}SHA1_CTX;

typedef union
{
    unsigned char ulC[64];
    unsigned long l[16];
}CHAR64LONG16;

void SHA1_Init(SHA1_CTX* pCtx);
void SHA1_Transform(unsigned long *pulState,unsigned char *pucBuf);
void SHA1_Update(SHA1_CTX* context, unsigned char* data, unsigned int len);
void SHA1_Final(unsigned char digest[20], SHA1_CTX* context);

#define SHA1_ROL(v,b) (((v) << (b))|((v) >> (32 - (b))))

#ifdef __BIG_ENDIAN
#define SHA1_BLK0(i) (pBlk->l[i])
#else
#define SHA1_BLK0(i) (pBlk->l[i] = (SHA1_ROL(pBlk->l[i],24)&0xFF00FF00)|(SHA1_ROL(pBlk->l[i],8)&0x00FF00FF))
#endif
#define SHA1_BLK(i) (pBlk->l[i&15] = SHA1_ROL(pBlk->l[(i+13)&15]^pBlk->l[(i+8)&15]^pBlk->l[(i+2)&15]^pBlk->l[i&15],1))

#define SHA1_R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+SHA1_BLK0(i)+0x5A827999+SHA1_ROL(v,5);w=SHA1_ROL(w,30);
#define SHA1_R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+SHA1_BLK(i)+0x5A827999+SHA1_ROL(v,5);w=SHA1_ROL(w,30);
#define SHA1_R2(v,w,x,y,z,i) z+=(w^x^y)+SHA1_BLK(i)+0x6ED9EBA1+SHA1_ROL(v,5);w=SHA1_ROL(w,30);
#define SHA1_R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+SHA1_BLK(i)+0x8F1BBCDC+SHA1_ROL(v,5);w=SHA1_ROL(w,30);
#define SHA1_R4(v,w,x,y,z,i) z+=(w^x^y)+SHA1_BLK(i)+0xCA62C1D6+SHA1_ROL(v,5);w=SHA1_ROL(w,30);


void SHA1_Init(SHA1_CTX *pCtx)
{
    pCtx->aulState[0] = 0x67452301;
    pCtx->aulState[1] = 0xEFCDAB89;
    pCtx->aulState[2] = 0x98BADCFE;
    pCtx->aulState[3] = 0x10325476;
    pCtx->aulState[4] = 0xC3D2E1F0;
    pCtx->aulCount[0] = pCtx->aulCount[1] = 0;
}

void SHA1_Transform(unsigned long *pulState, unsigned char *pucBuf)
{
    unsigned long ulA;
    unsigned long ulB;
    unsigned long ulC;
    unsigned long ulD;
    unsigned long ulE;
    CHAR64LONG16* pBlk,Blk;

#if 0
    pBlk = (CHAR64LONG16 *)pucBuf;
#else
	memcpy(&Blk,pucBuf,sizeof(CHAR64LONG16));
	pBlk = &Blk;
#endif
    ulA = pulState[0];
    ulB = pulState[1];
    ulC = pulState[2];
    ulD = pulState[3];
    ulE = pulState[4];
    SHA1_R0(ulA,ulB,ulC,ulD,ulE, 0);
    SHA1_R0(ulE,ulA,ulB,ulC,ulD, 1);
    SHA1_R0(ulD,ulE,ulA,ulB,ulC, 2);
    SHA1_R0(ulC,ulD,ulE,ulA,ulB, 3);
    SHA1_R0(ulB,ulC,ulD,ulE,ulA, 4);
    SHA1_R0(ulA,ulB,ulC,ulD,ulE, 5);
    SHA1_R0(ulE,ulA,ulB,ulC,ulD, 6);
    SHA1_R0(ulD,ulE,ulA,ulB,ulC, 7);
    SHA1_R0(ulC,ulD,ulE,ulA,ulB, 8);
    SHA1_R0(ulB,ulC,ulD,ulE,ulA, 9);
    SHA1_R0(ulA,ulB,ulC,ulD,ulE,10);
    SHA1_R0(ulE,ulA,ulB,ulC,ulD,11);
    SHA1_R0(ulD,ulE,ulA,ulB,ulC,12);
    SHA1_R0(ulC,ulD,ulE,ulA,ulB,13);
    SHA1_R0(ulB,ulC,ulD,ulE,ulA,14);
    SHA1_R0(ulA,ulB,ulC,ulD,ulE,15);
    SHA1_R1(ulE,ulA,ulB,ulC,ulD,16);
    SHA1_R1(ulD,ulE,ulA,ulB,ulC,17);
    SHA1_R1(ulC,ulD,ulE,ulA,ulB,18);
    SHA1_R1(ulB,ulC,ulD,ulE,ulA,19);
    SHA1_R2(ulA,ulB,ulC,ulD,ulE,20);
    SHA1_R2(ulE,ulA,ulB,ulC,ulD,21);
    SHA1_R2(ulD,ulE,ulA,ulB,ulC,22);
    SHA1_R2(ulC,ulD,ulE,ulA,ulB,23);
    SHA1_R2(ulB,ulC,ulD,ulE,ulA,24);
    SHA1_R2(ulA,ulB,ulC,ulD,ulE,25);
    SHA1_R2(ulE,ulA,ulB,ulC,ulD,26);
    SHA1_R2(ulD,ulE,ulA,ulB,ulC,27);
    SHA1_R2(ulC,ulD,ulE,ulA,ulB,28);
    SHA1_R2(ulB,ulC,ulD,ulE,ulA,29);
    SHA1_R2(ulA,ulB,ulC,ulD,ulE,30);
    SHA1_R2(ulE,ulA,ulB,ulC,ulD,31);
    SHA1_R2(ulD,ulE,ulA,ulB,ulC,32);
    SHA1_R2(ulC,ulD,ulE,ulA,ulB,33);
    SHA1_R2(ulB,ulC,ulD,ulE,ulA,34);
    SHA1_R2(ulA,ulB,ulC,ulD,ulE,35);
    SHA1_R2(ulE,ulA,ulB,ulC,ulD,36);
    SHA1_R2(ulD,ulE,ulA,ulB,ulC,37);
    SHA1_R2(ulC,ulD,ulE,ulA,ulB,38);
    SHA1_R2(ulB,ulC,ulD,ulE,ulA,39);
    SHA1_R3(ulA,ulB,ulC,ulD,ulE,40);
    SHA1_R3(ulE,ulA,ulB,ulC,ulD,41);
    SHA1_R3(ulD,ulE,ulA,ulB,ulC,42);
    SHA1_R3(ulC,ulD,ulE,ulA,ulB,43);
    SHA1_R3(ulB,ulC,ulD,ulE,ulA,44);
    SHA1_R3(ulA,ulB,ulC,ulD,ulE,45);
    SHA1_R3(ulE,ulA,ulB,ulC,ulD,46);
    SHA1_R3(ulD,ulE,ulA,ulB,ulC,47);
    SHA1_R3(ulC,ulD,ulE,ulA,ulB,48);
    SHA1_R3(ulB,ulC,ulD,ulE,ulA,49);
    SHA1_R3(ulA,ulB,ulC,ulD,ulE,50);
    SHA1_R3(ulE,ulA,ulB,ulC,ulD,51);
    SHA1_R3(ulD,ulE,ulA,ulB,ulC,52);
    SHA1_R3(ulC,ulD,ulE,ulA,ulB,53);
    SHA1_R3(ulB,ulC,ulD,ulE,ulA,54);
    SHA1_R3(ulA,ulB,ulC,ulD,ulE,55);
    SHA1_R3(ulE,ulA,ulB,ulC,ulD,56);
    SHA1_R3(ulD,ulE,ulA,ulB,ulC,57);
    SHA1_R3(ulC,ulD,ulE,ulA,ulB,58);
    SHA1_R3(ulB,ulC,ulD,ulE,ulA,59);
    SHA1_R4(ulA,ulB,ulC,ulD,ulE,60);
    SHA1_R4(ulE,ulA,ulB,ulC,ulD,61);
    SHA1_R4(ulD,ulE,ulA,ulB,ulC,62);
    SHA1_R4(ulC,ulD,ulE,ulA,ulB,63);
    SHA1_R4(ulB,ulC,ulD,ulE,ulA,64);
    SHA1_R4(ulA,ulB,ulC,ulD,ulE,65);
    SHA1_R4(ulE,ulA,ulB,ulC,ulD,66);
    SHA1_R4(ulD,ulE,ulA,ulB,ulC,67);
    SHA1_R4(ulC,ulD,ulE,ulA,ulB,68);
    SHA1_R4(ulB,ulC,ulD,ulE,ulA,69);
    SHA1_R4(ulA,ulB,ulC,ulD,ulE,70);
    SHA1_R4(ulE,ulA,ulB,ulC,ulD,71);
    SHA1_R4(ulD,ulE,ulA,ulB,ulC,72);
    SHA1_R4(ulC,ulD,ulE,ulA,ulB,73);
    SHA1_R4(ulB,ulC,ulD,ulE,ulA,74);
    SHA1_R4(ulA,ulB,ulC,ulD,ulE,75);
    SHA1_R4(ulE,ulA,ulB,ulC,ulD,76);
    SHA1_R4(ulD,ulE,ulA,ulB,ulC,77);
    SHA1_R4(ulC,ulD,ulE,ulA,ulB,78);
    SHA1_R4(ulB,ulC,ulD,ulE,ulA,79);
    pulState[0] += ulA;
    pulState[1] += ulB;
    pulState[2] += ulC;
    pulState[3] += ulD;
    pulState[4] += ulE;
}

void SHA1_Update(SHA1_CTX *pCtx,unsigned char *pucInData,unsigned int uiInLen)
{
    unsigned int uiI;
    unsigned int uiJ;

    uiJ = (pCtx->aulCount[0] >> 3) & 63;
    if( (pCtx->aulCount[0]+=uiInLen<<3)<(uiInLen << 3) )
        pCtx->aulCount[1]++;
    pCtx->aulCount[1] += (uiInLen >> 29);
    if( (uiJ+uiInLen) > 63)
    {
        uiI = 64-uiJ;
        memcpy(&pCtx->aucBuf[uiJ], pucInData,uiI);
        SHA1_Transform(pCtx->aulState, pCtx->aucBuf);
        for( ;(uiI+63)<uiInLen;uiI+=64)
            SHA1_Transform(pCtx->aulState, pucInData+uiI);
        uiJ = 0;
    }else
        uiI = 0;
    memcpy(&pCtx->aucBuf[uiJ],&pucInData[uiI],uiInLen-uiI);
}

void SHA1_Final(unsigned char *pucDigest, SHA1_CTX *pCtx)
{
    unsigned long ulI;
    unsigned char aucFinal[8];

    for(ulI=0;ulI< 8;ulI++)
    {
        aucFinal[ulI] = (unsigned char)((pCtx->aulCount[(ulI>=4 ? 0:1)]>>((3-(ulI&3))*8))&255);
    }
    SHA1_Update(pCtx, (unsigned char *)"\200", 1);
    while( (pCtx->aulCount[0]&504) != 448)
    {
        SHA1_Update(pCtx,(unsigned char *)"\0", 1);
    }
    SHA1_Update(pCtx, aucFinal, 8);
    for(ulI=0;ulI<20;ulI++)
    {
        pucDigest[ulI]=(unsigned char)((pCtx->aulState[ulI>>2]>>((3-(ulI&3))*8))&255);
    }
    memset(pCtx->aucBuf, 0, 64);
    memset(pCtx->aulState, 0, 20);
    memset(pCtx->aulCount, 0, 8);
}

void SHA1_Compute(unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucDigest)
{

	SHA1_CTX Context;

    SHA1_Init(&Context);

    SHA1_Update(&Context,pucInData,uiInLen);

    SHA1_Final(pucDigest,&Context);

}

