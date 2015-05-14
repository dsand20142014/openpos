/*
	Library Function
--------------------------------------------------------------------------
	FILE  libnn.c
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

static NN_DIGIT NN_AddDigitMult(NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *, unsigned int);
static NN_DIGIT NN_SubDigitMult(NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *, unsigned int);
static unsigned int NN_DigitBits(NN_DIGIT);

/* Decodes character string b into a, where character string is ordered
   from most to least significant.

   Lengths: a[digits], b[len].
   Assumes b[i] = 0 for i < len - digits * NN_DIGITLEN. (Otherwise most
   significant bytes are truncated.)
 */
void NN_Decode (NN_DIGIT *pDigit, unsigned int uiDigit, unsigned char *pucInData, unsigned int
uiInLen)
{
	NN_DIGIT Digit;
	unsigned int uiI;
	unsigned int uiJ;
  	int iJ;

  	for (uiI = 0, iJ= uiInLen - 1; uiI < uiDigit && iJ >= 0; uiI++)
	{
    		Digit = 0;
    		for (uiJ = 0; iJ >= 0 && uiJ < NN_DIGITBITS; iJ--, uiJ += 8)
      			Digit |= ((NN_DIGIT)pucInData[iJ]) << uiJ;
    		*(pDigit+uiI) = Digit;
  	}
  	for (; uiI < uiDigit; uiI++)
    		*(pDigit+uiI) = 0;
}

/* Encodes b into character string a, where character string is ordered
   from most to least significant.

   Lengths: a[len], b[digits].
   Assumes NN_Bits (b, digits) <= 8 * len. (Otherwise most significant
   digits are truncated.)
 */
void NN_Encode (unsigned char *pucOutData, unsigned int uiLen, NN_DIGIT *pDigit, unsigned int
uiDigit)
{
	NN_DIGIT Digit;
	unsigned int uiI;
	unsigned int uiJ;
	int iJ;

  	for (uiI = 0, iJ = uiLen - 1; uiI < uiDigit && iJ >= 0; uiI++)
	{
    		Digit = *(pDigit+uiI);
    		for (uiJ = 0; iJ >= 0 && uiJ < NN_DIGITBITS; iJ--, uiJ += 8)
      			pucOutData[iJ] = (unsigned char)(Digit >> uiJ);
  	}

  	for (; iJ >= 0; iJ--)
    		pucOutData[iJ] = 0;
}

/* Assigns a = b.

   Lengths: a[digits], b[digits].
 */
void NN_Assign (NN_DIGIT *pDest, NN_DIGIT *pSrc, unsigned int uiDigit)
{
	unsigned int uiI;

  	for (uiI = 0; uiI < uiDigit; uiI++)
    		*(pDest+uiI) = *(pSrc+uiI);
}

/* Assigns a = 0.

   Lengths: a[digits].
 */
void NN_AssignZero (NN_DIGIT *pDigit, unsigned int uiDigit)
{
  	unsigned int uiI;

  	for (uiI = 0; uiI < uiDigit; uiI++)
    		*(pDigit+uiI) = 0;
}

/* Assigns a = 2^b.

   Lengths: a[digits].
   Requires b < digits * NN_DIGITBITS.
 */
void NN_Assign2Exp (NN_DIGIT *pDigit,unsigned int uiLen,unsigned int uiDigit)
{
	NN_AssignZero (pDigit, uiDigit);

	if (uiLen >= uiDigit * NN_DIGITBITS)
		return;

  	*(pDigit+uiLen/NN_DIGITBITS) = (NN_DIGIT)1 << (uiLen%NN_DIGITBITS);
}

/* Computes pDest = pSrc1 + pSrc2. Returns carry.

   Lengths: pDest[digits], pSrc1[digits], pSrc2[digits].
 */
NN_DIGIT NN_Add (NN_DIGIT *pDest, NN_DIGIT *pSrc1, NN_DIGIT *pSrc2,unsigned int uiDigit)
{
  	NN_DIGIT Digit;
	NN_DIGIT Carry;
  	unsigned int uiI;

  	Carry = 0;
  	for (uiI = 0; uiI < uiDigit; uiI++)
	{
    		if ((Digit = pSrc1[uiI] + Carry) < Carry)
      			Digit = pSrc2[uiI];
    		else
			if ((Digit += pSrc2[uiI]) < pSrc2[uiI])
      				Carry = 1;
    			else
      				Carry = 0;
    		pDest[uiI] = Digit;
  	}

  	return (Carry);
}

