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
 * @def ENCRYPT_HASH_INDATA_MAXLEN
 * @brief hash 算法传入的最大数据长度。
 **/
#define ENCRYPT_HASH_INDATA_MAXLEN					1024*8 
/**
 * @def ENCRYPT_HASH_SHA1
 * @brief hash 算法的一种类型SHA1。
 **/
#define ENCRYPT_HASH_SHA1							1
/**
 * @def ENCRYPT_HASH_SHA256
 * @brief hash 算法的一种类型SHA256。
 **/
#define ENCRYPT_HASH_SHA256							2 
/**
 * @def ENCRYPT_HASH_SHA1_MAXLEN
 * @brief hash 算法SHA1计算出来的固定数据长度。
 **/
#define ENCRYPT_HASH_SHA1_MAXLEN					20
/**
 * @def HASH_SHA256_MAXLEN
 * @brief hash 算法SHA256计算出来的固定数据长度。
 **/
#define ENCRYPT_HASH_SHA256_MAXLEN					32
/**
 * @def ENCRYPT_HASH_SUCCESS
 * @brief 成功。
 **/
#define ENCRYPT_HASH_SUCCESS						0
/**
 * @def ENCRYPT_HASH_PARAMERR
 * @brief HASH算法传入的数据长度过限。
 **/
#define ENCRYPT_HASH_PARAMERR 						0xF0	
/**
 * @def ENCRYPT_RANDOM_DATALEN
 * @brief 随机数产生的最大长度。
 **/
#define ENCRYPT_RANDOM_DATALEN    					128
/**
 * @def   ENCRYPT_GETRANDOM_PARAMERR
 * @brief 要求产生的随机数长度过限。
 **/
#define ENCRYPT_GETRANDOM_PARAMERR					0xF0
/**
 * @def ENCRYPT_MAX_KEYDATA
 * @brief DES加解密传入的数据最大长度。
 **/
#define ENCRYPT_MAX_KEYDATA					16
/**
 * @def ENCRYPT_KEY_MAX_APPNUMBER
 * @brief 动态密钥系统支持的最大应用个数。
 **/
#define ENCRYPT_KEY_MAX_APPNUMBER					10
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
 * @def ENCRYPT_KEY_DEL_ALL
 * @brief 所有的应用号的密钥文件都删除。
 **/
#define ENCRYPT_KEY_DEL_ALL							0x01	
/**
 * @def ENCRYPT_KEY_DEL_PART
 * @brief 传入的应用号之外的应用号的密钥文件都删除。
 **/
#define ENCRYPT_KEY_DEL_PART						0x00

/**
 * @def ENCRYPT_MASTERKEY
 * @brief 主密钥标识。
 **/	
#define  ENCRYPT_MASTERKEY	              0
/**
 * @def ENCRYPT_PINKEY
 * @brief PIN密钥标识。
 **/	
#define  ENCRYPT_PINKEY	                  1
/**
 * @def ENCRYPT_MACKEY
 * @brief MAC密钥标识。
 **/	
#define  ENCRYPT_MACKEY   	              2
/**
 * @def ENCRYPT_KEYTYPE_SINGLE
 * @brief 单倍长密钥类型。
 **/	
#define  ENCRYPT_KEYTYPE_SINGLE						0
/**
 * @def ENCRYPT_KEYTYPE_DOUBLE
 * @brief 双倍长密钥类型。
 **/	
#define  ENCRYPT_KEYTYPE_DOUBLE						1
/**
 * @def ENCRYPT_CHECKTYPE_ZGYL_S
 * @brief 中国银联PIN和MAC单倍长算法。
 **/	
#define  ENCRYPT_CHECKTYPE_ZGYL_S          0
/**
 * @def ENCRYPT_CHECKTYPE_ZGYL_D
 * @brief 中国银联PIN和MAC双倍长算法。
 **/	
#define  ENCRYPT_CHECKTYPE_ZGYL_D          1

/**
 * @def ENCRYPT_DRV_DATA
 * @brief ENCRYPT_Decrypt_Load_Key函数使用的结构体。
 *        keytype	    函数功能类型(ENCRYPT_SAVE_BYPROTKEY,ENCRYPT_SAVE_BYTRANSKEY,ENCRYPT_DECRYPT_BYPROTKEY,ENCRYPT_DECRYPT_BYTRANSKEY,ENCRYPT_LOAD_PROTKEY)
 *        cryptindex	解密密钥索引          行序号*256+列序号
 *        saveindex	  要保存的密钥的索引号  行序号*256+列序号
 *        crypttype	  解密方式(ENCRYPT_DES_SINGLE,ENCRYPT_3DES_SINGLE,ENCRYPT_3DESDOUBLE,ENCRYPT_DES_SINGLE_DECRYPT,ENCRYPT_3DES_SINGLE_DECRYPT,ENCRYPT_3DESDOUBLE_DECRYPT)
 *        protindex	  保护密钥索引号    范围 0x30-0x39
 *        keyvalue	  传入的密钥数据   全部用asc格式
 *        outlen	    传出来的数据长度
 *        outdata	    传出来的数据     全部用asc格式
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
 * @def ENCRYPT_KEY_APPID_DATA
 * @brief ENCRYPT_Decrypt_Load_Key函数使用的结构体。
 *        delflag	    删除类型(ENCRYPT_KEY_DEL_ALL,ENCRYPT_KEY_DEL_PART)
 *        appnumber	  不要删除的应用号的个数
 *        appid	      具体应用号
 **/
