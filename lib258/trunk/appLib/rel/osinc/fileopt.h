#ifndef __LUXAPP_FILEOPS__
#define FILEOPS_EXTERN extern
#else
#define FILEOPS_EXTERN
#endif

#include "sandos_def.h"

/**
 * @defgroup 提供对系统文件进行读写删除的操作
 * @author 
 *				
 * @brief 文件操作
 * @{
 */


#define INI_LINE_LENGTH 64	/* 一行最多字符数*/
#define INI_MACHINE_ITEM_LINE  21+1  //
#define INI_TAXRATE_ITEM_LINE	6+1

#define OSFS_FLASHPREFIX	"./data"		/* 路径的前缀*/
#define OSFS_FLASHPREFIXLEN 8+5

/*文件操作过程返回码*/

/**
*@def FILESYSERR_SUCCESS
*@brief 操作成功
**/
#define FILESYSERR_SUCCESS      0x00

/**
*@def FILESYSERR_GENERAL
*@brief 文件代码已满，或初始失败，或系统操作文件失败
**/
#define FILESYSERR_GENERAL      0x01

/**
*@def FILESYSERR_INVALIDPARAM
*@brief 非法文件系数(文件代码, 文件名, 文件读写长度, 文件读写, 缓冲)
**/
#define FILESYSERR_INVALIDPARAM 0x02

/**
*@def FILESYSERR_CORRUPT
*@brief 文件阻碍系统建立失败(Flash文件系统)
**/
#define FILESYSERR_CORRUPT      0x03

/**
*@def FILESYSERR_NOFS
*@brief 文件系统不存在
**/
#define FILESYSERR_NOFS         0x04

/**
*@def FILESYSERR_NOSPACE
*@brief 文件空间不够
**/
#define FILESYSERR_NOSPACE      0x05

/**
*@def FILESYSERR_NOFAT
*@brief 未用
**/
#define FILESYSERR_NOFAT        0x06

/**
*@def FILESYSERR_FATBROKEN
*@brief 未用
**/
#define FILESYSERR_FATBROKEN    0x07

/**
*@def FILESYSERR_FATCRC
*@brief 未用
**/
#define FILESYSERR_FATCRC       0x08

/**
*@def FILESYSERR_READ
*@brief 读文件失败(Flash文件系统)
**/
#define FILESYSERR_READ         0x09

/**
*@def FILESYSERR_WRITE
*@brief 部分文件写入失败(文件未全部写入)
**/
#define FILESYSERR_WRITE        0x0A

/**
*@def FILESYSERR_INVALIDID
*@brief 非法文件操作
**/
#define FILESYSERR_INVALIDID    0x0B

/**
*@def FILESYSERR_FILENOTOPEN
*@brief 文件未打开
**/
#define FILESYSERR_FILENOTOPEN  0x0C

/**
*@def FILESYSERR_BADOFFSET
*@brief 文件偏移误差
**/
#define FILESYSERR_BADOFFSET    0x0D

/**
*@def FILESYSERR_NOPRIVILEGE
*@brief 文件属性错误
**/
#define FILESYSERR_NOPRIVILEGE  0x0E

/**
*@def FILESYSERR_FILENAME
*@brief 文件名错误
**/
#define FILESYSERR_FILENAME     0x0F

/**
*@def FILESYSERR_WRITE_2
*@brief 写文件失败(Flash文件系统)
**/
#define FILESYSERR_WRITE_2      0x10

#define FILEFS_FILENAMELEN      255


FILEOPS_EXTERN int  set_app_id(unsigned short appid);

/**
 * @fn int OSAPP_OpenFile(char *pcFileName,unsigned char ucMode)
 * @brief 	在不同的应用可以创建相同文件名的文件,在底层os会根据应用标示区分这两个文件.在同一应用不可创建两个相同文件名的文件。
 *
 * @param pcFileName	指向文件名字符串的指针,ucMode打开模式文件名最长为九个字符.
 * @param ucMode  	打开模式: O_RDWR 打开的文件可以进行读写操作
 * 							  O_CREATE 创建一个新文件.
 * 							  O_TRUNC 文件长度截为零
 *
 * @return 		成功返回打开的文件描述符,失败返回-1
 *
 */
FILEOPS_EXTERN int OSAPP_OpenFile(char *pcFileName,unsigned char ucMode);

FILEOPS_EXTERN int OSAPP_OpenFileEx(char *pcFileName,unsigned char ucMode,unsigned char *pucAttr);

