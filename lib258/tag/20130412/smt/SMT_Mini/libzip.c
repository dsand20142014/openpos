
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "libzip.h"


#define fail(x,r)   if (x) { *puiDestLen = olen; return r; }


int ZIP_NRV2Compress( const unsigned char * in, unsigned int in_len,
                                   unsigned char * out, unsigned int * out_len,
                                   ucl_progress_callback_p cb,
                                   int level,
                             const struct NRV2CONFIG * conf,
                                   unsigned int * result)
{
    const unsigned char * ii;
    unsigned int lit;
    unsigned int uiLen, uiOffset;
    NRV2COMPRESS c_buffer;
    NRV2COMPRESS * const c = &c_buffer;
#undef s
#if defined(ZIP_USEMALLOC)
    NRV2 the_swd;
#   define s (&the_swd)
#else
    NRV2 * s;
#endif
    unsigned int result_buffer[16];
    int r;

    struct swd_config_t
    {
        unsigned try_lazy;
        unsigned int good_length;
        unsigned int max_lazy;
        unsigned int nice_length;
        unsigned int max_chain;
        unsigned long flags;
        unsigned long max_offset;
    };
    const struct swd_config_t *sc;
    static const struct swd_config_t swd_config[10] = {
        /* faster compression */
        {   0,   0,                 0,                  8,                  4,                  0,  48*1024L },
        {   0,   0,                 0,                  16,                 8,                  0,  48*1024L },
        {   0,   0,                 0,                  32,                 16,                 0,  48*1024L },
        {   1,   4,                 4,                  16,                 16,                 0,  48*1024L },
        {   1,   8,                 16,                 32,                 32,                 0,  48*1024L },
        {   1,   8,                 16,                 128,                128,                0,  48*1024L },
        {   2,   8,                 32,                 128,                256,                0,  128*1024L },
        {   2,  32,                 128,                NRV2_MATCHLENMAX,   2048,               1,  128*1024L },
        {   2,  32,                 128,                NRV2_MATCHLENMAX,   2048,               1,  256*1024L },
        {   2,  NRV2_MATCHLENMAX,   NRV2_MATCHLENMAX,   NRV2_MATCHLENMAX,   4096,               1,  NRV2_BUFMAXSIZE }
        /* max. compression */
    };

    if (level < 1 || level > 10)
        return ZIPERR_INVALIDARGUMENT;
    sc = &swd_config[level - 1];

    memset(c, 0, sizeof(*c));
    memset(&c->conf, 0xff, sizeof(c->conf));
    c->ip = c->in = in;
    c->in_end = in + in_len;
    c->out = out;
    if (cb && cb->callback)
        c->cb = cb;
    cb = NULL;
    c->result = result ? result : (unsigned int *) result_buffer;
    result = NULL;
    memset(c->result, 0, 16*sizeof(*c->result));
    c->result[0] = c->result[2] = c->result[4] = NRV2_UINTMAX;
    if (conf)
        memcpy(&c->conf, conf, sizeof(c->conf));
    conf = NULL;
    r = ZIP_NRV2BitConfig(c, 0, 8);
    if (r == 0)
        r = ZIP_NRV2BitConfig(c, c->conf.bb_endian, c->conf.bb_size);
    if (r != 0)
        return ZIPERR_INVALIDARGUMENT;
    c->bb_op = out;

    ii = c->ip;             /* point to start of literal run */
    lit = 0;

#if !defined(s)
    s = (NRV2 *) malloc(sizeof(*s));
    if (!s)
        return ZIPERR_NOMEMORY;
#endif
    s->f = UCL_MIN((unsigned int)NRV2_MATCHLENMAX, c->conf.max_match);
    s->n = UCL_MIN((unsigned int)NRV2_BUFMAXSIZE, sc->max_offset);
    s->hmask = (65535);
    if (c->conf.max_offset != NRV2_UINTMAX)
        s->n = UCL_MIN(NRV2_BUFMAXSIZE, c->conf.max_offset);
    if (in_len < s->n)
        s->n = UCL_MAX(in_len, 256);
    if (s->f < 8 || s->n < 256)
        return ZIPERR_INVALIDARGUMENT;
    r = ZIP_NRV2InitMatch(c,s,NULL,0,sc->flags);
    if (r == ZIPERR_SUCCESS && (NRV2_HSIZE - 1 != s->hmask))
        r = ZIPERR_GENERAL;
    if (r != ZIPERR_SUCCESS)
    {
#if !defined(s)
        free(s);
#endif
        return r;
    }
    if (sc->max_chain > 0)
        s->max_chain = sc->max_chain;
    if (sc->nice_length > 0)
        s->nice_length = sc->nice_length;
    if (c->conf.max_match < s->nice_length)
        s->nice_length = c->conf.max_match;

    if (c->cb)
        (*c->cb->callback)(0,0,-1,c->cb->user);

    c->uiLastOffset = 1;
    r = ZIP_NRV2FindMatch(c,s,0,0);
    if (r != ZIPERR_SUCCESS)
        return r;
    while (c->look > 0)
    {
        unsigned int ahead;
        unsigned int max_ahead;
        int l1, l2;

        c->codesize = (unsigned int) (c->bb_op - out);

        uiLen = c->uiLen;
        uiOffset = c->uiOffset;

        if (lit == 0)
            ii = c->bp;

        if (uiLen < 2 || (uiLen == 2 && (uiOffset > NRV2_M2MAXOFFSET))
            || uiOffset > c->conf.max_offset)
        {
            /* a literal */
            lit++;
            s->max_chain = sc->max_chain;
            r = ZIP_NRV2FindMatch(c,s,1,0);
            continue;
        }

        /* shall we try a lazy match ? */
        ahead = 0;
        if (sc->try_lazy <= 0 || uiLen >= sc->max_lazy || uiOffset == c->uiLastOffset)
        {
            /* no */
            l1 = 0;
            max_ahead = 0;
        }
        else
        {
            /* yes, try a lazy match */
            l1 = ZIP_NRV2CodeMatchLen(c,uiLen,uiOffset);
            max_ahead = UCL_MIN((unsigned int)sc->try_lazy, uiLen - 1);
        }

        while (ahead < max_ahead && c->look > uiLen)
        {
            if (uiLen >= sc->good_length)
                s->max_chain = sc->max_chain >> 2;
            else
                s->max_chain = sc->max_chain;
            r = ZIP_NRV2FindMatch(c,s,1,0);
            ahead++;

            if (c->uiLen < 2)
                continue;

            l2 = ZIP_NRV2CodeMatchLen(c,c->uiLen,c->uiOffset);
            if (l2 < 0)
                continue;

            if (l1 + (int)(ahead + c->uiLen - uiLen) * 5 > l2 + (int)(ahead) * 9)
            {
                c->lazy++;

                lit += ahead;

                goto lazy_match_done;
            }
        }

        /* 1 - code run */
        ZIP_NRV2CodeRun(c,ii,lit);
        lit = 0;

        /* 2 - code match */
        ZIP_NRV2CodeMatch(c,uiLen,uiOffset);
        s->max_chain = sc->max_chain;
        r = ZIP_NRV2FindMatch(c,s,uiLen,1+ahead);

lazy_match_done: ;
    }

    /* store final run */
    ZIP_NRV2CodeRun(c,ii,lit);

    /* EOF */
    ZIP_NRV2BitPutBit(c, 0);
    ZIP_NRV2CodePrefix12(c, (0x1000000));
    ZIP_NRV2BitPutByte(c, 0xff);
    ZIP_NRV2BitFlushBits(c, 0);

    c->codesize = (unsigned int) (c->bb_op - out);
    *out_len = (unsigned int) (c->bb_op - out);
    if (c->cb)
        (*c->cb->callback)(c->textsize,c->codesize,4,c->cb->user);

    ZIP_NRV2Exit(s);
#if !defined(s)
    free(s);
#endif
    return ZIPERR_SUCCESS;
#undef s
}