/* Computes pDest = pSrc1 - pSrc2. Returns borrow.

   Lengths: pDest[uiDigit], pSrc1[uiDigit], pSrc2[uiDigit].
 */
NN_DIGIT NN_Sub (NN_DIGIT *pDest,NN_DIGIT * pSrc1,NN_DIGIT * pSrc2,unsigned int  uiDigit)
{
  	NN_DIGIT Digit;
	NN_DIGIT Borrow;
  	unsigned int uiI;

	Borrow = 0;

	for (uiI = 0; uiI < uiDigit; uiI++)
	{
    		if ((Digit = pSrc1[uiI] - Borrow) > (NN_MAXDIGIT - Borrow))
      			Digit = NN_MAXDIGIT - pSrc2[uiI];
    		else
			if ((Digit -= pSrc2[uiI]) > (NN_MAXDIGIT - pSrc2[uiI]))
      				Borrow = 1;
    			else
      				Borrow = 0;
    		pDest[uiI] = Digit;
  	}

  	return (Borrow);
}

/* Computes pDest = pSrc1 * pSrc2.

   Lengths: pDest[2*uiDigit], pSrc1[uiDigit], pSrc2[uiDigit].
   Assumes uiDigit < NN_MAXDIGITS.
 */
void NN_Mult (NN_DIGIT *pDest,NN_DIGIT * pSrc1,NN_DIGIT * pSrc2,unsigned int  uiDigit)
{
	NN_DIGIT aDigit[2*NN_MAXDIGITS];
	unsigned int uiDigit1;
  	unsigned int uiDigit2;
	unsigned int uiI;

  	NN_AssignZero (aDigit, 2 * uiDigit);

  	uiDigit1 = NN_Digits (pSrc1, uiDigit);
  	uiDigit2 = NN_Digits (pSrc2, uiDigit);

  	for (uiI = 0; uiI < uiDigit1; uiI++)
    		aDigit[uiI+uiDigit2] += NN_AddDigitMult (&aDigit[uiI], &aDigit[uiI], pSrc1[uiI], pSrc2,
uiDigit2);

  	NN_Assign (pDest, aDigit, 2 * uiDigit);

  	memset (aDigit, 0, sizeof (aDigit));
}

/* Computes pDest = pSrc1 * 2^uiSrc2 (i.e., shifts left c bits), returning carry.

   Lengths: pDest[uiDigit], pSrc1[uiDigit].
   Requires pSrc2 < NN_DIGITBITS.
 */
NN_DIGIT NN_LShift (NN_DIGIT *pDest,NN_DIGIT * pSrc1,unsigned int uiSrc2,unsigned int uiDigit)
{
	NN_DIGIT Digit;
	NN_DIGIT Carry;
  	unsigned int uiI;
	unsigned int uiJ;

  	if (uiSrc2 >= NN_DIGITBITS)
    		return (0);

  	uiJ = NN_DIGITBITS - uiSrc2;
  	Carry = 0;
  	for (uiI = 0; uiI < uiDigit; uiI++)
	{
    		Digit = pSrc1[uiI];
    		pDest[uiI] = (Digit << uiSrc2) | Carry;
    		Carry = uiSrc2 ? (Digit >> uiJ) : 0;
  	}

  	return (Carry);
}

/* Computes pDest = pSrc1 div 2^uiSrc2 (i.e., shifts right c bits), returning carry.

   Lengths: pDest[uiDigit], pSrc1[uiDigit].
   Requires: uiSrc2 < NN_DIGITBITS.
 */
