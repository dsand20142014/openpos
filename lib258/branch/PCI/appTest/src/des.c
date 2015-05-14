
#include <string.h>

#define TRUE    (1)
#define FALSE   (0)

/***************************************************************************
*  des加解密
***************************************************************************/ 
//#define DES_ENCRYPT						0	
//#define DES_DECRYPT						1 
#define GET_BIT(p_array, bit_index)  	((p_array[(bit_index) >> 3] >> (7 - ((bit_index) & 0x07))) & 0x01) 
const unsigned char Table_IP[64] =  // 初始置换   
{    
    58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,    
    62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,    
    57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,    
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7    
};       
const unsigned char Table_InverseIP[64] =  // 末置换  
{    
    40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,    
    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,    
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,    
    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25    
};    
const unsigned char Table_E[48] =    // 扩展置换 
{    
    32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,    
    8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,    
    16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,    
    24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1    
};    
const unsigned char Table_PC1[56] = // 密钥初始置换  
{    
    57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,    
    10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,    
    63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,    
    14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4    
};    
const signed char Table_Move[2][16] =   // 左右移运算  
{    
    {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1},   //加密左移  
    {0, -1, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -2, -2, -2, -1}  //解密右移   
};    
const unsigned char Table_PC2[48] =    // 密钥压缩置换 
{    
    14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,    
    23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,    
    41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,    
    44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32    
};    
const unsigned char Table_SBOX[8][4][16] =    // S盒
{    
    // S1     
    14, 4, 13, 1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,    
    0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,    
    4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,    
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,    
    // S2     
    15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,    
    3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,    
    0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,    
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,    
    // S3     
    10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,    
    13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,    
    13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,    
    1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,    
    // S4     
    7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,    
    13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,    
    10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,    
    3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,    
    // S5     
    2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,    
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,    
    4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,    
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,    
    // S6     
    12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,    
    10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,    
    9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,    
    4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,    
    // S7     
    4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,    
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,    
    1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,    
    6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,    
    // S8     
    13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,    
    1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,    
    7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,    
    2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11    
};    
const unsigned char Table_P[32] =   // P盒置换  
{    
    16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5, 18, 31, 10,    
    2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6, 22, 11, 4,  25    
};      			  
static void SET_BIT(unsigned char *p_array, unsigned char bit_index, unsigned char bit_val) 
{
	if (1 == (bit_val))  
	{ 
		p_array[(bit_index) >> 3] |= 0x01 << (7 - ((bit_index) & 0x07)); 
	} 
	else
	{
		p_array[(bit_index) >> 3] &= ~(0x01 << (7 - ((bit_index) & 0x07))); 
	} 
}			  
//对两块大小相同的内存区进行异或    
//异或结果保存到第一块内存    
//uint8 * p_buf_1       内存区1    
//const uint8 * p_buf_2 内存区2    
//uint8 bytes           内存区大小(单位：字节)    
static void Xor(unsigned char * p_buf_1, unsigned char * p_buf_2, unsigned char bytes)   
{   
    while(bytes > 0)   
    {   
        bytes--;   
   
        p_buf_1[bytes] ^= p_buf_2[bytes];   
    }   
}   
//将缓冲区从第bit_start位到第bit_end进行循环左移    
//offset只能是1，2    
//本段代码还可以优化。    
static void move_left(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, unsigned char offset)    
{    
    unsigned char b_val = 0;   
    unsigned char b_tmp1 = 0;   
    unsigned char b_tmp2 = 0;   
   
    //读取bit_start位    
    b_tmp1 = GET_BIT(p_input, bit_start);   
    b_tmp2 = GET_BIT(p_input, bit_start + 1);   
   
    //循环左移offset位    
    for(; bit_start <= (bit_end - offset); bit_start++)   
    {   
        b_val = GET_BIT(p_input, bit_start + offset);   
        SET_BIT(p_input, bit_start, b_val);   
    }   
   
    //将bit_start开始的offset位移到bit_end后头来    
    if (1 == offset)   
    {   
        SET_BIT(p_input, bit_end, b_tmp1);   
    }   
    else   
    {   
        SET_BIT(p_input, bit_end, b_tmp2);   
        SET_BIT(p_input, bit_end - 1, b_tmp1);   
    }   
}    
//将缓冲区从第bit_start位到第bit_end进行循环右移    
//offset只能是1，2    
//本段代码在性能上还可以优化。    
static void move_right(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, unsigned char offset)    
{    
    unsigned char b_val = 0;   
    unsigned char b_tmp1 = 0;   
    unsigned char b_tmp2 = 0;   
   
    //读取bit_end位    
    b_tmp1 = GET_BIT(p_input, bit_end);   
    b_tmp2 = GET_BIT(p_input, bit_end - 1);   
   
    //循环左移offset位    
    for(; bit_end >= (bit_start + offset); bit_end--)   
    {   
        b_val = GET_BIT(p_input, bit_end - offset);   
        SET_BIT(p_input, bit_end, b_val);   
    }   
   
    //将bit_end倒数的offset位移到bit_start来    
    if (1 == offset)   
    {   
        SET_BIT(p_input, bit_start, b_tmp1);   
    }   
    else   
    {   
        SET_BIT(p_input, bit_start, b_tmp2);   
        SET_BIT(p_input, bit_start + 1, b_tmp1);   
    }   
}      
//缓冲区移位    
//offset大于0时左移    
//offset小于0时右移    
static void move_bits(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, char offset)   
{   
    if(0 < offset)   //左移    
    {   
        move_left(p_input, bit_start, bit_end, offset);    
    }      
    else if(0 > offset)  //右移    
    {   
        move_right(p_input, bit_start, bit_end, -offset);   
    }   
}     
//通用置换函数, bits <= 64    
//p_input与p_output不能指向同一个地址，否则置换会出错。    
static void Permutation( unsigned char * p_input,  unsigned char * p_output, const unsigned char * Table, unsigned char bits)    
{    
    unsigned char b_val = FALSE;   
    unsigned char bit_index = 0;   
   
    for(bit_index = 0; bit_index < bits; bit_index++)    
    {   
        b_val = GET_BIT(p_input, Table[bit_index] - 1);   
           
        SET_BIT(p_output, bit_index, b_val);   
    }   
}       
//获取从bit_s为起始的第1, 6 位组成行    
static unsigned char S_GetLine(unsigned char * p_data_ext, unsigned char bit_s)   
{   
    return (GET_BIT(p_data_ext, bit_s + 0) << 1) + GET_BIT(p_data_ext, bit_s + 5);   
}     
//获取从bit_s为起始的第2,3,4,5位组成列    
static unsigned char S_GetRow(unsigned char * p_data_ext, unsigned char bit_s)   
{   
    unsigned char row;   
   
    //2,3,4,5位组成列    
    row = GET_BIT(p_data_ext, bit_s + 1);   
    row <<= 1;    
    row += GET_BIT(p_data_ext, bit_s + 2);   
    row <<= 1;    
    row += GET_BIT(p_data_ext, bit_s + 3);   
    row <<= 1;    
    row += GET_BIT(p_data_ext, bit_s + 4);   
   
    return row;   
}      
///////////////////////////////////////////////////////////////    
//  函 数 名 : des    
//  函数功能 : DES加解密    
//  处理过程 : 根据标准的DES加密算法用输入的64位密钥对64位密文进行加/解密    
//              并将加/解密结果存储到p_output里     
//  返 回 值 :     
//  参数说明 :  const char * p_data     输入, 加密时输入明文, 解密时输入密文, 64位(8字节)    
//              const char * p_key      输入, 密钥, 64位(8字节)    
//              char * p_output         输出, 加密时输出密文, 解密时输入明文, 64位(8字节)    
//              uint8 mode              DES_ENCRYPT 加密  DES_DECRYPT 解密    
///////////////////////////////////////////////////////////////    
void des(unsigned char * p_data, unsigned char * p_key, unsigned char * p_output, unsigned char mode)   
{   
    unsigned char loop = 0;     //16轮运算的循环计数器    
    unsigned char key_tmp[8];   //密钥运算时存储中间结果    
    unsigned char sub_key[6];   //用于存储子密钥    
   
    unsigned char * p_left;   
    unsigned char * p_right;   
   
    unsigned char p_right_ext[8];   //R[i]经过扩展置换生成的48位数据(6字节), 及最终结果的存储    
    unsigned char p_right_s[4];     //经过S_BOX置换后的32位数据(4字节)    
   
    unsigned char s_loop = 0;       //S_BOX置换的循环计数器    
   
    //密钥第一次缩小换位, 得到一组56位的密钥数据    
    Permutation(p_key, key_tmp, Table_PC1, 56);   
   
    //明文初始化置换    
    Permutation(p_data, p_output, Table_IP, 64);   
   
    p_left  = p_output;     //L0    
    p_right = &p_output[4]; //R0    
   
    for(loop = 0; loop < 16; loop++)   
    {   
        //把缩进小后的把这56位分为左28位和右28位,    
        //对左28位和右28位分别循环左/右移, 得到一组新数据    
        //加解密操作时只在移位时有差异    
        move_bits(key_tmp, 0, 27, Table_Move[mode][loop]);   
        move_bits(key_tmp, 28, 55, Table_Move[mode][loop]);   
   
        //密钥第二次缩小换位，得到一组子48位的子密钥    
        Permutation(key_tmp, sub_key, Table_PC2, 48);   
   
        //R0扩展置换    
        Permutation(p_right, p_right_ext, Table_E, 48);   
   
        //将R0扩展置换后得到的48位数据(6字节)与子密钥进行异或    
        Xor(p_right_ext, sub_key, 6);   
   
        //S_BOX置换    
        for(s_loop = 0; s_loop < 4; s_loop++)   
        {   
            unsigned char s_line = 0;   
            unsigned char s_row = 0;   
            unsigned char s_bit = s_loop * 12;   
   
            s_line = S_GetLine(p_right_ext, s_bit);   
            s_row  = S_GetRow(p_right_ext,  s_bit);   
               
            p_right_s[s_loop] = Table_SBOX[s_loop * 2][s_line][s_row];   
   
            s_bit += 6;   
               
            s_line = S_GetLine(p_right_ext, s_bit);   
            s_row  = S_GetRow(p_right_ext,  s_bit);   
               
            p_right_s[s_loop] <<= 4;   
            p_right_s[s_loop] += Table_SBOX[(s_loop * 2) + 1][s_line][s_row];   
        }   
   
        //P置换    
        Permutation(p_right_s, p_right_ext, Table_P, 32);   
   
        Xor(p_right_ext, p_left, 4);   
   
        memcpy(p_left, p_right, 4);   
        memcpy(p_right, p_right_ext, 4);   
    }   
   
    memcpy(&p_right_ext[4], p_left, 4);   
    memcpy(p_right_ext, p_right, 4);   
   
    //最后再进行一次逆置换, 得到最终加密结果    
    Permutation(p_right_ext, p_output, Table_InverseIP, 64);           
}