int ZIP_NRV2DDecompress(const unsigned char *pucSrc, unsigned int  uiSrcLen,
                        unsigned char *pucDest, unsigned int *puiDestLen)
{
    unsigned int bb = 0;
    unsigned int ilen = 0, olen = 0, last_m_off = 1;
    const unsigned int oend = *puiDestLen;
    const unsigned char * uiPosition;

    for (;;)
    {
        unsigned int uiOffset, uiLen;

        while (NRV2_GETBIT8(bb,pucSrc,ilen))
        {
            if( ilen >= uiSrcLen )
                return(ZIPERR_INPUTOVERLAP);
            fail(olen >= oend, ZIPERR_OUTPUTOVERLAP);
            pucDest[olen++] = pucSrc[ilen++];
        }
        uiOffset = 1;
        for (;;)
        {
            uiOffset = uiOffset*2 + NRV2_GETBIT8(bb,pucSrc,ilen);
            fail(ilen >= uiSrcLen, ZIPERR_INPUTOVERLAP);
            fail(uiOffset > (0xffffff) + 3, ZIPERR_OVERLAP);
            if (NRV2_GETBIT8(bb,pucSrc,ilen)) break;
            uiOffset = (uiOffset-1)*2 + NRV2_GETBIT8(bb,pucSrc,ilen);
        }
        if (uiOffset == 2)
        {
            uiOffset = last_m_off;
            uiLen = NRV2_GETBIT8(bb,pucSrc,ilen);
        }
        else
        {
            fail(ilen >= uiSrcLen, ZIPERR_INPUTOVERLAP);
            uiOffset = (uiOffset-3)*256 + pucSrc[ilen++];
            if (uiOffset == (0xffffffff))
                break;
            uiLen = (uiOffset ^ (0xffffffff)) & 1;
            uiOffset >>= 1;
            last_m_off = ++uiOffset;
        }
        uiLen = uiLen*2 + NRV2_GETBIT8(bb,pucSrc,ilen);
        if (uiLen == 0)
        {
            uiLen++;
            do {
                uiLen = uiLen*2 + NRV2_GETBIT8(bb,pucSrc,ilen);
                fail(ilen >= uiSrcLen, ZIPERR_INPUTOVERLAP);
                fail(uiLen >= oend, ZIPERR_OUTPUTOVERLAP);
            } while (!NRV2_GETBIT8(bb,pucSrc,ilen));
            uiLen += 2;
        }
        uiLen += (uiOffset > 0x500);
        fail(olen + uiLen > oend, ZIPERR_OUTPUTOVERLAP);
        fail(uiOffset > olen, ZIPERR_OVERLAP);
        {
            uiPosition = pucDest + olen - uiOffset;
            pucDest[olen++] = *uiPosition++;
            do pucDest[olen++] = *uiPosition++; while (--uiLen > 0);
        }
    }
    *puiDestLen = olen;
    return ilen == uiSrcLen ? ZIPERR_SUCCESS : (ilen < uiSrcLen ? ZIPERR_NOTFINISH : ZIPERR_INPUTOVERLAP);
}


