#ifndef	ENCRYPT_H_INCLUDED
#define	ENCRYPT_H_INCLUDED

/**
 * @defgroup 加解密函数
 * @author 卞正青
 * @author edit by 熊剑 20130402 
 *				添加注释
 * @brief 对数据进行加密和解密操作
 * @{
 */
 
 
  /**
 * @def ENCRYPT_FUNCTION_NORMAL
 * @brief ENCRYPT_DRV_DATA里面fundata缺省值。
 **/
#define ENCRYPT_FUNCTION_NORMAL					0
 /**
 * @def ENCRYPT_MAX_BACKUPLEN
 * @brief 保留数据的最大长度。
 **/
#define ENCRYPT_MAX_BACKUPLEN					100 
/**
 * @def HASH_INDATA_MAXLEN
 * @brief hash 算法传入的最大数据长度。
 **/
#define HASH_INDATA_MAXLEN					1024*8 
/**
 * @def HASH_SHA1
 * @brief hash 算法的一种类型SHA1。
 **/
#define HASH_SHA1							1
/**
 * @def HASH_SHA256
 * @brief hash 算法的一种类型SHA256。
 **/
#define HASH_SHA256							2 
/**
 * @def HASH_SHA1_MAXLEN
 * @brief hash 算法SHA1计算出来的固定数据长度。
 **/
#define HASH_SHA1_MAXLEN					20
/**
 * @def HASH_SHA256_MAXLEN
 * @brief hash 算法SHA256计算出来的固定数据长度。
 **/
#define HASH_SHA256_MAXLEN					32
/**
 * @def HASH_SUCCESS
 * @brief 成功。
 **/
#define HASH_SUCCESS						0
/**
 * @def HASH_PARAMERR
 * @brief HASH算法传入的数据长度过限。
 **/
#define HASH_PARAMERR 						0xF0	
/**
 * @def RANDOM_DATALEN
 * @brief 随机数产生的最大长度。
 **/
#define RANDOM_DATALEN    					128
/**
 * @def   GETRANDOM_PARAMERR
 * @brief 要求产生的随机数长度过限。
 **/
#define GETRANDOM_PARAMERR					0xF0
/**
 * @def ENCRYPT_MAX_KEYDATA
 * @brief DES加解密传入的数据最大长度。
 **/
#define ENCRYPT_MAX_KEYDATA					16
/**
 * @def KEY_MAX_APPNUMBER
 * @brief 动态密钥系统支持的最大应用个数。
 **/
#define KEY_MAX_APPNUMBER					10
/**
 * @def ENCRYPT_SAVE_BYPROTKEY
 * @brief 用保护密钥来解密然后保存密钥。
 **/
#define ENCRYPT_SAVE_BYPROTKEY				0x30	
/**
 * @def ENCRYPT_SAVE_BYTRANSKEY
 * @brief 用存储的密钥来解密然后保存密钥。
 **/
#define ENCRYPT_SAVE_BYTRANSKEY				0x31
/**
 * @def ENCRYPT_DECRYPT_BYPROTKEY
 * @brief 用保护密钥来加解密数据。
 **/	
#define ENCRYPT_DECRYPT_BYPROTKEY			0x32	
/**
 * @def ENCRYPT_DECRYPT_BYTRANSKEY
 * @brief 用存储的密钥来加解密数据。
 **/	
#define ENCRYPT_DECRYPT_BYTRANSKEY			0x33	
/**
 * @def ENCRYPT_LOAD_PROTKEY
 * @brief 保存保护密钥。
 **/
#define ENCRYPT_LOAD_PROTKEY				0x34	
/**
 * @def ENCRYPT_DES_SINGLE
 * @brief 单DES加密。
 **/
#define ENCRYPT_DES_SINGLE					0x30	
/**
 * @def ENCRYPT_3DES_SINGLE
 * @brief 3DES加密。
 **/
#define ENCRYPT_3DES_SINGLE					0x31	
/**
 * @def ENCRYPT_3DESDOUBLE
 * @brief 3DES双倍长加密。
 **/
#define ENCRYPT_3DESDOUBLE					0x32	
/**
 * @def ENCRYPT_DES_SINGLE_DECRYPT
 * @brief 单DES解密。
 **/
#define ENCRYPT_DES_SINGLE_DECRYPT			0x33	
/**
 * @def ENCRYPT_3DES_SINGLE_DECRYPT
 * @brief 3DES解密。
 **/
#define ENCRYPT_3DES_SINGLE_DECRYPT			0x34	
/**
 * @def ENCRYPT_DES_SINGLE_DECRYPT
 * @brief 3DES双倍长解密。
 **/
