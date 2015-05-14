#include <string.h>
#include "AES.h"
//#include "commonage.h"

#define byte unsigned char 



#define BPOLY 0x1b //!< Lower 8 bits of (x^8+x^4+x^3+x+1), ie. (x^4+x^3+x+1).
#define BLOCKSIZE 16 //!< Block size in number of bytes.



#define KEYBITS 128 //!< Use AES128.
#define ROUNDS 10 //!< Number of rounds.
#define KEYLENGTH 16 //!< Key length in number of bytes.


byte  block1[ 256 ]; //!< Workspace 1.
byte  block2[ 256 ]; //!< Worksapce 2.



byte  * powTbl; //!< Final location of exponentiation lookup table.
byte  * logTbl; //!< Final location of logarithm lookup table.
byte  * sBox; //!< Final location of s-box.
byte  * sBoxInv; //!< Final location of inverse s-box.
byte  * expandedKey; //!< Final location of expanded key.




void CalcPowLog( byte * powTbl, byte * logTbl )
{
        byte  i = 0;
        byte  t = 1;

        do {
  // Use 0x03 as root for exponentiation and logarithms.
         powTbl[i] = t;
         logTbl[t] = i;
         i++;

  // Muliply t by 3 in GF(2^8).
         t ^= (t << 1) ^ (t & 0x80 ? BPOLY : 0);
        } while( t != 1 ); // Cyclic properties ensure that i < 255.
        
        powTbl[255] = powTbl[0]; // 255 = '-0', 254 = -1, etc.
}



void CalcSBox( byte * sBox )
{
        byte  i, rot;
        byte  temp;
     byte  result;

 // Fill all entries of sBox[].
 i = 0;
 do {
                // Inverse in GF(2^8).
                if( i > 0 ) {
                 temp = powTbl[ 255 - logTbl[i] ];
         } else {
                  temp = 0;
  }

                // Affine transformation in GF(2).
                result = temp ^ 0x63; // Start with adding a vector in GF(2).
                for( rot = 0; rot < 4; rot++ ) {
                        // Rotate left.
   temp = (temp<<1) | (temp>>7);

   // Add rotated byte in GF(2).
   result ^= temp;
  }
   
  // Put result in table.
                sBox[i] = result;
 } while( ++i != 0 );
} 



void CalcSBoxInv( byte * sBox, byte * sBoxInv )
{
 byte  i = 0;
 byte  j = 0;

 // Iterate through all elements in sBoxInv using  i.
 do {
  // Search through sBox using j.
  //cleardog();
  do {
   // Check if current j is the inverse of current i.
   if( sBox[ j ] == i ) {
    // If so, set sBoxInc and indicate search finished.
    sBoxInv[ i ] = j;
    j = 255;
   }
  } while( ++j != 0 );
 } while( ++i != 0 );
}



void CycleLeft( byte * row )
{
 // Cycle 4 bytes in an array left once.
 byte  temp = row[0];
 row[0] = row[1];
 row[1] = row[2];
 row[2] = row[3];
 row[3] = temp;
}



void InvMixColumn( byte * column )
{
 byte  r0, r1, r2, r3;

 r0 = column[1] ^ column[2] ^ column[3];
 r1 = column[0] ^ column[2] ^ column[3];
 r2 = column[0] ^ column[1] ^ column[3];
 r3 = column[0] ^ column[1] ^ column[2];

  column[0] = (column[0] << 1) ^ (column[0] & 0x80 ? BPOLY : 0);
  column[1] = (column[1] << 1) ^ (column[1] & 0x80 ? BPOLY : 0);
    column[2] = (column[2] << 1) ^ (column[2] & 0x80 ? BPOLY : 0);
    column[3] = (column[3] << 1) ^ (column[3] & 0x80 ? BPOLY : 0);

 r0 ^= column[0] ^ column[1];
 r1 ^= column[1] ^ column[2];
 r2 ^= column[2] ^ column[3];
 r3 ^= column[0] ^ column[3];

    column[0] = (column[0] << 1) ^ (column[0] & 0x80 ? BPOLY : 0);
    column[1] = (column[1] << 1) ^ (column[1] & 0x80 ? BPOLY : 0);
    column[2] = (column[2] << 1) ^ (column[2] & 0x80 ? BPOLY : 0);
    column[3] = (column[3] << 1) ^ (column[3] & 0x80 ? BPOLY : 0);

 r0 ^= column[0] ^ column[2];
 r1 ^= column[1] ^ column[3];
 r2 ^= column[0] ^ column[2];
 r3 ^= column[1] ^ column[3];

    column[0] = (column[0] << 1) ^ (column[0] & 0x80 ? BPOLY : 0);
    column[1] = (column[1] << 1) ^ (column[1] & 0x80 ? BPOLY : 0);
    column[2] = (column[2] << 1) ^ (column[2] & 0x80 ? BPOLY : 0);
    column[3] = (column[3] << 1) ^ (column[3] & 0x80 ? BPOLY : 0);

 column[0] ^= column[1] ^ column[2] ^ column[3];
 r0 ^= column[0];
 r1 ^= column[0];
 r2 ^= column[0];
 r3 ^= column[0];

 column[0] = r0;
 column[1] = r1;
 column[2] = r2;
 column[3] = r3;
}