static int
ZIP_NRV2InitMatch ( NRV2COMPRESS *c, NRV2 *s,
             const unsigned char * dict, unsigned int dict_len,
             unsigned long flags )
{
    int r;

    c->init = 1;

    s->c = c;

    c->uiLastLen = c->uiLastOffset = 0;

    c->textsize = c->codesize = c->printcount = 0;
    c->lit_bytes = c->match_bytes = c->rep_bytes = 0;
    c->lazy = 0;

    r = ZIP_NRV2Init(s,dict,dict_len);
    if (r != ZIPERR_SUCCESS)
    {
        ZIP_NRV2Exit(s);
        return r;
    }

    s->use_best_off = (flags & 1) ? 1 : 0;
    return ZIPERR_SUCCESS;
}


/***********************************************************************
//
************************************************************************/

static int
ZIP_NRV2FindMatch ( NRV2COMPRESS *c, NRV2 *s,
             unsigned int this_len, unsigned int skip )
{

    if (skip > 0)
    {
        ZIP_NRV2Accept(s, this_len - skip);
        c->textsize += this_len - skip + 1;
    }
    else
    {
        c->textsize += this_len - skip;
    }

    s->uiLen = NRV2_MATCHLENMIN;

    ZIP_NRV2FindBest(s);
    c->uiLen = s->uiLen;
    c->uiOffset = s->uiOffset;

    ZIP_NRV2GetByte(s);

    if (s->b_char < 0)
    {
        c->look = 0;
        c->uiLen = 0;
        ZIP_NRV2Exit(s);
    }
    else
    {
        c->look = s->uiLook + 1;
    }
    c->bp = c->ip - c->look;

    if (c->cb && c->textsize > c->printcount)
    {
        (*c->cb->callback)(c->textsize,c->codesize,3,c->cb->user);
        c->printcount += 1024;
    }

    return ZIPERR_SUCCESS;
}


/***********************************************************************
// bit buffer
************************************************************************/

static int ZIP_NRV2BitConfig(NRV2COMPRESS *c, int endian, int bitsize)
{
    if (endian != -1)
    {
        if (endian != 0)
            return ZIPERR_GENERAL;
        c->bb_c_endian = endian;
    }
    if (bitsize != -1)
    {
        if (bitsize != 8 && bitsize != 16 && bitsize != 32)
            return ZIPERR_GENERAL;
        c->bb_c_s = bitsize;
        c->bb_c_s8 = bitsize / 8;
    }
    c->bb_b = 0; c->bb_k = 0;
    c->bb_p = NULL;
    c->bb_op = NULL;
    return ZIPERR_SUCCESS;
}