/***************************************************************************
* 3DES
***************************************************************************/
#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
    uint32 esk[32];    
    uint32 dsk[32];    
}
des_context;

typedef struct
{
    uint32 esk[96];    
    uint32 dsk[96];    
}
des3_context;

uint32 SB1[64] =
{
    0x01010400, 0x00000000, 0x00010000, 0x01010404,
    0x01010004, 0x00010404, 0x00000004, 0x00010000,
    0x00000400, 0x01010400, 0x01010404, 0x00000400,
    0x01000404, 0x01010004, 0x01000000, 0x00000004,
    0x00000404, 0x01000400, 0x01000400, 0x00010400,
    0x00010400, 0x01010000, 0x01010000, 0x01000404,
    0x00010004, 0x01000004, 0x01000004, 0x00010004,
    0x00000000, 0x00000404, 0x00010404, 0x01000000,
    0x00010000, 0x01010404, 0x00000004, 0x01010000,
    0x01010400, 0x01000000, 0x01000000, 0x00000400,
    0x01010004, 0x00010000, 0x00010400, 0x01000004,
    0x00000400, 0x00000004, 0x01000404, 0x00010404,
    0x01010404, 0x00010004, 0x01010000, 0x01000404,
    0x01000004, 0x00000404, 0x00010404, 0x01010400,
    0x00000404, 0x01000400, 0x01000400, 0x00000000,
    0x00010004, 0x00010400, 0x00000000, 0x01010004
};

