/*
	Library Function
--------------------------------------------------------------------------
	FILE  libzip.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-22		Xiaoxi Jiang
    Last modified :	2002-10-22		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef _LIBZIP_H
#define _LIBZIP_H


#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif
	
/* libzip.c */
#define ZIPERR_SUCCESS				0x00
#define ZIPERR_ERRMASK				0xD0
#define ZIPERR_OVERLAP              0xD1
#define ZIPERR_INPUTOVERLAP         0xD2
#define ZIPERR_OUTPUTOVERLAP        0xD3
#define ZIPERR_NOTFINISH            0xD4
#define ZIPERR_INVALIDARGUMENT      0xD5
#define ZIPERR_NOMEMORY             0xD6
#define ZIPERR_GENERAL              0xDF

#define ZIP_USEMALLOC

#define UCL_MAX(a,b)        ((a) >= (b) ? (a) : (b))
#define UCL_MIN(a,b)        ((a) <= (b) ? (a) : (b))

#define NRV2_BUFMAXSIZE     (8*1024*1024ul) /* max. size of ring buffer */
#define NRV2_MATCHLENMAX    2048            /* upper limit for match length */
#define NRV2_MATCHLENMIN    1               /* lower limit for match length */
#define NRV2_HMASK          ((65535))
#define NRV2_HSIZE          (NRV2_HMASK + 1)
#define NRV2_MAXCHAIN       2048
#define NRV2_UINTMAX        0xffffffffL

#define NRV2_M2MAXOFFSET    0x500


#define NRV2_GETBIT8(ucCh,pucSrc,iLen)  (((ucCh = ucCh & 0x7f ? ucCh*2 : ((unsigned)pucSrc[iLen++]*2+1)) >> 8) & 1)
#define NRV2_GETBYTE(c)                 ((c).ip < (c).in_end ? *((c).ip)++ : (-1))
#define NRV2_HEAD2(b,p)                 (b[p] ^ ((unsigned)b[p+1]<<8))
#define NRV2_HEAD3(b,p)                 (((0x9f5f*(((((unsigned long)b[p]<<5)^b[p+1])<<5)^b[p+2]))>>5) & NRV2_HMASK)


typedef struct
{
    //void (__cdecl *callback) (unsigned int, unsigned int, int, void *);
	void (__attribute__((stdcall)) *callback) (unsigned int, unsigned int, int, void *);
    void * user;
}
ucl_progress_callback_t;
#define ucl_progress_callback_p ucl_progress_callback_t *

struct NRV2CONFIG
{
    int bb_endian;
    int bb_size;
    unsigned int max_offset;
    unsigned int max_match;
    int s_level;
    int h_level;
    int p_level;
    int c_flags;
    unsigned int m_size;
};

typedef struct
{
    int init;

    unsigned int look;          

    unsigned int uiLen;
    unsigned int uiOffset;

    unsigned int uiLastLen;
    unsigned int uiLastOffset;

    const unsigned char * bp;
    const unsigned char * ip;
    const unsigned char * in;
    const unsigned char * in_end;
    unsigned char * out;

    unsigned long bb_b;
    unsigned bb_k;
    unsigned bb_c_endian;
    unsigned bb_c_s;
    unsigned bb_c_s8;
    unsigned char * bb_p;
    unsigned char * bb_op;

    struct NRV2CONFIG conf;
    unsigned int * result;

    ucl_progress_callback_p cb;

    unsigned int textsize;      /* text size counter */
    unsigned int codesize;      /* code size counter */
    unsigned int printcount;    /* counter for reporting progress every 1K bytes */

    /* some stats */
    unsigned long lit_bytes;
    unsigned long match_bytes;
    unsigned long rep_bytes;
    unsigned long lazy;
}NRV2COMPRESS;


