
/* Type definitions.
 */
typedef unsigned long NN_DIGIT;
typedef unsigned short NN_HALFDIGIT;

/* Constants.

   Note: NN_MAXDIGITS is long enough to hold any RSA modulus, plus
   one more digit as required by R_GeneratePEMKeys (for n and phiN,
   whose lengths must be even). All natural numbers have at most
   NN_MAXDIGITS digits, except for double-length intermediate values
   in NN_Mult (t), NN_ModMult (t), NN_ModInv (w), and NN_Div (c).
 */
/* Length of digit in bits */
#define NN_DIGITBITS 		32
#define NN_HALFDIGITBITS 	16
/* Length of digit in bytes */
#define NN_DIGITLEN 		(NN_DIGITBITS / 8)
/* Maximum length in digits */
#define NN_MAXDIGITS  	((RSA_MAXMODULUSLEN + NN_DIGITLEN - 1) / NN_DIGITLEN + 1)
/* Maximum digits */
#define NN_MAXDIGIT 		0xffffffff
#define NN_MAXHALFDIGIT 	0xffff

/* Macros.
 */
#define NN_LOWHALF(x) 	((x) & NN_MAXHALFDIGIT)
#define NN_HIGHHALF(x) 	(((x) >> NN_HALFDIGITBITS) & NN_MAXHALFDIGIT)
#define NN_TOHIGHHALF(x)	(((NN_DIGIT)(x)) << NN_HALFDIGITBITS)
#define NN_DIGITMSB(x) 	(unsigned int)(((x) >> (NN_DIGITBITS - 1)) & 1)
#define NN_DIGIT2MSB(x) 	(unsigned int)(((x) >> (NN_DIGITBITS - 2)) & 3)

/* CONVERSIONS
   NN_Decode (a, digits, b, len)   Decodes character string b into a.
   NN_Encode (a, len, b, digits)   Encodes a into character string b.

   ASSIGNMENTS
   NN_Assign (a, b, digits)        Assigns a = b.
   NN_ASSIGN_DIGIT (a, b, digits)  Assigns a = b, where b is a digit.
   NN_AssignZero (a, b, digits)    Assigns a = 0.
   NN_Assign2Exp (a, b, digits)    Assigns a = 2^b.

   ARITHMETIC OPERATIONS
   NN_Add (a, b, c, digits)        Computes a = b + c.
   NN_Sub (a, b, c, digits)        Computes a = b - c.
   NN_Mult (a, b, c, digits)       Computes a = b * c.
   NN_LShift (a, b, c, digits)     Computes a = b * 2^c.
   NN_RShift (a, b, c, digits)     Computes a = b / 2^c.
   NN_Div (a, b, c, cDigits, d, dDigits)  Computes a = c div d and b = c mod d.

   NUMBER THEORY
   NN_Mod (a, b, bDigits, c, cDigits)  Computes a = b mod c.
   NN_ModMult (a, b, c, d, digits) Computes a = b * c mod d.
   NN_ModExp (a, b, c, cDigits, d, dDigits)  Computes a = b^c mod d.
   NN_ModInv (a, b, c, digits)     Computes a = 1/b mod c.
   NN_Gcd (a, b, c, digits)        Computes a = gcd (b, c).

   OTHER OPERATIONS
   NN_EVEN (a, digits)             Returns 1 iff a is even.
   NN_Cmp (a, b, digits)           Returns sign of a - b.
   NN_EQUAL (a, digits)            Returns 1 iff a = b.
   NN_Zero (a, digits)             Returns 1 iff a = 0.
   NN_Digits (a, digits)           Returns significant length of a in digits.
   NN_Bits (a, digits)             Returns significant length of a in bits.
 */
void NN_Decode (NN_DIGIT *pDigit, unsigned int uiDigit, unsigned char *pucInData, unsigned int
uiInLen);
void NN_Encode (unsigned char *pucOutData, unsigned int uiLen, NN_DIGIT *pDigit, unsigned int
uiDigit);

void NN_Assign (NN_DIGIT *pDest, NN_DIGIT *pSrc, unsigned int uiDigit);
void NN_AssignZero (NN_DIGIT *pDigit, unsigned int uiDigit);
void NN_Assign2Exp (NN_DIGIT *pDigit,unsigned int uiLen,unsigned int uiDigit);

NN_DIGIT NN_Add (NN_DIGIT *pDest, NN_DIGIT *pSrc1, NN_DIGIT *pSrc2,unsigned int uiDigit);
NN_DIGIT NN_Sub (NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_Mult (NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_Div(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *,
    unsigned int);
NN_DIGIT NN_LShift(NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int);
NN_DIGIT NN_RShift(NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int);

void NN_Mod(NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int);
void NN_ModMult(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_ModExp(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *,
    unsigned int);
void NN_ModInv(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_Gcd(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);

int NN_Cmp (NN_DIGIT *, NN_DIGIT *, unsigned int);
int NN_Zero (NN_DIGIT *, unsigned int);
unsigned int NN_Bits (NN_DIGIT *, unsigned int);
unsigned int NN_Digits (NN_DIGIT *, unsigned int);

void NN_DigitMult (NN_DIGIT [2], NN_DIGIT, NN_DIGIT);
void NN_DigitDiv (NN_DIGIT *, NN_DIGIT [2], NN_DIGIT);

#define NN_ASSIGN_DIGIT(a, b, digits) {NN_AssignZero (a, digits); a[0] = b;}
#define NN_EQUAL(a, b, digits) (! NN_Cmp (a, b, digits))
#define NN_EVEN(a, digits) (((digits) == 0) || ! (a[0] & 1))