static uint32 SB2[64] =
{
    0x80108020, 0x80008000, 0x00008000, 0x00108020,
    0x00100000, 0x00000020, 0x80100020, 0x80008020,
    0x80000020, 0x80108020, 0x80108000, 0x80000000,
    0x80008000, 0x00100000, 0x00000020, 0x80100020,
    0x00108000, 0x00100020, 0x80008020, 0x00000000,
    0x80000000, 0x00008000, 0x00108020, 0x80100000,
    0x00100020, 0x80000020, 0x00000000, 0x00108000,
    0x00008020, 0x80108000, 0x80100000, 0x00008020,
    0x00000000, 0x00108020, 0x80100020, 0x00100000,
    0x80008020, 0x80100000, 0x80108000, 0x00008000,
    0x80100000, 0x80008000, 0x00000020, 0x80108020,
    0x00108020, 0x00000020, 0x00008000, 0x80000000,
    0x00008020, 0x80108000, 0x00100000, 0x80000020,
    0x00100020, 0x80008020, 0x80000020, 0x00100020,
    0x00108000, 0x00000000, 0x80008000, 0x00008020,
    0x80000000, 0x80100020, 0x80108020, 0x00108000
};

static uint32 SB3[64] =
{
    0x00000208, 0x08020200, 0x00000000, 0x08020008,
    0x08000200, 0x00000000, 0x00020208, 0x08000200,
    0x00020008, 0x08000008, 0x08000008, 0x00020000,
    0x08020208, 0x00020008, 0x08020000, 0x00000208,
    0x08000000, 0x00000008, 0x08020200, 0x00000200,
    0x00020200, 0x08020000, 0x08020008, 0x00020208,
    0x08000208, 0x00020200, 0x00020000, 0x08000208,
    0x00000008, 0x08020208, 0x00000200, 0x08000000,
    0x08020200, 0x08000000, 0x00020008, 0x00000208,
    0x00020000, 0x08020200, 0x08000200, 0x00000000,
    0x00000200, 0x00020008, 0x08020208, 0x08000200,
    0x08000008, 0x00000200, 0x00000000, 0x08020008,
    0x08000208, 0x00020000, 0x08000000, 0x08020208,
    0x00000008, 0x00020208, 0x00020200, 0x08000008,
    0x08020000, 0x08000208, 0x00000208, 0x08020000,
    0x00020208, 0x00000008, 0x08020008, 0x00020200
};

