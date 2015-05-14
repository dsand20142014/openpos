#ifndef OSINI_H_INCLUDED
#define OSINI_H_INCLUDED

#define LEN_BITMAP          (1+2+4+8+16)
#define NR_HEAP             5
#define MAX_LEN_FIXSTR      64

#define NR_SECTS_INC_ALLOC      8
#define NR_KEYS_INC_ALLOC       4
#define NR_KEYS_INIT_ALLOC      8
#define NR_SECTS_INIT_ALLOC     16

typedef unsigned int ETC_HANDLE;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_PATH 256

/**
 * \defgroup etc_fns INI文件操作
 * \author 曾舒
 * 所有的INI接口都移植自MINIGUI的开源部分代码，去掉LINUX中不需要的部分，并将
 * 文档做成中文。
 *
 * INI接口采用类似于Windows中的ini文件结构，每个文件包括若干个section，然后
 * 每个section包括若干个key，如下所示：
 *
 * \code
 * [system]
 * # GAL engine
 * gal_engine=fbcon
 *
 * # IAL engine
 * ial_engine=console
 *
 * mdev=/dev/mouse
 * mtype=PS2
 *
 * [fbcon]
 * defaultmode=1024x768-16bpp
 *
 * [qvfb]
 * defaultmode=640x480-16bpp
 * display=0
 * \endcode
 *
 * 假设这个配置文件叫做 \a my.cfg ，如果你想从 \a system 这个section中取出叫做 \a mdev 的
 * key的键值，你可以按照如下方式调用 \a GetValueFromEtcFile 函数：
 *
 * \code
 * char buffer [51];
 *
 * GetValueFromEtcFile ("my.cfg", "system", "mdev", buffer, 51);
 * \endcode
 *
 * @{
 */

/**
 * \def ETC_MAXLINE
 * \brief 配置文件所允许的最大行数。
 **/
#define ETC_MAXLINE             1024

/**
 * \def ETC_FILENOTFOUND
 * \brief 没有找到此配置文件。
 **/
#define ETC_FILENOTFOUND        -1
/**
 * \def ETC_SECTIONNOTFOUND
 * \brief 在配置文件中没有找到此section。
 **/
#define ETC_SECTIONNOTFOUND     -2
/**
 * \def ETC_KEYNOTFOUND
 * \brief 在此section中没有找到这个key。
 **/
#define ETC_KEYNOTFOUND         -3
/**
 * \def ETC_TMPFILEFAILED
 * \brief 创建缓存文件出错。
 **/
#define ETC_TMPFILEFAILED       -4
/**
 * \def ETC_FILEIOFAILED
 * \brief 配置文件读写操作错误。
 **/
#define ETC_FILEIOFAILED        -5
/**
 * \def ETC_INTCONV
 * \brief  将字符串转换成整型数时出错。
 **/
#define ETC_INTCONV             -6
/**
 * \def ETC_INVALIDOBJ
 * \brief 配置文件中有非法对象。
 **/
#define ETC_INVALIDOBJ          -7
/**
 * \def ETC_READONLYOBJ
 * \brief 配置文件是只读的。
 **/
#define ETC_READONLYOBJ         -8
/**
 * \def ETC_OK
 * \brief 配置文件操作成功。
 **/
#define ETC_OK                  0

/** ETCSECTION 当前的section信息 */
typedef struct _ETCSECTION
{
    /** 定位key的位置 */
    int key_nr_alloc;
    /** section中key的数量 */
    int key_nr;
    /** section的名称 */
    char *name;
    /** key的数组 */
    char** keys;
    /** value的数组 */
    char** values;
} ETCSECTION;

/** 将指向ETCSECTION的指针定义为PETCSECTION */
typedef ETCSECTION* PETCSECTION;

/** ETC_S 当前配置文件的信息*/
typedef struct _ETC_S
{
    /** 定位到section的位置 */
    int sect_nr_alloc;
    /** 配置文件中section的数量 */
    int section_nr;
    /** 指向sections的指针 */
    PETCSECTION sections;
} ETC_S;


