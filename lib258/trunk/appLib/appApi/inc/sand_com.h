/* skb25_communication.h */
#ifndef __SAND_COM__
#define COM_EXTERN extern
#else
#define COM_EXTERN
#endif

#include "osmodem.h"

/**
 * @defgroup 提供串口通信等相关函数
 * @author
 *				
 * @brief 打开串口,发送接收串口数据以及关闭串口等操作
 * @{
 */

extern int _g_com2_openned;
extern unsigned char Os__init_com(COM_PARAM *ComParameter);
extern unsigned char Os__txcar(unsigned char Charac);
extern unsigned short Os__rxcar(unsigned short TimeOut);

/**
 * @fn unsigned char OSUART_Init1(COM_PARAM *pa)
 * @brief 	初始化串口一,此串口为调试输出串口
 *
 * @param pa	串口设置信息,此串口包含的成员名如下
 * 				    unsigned char stop_bits;        // nbr of bit stop 1 or 2 
 * 				    unsigned int  speed;            // speed from 75 to 19200bds 
 * 				    unsigned char com_nr;           // 0 : modem, 1 : rs232 
 * 				    unsigned char data_bits;        // nbr of data bits 7 or 8 
 * 				    unsigned char parity;           // parity E ou O ou N 
 *
 * @return 		成功返回0,失败返回-1
 * @code
 * 	int main(int argv, char *argc[])
 * 	{
 * 		unsigned int rel; 
 * 		COM_PARAM param;
 * 		memset(&param,0,sizeof(param));
 * 		param.stop_bits = 1;
 * 		param.speed = 115200;
 * 		param.com_nr = 1;
 * 		param.data_bits = 8;
 * 		param.parity = 'N';
 * 		if(OSUART_Init1(&param) == 0)
 * 		{
 * 			printf("open uart OK!\n");
 * 			
 * 			if(argv == 1)
 * 			{
 * 				printf("recv data!\n");
 * 				while(1)
 * 				{
 * 					rel = OSUART_RxChar1(500);
 * 					if(rel/256 == 0)
 * 					{
 * 						//成功接收数据后返回字符9
 * 						OSUART_TxChar1(0x39);
 * 						printf("read date OK is %c\n",rel);
 * 					}
 * 					else
 * 						printf("read date error!\n");
 * 					sleep(2);	
 * 				}
 * 			}
 * 			else
 * 			{
 * 				printf("send data!\n");
 * 				while(1)
 * 				{
 * 					OSUART_TxChar1(0x31);
 * 					printf("send date ok!\n");	
 * 				}
 * 			}
 * 		}
 * 		else
 * 			printf("open uart error!\n");
 * 		OSUART_Close1();
 * 		return 0;
 * 	}
 * @endcode
 */
unsigned char OSUART_Init1(COM_PARAM *pa);

/**
 * @fn unsigned short OSUART_RxChar1(unsigned short uiTimeout)
 * @brief 	接收来自串口1的数据
 *
 * @param uiTimeout		超时时间,单位1ms
 *
 * @warning	返回值高位为状态位,地位为数据位,当成功时,状态位为0,低位为数据,失败时低位为0,高位为错误码
 * @return 		成功返回读到的字符,失败返回的状态值在高位,为非0值
 *
 */
unsigned short OSUART_RxChar1(unsigned short uiTimeout);

/**
 * @fn void OSUART_TxChar1(unsigned char ucCh)
 * @brief 	从串口1发送数据
 *
 * @param ucCh		要发送的数据
 *
 * @return 		无返回值
 *
 */
void OSUART_TxChar1(unsigned char ucCh);

/**
 * @fn void OSUART_Close1(void)
 * @brief 	关闭串口1
 *
 * @return 		无返回值
 *
 */
void OSUART_Close1(void);

/** @} end of etc_fns */