byte Multiply( unsigned char num, unsigned char factor )
{
 byte mask = 1;
 byte result = 0;

 while( mask != 0 ) {
  // Check bit of factor given by mask.
  if( mask & factor ) {
   // Add current multiple of num in GF(2).
   result ^= num;
  }

  // Shift mask to indicate next bit.
  mask <<= 1;

  // Double num.
         num = (num << 1) ^ (num & 0x80 ? BPOLY : 0);
 }

 return result;
}


byte DotProduct( unsigned char * vector1, unsigned char * vector2 )
{
 byte result = 0;

 result ^= Multiply( *vector1++, *vector2++ );
 result ^= Multiply( *vector1++, *vector2++ );
 result ^= Multiply( *vector1++, *vector2++ );
 result ^= Multiply( *vector1  , *vector2   );

 return result;
}

void MixColumn( byte * column )
{
 byte  row[8] = {
  0x02, 0x03, 0x01, 0x01,
  0x02, 0x03, 0x01, 0x01
 }; // Prepare first row of matrix twice, to eliminate need for cycling.

 byte  result[4];
 
 // Take dot products of each matrix row and the column vector.
 result[0] = DotProduct( row+0, column );
 result[1] = DotProduct( row+3, column );
 result[2] = DotProduct( row+2, column );
 result[3] = DotProduct( row+1, column );

 // Copy temporary result to original column.
 column[0] = result[0];
 column[1] = result[1];
 column[2] = result[2];
 column[3] = result[3];
}


void SubBytes( byte * bytes, byte count )
{
 do {
  *bytes = sBox[ *bytes ]; // Substitute every byte in state.
  bytes++;
 } while( --count );
}



void InvSubBytesAndXOR( byte * bytes, byte * key, byte count )
{
 do {
//  *bytes = sBoxInv[ *bytes ] ^ *key; // Inverse substitute every byte in state and add key.
  *bytes = block2[ *bytes ] ^ *key; // Use block2 directly. Increases speed.
  bytes++;
  key++;
 } while( --count );
}



void InvShiftRows( byte * state )
{
 byte temp;

 // Note: State is arranged column by column.

 // Cycle second row right one time.
 temp = state[ 1 + 3*4 ];
 state[ 1 + 3*4 ] = state[ 1 + 2*4 ];
 state[ 1 + 2*4 ] = state[ 1 + 1*4 ];
 state[ 1 + 1*4 ] = state[ 1 + 0*4 ];
 state[ 1 + 0*4 ] = temp;

 // Cycle third row right two times.
 temp = state[ 2 + 0*4 ];
 state[ 2 + 0*4 ] = state[ 2 + 2*4 ];
 state[ 2 + 2*4 ] = temp;
 temp = state[ 2 + 1*4 ];
 state[ 2 + 1*4 ] = state[ 2 + 3*4 ];
 state[ 2 + 3*4 ] = temp;

 // Cycle fourth row right three times, ie. left once.
 temp = state[ 3 + 0*4 ];
 state[ 3 + 0*4 ] = state[ 3 + 1*4 ];
 state[ 3 + 1*4 ] = state[ 3 + 2*4 ];
 state[ 3 + 2*4 ] = state[ 3 + 3*4 ];
 state[ 3 + 3*4 ] = temp;
}