/**
 * @fn int OSAPP_FileRead(int iHandle,unsigned char *pucOutData,unsigned int uiOutLen)
 * @brief 	从文件句柄为iHandle中读取最大长度为uiOutLen的数据
 *
 * @param iHandle		文件句柄
 * @param pucOutData  	数据指针,指向从文件中读出的数据存放的首地址
 * @param uiOutLen		从文件中要读的字节数
 *
 * @return 		成功返回实际读到的字节数,失败返回-1
 *
 */
FILEOPS_EXTERN int OSAPP_FileRead(int iHandle,unsigned char *pucOutData,unsigned int uiOutLen);

/**
 * @fn int OSAPP_FileWrite(int iHandle,unsigned char *pucInData,unsigned int uiInLen)
 * @brief 	向文件句柄为iHandle中写入长度为uiInLen的数据
 *
 * @param iHandle		文件句柄
 * @param pucInData  	数据指针,指向要写入文件数据的首地址
 * @param uiInLen		要写入文件中字节数
 *
 * @return 		成功返回实际写入的字节数,失败返回-1
 *
 */
FILEOPS_EXTERN int OSAPP_FileWrite(int iHandle,unsigned char *pucInData,unsigned int uiInLen);

/**
 * @fn int OSAPP_FileClose(int iHandle)
 * @brief 	关闭文件
 *
 * @param iHandle		文件句柄
 *
 * @return 		成功关闭文件返回0,失败返回-1
 *
 */
FILEOPS_EXTERN int OSAPP_FileClose(int iHandle);

/**
 * @fn int OSAPP_FileSeek(int iHandle,long lOffset,unsigned char ucLocation)
 * @brief 	向文件句柄为iHandle中写入长度为uiInLen的数据
 *
 * @param iHandle		文件句柄
 * @param lOffset  		读写位置偏移量
 * @param ucLocation	移动模式
 * 							SEEK_CUR: 以目前的读写位置往后增加offset个位移量
 * 							SEEK_SET: 参数offset即为新的读写位置
 * 							SEEK_END: 将读写位置指向文件尾后再增加offset个位移量
 *
 * @return 		成功返回0,失败返回1
 *
 */
FILEOPS_EXTERN int OSAPP_FileSeek(int iHandle,long lOffset,unsigned char ucLocation);

/**
 * @fn unsigned long OSAPP_GetUnusedFlashSize(void)
 * @brief 	获取全部剩余空间的大小
 *
 * @return 	返回剩余空间的大小
 *
 */
FILEOPS_EXTERN unsigned long OSAPP_GetUnusedFlashSize(void);

/**
 * @fn long int OSAPP_FileSize(char *pcFileName)
 * @brief 	返回文件的大小
 *
 * @param pcFileName	指向文件名字符串的指针
 *
 * @return 		返回文件的大小; -1表示文件不存在
 *
 */
FILEOPS_EXTERN long int OSAPP_FileSize(char *pcFileName);

/**
 * @fn long int  OSAPP_GetFlashSize(void)
 * @brief 	获取应用程序数据的空间大小
 *
 * @return 	返回应用程序数据的空间大小
 *
 */
FILEOPS_EXTERN long int  OSAPP_GetFlashSize(void);


/**
 * @fn char OSAPP_SetFlashSize(long int size)
 * @brief 	设置应用程序数据的空间大小
 *
 * @return 	返回0
 *
 */
FILEOPS_EXTERN char OSAPP_SetFlashSize(long int size);

/**
 * @fn int OSAPP_FileDelete(char *pcFileName)
 * @brief 	删除文件
 *
 * @param pcFileName	指向文件名字符串的指针
 *
 * @return	成功返回0,失败返回-1
 *
 */
FILEOPS_EXTERN int OSAPP_FileDelete(char *pcFileName);

/**
 * @fn int OSAPP_FileTrunc(int iHandle,unsigned int uiLen)
 * @brief 	将文件长度减少为uiLen,原文件从uiLen到结尾的内容被完全删除,相对应的内存将被释放
 *
 * @param iHandle	文件句柄
 * @param uiLen		文件截取后的文件大小
 *
 * @return	成功返回0,失败返回-1
 *
 */
FILEOPS_EXTERN int OSAPP_FileTrunc(int iHandle,unsigned int uiLen);

/**
 * @fn unsigned char OSAPP_FileGetLastError(void)
 * @brief 	获取文件最后操作值
 *
 *
 * @return	文件系统函数的最后一项操作值
 *
 */
FILEOPS_EXTERN unsigned char OSAPP_FileGetLastError(void);

FILEOPS_EXTERN unsigned char Os__saved_swap (unsigned char PageNb);
FILEOPS_EXTERN void Os__saved_copy(unsigned char * ptSource, unsigned char * ptDest, unsigned short Length);
FILEOPS_EXTERN void Os__saved_set(unsigned char * ptDest, unsigned char Charac, unsigned short Length);
