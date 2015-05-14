#ifndef APPLIST_H_INCLUDED
#define APPLIST_H_INCLUDED

/**
 * @defgroup 提供应用下载、应用列表信息等相关函数
 * @author 刘一峰
 * @author edit by 熊剑 20130108 
 *				
 * @brief 获取应用信息，挂载，解压缩pln文件等
 * @{
 */


typedef struct
{
	char id[6];						//应用号
	char name[25];					//应用名称	
	char editDate[20];				//修改日期
	char hidden[2];					//是否隐藏,0显示,1隐藏
	char version[20];				//版本号
	char itemCode[20];				//项目编号
	char owner[20];					//作者
	char checkCode[10];				//验证码
	char edit[2];					//3表示删除
}APPINFOLIST;


/**
 * @fn int OSSPD_smt_update(int _f)
 * @brief 	开始或者结束更新
 *
 * @param _f	1表示开始更新，其他为结束
 */
void OSSPD_smt_update(int _f);


/**
 * @fn int Os__mt_active(int _active,int _dev)
 * @brief 	挂载或卸载TF卡或USB设备,挂载成功,设备将挂载到"/mnt/mt"目录下
 *
 * @param _active	挂载或卸载标志,0表示卸载,1表示挂载
 * @param _dev  	要挂载的设备标示,2表示USB设备,否则表示TF卡设备
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int Os__mt_active(int _active,int _dev);

/**
 * @fn char *Os__pln_unzip(char *plnPath)
 * @brief 	将pln文件解压缩到"/tmp/pln"文件夹下,并返回"/tmp/pln/conf.ini"文件路径
 *
 * @param plnPath	pln文件路径
 *
 * @return 		成功返回"/tmp/pln/conf.ini"文件路径,失败返回NULL
 *
 */
char *Os__pln_unzip(char *plnPath);

/**
 * @fn int Os__msg_open(key_t key)
 * @brief	创建消息队列
 *
 * @param key	消息队列关联的键
 *
 * @return 	成功返回MSGID值,失败返回负值
 *
 */
int Os__msg_open(key_t key);

/**
 * @fn int Os__msg_send(int msqid, const void *msgp, size_t msgsz)
 * @brief	发送消息队列消息
 *
 * @param msqid	消息队列的识别码
 * @param msgp	指向消息缓冲区的指针
 * @param msgsz	消息的大小
 *
 * @return 	成功返回0,失败返回负值
 *
 */
int Os__msg_send(int msqid, const void *msgp, size_t msgsz);

/**
 * @fn int Os__msg_recv(int msqid, void *msgp, size_t msgsz, long msgtyp)
 * @brief	接收消息队列消息
 *
 * @param msqid	消息队列的识别码
 * @param msgp	指向消息缓冲区的指针
 * @param msgsz	消息的大小
 * @param msgsz	消息类型
 *
 * @return 	成功返回拷贝到消息的实际字节数,消息队列为空返回0,失败返回负值
 *
 */
int Os__msg_recv(int msqid, void *msgp, size_t msgsz, long msgtyp);

/**
 * @fn int Os__msg_close(int msqid)
 * @brief	接收消息队列消息
 *
 * @param msqid	消息队列的识别码
 *
 * @return 	成功返回0,失败返回负值
 *
 */
int Os__msg_close(int msqid);

/**
 * @fn int Os__applist_get(APPINFOLIST applist[], int appMaxNum)
 * @brief	获取应用列表详细信息
 *
 * @param applist	应用列表信息保存地址
 * @param appMaxNum	能够存储的最大应用个数	
 *
 * @return 	成功返回获取的应用数,失败返回-1
 *
 */
int Os__applist_get(APPINFOLIST applist[], int appMaxNum);

/**
 * @fn int Os__applist_set(APPINFOLIST applist[], int appNum)
 * @brief	设置应用详细信息到conf.ini文件中
 *
 * @param applist	应用列表信息保存地址
 * @param appNum	应用个数	
 *
 * @return 	成功返回0,失败返回-1
 *
 */
int Os__applist_set(APPINFOLIST applist[], int appNum);

/**
 * @fn int Os__applist_getAppCount(void)
 * @brief	获取应用数量
 *
 * @return 	返回应用的数量
 *
 */
int Os__applist_getAppCount(void);

/**
 * @fn unsigned short APPLIST_get_appid(void)
 * @brief	获取当前应用应用ID
 *
 * @return 	成功返回当前应用应用ID,失败返回0
 *
 */
unsigned short APPLIST_get_appid(void);


/**
 * @fn void APPLIST_rmove_appid(int pid)
 * @brief	移除无用的应用ID和相对应的PID
 *
 * @param	pid		所要删除的
 *
 * @return 	succeed return 0,else return -1
 *
 */
int APPLIST_rmove_appid(int pid);

/**
 * @fn int APPLIST_add_appid(unsigned short appid, int pid)
 * @brief	添加应用ID和对应的PID到列表中
 *
 * @param appid	应用ID
 * @param pid	应用所对应的进程ID
 * 
 * @return 	成功返回0,失败返回-1
 *
 */
int APPLIST_add_appid(unsigned short appid, int pid);

#endif  //APPLIST_H_INCLUDED
/** @} end of etc_fns */
