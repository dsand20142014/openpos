#ifndef MASSSTORGE_H
#define MASSSTORGE_H

/**
 * @defgroup 存储器文件操作
 * @brief 对USB、TF卡的文件操作
 * @author 刘一峰
 * @modefied by 李超,add tf card operate.
 *
 * @code
	int main()                                                  
	{                                                               
		int ret;                                                     
		long fileSize;                                               
		unsigned char pdata[100];                                    
		char *f="/mine.txt";
			 
		//检测TF卡是否挂载成功                                   
		ret=Os__massstorage_waitInsert(TYPE_TF,1);       
		printf("waiting %d\n",ret);                                  
		 	  
		//在TF卡中打开一个文件                                                           
		ret=Os__massstorage_selectFile(f);         
		printf("openfile %d\n",ret);                                 
		 	                                                             
		//获取打开文件的大小
		ret=Os__massstorage_getSize(&fileSize);
		printf("getsize %d %ld\n",ret,fileSize);                     
		 	  
		//读取打开的文件                                                           
		memset(pdata,0,sizeof(pdata));                               
		ret=Os__massstorage_read(0,pdata,&fileSize);                 
		printf("read %d [%ld] %s\n",ret,fileSize,pdata);            
				
		//写打开的文件                               
		ret=Os__massstorage_write(0,"hello world",11);               
		printf("write %d\n",ret);                                    
		 	  
		//读打开的文件                                                           
		memset(pdata,0,sizeof(pdata));                               
		fileSize=sizeof(pdata);                                      
		ret=Os__massstorage_read(0,pdata,&fileSize);                 
		printf("read %d [%ld] %s\n",ret,fileSize,pdata);             
		 	  
		//追加打开的文件                                                           
		ret=Os__massstorage_append(" !!!!",5);                       
		printf("append %d\n",ret);
				
		//读打开的文件                                                           
		memset(pdata,0,sizeof(pdata));                               
		fileSize=sizeof(pdata);                                      
		ret=Os__massstorage_read(0,pdata,&fileSize);                 
		printf("read %d [%ld] %s\n",ret,fileSize,pdata);                                 
		 	  
		//关闭打开的文件                                                           
		ret=Os__massstorage_closeFile();                             
		printf("close %d\n",ret);                                    
		 	  
		//删除创建的文件,需要open之后做                                                           
		ret=Os__massstorage_eraseFile();  
		printf("delete %d\n",ret);                                    

	} 
*/


/**
 * @def TYPE_TF
 * @brief TF CARD
 **/
#define TYPE_TF        0

/**
 * @def TYPE_USB
 * @brief USB DEVICE
 **/
#define TYPE_USB       1

/**
 * @def MASSSTORAGE_OK
 * @brief 操作成功。
 **/
#define MASSSTORAGE_OK        0

/**
 * @def MASSSTORAGE_FAIL
 * @brief 检测失败。
 **/
#define MASSSTORAGE_FAIL      -1

/**
 * @def MASSSTORAGE_NAMEERR
 * @brief 文件名有误。
 **/
#define MASSSTORAGE_NAMEERR  -2

/**
 * @def MASSSTORAGE_NAMETOOLONG
 * @brief 文件名过长。
 **/
#define MASSSTORAGE_NAMETOOLONG  -3

/**
 * @def MASSSTORAGE_OPTERR
 * @brief 文件操作失败
 **/
#define MASSSTORAGE_OPTERR  -4

/**
 * @def MASSSTORAGE_OPTLOCK
 * @brief 文件已打开，不能操作
 **/
#define MASSSTORAGE_OPTLOCK  -5

/**
 * @def MASSSTORAGE_MOUNTERR
 * @brief 设备已插入，但没挂上
 **/
#define MASSSTORAGE_MOUNTERR  -6

/**
 * @def MASSSTORAGE_UNMOUNTERR
 * @brief 设备卸载失败
 **/
#define MASSSTORAGE_UNMOUNTERR  -7


/**
 * @def MASSSTORAGE_MAXLENGTH
 * @brief 目录+文件最大长度。
 **/
#define MASSSTORAGE_MAXLENGTH   100


/**
 * @fn Os__massstorage_waitInsert(const char device_type, unsigned int timeout)
 * @brief 等待设备插入
 * @param device_type 设备类型 
 * @param timeout 超时时间 （秒）
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_waitInsert(const char device_type, unsigned int timeout);


/**
 * @fn unsigned int Os__massstorage_selectFile(const char *pPath)
 * @brief 打开或者创建文件, 不存在自动创建
 * @param pPath 文件路径+文件
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_selectFile(const char *pPath);


/**
 * @fn unsigned int Os__massstorage_closeFile(void)
 * @brief 关闭打开的文件,并移除挂载
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_closeFile(void);

/**
 * @fn unsigned int Os__massstorage_eraseFile(void)
 * @brief 删除文件或者文件夹
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_eraseFile(void);


/**
 * @fn unsigned int Os__massstorage_getSize(unsigned long *piSize)
 * @brief 获取文件大小
 * @param piSize  返回的目录或者文件实际大小 
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_getSize(unsigned long *piSize);


/**
 * @fn unsigned int Os__massstorage_read(unsigned int uiOffset, unsigned char *pucData,unsigned long *piLen)
 * @brief 在已打开文件指定偏移位置处读取相应长度的文件内容
 * @param uiOffset 要读取的文件偏移量。如果要读取的文件内容超出文件的最大长度，则已经读取的文件长度为0，文件内容为空。
 * @param pucData  返回已经读出的文件内容 
 * @param piLen 要读取文件的长度，同时，返回实际读取的文件长度。
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_read(unsigned int uiOffset, unsigned char *pucData,unsigned long *piLen);

/**
 * @fn unsigned int Os__massstorage_write(unsigned int uiOffset, unsigned char *pucData,unsigned int piLen)
 * @brief 在已打开文件指定偏移位置处写入相应长度的文件内容
 * @param uiOffset 要写入的文件偏移量。
 * @param pucData  待写的文件内容 
 * @param piLen 待写的文件长度
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_write(unsigned int uiOffset, unsigned char *pucData,unsigned int piLen);

/**
 * @fn unsigned int Os__massstorage_append(unsigned char *pucData,unsigned int piLen)
 * @brief 在已打开文件追加相应长度的文件内容
 * @param pucData  待写的文件内容 
 * @param piLen 待写的文件长度
 *
 * @return MASSSTORAGE_OK 成功, < 0 出错
 */
unsigned int Os__massstorage_append(unsigned char *pucData,unsigned int piLen);

#endif //MASSSTORGE_H_INCLUDED