static void ZIP_NRV2BitWriteBits(NRV2COMPRESS *c)
{
    unsigned char * p = c->bb_p;
    unsigned long b = c->bb_b;

    p[0] = (unsigned char)(b >>  0);
    if (c->bb_c_s >= 16)
    {
        p[1] = (unsigned char)(b >>  8);
        if (c->bb_c_s == 32)
        {
            p[2] = (unsigned char)(b >> 16);
            p[3] = (unsigned char)(b >> 24);
        }
    }
}


static void ZIP_NRV2BitPutBit(NRV2COMPRESS *c, unsigned bit)
{

    if (c->bb_k < c->bb_c_s)
    {
        if (c->bb_k == 0)
        {
            c->bb_p = c->bb_op;
            c->bb_op += c->bb_c_s8;
        }

        c->bb_b = (c->bb_b << 1) + bit;
        c->bb_k++;
    }else
    {

        ZIP_NRV2BitWriteBits(c);
        c->bb_p = c->bb_op;
        c->bb_op += c->bb_c_s8;
        c->bb_b = bit;
        c->bb_k = 1;
    }
}


static void ZIP_NRV2BitPutByte(NRV2COMPRESS *c, unsigned b)
{
    /**printf("putbyte %p %p %x  (%d)\n", op, bb_p, x, bb_k);*/
    *c->bb_op++ = (unsigned char)(b);
}


static void ZIP_NRV2BitFlushBits(NRV2COMPRESS *c, unsigned filler_bit)
{
    if (c->bb_k > 0)
    {
        while (c->bb_k != c->bb_c_s)
            ZIP_NRV2BitPutBit(c, filler_bit);
        ZIP_NRV2BitWriteBits(c);
        c->bb_k = 0;
    }
    c->bb_p = NULL;
}

static
int ZIP_NRV2Init(NRV2 *s, const unsigned char * dict, unsigned int dict_len)
{
#if defined(ZIP_USEMALLOC)
    s->aucBlock = NULL;
    s->auiHead3 = NULL;
    s->succ3 = NULL;
    s->best3 = NULL;
    s->llen3 = NULL;
    s->auiHead2 = NULL;
#endif

    if (s->n == 0)
        s->n = NRV2_BUFMAXSIZE;
    if (s->f == 0)
        s->f = NRV2_MATCHLENMAX;
    s->threshold = NRV2_MATCHLENMIN;
    if (s->n > NRV2_BUFMAXSIZE || s->f > NRV2_MATCHLENMAX)
        return ZIPERR_INVALIDARGUMENT;

#if defined(ZIP_USEMALLOC)
    s->aucBlock = (unsigned char *) malloc(s->n + s->f + s->f);
    s->auiHead3 = (unsigned int *) malloc((NRV2_HSIZE)*sizeof(*s->auiHead3));
    s->succ3 = (unsigned int *) malloc((s->n + s->f)*sizeof(*s->succ3));
    s->best3 = (unsigned int *) malloc((s->n + s->f)*sizeof(*s->best3));
    s->llen3 = (unsigned int *) malloc((NRV2_HSIZE)*sizeof(*s->llen3));
    if (!s->aucBlock || !s->auiHead3  || !s->succ3 || !s->best3 || !s->llen3)
        return ZIPERR_NOMEMORY;
    s->auiHead2 = (unsigned int *) malloc((65536)*sizeof(*s->auiHead2));
    if (!s->auiHead2)
        return ZIPERR_NOMEMORY;
#endif

    /* defaults */
    s->max_chain = NRV2_MAXCHAIN;
    s->nice_length = s->f;
    s->use_best_off = 0;
    s->lazy_insert = 0;

    s->uiBlockSize = s->n + s->f;
    if (s->uiBlockSize + s->f >= NRV2_UINTMAX)
        return ZIPERR_GENERAL;
    s->b_wrap = s->aucBlock + s->uiBlockSize;
    s->uiNodeCount = s->n;

    memset(s->llen3, 0, (unsigned int)sizeof(s->llen3[0]) * NRV2_HSIZE);
    memset(s->auiHead2, 0xff, (unsigned int)sizeof(s->auiHead2[0]) * (65536));

    s->uiInPtr = 0;
    ZIP_NRV2InitDict(s,dict,dict_len);
    s->uiBufPtr = s->uiInPtr;
    s->uiFirstRemovePtr = s->uiInPtr;

    s->uiLook = (unsigned int) (s->c->in_end - s->c->ip);
    if (s->uiLook > 0)
    {
        if (s->uiLook > s->f)
            s->uiLook = s->f;
        memcpy(&s->aucBlock[s->uiInPtr],s->c->ip,s->uiLook);
        s->c->ip += s->uiLook;
        s->uiInPtr += s->uiLook;
    }
    if (s->uiInPtr == s->uiBlockSize)
        s->uiInPtr = 0;

    if (s->uiLook >= 2 && s->uiDictLen > 0)
        ZIP_NRV2InsertDict(s,0,s->uiDictLen);

    s->uiRemovePtr = s->uiFirstRemovePtr;
    if (s->uiRemovePtr >= s->uiNodeCount)
        s->uiRemovePtr -= s->uiNodeCount;
    else
        s->uiRemovePtr += s->uiBlockSize - s->uiNodeCount;

    return ZIPERR_SUCCESS;
}


