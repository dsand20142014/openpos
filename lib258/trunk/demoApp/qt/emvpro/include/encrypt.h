#ifndef	ENCRYPT_H_INCLUDED
#define	ENCRYPT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#define HASH_INDATA_MAXLEN     1024*8 
#define HASH_SHA1              1
#define HASH_SHA256            2 

#define HASH_SHA1_MAXLEN       20
#define HASH_SHA256_MAXLEN     32
#define ENCRYPT_MAX_KEYDATA    16
#define KEY_MAX_APPNUMBER      10

#define ENCRYPT_SAVE_BYPROTKEY       0x30   // 用保护密钥解压保存传输密钥
#define ENCRYPT_SAVE_BYTRANSKEY      0x31   // 用存储的密钥解压保存传输密钥
#define ENCRYPT_DECRYPT_BYPROTKEY    0x32   // 用保护密钥加解密
#define ENCRYPT_DECRYPT_BYTRANSKEY   0x33   // 用存储的密钥加解密
#define ENCRYPT_LOAD_PROTKEY         0x34   // 下载保护密钥

#define ENCRYPT_DES_SINGLE                   0x30   //单DES加密
#define ENCRYPT_3DES_SINGLE                  0x31   //3DES加密
#define ENCRYPT_3DESDOUBLE                   0x32   //3DES双倍长加密
#define ENCRYPT_DES_SINGLE_DECRYPT           0x33   //单DES解密
#define ENCRYPT_3DES_SINGLE_DECRYPT          0x34   //3DES解密
#define ENCRYPT_3DESDOUBLE_DECRYPT           0x35   //3DES双倍长解密

#define KEY_DEL_ALL            0x01    //全部删除
#define KEY_DEL_PART           0x00    //部分删除


typedef struct
{
	 unsigned char        hashtype;                                             // HASH_SHA1 ,HASH_SHA256
   unsigned short       hashlen;                                             // 传进来数据的长度,同样也是传出去的数据
   unsigned char        hashindata[HASH_INDATA_MAXLEN];                      //  传进来数据
   unsigned char        hashoutdata[HASH_SHA256_MAXLEN];                  // 计算出来的HASH值，一种是20，一种是32
}ENCRYPT_HASH_DATA;

typedef struct
{
	 unsigned char        keytype;                                             // 类型  0x30(用保护密钥解压保存传输密钥) 0x31用存储的密钥解压保存传输密钥) 0x32(用保护密钥加解密)0x33(用存储的密钥加解密 ) 0x34 (下载保护密钥)
   unsigned short       saveappid;                                           //保存传输密钥的应用号
   unsigned short       cryptappid;                                       //  解密传输密钥的应用号
   unsigned short       cryptindex;                                         // 解密密钥索引
   unsigned short       saveindex;                                         // 存密钥索引
   unsigned char        crypttype;                                         // .解密方式      0x30 单des加密 单倍长  0x31 3des加密  0x32 3des双倍长加密  0x33 单DES解密 0x34 3DES解密  0x35   3DES双倍长解密
   unsigned char        protindex;                                          // 保护密钥索引
   unsigned char        keyvalue[ENCRYPT_MAX_KEYDATA*2];                                         // 密钥
   unsigned char        outlen;                                            // 传出来的数据长度
   unsigned char        outdata[ENCRYPT_MAX_KEYDATA*2];                                         //  传出来的数据
}ENCRYPT_DRV_DATA; 

typedef struct
{
	 unsigned char        delflag;                                             // 全部删除，还是部分删除
   unsigned char        appnumber;                                             // 删除密钥的应用个数
   unsigned short       appid[KEY_MAX_APPNUMBER];                             //  具体应用号
}KEY_APPID_DATA;



int ENCRYPT_Hash(ENCRYPT_HASH_DATA * hashdata);
int ENCRYPT_Decrypt_Load_Key(ENCRYPT_DRV_DATA *keydrvdata);
int KEY_KeyCheckApp(KEY_APPID_DATA *keyappiddata);

#ifdef __cplusplus
}
#endif

#endif //ENCRYPT_H_INCLUDED
