#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

/**
*@def UART_OK
*@brief 串口操作成功
**/
#define UART_OK 0

/**
*@def UART_NOK
*@brief 串口操作失败
**/
#define UART_NOK -1

/**
*@def UART_SENDERROR
*@brief 串口发送失败
**/
#define UART_SENDERROR -2

/**
*@def UART_RECVERROR
*@brief 串口接收失败
**/
#define UART_RECVERROR -3

/**
*@def UART_SELECTERROR
*@brief 串口Select错误
**/
#define UART_SELECTERROR -4

/**
*@def UART_TIMEOUT
*@brief 串口接收超时
**/
#define UART_TIMEOUT -5

/**
 * @fn int UART_Init(char *dev, COM_PARAM *pa)
 * @brief 初始化串口设备
 *
 * @return >0 初始化成功返回文件描述符, UART_NOK 初始化失败.
 */
int UART_Init(char *dev, COM_PARAM *pa);

/**
 * @fn void UartClose(int fd)
 * @brief 关闭串口设备
 *
 * @return void.
 */
void UartClose(int fd);

/**
 * @fn int UartSend(int fd, unsigned char* inBuf, long inLen)
 * @brief 指定串口发送一个字符
 *
 * @return UART_OK 发送成功，UART_SENDERROR 发送失败.
 */
int UartSend(int fd, unsigned char* inBuf, long inLen);

/**
 * @fn int UartRecv(int fd,unsigned short timeout, unsigned char *recvBuf, size_t recvLen)
 * @brief 指定串口接收一个字符
 *
 * @return UART_OK 接收成功, UART_RECVERROR 接收失败，UART_TIMEOUT 接收超时，UART_SELECTERROR 接收Select错误.
 */
int UartRecv(int fd,unsigned short timeout, unsigned char *recvBuf, size_t recvLen);