/**
 * \fn int  GetValueFromEtcFile (const char* pEtcFile,\
 const char* pSection, const char* pKey, char* pValue, int iLen)
 * \brief 从配置文件中取得键值
 *
 * 这个函数从 \a pEtcFile 配置文件中 \a pSection 节的 \a pKey 中取出键值，将其存在
 * \a pValue 指向的buffer中。
 *
 * \param pEtcFile 配置文件的路径名称。
 * \param pSection 需要取值的section名称。
 * \param pKey 需要取值的key名称。
 * \param pValue 取出的值将存在此buffer中。
 * \param iLen buffer的大小，以BYTE为单位。
 *
 * \return ETC_OK 成功, < 0 出错.
 *
 * \retval ETC_OK               成功取值。
 * \retval ETC_FILENOTFOUND     未找到指定的配置文件。
 * \retval ETC_SECTIONNOTFOUND  未找到指定的section。
 * \retval ETC_KEYNOTFOUND      在section中未找到指定的key。
 * \retval ETC_FILEIOFAILED     文件I/O操作发生错误。
 *
 * \note 配置文件接口使用 \a strncpy 将key的真值拷贝到 \a pValue. 因此，
 *      如果真正的键值的长度大于 \a iLen, 那么拷贝到 \a pValue 的结果将
 *		 \em 不会 由null结尾。
 *
 * \sa GetIntValueFromEtcFile, SetValueToEtcFile, strncpy(3)
 */
int  GetValueFromEtcFile (const char* pEtcFile,
                          const char* pSection, const char* pKey, char* pValue, int iLen);

/**
 * \fn int  GetIntValueFromEtcFile (const char* pEtcFile,\
 const char* pSection, const char* pKey, int* value)
 * \brief 从配置文件中取得一个整型值。
 *
 * 这个函数从 \a pEtcFile 配置文件中 \a pSection 节的 \a pKey 中取出一个整型值，
 * 并将此整型值存于 \a value 指向的buffer中。
 *
 * \param pEtcFile 配置文件的路径名称。
 * \param pSection 需要取值的section名称。
 * \param pKey 需要取值的key名称。
 * \param value 取得的整型值将存于此buffer中。
 *
 * \return ETC_OK 成功，< 0 失败。
 *
 * \retval ETC_OK               成功取值。
 * \retval ETC_FILENOTFOUND     未找到指定的配置文件。
 * \retval ETC_SECTIONNOTFOUND  未找到指定的section。
 * \retval ETC_KEYNOTFOUND      在section中未找到指定的key。
 * \retval ETC_FILEIOFAILED     文件I/O操作发生错误。
 * \retval ETC_INTCONV          不能将此键值转换为整型数。
 *
 * \note 配置文件接口使用 \a strtol 来将字符串转换为一个整型值并作为十进制数传递，
 * 	     因此, 一个合法的能够被转换的字符串应当遵守如下格式:
 *
 *  - [+|-]0x[0-9|A-F]*\n
 *    将被作为16进制数读取。
 *  - [+|-]0[0-7]*\n
 *    将被作为8进制数读取。
 *  - [+|-][1-9][0-9]*\n
 *    将被作为10进制数读取。
 *
 * \sa GetValueFromEtcFile, SetValueToEtcFile, strtol(3)
 */
int  GetIntValueFromEtcFile (const char* pEtcFile,
                             const char* pSection, const char* pKey, int* value);

/**
 * \fn int  SetValueToEtcFile (const char* pEtcFile,\
 const char* pSection, const char* pKey, char* pValue)
 * \brief 将一个值存在配置文件中。
 *
 * 这个函数将 \a pValue 指向的字符串值存于 \a pEtcFile 指定的配置文件中，
 * 其section由 \a pSection 指定，key由 \a pKey 来指定。
 *
 * \param pEtcFile 配置文件的路径名称。
 * \param pSection 需要设置值的section名称。
 * \param pKey 需要设置值的key名称。
 * \param pValue 以null结尾的字符串。
 *
 * \return ETC_OK 成功， < 0 失败。
 *
 * \retval ETC_OK               设置值成功。
 * \retval ETC_FILEIOFAILED     文件I/O操作发生错误。
 * \retval ETC_TMPFILEFAILED    不能创建缓存文件。
 *
 * \note 如果指定的配置文件不存在，配置文件接口将尝试创建此文件。
 *
 * \sa GetValueFromEtcFile, GetIntValueFromEtcFile
 */
int  SetValueToEtcFile (const char* pEtcFile,
                        const char* pSection, const char* pKey, char* pValue);

/**
 * \fn int  RemoveSectionInEtcFile (const char* pEtcFile,\
 const char* pSection)
 * \brief 在配置文件中删除一个section。
 *
 * 此函数将在路径名称叫做 \a pEtcFile 的配置文件中删除一个叫做 \a pSection 的section。
 *
 * \param pEtcFile 配置文件的路径名称。
 * \param pSection 需要移除的section名称。
 *
 * \return ETC_OK 成功, < 0 失败.
 *
 * \retval ETC_OK               删除section成功。
 * \retval ETC_FILEIOFAILED     文件I/O操作发生错误。
 * \retval ETC_SECTIONNOTFOUND  在配置文件中没有发现此section。
 *
 * \sa RemoveSectionInEtc
 */
