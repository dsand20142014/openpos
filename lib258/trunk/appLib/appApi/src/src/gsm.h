#ifndef GSM_H_INCLUDED
#define GSM_H_INCLUDED

/**
 * @defgroup 提供GSM设置等相关函数
 * @author 刘一峰
 *				
 * @brief 设置GSM频道和APN等
 * @{
 */

/**
 * @fn int GSM__dial_check(unsigned short uiTimeout)
 * @brief 	检查GSM拨号是否成功
 *
 * @param uiTimeout	超时退出次数
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int GSM__dial_check(unsigned short uiTimeout);

/**
 * @fn void GSM__setParam_gprs(int _bands)
 * @brief 	设置GSM的波段
 *
 * @param _bands	GSM波段
 *
 * @return 		无返回值
 *
 */
void GSM__setParam_gprs(int _bands);

/**
 * @fn int GSM_set_APN(unsigned char* ucAPN)
 * @brief 	设置GSM的APN
 *
 * @param ucAPN	GSM的APN
 *
 * @warning		此函数只改变GSM的APN,不会重置服务和修改APN配置,只做内部使用
 * @return 		成功返回0,失败返回-1
 *
 */
int GSM_set_APN(unsigned char* ucAPN);

void GSM_config_pap_chap(char *username, char *password);

/**
 * @fn int Os__change_APN(unsigned char* ucAPN)
 * @brief 	变更GSM的APN
 *
 * @param ucAPN	GSM的APN
 *
 * @warning		此函数会改变GSM的APN,并对配置文件进行修改后重启GSM拨号服务
 * @return 		成功返回0,失败返回-1
 *
 */
int Os__change_APN(unsigned char* ucAPN);

/**
 * @fn int GSM_AT_cmd(char *cmd,unsigned char *rbuf,int rlen,int timeout)
 * @brief 	给GSM模块发送AT命令
 *
 * @param cmd		AT命令
 * @param rbuf		发送命令后收到的信息存储地址
 * @param rlen		收信息buf的长度
 * @param timeout	超时时间,单位秒
 *
 * @warning		此函数会改变GSM的APN,并对配置文件进行修改后重启GSM拨号服务
 * @return 		成功返回0,失败返回-1
 *
 */
int GSM_AT_cmd(char *cmd,unsigned char *rbuf,int rlen,int timeout);

#endif //GSM_H_INCLUDED
/** @} end of etc_fns */