NN_DIGIT NN_RShift (NN_DIGIT *pDest,NN_DIGIT * pSrc1,unsigned int uiSrc2,unsigned int uiDigit)
{
  	NN_DIGIT Digit;
	NN_DIGIT Carry;
  	int iI;
  	unsigned int uiJ;

  	if (uiSrc2 >= NN_DIGITBITS)
    		return (0);

  	uiJ = NN_DIGITBITS - uiSrc2;
  	Carry = 0;
  	for (iI = uiDigit - 1; iI >= 0; iI--)
	{
    		Digit = pSrc1[iI];
    		pDest[iI] = (Digit >> uiSrc2) | Carry;
    		Carry = uiSrc2 ? (Digit << uiJ) : 0;
  	}

  	return (Carry);
}

/* Computes pDest1 = pSrc1 div pSrc2 and pDest2 = pSrc1 mod pSrc2.

   Lengths: pDest1[uiDigit1], pDest2[uiDigit2], pSrc1[uiDigit1], pSrc2[uiDigit2].
   Assumes pSrc2 > 0, uiDigit1 < 2 * NN_MAXDIGITS,
           uiDigit2 < NN_MAXDIGITS.
 */
void NN_Div(NN_DIGIT *pDest1,NN_DIGIT * pDest2,NN_DIGIT * pSrc1,unsigned int uiDigit1,NN_DIGIT *
pSrc2,unsigned int uiDigit2)
{
	NN_DIGIT Digit1;
	NN_DIGIT Digit2;
	NN_DIGIT aDigit1[2*NN_MAXDIGITS+1], aDigit2[NN_MAXDIGITS];
  	int iI;
  	unsigned int uiDigit3, uiShift;

  	uiDigit3 = NN_Digits (pSrc2, uiDigit2);
  	if (uiDigit3 == 0)
    		return;

  	/* Normalize operands.
   	*/
  	uiShift = NN_DIGITBITS - NN_DigitBits (pSrc2[uiDigit3-1]);
  	NN_AssignZero (aDigit1, uiDigit3);
  	aDigit1[uiDigit1] = NN_LShift (aDigit1, pSrc1, uiShift, uiDigit1);
  	NN_LShift (aDigit2, pSrc2, uiShift, uiDigit3);
  	Digit2 = aDigit2[uiDigit3-1];

  	NN_AssignZero (pDest1, uiDigit1);

  	for (iI = uiDigit1-uiDigit3; iI >= 0; iI--)
	{
    		/* Underestimate quotient digit and subtract.
     		*/
    		if (Digit2 == NN_MAXDIGIT)
      			Digit1 = aDigit1[iI+uiDigit3];
    		else
      			NN_DigitDiv (&Digit1, &aDigit1[iI+uiDigit3-1], Digit2 + 1);
    		aDigit1[iI+uiDigit3] -= NN_SubDigitMult (&aDigit1[iI], &aDigit1[iI], Digit1, aDigit2,
uiDigit3);

    		/* Correct estimate.
     		*/
    		while (aDigit1[iI+uiDigit3] || (NN_Cmp (&aDigit1[iI], aDigit2, uiDigit3) >= 0))
		{
      			Digit1++;
      			aDigit1[iI+uiDigit3] -= NN_Sub (&aDigit1[iI], &aDigit1[iI], aDigit2, uiDigit3);
    		}
    		pDest1[iI] = Digit1;
  	}

  	/* Restore result.
   	*/
  	NN_AssignZero (pDest2, uiDigit2);
  	NN_RShift (pDest2, aDigit1, uiShift, uiDigit3);

  	/* Zeroize potentially sensitive information.
   	*/
  	memset (aDigit1, 0, sizeof (aDigit1));
  	memset (aDigit2, 0, sizeof (aDigit2));
}

/* Computes pDest = pSrc1 mod pSrc2.

   Lengths: pDest[uiDigit2], pSrc1[uiDigit1], pSrc2[uiDigit2].
   Assumes pSrc2 > 0, uiDigit1 < 2 * NN_MAXDIGITS, uiDigit2 < NN_MAXDIGITS.
 */
void NN_Mod (NN_DIGIT *pDest,NN_DIGIT *pSrc1,unsigned int uiDigit1,NN_DIGIT *pSrc2,unsigned int
uiDigit2)
{
	NN_DIGIT aDigit[2 * NN_MAXDIGITS];

  	NN_Div (aDigit, pDest, pSrc1, uiDigit1, pSrc2, uiDigit2);

  	/* Zeroize potentially sensitive information.
   	*/
  	memset(aDigit, 0, sizeof (aDigit));
}