typedef struct
{
/* public - "built-in" */
    unsigned int n;
    unsigned int f;
    unsigned int threshold;
    unsigned int hmask;

/* public - configuration */
    unsigned int max_chain;
    unsigned int nice_length;
    int use_best_off;
    unsigned int lazy_insert;

/* public - output */
    unsigned int uiLen;
    unsigned int uiOffset;
    unsigned int uiLook;
    int b_char;

/* semi public */
    NRV2COMPRESS *c;
    unsigned int uiPosition;

/* private */
    const unsigned char * pucDict;
    const unsigned char * pucDictEnd;
    unsigned int uiDictLen;

/* private */
    unsigned int uiInPtr;                /* input pointer (lookahead) */
    unsigned int uiBufPtr;                /* buffer pointer */
    unsigned int uiRemovePtr;                /* remove pointer */
    unsigned int uiBlockSize;

    unsigned char * b_wrap;

    unsigned int uiNodeCount;
    unsigned int uiFirstRemovePtr;

#if defined(ZIP_USEMALLOC)
    unsigned int * auiHead2;
    unsigned char * aucBlock;
    unsigned int * auiHead3;
    unsigned int * succ3;
    unsigned int * best3;
    unsigned int * llen3;
#else
    unsigned int auiHead2 [ 65536 ];
    unsigned char aucBlock [ NRV2_BUFMAXSIZE + NRV2_MATCHLENMAX + NRV2_MATCHLENMAX ];
    unsigned int auiHead3 [ NRV2_HSIZE ];
    unsigned int succ3 [ NRV2_BUFMAXSIZE + NRV2_MATCHLENMAX ];
    unsigned int best3 [ NRV2_BUFMAXSIZE + NRV2_MATCHLENMAX ];
    unsigned int llen3 [ NRV2_HSIZE ];
#endif
}NRV2;

typedef struct {
	unsigned char bintype;//0x41 应用类型：1byte（HEX）(1,os 2, app 3, font 4, data)
    unsigned int  bintypecode;//0x42 应用类型编号即APP ID
	unsigned char binname[50];//0x43 应用文件名称信息
	unsigned char bindesc[50];//0x44 应用标签信息
	unsigned char binversion[4];// 0x45 应用版本信息
	unsigned char bincountryid[4]; //0x49 国家代码信息
    unsigned char bincputype; //0x4a CPU类型

}PACK_INFO;

static int ZIP_NRV2InitMatch ( NRV2COMPRESS *c, NRV2 *s,
             const unsigned char * dict, unsigned int dict_len,
             unsigned long flags );
static int ZIP_NRV2FindMatch ( NRV2COMPRESS *c, NRV2 *s,
             unsigned int this_len, unsigned int skip );

static int  ZIP_NRV2BitConfig(NRV2COMPRESS *c, int endian, int bitsize);
static void ZIP_NRV2BitWriteBits(NRV2COMPRESS *c);
static void ZIP_NRV2BitPutBit(NRV2COMPRESS *c, unsigned bit);
static void ZIP_NRV2BitPutByte(NRV2COMPRESS *c, unsigned b);
static void ZIP_NRV2BitFlushBits(NRV2COMPRESS *c, unsigned filler_bit);

static int  ZIP_NRV2Init(NRV2 *s, const unsigned char * dict, unsigned int dict_len);
static void ZIP_NRV2Exit(NRV2 *s);
static void ZIP_NRV2GetByte(NRV2 *s);
static void ZIP_NRV2RemoveNode(NRV2 *s, unsigned int node);
static void ZIP_NRV2Accept(NRV2 *s, unsigned int n);
static void ZIP_NRV2FindBest(NRV2 *s);
static void ZIP_NRV2InitDict(NRV2 *s, const unsigned char * dict, unsigned int dict_len);
static void ZIP_NRV2InsertDict(NRV2 *s, unsigned int node, unsigned int len);
static void ZIP_NRV2Search(NRV2 *s, unsigned int node, unsigned int cnt);
static int  ZIP_NRV2Search2(NRV2 *s);

static void ZIP_NRV2CodePrefix11(NRV2COMPRESS *c, unsigned long i);
static int  ZIP_NRV2CodeMatchLen(NRV2COMPRESS *c, unsigned int uiLen, unsigned int uiOffset);
static void ZIP_NRV2CodeRun(NRV2COMPRESS *c, const unsigned char * ii, unsigned int lit);
static void ZIP_NRV2CodeMatch(NRV2COMPRESS *c, unsigned int uiLen, const unsigned int uiOffset);
static void ZIP_NRV2CodePrefix12(NRV2COMPRESS *c, unsigned long i);

int ZIP_NRV2DDecompress(const unsigned char *pucSrc, unsigned int  uiSrcLen,
                        unsigned char *pucDest, unsigned int *puiDestLen);
#if 1//add 2005-04-06
int ZIP_NRV2Compress( const unsigned char * in, unsigned int in_len,
                                   unsigned char * out, unsigned int * out_len,
                                   ucl_progress_callback_p cb,
                                   int level,
                             const struct NRV2CONFIG * conf,
                                   unsigned int * result);

#endif

int ZIP_NRVData2Compress(PACK_INFO *headInfo,char *inData, int inLen, char *outData, int outLen);

#ifdef __cplusplus
}
#endif

#endif
