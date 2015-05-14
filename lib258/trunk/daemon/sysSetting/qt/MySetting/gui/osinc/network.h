#ifndef NETWORK_H_INCLUDE
#define NETWORK_H_INCLUDE

/**
 * @defgroup 网络函数库
 * @author 熊剑
 * @brief 提供对网络进行操作的相关函数
 * @{
 */



/**
 * @fn int Os__checkRouter(void)
 * @brief return effective router in current system.
 * @param none
 *
 * @return -1 error, 0 none router, 1 gsm router, 2 wifi router
 * @code
 *	int main(void)
 *	{
 *		int ret = -1;
 * 		ret = Os_checkRouter();
 *		if(ret == 0)
 *			printf("there is no router\r\n");
 *		else if(ret == -1)
 *			printf("there some error when check router\r\n");
 *		else if(ret == 1)
 *			printf("current effective router is gsm\r\n");
 *		else if(ret == 2)
 *			printf("current effective router is wifi\r\n");
		return 0;
 *	}
 * @endcode
 */
int Os_checkRouter(void);
#endif // NETWORK_H_INCLUDE
/** @} end of etc_fns */