/* Computes pDest = pSrc1 * pSrc2 mod pSrc3.

   Lengths: pDest[uiDigit], pSrc1[uiDigit], pSrc2[uiDigit], pSrc3[uiDigit].
   Assumes pSrc3 > 0, uiDigit < NN_MAXDIGITS.
 */
void NN_ModMult (NN_DIGIT *pDest, NN_DIGIT *pSrc1, NN_DIGIT *pSrc2, NN_DIGIT *pSrc3,unsigned int
uiDigit)
{
	NN_DIGIT aDigit[2*NN_MAXDIGITS];

  	NN_Mult (aDigit, pSrc1, pSrc2, uiDigit);
  	NN_Mod (pDest, aDigit, 2 * uiDigit, pSrc3, uiDigit);

  	/* Zeroize potentially sensitive information.
   	*/
  	memset(aDigit, 0, sizeof (aDigit));
}

/* Computes pDest = pSrc1^pSrc2 mod pSrc3.

   Lengths: pDest[uiDigit2], pSrc1[uiDigit2], pSrc2[uiDigit1], pSrc3[uiDigit2].
   Assumes pSrc3 > 0, uiDigit1 > 0, uiDigit2 < NN_MAXDIGITS.
 */
void NN_ModExp (NN_DIGIT *pDest,NN_DIGIT * pSrc1,NN_DIGIT * pSrc2,unsigned int uiDigit1,NN_DIGIT
* pSrc3,unsigned int uiDigit2)
{
  	NN_DIGIT aPower[3][NN_MAXDIGITS];
	NN_DIGIT Digit;
	NN_DIGIT aDigit[NN_MAXDIGITS];
  	int iI;
  	unsigned int uiBits;
	unsigned int uiJ;
	unsigned int uiS;

  	/* Store pSrc1, pSrc1^2 mod pSrc3, and pSrc1^3 mod pSrc3.
   	*/
  	NN_Assign (aPower[0], pSrc1, uiDigit2);
  	NN_ModMult (aPower[1], aPower[0], pSrc1, pSrc3, uiDigit2);
  	NN_ModMult (aPower[2], aPower[1], pSrc1, pSrc3, uiDigit2);
  	NN_ASSIGN_DIGIT (aDigit, 1, uiDigit2);
  	uiDigit1 = NN_Digits (pSrc2, uiDigit1);
  	for (iI = uiDigit1 - 1; iI >= 0; iI--)
	{
    		Digit = pSrc2[iI];
    		uiBits = NN_DIGITBITS;

    		/* Scan past leading zero bits of most significant digit.
     		*/
    		if (iI == (int)(uiDigit1 - 1))
		{
      			while (! NN_DIGIT2MSB (Digit))
			{
        			Digit <<= 2;
        			uiBits -= 2;
      			}
    		}

    		for (uiJ = 0; uiJ < uiBits; uiJ += 2, Digit <<= 2)
		{
      			/* Compute aDigit = aDigit^4 * pSrc1^uiS mod pSrc3, where uiS = two MSB'uiS of Digit.
       		*/
      			NN_ModMult (aDigit, aDigit, aDigit, pSrc3, uiDigit2);
      			NN_ModMult (aDigit, aDigit, aDigit, pSrc3, uiDigit2);
      			if ((uiS = NN_DIGIT2MSB (Digit)) != 0)
        			NN_ModMult (aDigit, aDigit, aPower[uiS-1], pSrc3, uiDigit2);
    		}
  	}

  	NN_Assign (pDest, aDigit, uiDigit2);

  	/* Zeroize potentially sensitive information.
   	*/
  	memset (aPower, 0, sizeof (aPower));
  	memset (aDigit, 0, sizeof (aDigit));
}

/* Compute pDest = 1/pSrc1 mod pSrc2, assuming inverse exists.

   Lengths: pDest[uiDigit], pSrc1[uiDigit], pSrc2[uiDigit].
   Assumes gcd (pSrc1, pSrc2) = 1, uiDigit < NN_MAXDIGITS.
 */