int  RemoveSectionInEtcFile (const char* pEtcFile,
                             const char* pSection);

/**
 * \fn ETC_HANDLE  LoadEtcFile (const char * pEtcFile)
 * \brief 将配置文件读入内存。
 *
 * 此函数将把指定的配置文件读入内存，之后可以使用 \a GetValueFromEtc
 * 函数从内存中读取内容。
 *
 * \param pEtcFile 配置文件的路径名称。
 *       如果 pEtcFile 是NULL，那么此函数将创建一个空的配置文件。
 *
 * \return 成功将返回配置文件的句柄，失败返回NULL。
 *
 * \sa UnloadEtcFile, GetValueFromEtc
 */
ETC_HANDLE  LoadEtcFile (const char * pEtcFile);

/**
 * \fn int  SaveEtcToFile (ETC_HANDLE hEtc, const char* file_name);
 * \brief 将内存中的配置信息存入文件。
 *
 * 此函数将内存中的配置文件实例存入叫做 \a file_name 的文件中。
 *
 * \param hEtc 配置文件在内存中的句柄。
 * \param file_name 存储配置文件的路径名称。
 *
 * \return ETC_OK 成功，< 0 失败。
 *
 * \retval ETC_OK               存储配置文件成功。
 * \retval ETC_INVALIDOBJ       非法的配置文件实例。
 * \retval ETC_FILEIOFAILED     文件I/O错误。
 *
 * \sa LoadEtcFile
 */
int  SaveEtcToFile (ETC_HANDLE hEtc, const char* file_name);

/**
 * \fn  UnloadEtcFile (ETC_HANDLE hEtc)
 * \brief 将一个配置文件从内存中卸载。
 *
 * 这个函数将把用 \a LoadEtcFile 创建的配置文件内存实例卸载。
 *
 * \param hEtc 配置文件在内存中实例的句柄。
 *
 * \return 0 成功, -1 出错.
 *
 * \sa LoadEtcFile, GetValueFromEtc
 */
int  UnloadEtcFile (ETC_HANDLE hEtc);

/**
 * \fn  GetValueFromEtc (ETC_HANDLE hEtc, const char* pSection,\
 const char* pKey, char* pValue, int iLen)
 * \brief 从配置文件内存实例中取值。
 *
 * 此函数从配置文件内存实例中取值，类似于 \a GetValueFromEtcFile。
 * 此函数从 \a hEtc 标识的配置文件内存实例中取值，其中 \a pSection 标识值的section，
 * \a pKey标识该值的key。取得值以后，将其存于 \a pValue 指向的buffer中。
 *
 * \param hEtc 配置文件内存实例句柄。
 * \param pSection 需要取值的section名称。
 * \param pKey 值的键名。
 * \param pValue 取出的值将存于此buffer中。
 * \param iLen buffer的长度，以BYTE为单位。如果iLen<0则认为是设值而非读取。
 *
 * \return ETC_OK 成功，< 0 失败。
 *
 * \retval ETC_OK               取值成功。
 * \retval ETC_INVALIDOBJ       非法的配置文件内存实例。
 * \retval ETC_SECTIONNOTFOUND  找不到指定的section。
 * \retval ETC_KEYNOTFOUND      找不到指定的key。
 * \retval ETC_READONLYOBJ      配置文件对象是只读的。
 *
 * \sa GetValueFromEtcFile, LoadEtcFile, UnloadEtcFile
 */
int  GetValueFromEtc (ETC_HANDLE hEtc, const char* pSection,
                      const char* pKey, char* pValue, int iLen);

/**
 * \fn int  GetIntValueFromEtc (ETC_HANDLE hEtc, const char* pSection,\
 const char* pKey, int* pValue)
 * \brief 从配置文件内存实例中取得整型值。
 *
 * \sa GetValueFromEtc, GetIntValueFromEtcFile
 */
int  GetIntValueFromEtc (ETC_HANDLE hEtc, const char* pSection,
                         const char* pKey, int* pValue);

/**
 * \def SetValueToEtc(ETC_HANDLE hEtc, const char* pSection,\
 const char* pKey, char* pValue)
 * \brief 向配置文件内存实例中设值。
 *
 * This fuctions sets the value in the etc object, somewhat similiar
 * to \sa SetValueToEtcFile.
 * 此函数将给配置文件内存实例设值，类似于 \sa SetValueToEtcFile 。
 *
 * \sa SetValueToEtcFile, GetValueFromEtc
 */
#define SetValueToEtc(hEtc, pSection, pKey, pValue) \
		GetValueFromEtc(hEtc, pSection, pKey, pValue, -1)