static uint32 SB4[64] =
{
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802080, 0x00800081, 0x00800001, 0x00002001,
    0x00000000, 0x00802000, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00800080, 0x00800001,
    0x00000001, 0x00002000, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002001, 0x00002080,
    0x00800081, 0x00000001, 0x00002080, 0x00800080,
    0x00002000, 0x00802080, 0x00802081, 0x00000081,
    0x00800080, 0x00800001, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00000000, 0x00802000,
    0x00002080, 0x00800080, 0x00800081, 0x00000001,
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802081, 0x00000081, 0x00000001, 0x00002000,
    0x00800001, 0x00002001, 0x00802080, 0x00800081,
    0x00002001, 0x00002080, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002000, 0x00802080
};

static uint32 SB5[64] =
{
    0x00000100, 0x02080100, 0x02080000, 0x42000100,
    0x00080000, 0x00000100, 0x40000000, 0x02080000,
    0x40080100, 0x00080000, 0x02000100, 0x40080100,
    0x42000100, 0x42080000, 0x00080100, 0x40000000,
    0x02000000, 0x40080000, 0x40080000, 0x00000000,
    0x40000100, 0x42080100, 0x42080100, 0x02000100,
    0x42080000, 0x40000100, 0x00000000, 0x42000000,
    0x02080100, 0x02000000, 0x42000000, 0x00080100,
    0x00080000, 0x42000100, 0x00000100, 0x02000000,
    0x40000000, 0x02080000, 0x42000100, 0x40080100,
    0x02000100, 0x40000000, 0x42080000, 0x02080100,
    0x40080100, 0x00000100, 0x02000000, 0x42080000,
    0x42080100, 0x00080100, 0x42000000, 0x42080100,
    0x02080000, 0x00000000, 0x40080000, 0x42000000,
    0x00080100, 0x02000100, 0x40000100, 0x00080000,
    0x00000000, 0x40080000, 0x02080100, 0x40000100
};

