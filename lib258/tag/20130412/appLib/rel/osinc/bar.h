#ifndef BAR_H_INCLUDED
#define BAR_H_INCLUDED


/**
 * @defgroup 红外条码枪操作
 * @brief 提供红外条码枪操作的api
 * @author 刘一峰
 
 * @code
 *int main(void)
 *{
 *		unsigned char buf[256];
 *		int ret=0;
 *		Os__bar_open();
 *		while(1)
 *		{
 *			memset(buf,0,sizeof(buf));
 *			ret=Os__bar_read(buf,sizeof(buf));
 *			if(ret)
 *			{
 *				printf("bar read : %s\n",buf);
 *				break;
 *			}
 *			else
 *				printf("ret %d\n",ret);
 *			
 *			sleep(1);
 *		}
 *		Os__bar_close();
 *		return 0;
 *}
 * @{
 */


/**
*@def BAR_OK
*@brief 操作成功
**/
#define BAR_OK 0

/**
*@def BAR_COMERR
*@brief 串口错误
**/
#define BAR_COMERR -1

/**
*@def BAR_INUSE
*@brief 串口打开失
**/
#define BAR_INUSE -2

/**
*@def BAR_DRIVER
*@brief 驱动未加载或者驱动错误
**/
#define BAR_DRIVER -3

/**
*@def BAR_TIMEOUT
*@brief 读取超时
**/
#define BAR_TIMEOUT -4


/**
 * @fn void Os__bar_open(void)
 * @brief 打开条码设备，信息将会输出到串口
 *
 * @return BAR_OK 成功, < 0 出错.
 */
int Os__bar_open();

/**
 * @fn void Os__bar_read(unsigned char *buf,int len)
 * @brief 从串口读条码信息
 *
 * @return PARAM_OK 成功, < 0 出错.
 */
int Os__bar_read(unsigned char *buf, int len);

/**
 * @fn void Os__bar_close(void)
 * @brief 关闭串口和条码枪
 *
 * @return 
 */
void Os__bar_close();

#endif