static
void ZIP_NRV2Exit(NRV2 *s)
{
#if defined(ZIP_USEMALLOC)
    /* free in reverse order of allocations */
    free(s->auiHead2); 
    s->auiHead2 = NULL;
    free(s->llen3); 
    s->llen3 = NULL;
    free(s->best3); 
    s->best3 = NULL;
    free(s->succ3); 
    s->succ3 = NULL;
    free(s->auiHead3); 
    s->auiHead3 = NULL;
    free(s->aucBlock); 
    s->aucBlock = NULL;
#else
#endif
}



static
void ZIP_NRV2GetByte(NRV2 *s)
{
    int c;

    if ((c = NRV2_GETBYTE(*(s->c))) < 0)
    {
        if (s->uiLook > 0)
            --s->uiLook;
    }else
    {
        s->aucBlock[s->uiInPtr] = (unsigned char)(c);
        if (s->uiInPtr < s->f)
            s->b_wrap[s->uiInPtr] = (unsigned char)(c);
    }
    if (++s->uiInPtr == s->uiBlockSize)
        s->uiInPtr = 0;
    if (++s->uiBufPtr == s->uiBlockSize)
        s->uiBufPtr = 0;
    if (++s->uiRemovePtr == s->uiBlockSize)
        s->uiRemovePtr = 0;
}


/***********************************************************************
// remove node from lists
************************************************************************/

static
void ZIP_NRV2RemoveNode(NRV2 *s, unsigned int node)
{
    if (s->uiNodeCount == 0)
    {
        unsigned int key;

        key = NRV2_HEAD3(s->aucBlock,node);
        --s->llen3[key];

        key = NRV2_HEAD2(s->aucBlock,node);
        if ((unsigned int) s->auiHead2[key] == node)
            s->auiHead2[key] = NRV2_UINTMAX;
    }
    else
        --s->uiNodeCount;
}


/***********************************************************************
//
************************************************************************/

static
void ZIP_NRV2Accept(NRV2 *s, unsigned int n)
{

    if (n > 0) do
    {
        unsigned int key;

        ZIP_NRV2RemoveNode(s,s->uiRemovePtr);

        /* add bp into HEAD3 */
        key = NRV2_HEAD3(s->aucBlock,s->uiBufPtr);
        s->succ3[s->uiBufPtr] = s->auiHead3[key];
        s->auiHead3[key] = (unsigned int)(s->uiBufPtr);
        s->best3[s->uiBufPtr] = (unsigned int)(s->f + 1);
        s->llen3[key]++;

            /* add bp into HEAD2 */
        key = NRV2_HEAD2(s->aucBlock,s->uiBufPtr);
        s->auiHead2[key] = (unsigned int)(s->uiBufPtr);

        ZIP_NRV2GetByte(s);
    } while (--n > 0);
}

static int
ZIP_NRV2CodeMatchLen(NRV2COMPRESS *c, unsigned int uiLen, unsigned int uiOffset)
{
    int b;
    if (uiLen < 2 || (uiLen == 2 && (uiOffset > NRV2_M2MAXOFFSET))
        || uiOffset > c->conf.max_offset)
        return -1;

    uiLen = uiLen - 2 - (uiOffset > NRV2_M2MAXOFFSET);

    if (uiOffset == c->uiLastOffset)
        b = 1 + 2;
    else
    {
        b = 1 + 9;
        uiOffset = (uiOffset - 1) >> 7;
        while (uiOffset > 0)
        {
            b += 3;
            uiOffset >>= 2;
        }
    }

    b += 2;
    if (uiLen < 3)
        return b;
    uiLen -= 3;
    do {
        b += 2;
        uiLen >>= 1;
    } while (uiLen > 0);

    return b;
}

