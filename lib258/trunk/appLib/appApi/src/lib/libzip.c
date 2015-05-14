
#include <tools.h>
#include <libzip.h>


#define fail(x,r)   if (x) { *puiDestLen = olen; return r; }


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


int ZIP_NRV2InitMatch ( NRV2COMPRESS *c, NRV2 *s,
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

int ZIP_NRV2FindMatch ( NRV2COMPRESS *c, NRV2 *s,
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

int ZIP_NRV2BitConfig(NRV2COMPRESS *c, int endian, int bitsize)
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


void ZIP_NRV2BitWriteBits(NRV2COMPRESS *c)
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


void ZIP_NRV2BitPutBit(NRV2COMPRESS *c, unsigned bit)
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


void ZIP_NRV2BitPutByte(NRV2COMPRESS *c, unsigned b)
{
    /**printf("putbyte %p %p %x  (%d)\n", op, bb_p, x, bb_k);*/
    *c->bb_op++ = (unsigned char)(b);
}


void ZIP_NRV2BitFlushBits(NRV2COMPRESS *c, unsigned filler_bit)
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

int ZIP_NRV2CodeMatchLen(NRV2COMPRESS *c, unsigned int uiLen, unsigned int uiOffset)
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

void ZIP_NRV2CodeRun(NRV2COMPRESS *c, const unsigned char * ii, unsigned int lit)
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

void ZIP_NRV2CodeMatch(NRV2COMPRESS *c, unsigned int uiLen, const unsigned int uiOffset)
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

void ZIP_NRV2CodePrefix12(NRV2COMPRESS *c, unsigned long i)
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

void ZIP_NRV2InitDict(NRV2 *s, const unsigned char * dict, unsigned int dict_len)
{
    s->pucDict = s->pucDictEnd = NULL;
    s->uiDictLen = 0;

    if(  (!dict)
/*       ||(dict_len <= 0)*/
      )
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

void ZIP_NRV2CodePrefix11(NRV2COMPRESS *c, unsigned long i)
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

int ZIP_NRV2Search2(NRV2 *pNRV2)
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

