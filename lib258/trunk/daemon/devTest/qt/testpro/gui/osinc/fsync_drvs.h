#ifndef __FSYNC_DRVS_H_
#define __FSYNC_DRVS_H_

/**
 * @defgroup 异步通知方式驱动接口函数
 * @brief 监控键盘、磁卡、IC卡、非接卡当中一个或多个驱动，目前只支持进程调用，暂不支持线程调用
 * @author 许小波
 * @author edit by 刘一峰 20120919
 *
 **/
#include "osdriver.h"
#ifdef __cplusplus
extern "C" {
#endif


#define KEY_DEVICE "/dev/input/keyboard0"
#define MAG_DEVICE "/dev/s3c2410_mag"
#define MFR_DEVICE "/dev/rc531"
#define ICC_DEVICE "/dev/TDA8007"
#include "osdriver.h"
/**
 * @def KEY_SIG
 * @brief 键盘信号值
 **/
#define  KEY_SIG  (SIGIO)

/**
 * @def MAG_SIG
 * @brief 磁卡信号值
 **/
#define  MAG_SIG  (SIGRTMIN+6)

/**
 * @def MFR_SIG
 * @brief 非接卡信号值
 **/
#define  MFR_SIG  (SIGRTMIN+7)

/**
 * @def ICC_SIG
 * @brief IC卡信号值
 **/
#define  ICC_SIG  (SIGRTMIN+8)

/**
 * @def APP_KEY_SIG
 * @brief 应用SET KEY信号值
 **/
#define  APP_KEY_SIG  (SIGRTMIN+9)

/**
 * @def SECURITY_SIG
 * @brief 安全模块信号值
 **/
#define  SECURITY_SIG   (SIGRTMIN+10)

/**
 * @def DRV_TIMEOUT
 * @brief 超时事件标识
 **/
#define DRV_TIMEOUT     (1<<0)

/**
 * @def KEY_DRV
 * @brief 按键事件标识
 **/
#define KEY_DRV         (1<<1)

/**
 * @def MAG_DRV
 * @brief 磁卡事件标识
 **/
#define MAG_DRV         (1<<2)

/**
 * @def MFR_DRV
 * @brief 非接卡事件标识
 **/
#define MFR_DRV         (1<<3)

/**
 * @def ICC_DRV
 * @brief IC卡事件标识
 **/
#define ICC_DRV         (1<<4)

/**
 * @def DRV_EVENT_SUCCESS
 * @brief Os_Wait_Event执行成功
 **/
#define DRV_EVENT_SUCCESS     (0)

/**
 * @def KEY_DRV_ERROR
 * @brief 按键驱动错误
 **/
#define KEY_DRV_ERROR         (1<<1)

/**
 * @def MAG_DRV_ERROR
 * @brief 磁卡驱动错误
 **/
#define MAG_DRV_ERROR         (1<<2)

/**
 * @def MFR_DRV_ERROR
 * @brief 非接卡驱动错误
 **/
#define MFR_DRV_ERROR         (1<<3)

/**
 * @def ICC_DRV_ERROR
 * @brief IC卡驱动错误
 **/
#define ICC_DRV_ERROR         (1<<4)

/**
 * @def DRV_ID_ERROR
 * @brief 事件标识传入错误
 **/
#define DRV_ID_ERROR          (1<<5)

/**
 * @def MULT_CALL_ERROR
 * @brief 重复调用错误
 **/
#define MULT_CALL_ERROR       (1<<6)

/**
 * @def INT_ERROR
 * @brief 函数被中断
 **/
#define INT_ERROR             (1<<7)

/**
 * @def NEW_DRV_TYPE
 * @brief 异步通知方式驱动数据结构
 **/
typedef struct
{
    unsigned int drv_type;
    DRV_OUT drv_data;
} NEW_DRV_TYPE;


void Os_Event_Release(void);

/**
 * @fn unsigned int Os_Wait_Event(unsigned int drv_id, NEW_DRV_TYPE *new_drv, unsigned int timeout)
 * @brief 等待驱动事件
 * @param drv_id    要监控的驱动事件标识，标识类型见上面定义
 * @param new_drv   传出驱动数据地址，驱动数据类型结构见上面定义
 * @param timeout   超时时间，单位为ms
 *
 * @return DRV_EVENT_SUCCESS,0 成功: 大于0,失败,错误代码标识见上面定义
 */
extern unsigned int Os_Wait_Event(unsigned int drv_id, NEW_DRV_TYPE *new_drv, unsigned int timeout);

/*
 * 为了解决linuxpos和qt一起收到按键的问题，把linuxpos里面收按键的地方屏蔽底层的按键，只能通过qt使用os_set_key来获得按键
 *
*/
unsigned int Os_Wait_Event_NoKey(unsigned int drv_id, NEW_DRV_TYPE *new_drv, unsigned int timeout);

/**
 *
 * @code
 *
 * 	¡­¡­¡­¡­
 * 	unsigned int ret;
 * 	NEW_DRV_TYPE  new_drv;
 * 	¡­¡­¡­¡­
 *
 * 	¡­¡­¡­¡­
 * 	ret = Os_Wait_Event(KEY_DRV | MAG_DRV | MFR_DRV | ICC_DRV , &new_drv, 5000);
 *
 * 	if(ret == DRV_EVENT_SUCCESS)
 * 	{
 *           if(new_drv.drv_type == KEY_DRV)
 *           {
 *               按键数据处理;
 *           }
 *           else if(new_drv.drv_type == MAG_DRV)
 *           {
 *               磁卡数据处理;
 *           }
 *           else if(new_drv.drv_type == MFR_DRV)
 *           {
 *               非接卡轮询结果处理;
 *           }
 *           else if(new_drv.drv_type == ICC_DRV)
 *           {
 *               IC卡插卡结果处理;
 *           }
 *           else if(new_drv.drv_type == DRV_TIMEOUT)
 *           {
 *               超时处理;
 *           }
 * 	}
 * 	else
 * 	{
 *           if(ret == KEY_DRV_ERROR)
 *           {
 *               按键驱动打开失败处理;
 *           }
 *           else if(ret == MAG_DRV_ERROR)
 *           {
 *               磁卡驱动打开失败处理;
 *           }
 *           else if(ret == MFR_DRV_ERROR)
 *           {
 *               非接卡驱动打开失败处理;
 *           }
 *           else if(ret == ICC_DRV_ERROR)
 *           {
 *               IC卡驱动打开失败处理;
 *           }
 *           else
 *           {
 *               其他错误处理;
 *           }
 *
 * 	}
 *
 *
 * @endcode
 *
 * @{
 */





#ifdef __cplusplus
}
#endif

#endif