static void
ZIP_NRV2CodeRun(NRV2COMPRESS *c, const unsigned char * ii, unsigned int lit)
{
    if (lit == 0)
        return;
    c->lit_bytes += lit;
    if (lit > c->result[5])
        c->result[5] = lit;
    do {
        ZIP_NRV2BitPutBit(c, 1);
        ZIP_NRV2BitPutByte(c, *ii++);
    } while (--lit > 0);
}

static void
ZIP_NRV2CodeMatch(NRV2COMPRESS *c, unsigned int uiLen, const unsigned int uiOffset)
{
    unsigned m_low = 0;

    while (uiLen > c->conf.max_match)
    {
        ZIP_NRV2CodeMatch(c, c->conf.max_match - 3, uiOffset);
        uiLen -= c->conf.max_match - 3;
    }

    c->match_bytes += uiLen;
    if (uiLen > c->result[3])
        c->result[3] = uiLen;
    if (uiOffset > c->result[1])
        c->result[1] = uiOffset;

    ZIP_NRV2BitPutBit(c, 0);

    uiLen = uiLen - 1 - (uiOffset > NRV2_M2MAXOFFSET);
    m_low = (uiLen >= 4) ? 0u : (unsigned) uiLen;
    if (uiOffset == c->uiLastOffset)
    {
        ZIP_NRV2BitPutBit(c, 0);
        ZIP_NRV2BitPutBit(c, 1);
        ZIP_NRV2BitPutBit(c, m_low > 1);
        ZIP_NRV2BitPutBit(c, m_low & 1);
    }
    else
    {
        ZIP_NRV2CodePrefix12(c, 1 + ((uiOffset - 1) >> 7));
        ZIP_NRV2BitPutByte(c, ((((unsigned)uiOffset - 1) & 0x7f) << 1) | ((m_low > 1) ? 0 : 1));
        ZIP_NRV2BitPutBit(c, m_low & 1);
    }
    if (uiLen >= 4)
        ZIP_NRV2CodePrefix11(c, uiLen - 4);

    c->uiLastOffset = uiOffset;
}

static void ZIP_NRV2CodePrefix12(NRV2COMPRESS *c, unsigned long i)
{
    if (i >= 2)
    {
        unsigned long t = 2;
        do {
            i -= t;
            t <<= 2;
        } while (i >= t);
        do {
            t >>= 1;
            ZIP_NRV2BitPutBit(c, (i & t) ? 1 : 0);
            ZIP_NRV2BitPutBit(c, 0);
            t >>= 1;
            ZIP_NRV2BitPutBit(c, (i & t) ? 1 : 0);
        } while (t > 2);
    }
    ZIP_NRV2BitPutBit(c, (unsigned)i & 1);
    ZIP_NRV2BitPutBit(c, 1);
}

static
void ZIP_NRV2FindBest(NRV2 *s)
{
    unsigned int key;
    unsigned int cnt, node;
    unsigned int len;

    /* get current head, add bp into HEAD3 */
    key = NRV2_HEAD3(s->aucBlock,s->uiBufPtr);
    node = s->succ3[s->uiBufPtr] = s->auiHead3[key];
    cnt = s->llen3[key]++;
    if (cnt > s->max_chain && s->max_chain > 0)
        cnt = s->max_chain;
    s->auiHead3[key] = (unsigned int)(s->uiBufPtr);

    s->b_char = s->aucBlock[s->uiBufPtr];
    len = s->uiLen;
    if (s->uiLen >= s->uiLook)
    {
        if (s->uiLook == 0)
            s->b_char = -1;
        s->uiOffset = 0;
        s->best3[s->uiBufPtr] = (unsigned int)(s->f + 1);
    }
    else
    {
        if (ZIP_NRV2Search2(s) && s->uiLook >= 3)
            ZIP_NRV2Search(s,node,cnt);

        if (s->uiLen > len)
            s->uiOffset = (s->uiBufPtr > (s->uiPosition) ? s->uiBufPtr - (s->uiPosition) : s->uiBlockSize - ((s->uiPosition) - s->uiBufPtr));
        s->best3[s->uiBufPtr] = (unsigned int)(s->uiLen);
    }

    ZIP_NRV2RemoveNode(s,s->uiRemovePtr);

    key = NRV2_HEAD2(s->aucBlock,s->uiBufPtr);
    s->auiHead2[key] = (unsigned int)(s->uiBufPtr);
}

