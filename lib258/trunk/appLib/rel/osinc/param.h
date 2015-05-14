#ifndef PARAM_H_INCLUDED
#define PARAM_H_INCLUDED

/**
 * @defgroup INI文件操作
 * @brief 提供ini文件操作的api
 * @author 刘一峰
 
 * ***提供给POS应用的文件操作***
 * @code
 * void main()
 * {  
 * unsigned char buffer[20];
 * Os__param_appIni(); //放在poweron事件中
 * Os__param_appSet("key1","a1");
 * Os__param_appSet("key2","a2");
 * 
 * Os__param_appGet("key1",buffer,sizeof(buffer));
 * printf("%s\n",buffer); //-->"a1"
 * }
 * @endcode
 *
 * 经过操作之后 文件conf.ini结构如下所示
 *
 * @code
 * [SYS]
 * key1=a1
 * key2=a2
 *
 * @endcode
 
 * ***基本INI文件操作***
 
 * 文件conf.ini结构如下所示
 *
 * @code
 * [system1]
 * key1=value_x
 * key2=value_y
 *
 * [system2]
 * key1=value_a
 * key2=value_b
 * @endcode
 *
 * 对文件操作如下所示
 *
 * @code
 * #define FILENAME "conf"
 * void main()
 * {
 * ETC_HANDLE etcFile;
 * ETC_HANDLE etcSection1,etcSection2,etcSection3;
 * unsigned char buffer[20];
 *
 * etcFile=Os__param_load(FILENAME);
 * 
 * etcSection1=Os__param_findSection(etcFile,"mineS1");
 * etcSection2=Os__param_findSection(etcFile,"mineS2");
 * etcSection3=Os__param_findSection(etcFile,"system2");
 *
 * Os__param_set(etcSection1,"IP","172.16.1.1");
 * Os__param_set(etcSection1,"key2","a2");
 * Os__param_set(etcSection2,"key1","b1");
 * Os__param_set(etcSection2,"key2","b2");
 * 
 * Os__param_save(etcFile,FILENAME);
 *
 * memset(buffer,0,sizeof(buffer));
 * Os__param_get(etcSection3,"key1",buffer,sizeof(buffer));
 * printf("%s\n",buffer); //-->"value_a"
 *
 * memset(buffer,0,sizeof(buffer));
 * Os__param_get(etcSection1,"key1",buffer,sizeof(buffer));
 * printf("%s\n",buffer); //-->"a1"
 *
 * Os__param_unload(etcFile);
 * }
 * @endcode
 *
 * 经过操作之后 文件conf.ini结构如下所示
 *
 * @code
 * [system1]
 * key1=value_x
 * key2=value_y
 *
 * [system2]
 * key1=value_a
 * key2=value_b
 *
 * [mineS1]
 * key1=a1
 * key2=a2
 *
 * [mineS2]
 * key1=b1
 * key2=b2
 *
 * @endcode
 
 * @{
 */


/**
 * @def PARAM_PATH
 * @brief 目标文件夹名。
 **/
#define PARAM_PATH "data" 

/**
 * @def PARAM_MAXLINE
 * @brief 配置文件所允许的最大行数。
 **/
#define PARAM_MAXLINE             1024
/**
 * @def PARAM_MAXFILENAME
 * @brief 配置文件名最大长度。
 **/
#define PARAM_MAXFILENAME         20
/**
 * @def PARAM_FILENOTFOUND
 * @brief 没有找到此配置文件。
 **/
#define PARAM_FILENOTFOUND        -1
/**
 * @def PARAM_SECTIONNOTFOUND
 * @brief 在配置文件中没有找到此section。
 **/
#define PARAM_SECTIONNOTFOUND     -2
/**
 * @def PARAM_KEYNOTFOUND
 * @brief 在此section中没有找到这个key。
 **/
#define PARAM_KEYNOTFOUND         -3
/**
 * @def PARAM_TMPFILEFAILED
 * @brief 创建缓存文件出错。
 **/
#define PARAM_TMPFILEFAILED       -4
/**
 * @def PARAM_FILEIOFAILED
 * @brief 配置文件读写操作错误。
 **/
#define PARAM_FILEIOFAILED        -5
/**
 * @def PARAM_INTCONV
 * @brief  将字符串转换成整型数时出错。
 **/