void NN_ModInv (NN_DIGIT *pDest, NN_DIGIT *pSrc1, NN_DIGIT *pSrc2,unsigned int uiDigit)
{
  	NN_DIGIT q[NN_MAXDIGITS];
	NN_DIGIT t1[NN_MAXDIGITS];
	NN_DIGIT t3[NN_MAXDIGITS];
    	NN_DIGIT u1[NN_MAXDIGITS];
	NN_DIGIT u3[NN_MAXDIGITS];
	NN_DIGIT v1[NN_MAXDIGITS];
    	NN_DIGIT v3[NN_MAXDIGITS];
	NN_DIGIT w[2*NN_MAXDIGITS];
  	int iSign;

  	/* Apply extended Euclidean algorithm, modified to avoid negative
     	numbers.
   	*/
  	NN_ASSIGN_DIGIT (u1, 1, uiDigit);
  	NN_AssignZero (v1, uiDigit);
  	NN_Assign (u3, pSrc1, uiDigit);
  	NN_Assign (v3, pSrc2, uiDigit);
  	iSign = 1;

  	while (! NN_Zero (v3, uiDigit))
	{
    		NN_Div (q, t3, u3, uiDigit, v3, uiDigit);
    		NN_Mult (w, q, v1, uiDigit);
    		NN_Add (t1, u1, w, uiDigit);
    		NN_Assign (u1, v1, uiDigit);
    		NN_Assign (v1, t1, uiDigit);
    		NN_Assign (u3, v3, uiDigit);
    		NN_Assign (v3, t3, uiDigit);
    		iSign = -iSign;
  	}

  	/* Negate result if sign is negative.
    	*/
  	if (iSign < 0)
    		NN_Sub (pDest, pSrc2, u1, uiDigit);
  	else
    		NN_Assign (pDest, u1, uiDigit);

  	/* Zeroize potentially sensitive information.
   	*/
  	memset(q, 0, sizeof (q));
  	memset(t1, 0, sizeof (t1));
  	memset(t3, 0, sizeof (t3));
  	memset(u1, 0, sizeof (u1));
  	memset(u3, 0, sizeof (u3));
  	memset(v1, 0, sizeof (v1));
  	memset(v3, 0, sizeof (v3));
  	memset(w, 0, sizeof (w));
}

/* Computes pDest = gcd(pSrc1, pSrc2).

   Lengths: pDest[uiDigit], pSrc1[uiDigit], pSrc2[uiDigit].
   Assumes pSrc1 > pSrc2, uiDigit < NN_MAXDIGITS.
 */
void NN_Gcd (NN_DIGIT *pDest, NN_DIGIT *pSrc1, NN_DIGIT *pSrc2, unsigned int uiDigit)
{
  	NN_DIGIT t[NN_MAXDIGITS];
	NN_DIGIT u[NN_MAXDIGITS];
	NN_DIGIT v[NN_MAXDIGITS];

  	NN_Assign (u, pSrc1, uiDigit);
  	NN_Assign (v, pSrc2, uiDigit);

  	while (! NN_Zero (v, uiDigit))
	{
    		NN_Mod (t, u, uiDigit, v, uiDigit);
    		NN_Assign (u, v, uiDigit);
    		NN_Assign (v, t, uiDigit);
  	}

  	NN_Assign (pDest, u, uiDigit);

  	/* Zeroize potentially sensitive information.
   	*/
  	memset (t, 0, sizeof (t));
  	memset (u, 0, sizeof (u));
  	memset (v, 0, sizeof (v));
}

/* Returns sign of pDest - pSrc.

   Lengths: pDest[uiDigit], pSrc[uiDigit].
 */
int NN_Cmp (NN_DIGIT *pDest, NN_DIGIT *pSrc,unsigned int uiDigit)
{
  	int iI;

  	for (iI = uiDigit - 1; iI >= 0; iI--)
	{
    		if (pDest[iI] > pSrc[iI])
      			return (1);
    		if (pDest[iI] < pSrc[iI])
      			return (-1);
  	}

  	return (0);
}