static
void ZIP_NRV2InitDict(NRV2 *s, const unsigned char * dict, unsigned int dict_len)
{
    s->pucDict = s->pucDictEnd = NULL;
    s->uiDictLen = 0;

    if (!dict || dict_len <= 0)
        return;
    if (dict_len > s->n)
    {
        dict += dict_len - s->n;
        dict_len = s->n;
    }

    s->pucDict = dict;
    s->uiDictLen = dict_len;
    s->pucDictEnd = dict + dict_len;
    memcpy(s->aucBlock,dict,dict_len);
    s->uiInPtr = dict_len;
}


static
void ZIP_NRV2InsertDict(NRV2 *s, unsigned int node, unsigned int len)
{
    unsigned int key;

    s->uiNodeCount = s->n - len;
    s->uiFirstRemovePtr = node;

    while (len-- > 0)
    {
        key = NRV2_HEAD3(s->aucBlock,node);
        s->succ3[node] = s->auiHead3[key];
        s->auiHead3[key] = (unsigned int)(node);
        s->best3[node] = (unsigned int)(s->f + 1);
        s->llen3[key]++;

        key = NRV2_HEAD2(s->aucBlock,node);
        s->auiHead2[key] = (unsigned int)(node);

        node++;
    }
}

static void ZIP_NRV2CodePrefix11(NRV2COMPRESS *c, unsigned long i)
{
    if (i >= 2)
    {
        unsigned long t = 4;
        i += 2;
        do {
            t <<= 1;
        } while (i >= t);
        t >>= 1;
        do {
            t >>= 1;
            ZIP_NRV2BitPutBit(c, (i & t) ? 1 : 0);
            ZIP_NRV2BitPutBit(c, 0);
        } while (t > 2);
    }
    ZIP_NRV2BitPutBit(c, (unsigned)i & 1);
    ZIP_NRV2BitPutBit(c, 1);
}

static
void ZIP_NRV2Search(NRV2 *s, unsigned int node, unsigned int cnt)
{
    const unsigned char * p1;
    const unsigned char * p2;
    const unsigned char * px;
    unsigned int uiLen = s->uiLen;
    const unsigned char * b  = s->aucBlock;
    const unsigned char * bp = s->aucBlock + s->uiBufPtr;
    const unsigned char * bx = s->aucBlock + s->uiBufPtr + s->uiLook;
    unsigned char scan_end1;

    scan_end1 = bp[uiLen - 1];
    for ( ; cnt-- > 0; node = s->succ3[node])
    {
        p1 = bp;
        p2 = b + node;
        px = bx;

        if (
            p2[uiLen - 1] == scan_end1 &&
            p2[uiLen] == p1[uiLen] &&
            p2[0] == p1[0] &&
            p2[1] == p1[1])
        {
            unsigned int i;

            p1 += 2; p2 += 2;
            do {} while (++p1 < px && *p1 == *++p2);
            i = (unsigned int) (p1 - bp);

            if (i > uiLen)
            {
                s->uiLen = uiLen = i;
                s->uiPosition = node;
                if (uiLen == s->uiLook)
                    return;
                if (uiLen >= s->nice_length)
                    return;
                if (uiLen > (unsigned int) s->best3[node])
                    return;
                scan_end1 = bp[uiLen - 1];
            }
        }
    }
}

static int ZIP_NRV2Search2(NRV2 *pNRV2)
{
    unsigned int uiKey;

    uiKey = pNRV2->auiHead2[ NRV2_HEAD2(pNRV2->aucBlock,pNRV2->uiBufPtr) ];
    if (uiKey == NRV2_UINTMAX)
        return 0;

    if (pNRV2->uiLen < 2)
    {
        pNRV2->uiLen = 2;
        pNRV2->uiPosition = uiKey;
    }
    return 1;
}

#define MAXLEN	1024*1024UL

const unsigned long CRC32TABLE[256]=
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static unsigned long CRC_Calc32(unsigned long ulCRC,const unsigned char *pucInData,unsigned long ulInLen)
{
	if( pucInData && ulInLen )
	{
    	do{
       		ulCRC = ((ulCRC >> 8) & 0xFFFFFF) ^ CRC32TABLE[(unsigned char)( (ulCRC & 0xff) ^ *pucInData++ )];
     	} while(--ulInLen);
	}
  	return ulCRC;
}