#define PARAM_INTCONV             -6
/**
 * @def PARAM_INVALIDOBJ
 * @brief 配置文件中有非法对象。
 **/
#define PARAM_INVALIDOBJ          -7
/**
 * @def PARAM_READONLYOBJ
 * @brief 配置文件是只读的。
 **/
#define PARAM_READONLYOBJ         -8

/**
 * @def PARAM_FILENAME
 * @brief 文件名过长。
 **/
#define PARAM_FILENAME            -9

/**
 * @def PARAM_OK
 * @brief 配置文件操作成功。
 **/
#define PARAM_OK                  0


/**
 * @fn void Os__param_appIni(void)
 * @brief 初始化配置 默认文件名为config.ini,section名为SYS
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
void Os__param_appIni(void);

/**
 * @fn int Os__param_appSet(char* pKey, char* pValue)
 * @brief 设置key值 并自动保存到配置文件
 * @param pKey 被设置的key名称
 * @param pValue key填充数据
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_appSet(char* pKey, char* pValue);

/**
 * @fn int Os__param_appGet(char* pKey, char* pValue,int iLen)
 * @brief 获取key的值
 * @param pKey 需要查找key名称
 * @param pValue 取出的值存于此buffer中
 * @param iLen 需要取出的长度
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_appGet(char* pKey, char* pValue,int iLen);

/**
 * @fn int Os__param_load(char * pFileName)
 * @brief 读配置文件 如果不存在 函数自己创建
 * @param pFileName 配置文件名称
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_load(char * pFileName);

int Os__param_loadFullPath(char *pFileName);


/**
 * @fn int Os__param_unload(unsigned int hEtc)
 * @brief 释放资源
 * @param hEtc 文件句柄
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_unload(unsigned int hEtc);


/**
 * @fn int Os__param_findSection(unsigned int hEtc,char* pSection,int create)
 * @brief 在配置文件寻找或创建一个section 并读取到内存里
 * @param hEtc 文件句柄 
 * @param pSection 需要查找或者创建的section名称
 * @param create 是否要新建 1-是 0-不要
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_findSection(unsigned int hEtc,char* pSection,int create);

/**
 * @fn int Os__param_rmSection(unsigned int hEtc,char* pSection)
 * @brief 在配置文件删除一个section
 * @param hEtc 文件句柄 
 * @param pSection 需要查找或者创建的section名称
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_rmSection(unsigned int hEtc,char* pSection);

/**
 * @fn int Os__param_get(unsigned int hSect,char* pKey, char* pValue,int iLen)
 * @brief 从内存中获取key的值
 * @param hSect section句柄
 * @param pKey 需要查找key名称
 * @param pValue 取出的值存于此buffer中
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_get(unsigned int hSect,char* pKey, char* pValue,int iLen);

/**
 * @fn int Os__param_set(unsigned int hSect, char* pKey, char* pValue)
 * @brief 设置内存中的key值
 * @param hSect section句柄
 * @param pKey 被设置的key名称
 * @param pValue key填充数据
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_set(unsigned int hSect, char* pKey, char* pValue);

/**
 * @fn int Os__param_save(unsigned int hSect, char* pFileName)
 * @brief 将内存中的ini表存文件
 * @param hSect section句柄
 * @param pFileName 配置文件名称
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_save(unsigned int hSect, char* pFileName);

int Os__param_saveFullPath(unsigned int hSect, char* pFileName);

/**
 * @fn int Os__param_reload(unsigned int hEtc,char * pFileName)
 * @brief 重新读取配置表 只有在非本地操作时才需要重新载入
 * @param hEtc 文件句柄
 * @param pFileName 配置文件名称
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__param_reload(unsigned int hEtc,char * pFileName);

/**
 * @fn int Os__param_getAllSection(unsigned int hEtc, char section[][50],int sectionSize)
 * @brief 	获取配置表中section列表
 * @param hEtc 文件句柄
 * @param section 用于存放section的列表地址
 * @param sectionSize section列表最大能够保持的section个数
 *
 * @return  成功返回获取到的section个数,出错返回-1.
 */
int Os__param_getAllSection(unsigned int hEtc, char section[][50],int sectionSize);

#endif //PARAM_H_INCLUDED