/* Returns nonzero iff pDest is zero.

   Lengths: pDest[uiDigit].
 */
int NN_Zero (NN_DIGIT *pDest, unsigned int uiDigit)
{
  	unsigned int iI;

  	for (iI = 0; iI < uiDigit; iI++)
    		if (pDest[iI])
      			return (0);

  	return (1);
}

/* Returns the significant length of pDest in bits.

   Lengths: pDest[uiDigit].
 */
unsigned int NN_Bits (NN_DIGIT *pDest, unsigned int uiDigit)
{
  	if ((uiDigit = NN_Digits (pDest, uiDigit)) == 0)
    		return (0);

  	return ((uiDigit - 1) * NN_DIGITBITS + NN_DigitBits (pDest[uiDigit-1]));
}

/* Returns the significant length of pDigit in uiDigit.

   Lengths: pDigit[uiDigit].
 */
unsigned int NN_Digits (NN_DIGIT *pDigit,unsigned int uiDigit)
{
	int iI;

  	for (iI = uiDigit - 1; iI >= 0; iI--)
    		if (pDigit[iI])
      			break;

  	return (iI + 1);
}

/* Computes pDest = pSrc1 + pSrc2*pSrc3, where pSrc2 is pDest digit. Returns Carry.

   Lengths: pDest[digits], pSrc1[digits], pSrc3[digits].
 */
static NN_DIGIT NN_AddDigitMult (NN_DIGIT *pDest, NN_DIGIT *pSrc1, NN_DIGIT pSrc2, NN_DIGIT *pSrc3
,unsigned int digits)
{
  	NN_DIGIT Carry;
	NN_DIGIT aDigit[2];
  	unsigned int uiI;

  	if (pSrc2 == 0)
    		return (0);

  	Carry = 0;
  	for (uiI = 0; uiI < digits; uiI++)
	{
   		NN_DigitMult (aDigit, pSrc2, pSrc3[uiI]);
    		if ((pDest[uiI] = pSrc1[uiI] + Carry) < Carry)
      			Carry = 1;
    		else
      			Carry = 0;
    		if ((pDest[uiI] += aDigit[0]) < aDigit[0])
      			Carry++;
    		Carry += aDigit[1];
  	}

  	return (Carry);
}

/* Computes pDest = pSrc1 - pSrc2*pSrc3, where pSrc2 is pDest digit. Returns Borrow.

   Lengths: pDest[digits], pSrc1[digits], pSrc3[digits].
 */
static NN_DIGIT NN_SubDigitMult (NN_DIGIT *pDest,NN_DIGIT *pSrc1, NN_DIGIT pSrc2, NN_DIGIT *pSrc3,
unsigned int digits)
{
  	NN_DIGIT Borrow;
	NN_DIGIT aDigit[2];
  	unsigned int uiI;

  	if (pSrc2 == 0)
    		return (0);

  	Borrow = 0;
  	for (uiI = 0; uiI < digits; uiI++)
	{
    		NN_DigitMult (aDigit, pSrc2, pSrc3[uiI]);
    		if ((pDest[uiI] = pSrc1[uiI] - Borrow) > (NN_MAXDIGIT - Borrow))
      			Borrow = 1;
    		else
      			Borrow = 0;
    		if ((pDest[uiI] -= aDigit[0]) > (NN_MAXDIGIT - aDigit[0]))
      			Borrow++;
    		Borrow += aDigit[1];
  	}

  	return (Borrow);
}

/* Returns the significant length of Digit in bits, where Digit is Digit digit.
 */
static unsigned int NN_DigitBits(NN_DIGIT Digit)
{
  	unsigned int uiI;

  	for (uiI = 0; uiI < NN_DIGITBITS; uiI++, Digit >>= 1)
    		if (Digit == 0)
      			break;

  	return (uiI);
}

/* Computes a = b * c, where b and c are digits.

   Lengths: a[2].
 */