void ShiftRows( byte * state )
{
 byte temp;

 // Note: State is arranged column by column.

 // Cycle second row left one time.
 temp = state[ 1 + 0*4 ];
 state[ 1 + 0*4 ] = state[ 1 + 1*4 ];
 state[ 1 + 1*4 ] = state[ 1 + 2*4 ];
 state[ 1 + 2*4 ] = state[ 1 + 3*4 ];
 state[ 1 + 3*4 ] = temp;

 // Cycle third row left two times.
 temp = state[ 2 + 0*4 ];
 state[ 2 + 0*4 ] = state[ 2 + 2*4 ];
 state[ 2 + 2*4 ] = temp;
 temp = state[ 2 + 1*4 ];
 state[ 2 + 1*4 ] = state[ 2 + 3*4 ];
 state[ 2 + 3*4 ] = temp;

 // Cycle fourth row left three times, ie. right once.
 temp = state[ 3 + 3*4 ];
 state[ 3 + 3*4 ] = state[ 3 + 2*4 ];
 state[ 3 + 2*4 ] = state[ 3 + 1*4 ];
 state[ 3 + 1*4 ] = state[ 3 + 0*4 ];
 state[ 3 + 0*4 ] = temp;
}


void InvMixColumns( byte * state )
{
 InvMixColumn( state + 0*4 );
 InvMixColumn( state + 1*4 );
 InvMixColumn( state + 2*4 );
 InvMixColumn( state + 3*4 );
}

void MixColumns( byte * state )
{
 MixColumn( state + 0*4 );
 MixColumn( state + 1*4 );
 MixColumn( state + 2*4 );
 MixColumn( state + 3*4 );
}


void XORBytes( byte * bytes1, byte * bytes2, byte count )
{
 do {
  *bytes1 ^= *bytes2; // Add in GF(2), ie. XOR.
  bytes1++;
  bytes2++;
 } while( --count );
}



void CopyBytes( byte * to, byte * from, byte count )
{
 do {
  *to = *from;
  to++;
  from++;
 } while( --count );
}



void KeyExpansion( byte * expandedKey )
{
 byte  temp[4];
 byte i;
 byte  Rcon[4] = { 0x01, 0x00, 0x00, 0x00 }; // Round constant.
 
 unsigned char  *key;
 unsigned char  a[16];
 key=a;
 //以下为加解密密码，共16字节。可以选择任意值
 key[0]=0x30;
 key[1]=0x30;
 key[2]=0x30;
 key[3]=0x30;
 key[4]=0x30;
 key[5]=0x30;
 key[6]=0x30;
 key[7]=0x30;
 key[8]=0x30;
 key[9]=0x30;
 key[10]=0x30;
 key[11]=0x30;
 key[12]=0x30;
 key[13]=0x30;
 key[14]=0x30;
 key[15]=0x30;
 ////////////////////////////////////////////

 // Copy key to start of expanded key.
 i = KEYLENGTH;
 do {
  *expandedKey = *key;
  expandedKey++;
  key++;
 } while( --i );

 // Prepare last 4 bytes of key in temp.
 expandedKey -= 4;
 temp[0] = *(expandedKey++);
 temp[1] = *(expandedKey++);
 temp[2] = *(expandedKey++);
 temp[3] = *(expandedKey++);

 // Expand key.
 i = KEYLENGTH;
 while( i < BLOCKSIZE*(ROUNDS+1) ) {
  // Are we at the start of a multiple of the key size?
  if( (i % KEYLENGTH) == 0 ) {
   CycleLeft( temp ); // Cycle left once.
   SubBytes( temp, 4 ); // Substitute each byte.
   XORBytes( temp, Rcon, 4 ); // Add constant in GF(2).
   *Rcon = (*Rcon << 1) ^ (*Rcon & 0x80 ? BPOLY : 0);
  }

  // Keysize larger than 24 bytes, ie. larger that 192 bits?
  #if KEYLENGTH > 24
  // Are we right past a block size?
  else if( (i % KEYLENGTH) == BLOCKSIZE ) {
   SubBytes( temp, 4 ); // Substitute each byte.
  }
  #endif

  // Add bytes in GF(2) one KEYLENGTH away.
  XORBytes( temp, expandedKey - KEYLENGTH, 4 );

  // Copy result to current 4 bytes.
  *(expandedKey++) = temp[ 0 ];
  *(expandedKey++) = temp[ 1 ];
  *(expandedKey++) = temp[ 2 ];
  *(expandedKey++) = temp[ 3 ];

  i += 4; // Next 4 bytes.
 } 
}



void InvCipher( byte * block, byte * expandedKey )
{
 byte round = ROUNDS-1;
 expandedKey += BLOCKSIZE * ROUNDS;

 XORBytes( block, expandedKey, 16 );
 expandedKey -= BLOCKSIZE;

 do {
  InvShiftRows( block );
  InvSubBytesAndXOR( block, expandedKey, 16 );
  expandedKey -= BLOCKSIZE;
  InvMixColumns( block );
 } while( --round );

 InvShiftRows( block );
 InvSubBytesAndXOR( block, expandedKey, 16 );
}