static uint32 SB6[64] =
{
    0x20000010, 0x20400000, 0x00004000, 0x20404010,
    0x20400000, 0x00000010, 0x20404010, 0x00400000,
    0x20004000, 0x00404010, 0x00400000, 0x20000010,
    0x00400010, 0x20004000, 0x20000000, 0x00004010,
    0x00000000, 0x00400010, 0x20004010, 0x00004000,
    0x00404000, 0x20004010, 0x00000010, 0x20400010,
    0x20400010, 0x00000000, 0x00404010, 0x20404000,
    0x00004010, 0x00404000, 0x20404000, 0x20000000,
    0x20004000, 0x00000010, 0x20400010, 0x00404000,
    0x20404010, 0x00400000, 0x00004010, 0x20000010,
    0x00400000, 0x20004000, 0x20000000, 0x00004010,
    0x20000010, 0x20404010, 0x00404000, 0x20400000,
    0x00404010, 0x20404000, 0x00000000, 0x20400010,
    0x00000010, 0x00004000, 0x20400000, 0x00404010,
    0x00004000, 0x00400010, 0x20004010, 0x00000000,
    0x20404000, 0x20000000, 0x00400010, 0x20004010
};

static uint32 SB7[64] =
{
    0x00200000, 0x04200002, 0x04000802, 0x00000000,
    0x00000800, 0x04000802, 0x00200802, 0x04200800,
    0x04200802, 0x00200000, 0x00000000, 0x04000002,
    0x00000002, 0x04000000, 0x04200002, 0x00000802,
    0x04000800, 0x00200802, 0x00200002, 0x04000800,
    0x04000002, 0x04200000, 0x04200800, 0x00200002,
    0x04200000, 0x00000800, 0x00000802, 0x04200802,
    0x00200800, 0x00000002, 0x04000000, 0x00200800,
    0x04000000, 0x00200800, 0x00200000, 0x04000802,
    0x04000802, 0x04200002, 0x04200002, 0x00000002,
    0x00200002, 0x04000000, 0x04000800, 0x00200000,
    0x04200800, 0x00000802, 0x00200802, 0x04200800,
    0x00000802, 0x04000002, 0x04200802, 0x04200000,
    0x00200800, 0x00000000, 0x00000002, 0x04200802,
    0x00000000, 0x00200802, 0x04200000, 0x00000800,
    0x04000002, 0x04000800, 0x00000800, 0x00200002
};

static uint32 SB8[64] =
{
    0x10001040, 0x00001000, 0x00040000, 0x10041040,
    0x10000000, 0x10001040, 0x00000040, 0x10000000,
    0x00040040, 0x10040000, 0x10041040, 0x00041000,
    0x10041000, 0x00041040, 0x00001000, 0x00000040,
    0x10040000, 0x10000040, 0x10001000, 0x00001040,
    0x00041000, 0x00040040, 0x10040040, 0x10041000,
    0x00001040, 0x00000000, 0x00000000, 0x10040040,
    0x10000040, 0x10001000, 0x00041040, 0x00040000,
    0x00041040, 0x00040000, 0x10041000, 0x00001000,
    0x00000040, 0x10040040, 0x00001000, 0x00041040,
    0x10001000, 0x00000040, 0x10000040, 0x10040000,
    0x10040040, 0x10000000, 0x00040000, 0x10001040,
    0x00000000, 0x10041040, 0x00040040, 0x10000040,
    0x10040000, 0x10001000, 0x10001040, 0x00000000,
    0x10041040, 0x00041000, 0x00041000, 0x00001040,
    0x00001040, 0x00040040, 0x10000000, 0x10041000
};
 