void NN_DigitMult (NN_DIGIT a[2],NN_DIGIT b,NN_DIGIT c)
{
  	NN_DIGIT t, u;
  	NN_HALFDIGIT bHigh, bLow, cHigh, cLow;

  	bHigh = (NN_HALFDIGIT)NN_HIGHHALF (b);
  	bLow = (NN_HALFDIGIT)NN_LOWHALF (b);
  	cHigh = (NN_HALFDIGIT)NN_HIGHHALF (c);
  	cLow = (NN_HALFDIGIT)NN_LOWHALF (c);

  	a[0] = (NN_DIGIT)bLow * (NN_DIGIT)cLow;
  	t = (NN_DIGIT)bLow * (NN_DIGIT)cHigh;
  	u = (NN_DIGIT)bHigh * (NN_DIGIT)cLow;
  	a[1] = (NN_DIGIT)bHigh * (NN_DIGIT)cHigh;

  	if ((t += u) < u)
    		a[1] += NN_TOHIGHHALF (1);
  	u = NN_TOHIGHHALF (t);

  	if ((a[0] += u) < u)
    		a[1]++;
  	a[1] += NN_HIGHHALF (t);
}

/* Sets a = b / c, where a and c are digits.

   Lengths: b[2].
   Assumes b[1] < c and NN_HIGHHALF (c) > 0. For efficiency, c should be
   normalized.
 */
void NN_DigitDiv (NN_DIGIT *a,NN_DIGIT b[2],NN_DIGIT c)
{
  	NN_DIGIT t[2], u, v;
  	NN_HALFDIGIT aHigh, aLow, cHigh, cLow;

  	cHigh = (NN_HALFDIGIT)NN_HIGHHALF (c);
  	cLow = (NN_HALFDIGIT)NN_LOWHALF (c);

  	t[0] = b[0];
  	t[1] = b[1];

  	/* Underestimate high half of quotient and subtract.
   	*/
  	if (cHigh == NN_MAXHALFDIGIT)
    		aHigh = (NN_HALFDIGIT)NN_HIGHHALF (t[1]);
  	else
    		aHigh = (NN_HALFDIGIT)(t[1] / (cHigh + 1));
  	u = (NN_DIGIT)aHigh * (NN_DIGIT)cLow;
  	v = (NN_DIGIT)aHigh * (NN_DIGIT)cHigh;
  	if ((t[0] -= NN_TOHIGHHALF (u)) > (NN_MAXDIGIT - NN_TOHIGHHALF (u)))
    		t[1]--;
  	t[1] -= NN_HIGHHALF (u);
  	t[1] -= v;

  	/* Correct estimate.
   	*/
  	while(  (t[1] > cHigh)
		  ||(  (t[1] == cHigh)
		      &&(t[0] >= NN_TOHIGHHALF (cLow))
		     )
		 )
	{
    		if((t[0] -= NN_TOHIGHHALF (cLow)) > NN_MAXDIGIT - NN_TOHIGHHALF (cLow))
      			t[1]--;
    		t[1] -= cHigh;
    		aHigh++;
  	}

  	/* Underestimate low half of quotient and subtract.
   	*/
  	if (cHigh == NN_MAXHALFDIGIT)
    		aLow = (NN_HALFDIGIT)NN_LOWHALF (t[1]);
  	else
    		aLow = (NN_HALFDIGIT)((NN_TOHIGHHALF (t[1]) + NN_HIGHHALF (t[0])) / (cHigh + 1));
  	u = (NN_DIGIT)aLow * (NN_DIGIT)cLow;
  	v = (NN_DIGIT)aLow * (NN_DIGIT)cHigh;
  	if ((t[0] -= u) > (NN_MAXDIGIT - u))
    		t[1]--;
  	if ((t[0] -= NN_TOHIGHHALF (v)) > (NN_MAXDIGIT - NN_TOHIGHHALF (v)))
    		t[1]--;
  	t[1] -= NN_HIGHHALF (v);

  	/* Correct estimate.
   	*/
  	while(  (t[1] > 0)
		  ||(  (t[1] == 0)
		      &&(t[0] >= c)
		     )
		 )
	{
    		if ((t[0] -= c) > (NN_MAXDIGIT - c))
      			t[1]--;
    		aLow++;
  	}

  	*a = NN_TOHIGHHALF (aHigh) + aLow;
}