void Cipher( byte * block, byte * expandedKey )    //完成一个块(16字节，128bit)的加密
{
 byte round = ROUNDS-1;

 XORBytes( block, expandedKey, 16 );
 expandedKey += BLOCKSIZE;

 do {
  SubBytes( block, 16 );
  ShiftRows( block );
  MixColumns( block );
  XORBytes( block, expandedKey, 16 );
  expandedKey += BLOCKSIZE;
 } while( --round );

 SubBytes( block, 16 );
 ShiftRows( block );
 XORBytes( block, expandedKey, 16 );
}

void aesInit( unsigned char * tempbuf )
{
 powTbl = block1;
 logTbl = block2;
 CalcPowLog( powTbl, logTbl );

 sBox = tempbuf;
 CalcSBox( sBox );

 expandedKey = block1;  //至此block1用来存贮密码表
 KeyExpansion( expandedKey );
 
 sBoxInv = block2; // Must be block2. block2至此开始只用来存贮SBOXINV
 CalcSBoxInv( sBox, sBoxInv );
} 


//对一个16字节块解密,参数buffer是解密密缓存，chainBlock是要解密的块
void aesDecrypt( unsigned char * buffer, unsigned char * chainBlock )
{
 //byte  temp[ BLOCKSIZE ];

 //CopyBytes( temp, buffer, BLOCKSIZE );
 CopyBytes(buffer,chainBlock,BLOCKSIZE);
 InvCipher( buffer, expandedKey );
 //XORBytes( buffer, chainBlock, BLOCKSIZE );
 CopyBytes( chainBlock, buffer, BLOCKSIZE );
}

//对一个16字节块完成加密，参数buffer是加密缓存，chainBlock是要加密的块
void aesEncrypt( unsigned char * buffer, unsigned char * chainBlock )
{
 CopyBytes( buffer, chainBlock, BLOCKSIZE );
 //XORBytes( buffer, chainBlock, BLOCKSIZE );
 Cipher( buffer, expandedKey );
 CopyBytes( chainBlock, buffer, BLOCKSIZE );
}

//加解密函数，参数为加解密标志，要加解密的数据缓存起始指针，要加解密的数据长度（如果解密运算，必须是16的整数倍。）
unsigned char aesBlockDecrypt(byte Direct,unsigned char *ChiperDataBuf,unsigned char DataLen)
{
    unsigned char  i;
 unsigned char  Blocks;
 unsigned char  sBoxbuf[256];
 unsigned char  tempbuf[16];
 unsigned long int  OrignLen=0; //未加密数据的原始长度

 if(Direct==0)
 {
    *((unsigned char *)&OrignLen+3)=ChiperDataBuf[0];
    *((unsigned char *)&OrignLen+2)=ChiperDataBuf[1];
    *((unsigned char *)&OrignLen+1)=ChiperDataBuf[2];
    *((unsigned char *)&OrignLen)=ChiperDataBuf[3];
    DataLen=DataLen-4;
 }
 else
 {
    memmove(ChiperDataBuf+4,ChiperDataBuf,DataLen);
    OrignLen=DataLen;
    ChiperDataBuf[0]=OrignLen;
    ChiperDataBuf[1]=OrignLen>>8;
    ChiperDataBuf[2]=OrignLen>>16;
    ChiperDataBuf[3]=OrignLen>>24;
 }
 //cleardog();
 aesInit(sBoxbuf);   //初始化
 if(Direct==0)    //解密
 {
    Blocks=DataLen/16;
    for(i=0;i<Blocks;i++)
    {
         //cleardog();
   aesDecrypt(tempbuf,ChiperDataBuf+4+16*i);
    }
    memmove(ChiperDataBuf,ChiperDataBuf+4,OrignLen);
    //cleardog();
    return(OrignLen);
 }
 else    //加密
 {
     if(DataLen%16!=0)
    {  
        Blocks=DataLen/16+1;
        //memset(ChiperDataBuf+4+Blocks*16-(DataLen%16),0x00,DataLen%16); //不足16字节的块补零处理
    }
     else
    {
        Blocks=DataLen/16;
    }

     for(i=0;i<Blocks;i++)
    {
         //cleardog();
   aesEncrypt(tempbuf,ChiperDataBuf+4+16*i);
    }
    //cleardog();
    return(Blocks*16+4);
 }
 
}



//#endif