static uint32 LHs[16] =
{
    0x00000000, 0x00000001, 0x00000100, 0x00000101,
    0x00010000, 0x00010001, 0x00010100, 0x00010101,
    0x01000000, 0x01000001, 0x01000100, 0x01000101,
    0x01010000, 0x01010001, 0x01010100, 0x01010101
};

static uint32 RHs[16] =
{
    0x00000000, 0x01000000, 0x00010000, 0x01010000,
    0x00000100, 0x01000100, 0x00010100, 0x01010100,
    0x00000001, 0x01000001, 0x00010001, 0x01010001,
    0x00000101, 0x01000101, 0x00010101, 0x01010101,
};

#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (uint32) (b)[(i)    ] << 24 )       \
        | ( (uint32) (b)[(i) + 1] << 16 )       \
        | ( (uint32) (b)[(i) + 2] <<  8 )       \
        | ( (uint32) (b)[(i) + 3]       );      \
}

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (uint8) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uint8) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uint8) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uint8) ( (n)       );       \
}

#define DES_IP(X,Y)                                             \
{                                                               \
    T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4);   \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);   \
    T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);   \
    T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);   \
    Y = ((Y << 1) | (Y >> 31)) & 0xFFFFFFFF;                    \
    T = (X ^ Y) & 0xAAAAAAAA; Y ^= T; X ^= T;                   \
    X = ((X << 1) | (X >> 31)) & 0xFFFFFFFF;                    \
}

#define DES_FP(X,Y)                                             \
{                                                               \
    X = ((X << 31) | (X >> 1)) & 0xFFFFFFFF;                    \
    T = (X ^ Y) & 0xAAAAAAAA; X ^= T; Y ^= T;                   \
    Y = ((Y << 31) | (Y >> 1)) & 0xFFFFFFFF;                    \
    T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);   \
    T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);   \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);   \
    T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4);   \
}

#define DES_ROUND(X,Y)                          \
{                                               \
    T = *SK++ ^ X;                              \
    Y ^= SB8[ (T      ) & 0x3F ] ^              \
         SB6[ (T >>  8) & 0x3F ] ^              \
         SB4[ (T >> 16) & 0x3F ] ^              \
         SB2[ (T >> 24) & 0x3F ];               \
                                                \
    T = *SK++ ^ ((X << 28) | (X >> 4));         \
    Y ^= SB7[ (T      ) & 0x3F ] ^              \
         SB5[ (T >>  8) & 0x3F ] ^              \
         SB3[ (T >> 16) & 0x3F ] ^              \
         SB1[ (T >> 24) & 0x3F ];               \
}

