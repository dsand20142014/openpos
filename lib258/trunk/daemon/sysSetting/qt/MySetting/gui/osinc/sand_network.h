#ifndef SAND_NETWORK_H
#define SAND_NETWORK_H

/**
 * @defgroup 提供网络通信方面的收发操作
 * @author 
 *				
 * @brief 网络通信操作函数
 * @{
 */

#define DEV_COMM_GSM 0
#define DEV_COMM_WIFI 1
#define DEV_COMM_ETHERNET 2
#define DEV_COMM_MODEN 3

unsigned char TCP_CheckOSVersion(void);

/**
 * @fn unsigned char TCP_Open(int *piTCPHandle)
 * @brief 	打开当前POS空闲会话,并返回会话句柄
 *
 * @param iHandle		会话句柄的指针
 *
 * @return 		成功返回0,返回其他则失败
 *
 */
unsigned char TCP_Open(int *piTCPHandle);
//unsigned char TCP_Connect(int iTCPHandle,unsigned char *ulPeerAddr,unsigned short uiPeerPort);


/**
 * @fn unsigned char TCP_Connect(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort)
 * @brief 	用打开的会话连接主机
 *
 * @param iTCPHandle		会话句柄
 * @param ulPeerAddr	主机IP地址
 * @param uiPeerPort	主机端口号
 *
 * @warning IP地址用点分十进制表示法对应的四个十进制数对应的十六进制数拼成一个ULONG数
 * @return 		成功返回0,返回其他则失败
 *
 */
unsigned char TCP_Connect(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);

/**
 * @fn unsigned char TCP_Send(int iTCPHandle,unsigned char *pucInBuf,unsigned short uiInLen)
 * @brief 	用打开的会话连接主机
 *
 * @param iTCPHandle	会话句柄
 * @param pucInBuf		数据BUFF的指针
 * @param uiInLen		数据BUFF的长度
 * 
 * @warning 			iTCPHandle句柄必先打开后连接，才能发送接收数据
 * @return 		成功返回0,返回其他则失败
 *
 */
unsigned char TCP_Send(int iTCPHandle,unsigned char *pucInBuf,unsigned short uiInLen);

/**
 * @fn unsigned char TCP_Recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout)
 * @brief 	用打开的会话连接主机
 *
 * @param iTCPHandle	会话句柄
 * @param pucInBuf		接收数据BUFF的指针
 * @param puiInLen		接收数据BUFF的实际长度
 * @param uiTimeout		接收超时时间（秒）
 * 
 * @warning 			iTCPHandle句柄必先打开后连接,才能发送接收数据,在发送数据和接收数据之间尽量不使用费时语句(如UART＿PRINTF函数),否则会造成有时数据接收不到
 * @return 		成功返回0,返回其他则失败
 *
 */
unsigned char TCP_Recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);

/**
 * @fn unsigned char TCP_Close(int iTCPHandle)
 * @brief 	关闭会话
 *
 * @param iTCPHandle	会话句柄
 * 
 * @return 		返回0
 *
 */
unsigned char TCP_Close(int iTCPHandle);

//if succeed return 0,else return -1;
//only support gsm and wifi right now.
/**
 * @fn int activeDev(int idev)
 * @brief 	设置网络通信路由表,目前只支持GSM和WIFI
 *
 * @param idev	网络通信中,想要使用的网络模块,包括GSM,WIFI,ETHERNET和MODEN
 * 
 * @return 		返回0
 *
 */
int activeDev(int idev);

/* udp*/
unsigned char UDP_Open(int *piUDPHandle);
unsigned char UDP_Config(int iUDPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);
unsigned char UDP_Send(int iUDPHandle,unsigned char *pucInBuf,unsigned short uiInLen);
unsigned char UDP_Recv(int iUDPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
unsigned char UDP_Close(int iUDPHandle);


#endif