#define ENCRYPT_3DESDOUBLE_DECRYPT			0x35	
/**
 * @def KEY_DEL_ALL
 * @brief 所有的应用号的密钥文件都删除。
 **/
#define KEY_DEL_ALL							0x01	
/**
 * @def KEY_DEL_PART
 * @brief 传入的应用号之外的应用号的密钥文件都删除。
 **/
#define KEY_DEL_PART						0x00	
/**
 * @def ENCRYPT_DRV_DATA
 * @brief ENCRYPT_Decrypt_Load_Key函数使用的结构体。
 *        keytype	    函数功能类型(ENCRYPT_SAVE_BYPROTKEY,ENCRYPT_SAVE_BYTRANSKEY,ENCRYPT_DECRYPT_BYPROTKEY,ENCRYPT_DECRYPT_BYTRANSKEY,ENCRYPT_LOAD_PROTKEY)
 *        cryptindex	解密密钥索引          行序号*256+列序号
 *        saveindex	  要保存的密钥的索引号  行序号*256+列序号
 *        crypttype	  解密方式(ENCRYPT_DES_SINGLE,ENCRYPT_3DES_SINGLE,ENCRYPT_3DESDOUBLE,ENCRYPT_DES_SINGLE_DECRYPT,ENCRYPT_3DES_SINGLE_DECRYPT,ENCRYPT_3DESDOUBLE_DECRYPT)
 *        protindex	  保护密钥索引号
 *        keyvalue	  传入的密钥数据   hex格式
 *        outlen	    传出来的数据长度
 *        outdata	    传出来的数据      hex格式
 *        cryptflag	  标识,当前版本只支持ENCRYPT_FUNCTION_NORMAL
 *        reserve   	可扩展数据
 **/
typedef struct
{
	unsigned char	  keytype;                        	            		
	unsigned short	cryptindex;               			
	unsigned short	saveindex;                        	
	unsigned char	  crypttype;                   		
	unsigned char	  protindex;                      	
	unsigned char	  keyvalue[ENCRYPT_MAX_KEYDATA*2];	
	unsigned char	  outlen;                         	
	unsigned char	  outdata[ENCRYPT_MAX_KEYDATA*2];		
	unsigned char   cryptflag;
	unsigned char   reserve[ENCRYPT_MAX_BACKUPLEN];
}ENCRYPT_DRV_DATA; 
/**
 * @def KEY_APPID_DATA
 * @brief ENCRYPT_Decrypt_Load_Key函数使用的结构体。
 *        delflag	    删除类型(KEY_DEL_ALL,KEY_DEL_PART)
 *        appnumber	  不要删除的应用号的个数
 *        appid	      具体应用号
 **/
typedef struct
{
	unsigned char	  delflag;                    	
	unsigned char	  appnumber;               		
	unsigned short	appid[KEY_MAX_APPNUMBER];		
}KEY_APPID_DATA;


/**
 * @fn int ENCRYPT_Decrypt_Load_Key(ENCRYPT_DRV_DATA *keydrvdata)
 * @brief 	对数据进行加解密操作和存储密钥
 *
 * @param keydrvdata	ENCRYPT_DRV_DATA类型结构体指针
 *
 * @return 		成功返回0,其它均为失败
 *
 */
int ENCRYPT_Decrypt_Load_Key(ENCRYPT_DRV_DATA *keydrvdata);

/**
 * @fn int KEY_KeyCheckApp(KEY_APPID_DATA *keyappiddata)
 * @brief 	密钥系统的密钥清除功能，可以部分删除密钥文件，也可以完全删除，只是保护密钥永远无法删除，只能覆盖
 *
 * @param keyappiddata	KEY_APPID_DATA类型结构体指针
 *
 * @return 		成功返回0,其它均为失败
 *
 */
int KEY_KeyCheckApp(KEY_APPID_DATA *keyappiddata);

/**
 * @fn int HASH_Hardware(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata)
 * @brief 	HASH算法
 *
 * @param aucIndata	 待HASH算法的数据
 *        uiLen	     待HASH算法的数据的长度
 *        ucType     HASH算法类型(HASH_SHA1,HASH_SHA256)
 *        aucOutdata HASH算法后的数据
 *
 * @return 		成功返回0,其它均为失败
 *
 */
int HASH_Hardware(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata);

/**
 * @fn int Get_Random(unsigned char uiLen,unsigned char *pucRandom)
 * @brief 	获取硬件产生的随机数
 *
 * @param uiLen	         需要传出的随机数的长度
 *        pucRandom	     传出的随机数
 *        
 * @return 		成功返回0,其它均为失败
 *
 */
int Get_Random(unsigned char uiLen,unsigned char *pucRandom);


#endif //ENCRYPT_H_INCLUDED
/** @} end of etc_fns */