int des_main_ks( uint32 SK[32], uint8 key[8] )
{
    int i;
    uint32 X, Y, T;

    GET_UINT32( X, key, 0 );
    GET_UINT32( Y, key, 4 );

    T =  ((Y >>  4) ^ X) & 0x0F0F0F0F;  X ^= T; Y ^= (T <<  4);
    T =  ((Y      ) ^ X) & 0x10101010;  X ^= T; Y ^= (T      );

    X =   (LHs[ (X      ) & 0xF] << 3) | (LHs[ (X >>  8) & 0xF ] << 2)
        | (LHs[ (X >> 16) & 0xF] << 1) | (LHs[ (X >> 24) & 0xF ]     )
        | (LHs[ (X >>  5) & 0xF] << 7) | (LHs[ (X >> 13) & 0xF ] << 6)
        | (LHs[ (X >> 21) & 0xF] << 5) | (LHs[ (X >> 29) & 0xF ] << 4);

    Y =   (RHs[ (Y >>  1) & 0xF] << 3) | (RHs[ (Y >>  9) & 0xF ] << 2)
        | (RHs[ (Y >> 17) & 0xF] << 1) | (RHs[ (Y >> 25) & 0xF ]     )
        | (RHs[ (Y >>  4) & 0xF] << 7) | (RHs[ (Y >> 12) & 0xF ] << 6)
        | (RHs[ (Y >> 20) & 0xF] << 5) | (RHs[ (Y >> 28) & 0xF ] << 4);

    X &= 0x0FFFFFFF;
    Y &= 0x0FFFFFFF;

    for( i = 0; i < 16; i++ )
    {
        if( i < 2 || i == 8 || i == 15 )
        {
            X = ((X <<  1) | (X >> 27)) & 0x0FFFFFFF;
            Y = ((Y <<  1) | (Y >> 27)) & 0x0FFFFFFF;
        }
        else
        {
            X = ((X <<  2) | (X >> 26)) & 0x0FFFFFFF;
            Y = ((Y <<  2) | (Y >> 26)) & 0x0FFFFFFF;
        }

        *SK++ =   ((X <<  4) & 0x24000000) | ((X << 28) & 0x10000000)
                | ((X << 14) & 0x08000000) | ((X << 18) & 0x02080000)
                | ((X <<  6) & 0x01000000) | ((X <<  9) & 0x00200000)
                | ((X >>  1) & 0x00100000) | ((X << 10) & 0x00040000)
                | ((X <<  2) & 0x00020000) | ((X >> 10) & 0x00010000)
                | ((Y >> 13) & 0x00002000) | ((Y >>  4) & 0x00001000)
                | ((Y <<  6) & 0x00000800) | ((Y >>  1) & 0x00000400)
                | ((Y >> 14) & 0x00000200) | ((Y      ) & 0x00000100)
                | ((Y >>  5) & 0x00000020) | ((Y >> 10) & 0x00000010)
                | ((Y >>  3) & 0x00000008) | ((Y >> 18) & 0x00000004)
                | ((Y >> 26) & 0x00000002) | ((Y >> 24) & 0x00000001);

        *SK++ =   ((X << 15) & 0x20000000) | ((X << 17) & 0x10000000)
                | ((X << 10) & 0x08000000) | ((X << 22) & 0x04000000)
                | ((X >>  2) & 0x02000000) | ((X <<  1) & 0x01000000)
                | ((X << 16) & 0x00200000) | ((X << 11) & 0x00100000)
                | ((X <<  3) & 0x00080000) | ((X >>  6) & 0x00040000)
                | ((X << 15) & 0x00020000) | ((X >>  4) & 0x00010000)
                | ((Y >>  2) & 0x00002000) | ((Y <<  8) & 0x00001000)
                | ((Y >> 14) & 0x00000808) | ((Y >>  9) & 0x00000400)
                | ((Y      ) & 0x00000200) | ((Y <<  7) & 0x00000100)
                | ((Y >>  7) & 0x00000020) | ((Y >>  3) & 0x00000011)
                | ((Y <<  2) & 0x00000004) | ((Y >> 21) & 0x00000002);
    }
    return( 0 );
}

int des_set_key( des_context *ctx, uint8 key[8] )
{
    int i;

    des_main_ks( ctx->esk, key );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i    ] = ctx->esk[30 - i];
        ctx->dsk[i + 1] = ctx->esk[31 - i];
    }

    return( 0 );
}

 

void des_crypt( uint32 SK[32], uint8 input[8], uint8 output[8] )
{
    uint32 X, Y, T;

    GET_UINT32( X, input, 0 );
    GET_UINT32( Y, input, 4 );

    DES_IP( X, Y );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_FP( Y, X );

    PUT_UINT32( Y, output, 0 );
    PUT_UINT32( X, output, 4 );
}

void des_encrypt( des_context *ctx, uint8 input[8], uint8 output[8] )
{
    des_crypt( ctx->esk, input, output );
}