/**
 * \fn ETC_HANDLE  FindSectionInEtc (ETC_HANDLE hEtc,\
 const char* pSection, int bCreateNew)
 * \brief 在配置文件内存实例中寻找或者创建一个section。
 *
 * 此函数在 \a hEtc 标识的配置文件内存实例中寻找叫做 \a pSection 的section。
 * 如果在实例中没有找到此section且 \a bCreateNew 为真，则此函数将创建一个新的section。
 *
 * \param hEtc 配置文件内存实例的句柄。
 * \param pSection 查找的section名称。
 * \param bCreateNew 标识是否创建新的section。
 *
 * \return section的句柄，如果没有找到或者创建失败，则返回0。
 *
 * \sa GetValueFromEtcSec, GetIntValueFromEtcSec, SetValueInEtcSec
 */
ETC_HANDLE  FindSectionInEtc (ETC_HANDLE hEtc,
                           const char* pSection, int bCreateNew);

/**
 * \fn int  GetValueFromEtcSec (ETC_HANDLE hSect,\
 const char* pKey, char* pValue, int iLen)
 * \brief 从配置文件section对象实例中取值。
 *
 * 此函数从配置文件section对象中取值，类似于 \sa GetValueFromEtc
 * ，它将 \a pKey 中的键值取出，存于 \a pValue 中。
 *
 * \param hSect 配置文件section对象实例的句柄。
 * \param pKey 键名。
 * \param pValue 取得的值将存在此buffer中。
 * \param iLen 以BYTE为单位的buffer长度。若iLen<1在此函数认为这是设值而非读取。
 *
 * \return ETC_OK 成功, < 0 出错.
 *
 * \retval ETC_OK               成功取值。
 * \retval ETC_INVALIDOBJ       非法的配置文件对象。
 * \retval ETC_KEYNOTFOUND      不能找到对应的键。
 * \retval ETC_READONLYOBJ      此对象为只读状态。
 *
 * \sa GetValueFromEtcFile, GetValueFromEtc, FindSectionInEtc
 */
int  GetValueFromEtcSec (ETC_HANDLE hSect,
                         const char* pKey, char* pValue, int iLen);

/**
 * \fn int  GetIntValueFromEtcSec (ETC_HANDLE hSect,\
 const char* pKey, int* pValue)
 * \brief 从配置文件内存section实例中取得一个整型值。
 *
 * 此函数从配置文件内存section实例中取得一个整型值，类似于 \sa GetIntValueFromEtc 。
 *
 * \param hSect 配置文件内存section实例的句柄。
 * \param pKey 键名。
 * \param pValue 值将被存在此buffer中。
 *
 * \return ETC_OK 成功，< 0 失败。
 *
 * \retval ETC_OK               取值成功。
 * \retval ETC_INVALIDOBJ       非法对象。
 * \retval ETC_KEYNOTFOUND      在section中找不到指定的键。
 * \retval ETC_INTCONV          不能将字符串转换为整型值。
 *
 * \sa GetValueFromEtcFile, GetValueFromEtc, FindSectionInEtc
 */
int  GetIntValueFromEtcSec (ETC_HANDLE hSect,
                            const char* pKey, int* pValue);

/**
 * \fn int  SetValueToEtcSec (ETC_HANDLE hSect,\
 const char* pKey, char* pValue)
 * \brief 在配置文件内存section实例中设值。
 *
 * 此函数在 \a hSect 标识的配置文件内存section实例中设值。
 * 类似于 \sa SetValueToEtc.
 *
 * \sa GetValueFromEtc, FindSectionInEtc
 */
int  SetValueToEtcSec (ETC_HANDLE hSect,
                       const char* pKey, char* pValue);

/**
 * \fn int RemoveSectionInEtc (ETC_HANDLE hEtc, const char* pSection)
 * \brief 在配置文件内存实例中移除一个section。
 *
 * 此函数将在 \a hEtc 标识的配置文件内存实例中移除 \a pSection 标明的section。
 *
 * \param hEtc 配置文件内存实例的句柄。
 * \param pSection section的名称。
 *
 * \return ETC_OK 成功, < 0 失败.
 *
 * \retval ETC_OK               取值成功。
 * \retval ETC_INVALIDOBJ       非法对象。
 * \retval ETC_READONLYOBJ      只读对象。
 * \retval ETC_SECTIONNOTFOUND  未找到指定的section。
 *
 * \sa RemoveSectionInEtcFile
 */
int RemoveSectionInEtc (ETC_HANDLE hEtc, const char* pSection);

/** @} end of etc_fns */

#endif //OSINI_H_INCLUDED