typedef struct
{
	unsigned char	  delflag;                    	
	unsigned char	  appnumber;               		
	unsigned short	appid[ENCRYPT_KEY_MAX_APPNUMBER];		
}ENCRYPT_KEY_APPID_DATA;


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
 * @fn int ENCRYPT_KEY_KeyCheckApp(ENCRYPT_KEY_APPID_DATA *keyappiddata)
 * @brief 	密钥系统的密钥清除功能，可以部分删除密钥文件，也可以完全删除，只是保护密钥永远无法删除，只能覆盖
 *
 * @param keyappiddata	ENCRYPT_KEY_APPID_DATA类型结构体指针
 *
 * @return 		成功返回0,其它均为失败
 *
 */
int ENCRYPT_KEY_KeyCheckApp(ENCRYPT_KEY_APPID_DATA *keyappiddata);

/**
 * @fn int ENCRYPT_HASH_Hardware(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata)
 * @brief 	HASH算法
 *
 * @param aucIndata	 待HASH算法的数据
 *        uiLen	     待HASH算法的数据的长度
 *        ucType     HASH算法类型(ENCRYPT_HASH_SHA1,ENCRYPT_HASH_SHA256)
 *        aucOutdata HASH算法后的数据
 *
 * @return 		成功返回0,其它均为失败
 *
 */
int ENCRYPT_HASH_Hardware(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata);

/**
 * @fn int ENCRYPT_Get_Random(unsigned char uiLen,unsigned char *pucRandom)
 * @brief 	获取硬件产生的随机数
 *
 * @param uiLen	         需要传出的随机数的长度
 *        pucRandom	     传出的随机数
 *        
 * @return 		成功返回0,其它均为失败
 *
 */
int ENCRYPT_Get_Random(unsigned char uiLen,unsigned char *pucRandom);

/**
 * @fn int ENCRYPT_LoadSpecialKey(unsigned char ucKeytype,unsigned char ucCrypttype,unsigned char ucDoubleflag,unsigned char *aucIndata,unsigned char ucChecktype)
 * @brief 	保存一些特殊密钥(主密钥,MAC,PIN等)
 *
 * @param ucKeytype	     密钥类型(ENCRYPT_MASTERKEY,ENCRYPT_PINKEY,ENCRYPT_MACKEY)
 *        ucCrypttype	   解密方式(ENCRYPT_DES_SINGLE_DECRYPT,ENCRYPT_3DES_SINGLE_DECRYPT)
 *        ucDoubleflag	 密钥长度类型(ENCRYPT_KEYTYPE_SINGLE,ENCRYPT_KEYTYPE_DOUBLE)
 *        aucIndata	     传进的密钥源数据
 *        ucChecktype	   算法类型(ENCRYPT_CHECKTYPE_ZGYL_S,ENCRYPT_CHECKTYPE_ZGYL_D)
 *        
 * @return 		成功返回0,其它均为失败
 *
 */
int ENCRYPT_LoadSpecialKey(unsigned char ucKeytype,unsigned char ucCrypttype,unsigned char ucDoubleflag,unsigned char *aucIndata,unsigned char ucChecktype);

/**
 * @fn int ENCRYPT_MAC_Process(unsigned char *aucIndata,unsinged int uiInlen,unsigned char uctype,unsigned char *aucOutdata)
 * @brief 	MAC算法
 *
 * @param aucIndata	     传进的MAC计算源数据
 *        uiInlen	       传进的MAC计算源数据的长度
 *        uctype	       算法类型(ENCRYPT_CHECKTYPE_ZGYL_S,ENCRYPT_CHECKTYPE_ZGYL_D)
 *        aucOutdata	   传出的MAC计算结果
 *        
 * @return 		成功返回0,其它均为失败
 *
 */
int ENCRYPT_MAC_Process(unsigned char *aucIndata,unsigned int uiInlen,unsigned char uctype,unsigned char *aucOutdata);

/**
 * @fn int ENCRYPT_PIN_Process(unsigned char *aucIndata,unsigned long ulAmount,unsigned char uctype,unsigned char *aucOutdata)
 * @brief 	PIN算法
 *
 * @param aucIndata	     传进的PIN计算源数据
 *        ulAmount	     交易金额
 *        uctype	       算法类型(ENCRYPT_CHECKTYPE_ZGYL_S,ENCRYPT_CHECKTYPE_ZGYL_D)
 *        aucOutdata	   传出的PIN计算结果(首字节是长度,后面跟PIN值,如果首字节是0,代表密码为空)
 *        timeout	       交易超时时间(秒为单位)
 *        
 * @return 		成功返回0,其它均为失败
 *
 */
int ENCRYPT_PIN_Process(unsigned char *aucIndata,unsigned long ulAmount,unsigned char uctype,unsigned char *aucOutdata,unsigned short timeout);






#endif //ENCRYPT_H_INCLUDED
/** @} end of etc_fns */
