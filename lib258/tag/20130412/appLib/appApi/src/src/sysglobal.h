#ifndef SYSGLOBAL_H_INCLUDED
#define SYSGLOBAL_H_INCLUDED


#define MODULE_DEVICE "/dev/beep_backlight"


/**
 * @defgroup 系统初始化使用函数
 * @brief 在系统初始化时使用的函数
 * @author 刘一峰
 * @{
 **/

/**
 * @def SPARAM
 * @brief 机器序列号和密码键盘序列号的结构体
 **/
typedef struct{
	unsigned char aucSerialNo[16];
	unsigned char aucPADSerialNo[16];
}SPARAM;


int  set_app_id(unsigned short appid);

/**
 * @fn SPARAM Os_readParam(void)
 * @brief 获取系统设置中的machineNo和pinpadNo
 * @Param 无参数
 * @return 返回SPARAM类型结构体
 */
SPARAM Os_readParam(void);

/**
 * @fn SYSGLOBAL_sys_poweron(void)
 * @brief 开机初始化参数的函数，这个函数不提供应用调用
 * @Param 无参数
 * @return 返回VOID
 */
void SYSGLOBAL_sys_poweron(void);


/**
 * @fn SYSGLOBAL_sys_suspend(void)
 * @brief 进休眠初始化参数的函数，这个函数不提供应用调用
 * @Param 无参数
 * @return 返回VOID
 */
void SYSGLOBAL_sys_suspend(void);


#endif //SYSGLOBAL_H_INCLUDED
/**
 * @}
 **/