void des_decrypt( des_context *ctx, uint8 input[8], uint8 output[8] )
{
    des_crypt( ctx->dsk, input, output );
}

 

int des3_set_2keys( des3_context *ctx, uint8 key1[8], uint8 key2[8] )
{
    int i;

    des_main_ks( ctx->esk     , key1 );
    des_main_ks( ctx->dsk + 32, key2 );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i     ] = ctx->esk[30 - i];
        ctx->dsk[i +  1] = ctx->esk[31 - i];

        ctx->esk[i + 32] = ctx->dsk[62 - i];
        ctx->esk[i + 33] = ctx->dsk[63 - i];

        ctx->esk[i + 64] = ctx->esk[     i];
        ctx->esk[i + 65] = ctx->esk[ 1 + i];

        ctx->dsk[i + 64] = ctx->dsk[     i];
        ctx->dsk[i + 65] = ctx->dsk[ 1 + i];
    }

    return( 0 );
}

int des3_set_3keys( des3_context *ctx, uint8 key1[8], uint8 key2[8],
                                       uint8 key3[8] )
{
    int i;

    des_main_ks( ctx->esk     , key1 );
    des_main_ks( ctx->dsk + 32, key2 );
    des_main_ks( ctx->esk + 64, key3 );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i     ] = ctx->esk[94 - i];
        ctx->dsk[i +  1] = ctx->esk[95 - i];

        ctx->esk[i + 32] = ctx->dsk[62 - i];
        ctx->esk[i + 33] = ctx->dsk[63 - i];

        ctx->dsk[i + 64] = ctx->esk[30 - i];
        ctx->dsk[i + 65] = ctx->esk[31 - i];
    }

    return( 0 );
}

 

void des3_crypt( uint32 SK[96], uint8 input[8], uint8 output[8] )
{
    uint32 X, Y, T;

    GET_UINT32( X, input, 0 );
    GET_UINT32( Y, input, 4 );

    DES_IP( X, Y );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_FP( Y, X );

    PUT_UINT32( Y, output, 0 );
    PUT_UINT32( X, output, 4 );
}
/*
void des3_encrypt( des3_context *ctx, uint8 input[8], uint8 output[8] )
{
    des3_crypt( ctx->esk, input, output );
}

void des3_decrypt( des3_context *ctx, uint8 input[8], uint8 output[8] )
{
    des3_crypt( ctx->dsk, input, output );
}
*/

void des3_encrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8)
{
	des3_context ctx;
	int i;

	des3_set_3keys(&ctx, &key[0], &key[8], &key[16]);
	for(i=0; i<num8; i++)
		des3_crypt( ctx.esk, &input[8*i], &output[8*i] );
}

void des3_decrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8)
{
	des3_context ctx;
	int i;

	des3_set_3keys(&ctx, &key[0], &key[8], &key[16]);
	for(i=0; i<num8; i++)
		des3_crypt( ctx.dsk, &input[8*i], &output[8*i] );
}

void DesEncrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8)
{
    des_context ctx;
    int i;

    des_set_key(&ctx, key);
    for(i=0; i<num8; i++)
        des_crypt( ctx.esk, &input[8*i], &output[8*i] );
}

void DesDecrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8)
{
    des_context ctx;
    int i;

    des_set_key(&ctx, key);
    for(i=0; i<num8; i++)
        des_crypt( ctx.dsk, &input[8*i], &output[8*i] );
}

void Des3Key2Encrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8)
{
    des3_context ctx;
    int i;

    des3_set_3keys(&ctx, &key[0], &key[8], &key[0]);
    for(i=0; i<num8; i++)
        des3_crypt( ctx.esk, &input[8*i], &output[8*i] );
}

void Des3Key2Decrypt(unsigned char *key, unsigned char *input, unsigned char *output, int num8)
{
    des3_context ctx;
    int i;

    des3_set_3keys(&ctx, &key[0], &key[8], &key[0]);
    for(i=0; i<num8; i++)
        des3_crypt( ctx.dsk, &input[8*i], &output[8*i] );
}