int ZIP_NRVData2Compress(PACK_INFO *packInfo,char *inData, int inLen, char *outData, int outLen)
{
	int count = 0;
	char tempBuf[300];
	time_t timep;
	struct tm *pt;
	int i = 0;
	unsigned long CRC32Val;
	
	memset(tempBuf,0x00,sizeof(tempBuf));
	
    memcpy(&outData[count],"%DWN-1.0-SAND%",14);
	count += 14;
	
	//初始化应用类型
    switch(packInfo->bintype)
	{
		case 0x01 ... 0x04:
			outData[count++] = 0x41;
			outData[count++] = 0x01;
            outData[count++] = packInfo->bintype;
			break;
		default:
			printf("Bin type error!\n");
			return -1;
	}
	
	//初始化应用编号
	outData[count++] = 0x42;
    switch(packInfo->bintypecode)
	{
		case 0 ... 255:
			outData[count++] = 0x01;
            outData[count++] = packInfo->bintypecode;
			break;
		case 256 ... 65535:
			outData[count++] = 0x02;
            outData[count++] = packInfo->bintypecode>>8;
            outData[count++] = packInfo->bintypecode;
			break;
		default:
			printf("Bin type code error!\n");
			return -2;
	}
	
	//初始化应用文件名称信息
    if(packInfo->binname != NULL)
	{
		outData[count++] = 0x43;
        outData[count++] = strlen(packInfo->binname);
        memcpy(&outData[count],packInfo->binname,outData[count-1]);
		count += outData[count-1];
	}
	else
	{
		printf("Bin name error!\n");
		return -3;
	}
	
	//初始化应用标签信息
    if(packInfo->bindesc != NULL)
	{
		outData[count++] = 0x44;
        outData[count++] = strlen(packInfo->bindesc);
        memcpy(&outData[count],packInfo->bindesc,outData[count-1]);
		count += outData[count-1];
	}
	else
	{
		printf("Bin describe error!\n");
		return -4;
	}
	
	//初始化应用版本信息
	outData[count++] = 0x45;
    if(!packInfo->binversion[0])
	{
		outData[count++] = 0x02;
		outData[count++] = 0x31;
		outData[count++] = 0x30;
	}
	else
	{
        outData[count++] = strlen(packInfo->binversion);
        memcpy(&outData[count],packInfo->binversion,outData[count-1]);
		count += outData[count-1];
	}
	
	//初始化压缩类型信息
	outData[count++] = 0x46;
	outData[count++] = 0x01;
	outData[count++] = 0x01;
	
	//初始化创建压缩文件时间信息
	time(&timep);
	pt = localtime(&timep);
	pt->tm_mon++;
	pt->tm_year -= 100;
	
	outData[count++] = 0x48;
	outData[count++] = 0x0E;
	sprintf(tempBuf,"20%02d%02d%02d%02d%02d%02d",
			pt->tm_year,pt->tm_mon,pt->tm_mday,
			pt->tm_hour,pt->tm_min,pt->tm_sec);
	memcpy(&outData[count],tempBuf,14);
	count += 14;
	
	//初始化国家代码信息
	outData[count++] = 0x49;
    if(!packInfo->bincountryid[0])
	{
		outData[count++] = 0x03;
		memcpy(&outData[count],"086",14);
		count += 3;
	}
	else
	{
        outData[count++] = strlen(packInfo->bincountryid);
        memcpy(&outData[count],packInfo->bincountryid,outData[count-1]);
		count += outData[count-1];
	}

    //初始化CPU类型
    outData[count++] = 0x4a;
    outData[count++] = 0x01;
    outData[count++] = packInfo->bincputype;
	
	//初始化被压缩源文件长度信息
	outData[count++] = 0x4D;
	outData[count++] = 0x04;
	if(inLen)
	{
		outData[count++] = inLen >> 24;
		outData[count++] = inLen >> 16;
		outData[count++] = inLen >> 8;
		outData[count++] = inLen;
	}
	else
	{
		printf("Bin src size error!\n");
		return -5;
	}
	
	//初始化压缩文件数据CRC-32信息
	outData[count++] = 0x4E;
	outData[count++] = 0x04;
	ZIP_NRV2Compress( inData, inLen,&outData[count+9], &outLen,  NULL,5, NULL,NULL);
	CRC32Val = CRC_Calc32(0xFFFFFFFF,&outData[count+9],outLen);
	if(CRC32Val != 0)
	{
		outData[count++] = CRC32Val >> 24;
		outData[count++] = CRC32Val >> 16;
		outData[count++] = CRC32Val >> 8;
		outData[count++] = CRC32Val;
	}
	else
	{
		printf("Bin CRC error!\n");
		return -6;
	}
	
	//初始化压缩后的数据大小
	outData[count++] = 0x4F;
	outData[count++] = 0x83;
	if(outLen)
	{
		outData[count++] = outLen >> 16;
		outData[count++] = outLen >> 8;
		outData[count++] = outLen;
	}
	else
	{
		printf("Bin zip size error!\n");
		return -7;
	}
	
	return count+outLen;
}

