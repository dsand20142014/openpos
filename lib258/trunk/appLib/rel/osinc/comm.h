#ifndef COMM_H_INCLUDED
#define COMM_H_INCLUDED


#define COMM_OK		0

//通讯类型不存在
#define COMM_MODE      -1

//设备不可重复使用，针对串口
#define COMM_INUSE	-2

//设备打开失败
#define COMM_ERR        -3

//需要把当前连接关闭才可切换通讯方式
#define COMM_REENTRANT  -4

//发送失败
#define COMM_SEND 	 -5

//receive error
#define COMM_RECV 	 -6

//连接失败
#define COMM_CONN 	 -7

//UART
#define MODE_UART	1

//TCP/IP
#define MODE_TCPIP	2

//GSM
#define MODE_GSM	3

//WIFI
#define MODE_WIFI	4

//CDMA
#define MODE_CDMA	5

//MODEM
#define MODE_MODEM	6

typedef struct {
  int commMode;
  void *commParam;
}COMM_PARAM;
 

/*
  串口参数
*/
typedef struct {
    unsigned int port;       
    unsigned int  speed;            
    unsigned int dataBits;      
    unsigned int parity;
    unsigned int stopBits;
}P_UART;

/*
  网口参数
*/
typedef struct {
   unsigned char linkdescribe[100];   //ip or url
   unsigned short port;
}P_TCPIP;


/**
 * @fn int Os_comm_init(COMM_PARAM *cp)
 * @brief  initialize global parameters    
 * @author lichao
 * @date   2013.4.16
 * @param  cp struct pointer,point to a struct that contains serail information
 *
 * @return succeed return COMM_OK,failured return COMM_ERR.
 */
int Os_comm_init(COMM_PARAM *cp);


/**
 * @fn int Os_comm_connect()
 * @brief  open device, if succeed connect the handle and return it,else return error no
 * @author lichao
 * @date   2013.4.16
 * @param  none
 *
 * @return succeed return connect handle,failured return error no.
 */
int Os_comm_connect();


/**
 * @fn int Os_comm_send(int iHandle, unsigned char *pucInbuf, long lInLen)
 * @brief  send data through existed and connected handle
 * @author lichao
 * @date   2013.4.16
 * @param  iHandle	existed and connected handle.
 * @param  pucInbuf	send data buffer.
 * @param  lInLen	send data length.
 *
 * @return succeed return the bytes has been send,else return error no.
 */
int Os__comm_send(int iHandle,unsigned char *pucInbuf,long lInLen);


/**
 * @fn int Os_comm_recv(int iHandle, unsigned char *pucInbuf, long lInLen, int iTimeout)
 * @brief  receive data through existed and connected handle
 * @author lichao
 * @date   2013.4.16
 * @param  iHandle	existed and connected handle.
 * @param  pucInbuf	receive data buffer.
 * @param  lInLen	return real receive data length.
 * @param  iTimeout	timeout time
 *
 * @return succeed return COMM_OK,else return error no.
 */
int Os__comm_recv(int iHandle,unsigned char *pucInbuf,unsigned short *pucInLen,int iTimeout);


/**
 * @fn int Os_comm_close(int iHandle)
 * @brief  close existed handle,release resource.
 * @author lichao
 * @date   2013.4.16
 * @param  iHandle       existed handle.
 *
 * @return succeed return 0,else return -1.
 */
int Os__comm_close(int iHandle);


//检查连接是否成功
int Os__comm_check();

#if 0
int main(int argc,char** argv)
{
        int ret=0;
        int ihandle_wifi = -1;
        int ihandle_gsm = -1;
        int ihandle_uart= -1;
        unsigned short len = 0;
        unsigned char ucSendBuf[100];

        COMM_PARAM param_wifi;
        COMM_PARAM param_gsm;
        COMM_PARAM param_uart;

        memset(ucSendBuf,0,sizeof(ucSendBuf));
        memcpy(ucSendBuf,"12345678abcd\r\n",22);

        P_UART p_uart =
        {
          1,            // com 1
          115200,       // Speed 
          8,            // Data bit
          'N',
          0             // Stop bit      
        };
        memset(&p_uart,0,sizeof(p_uart));
	param_uart.commMode=MODE_UART;   //设置使用串口通讯
        param_uart.commParam=&p_uart;

        P_TCPIP p_wifi=
        {
                "172.16.3.247",
                8888
        };
        P_TCPIP p_gsm=
        {
                "http://www.baidu.com",
                80
        };

        param_wifi.commMode = MODE_WIFI;        //设置使用WIFI
        param_wifi.commParam = &p_wifi;

        param_gsm.commMode = MODE_GSM;  //设置使用GSM
        param_gsm.commParam = &p_gsm;

	//uart test
        if(*argv[1] == '1')
        {
                ret = Os__comm_init(&param_uart);
                printf("init,uart ret = %d\n",ret);
                ihandle_uart = Os_comm_connect();
                printf("connect,uart ihandle = %d\r\n",ihandle_uart);
                if(ihandle_uart> 0)
                {
                        ret = Os__comm_send(ihandle_uart,ucSendBuf,12);
                        printf("uart send,result = %d\n",ret);

                        memset(ucSendBuf,0,sizeof(ucSendBuf));
                        len = 5;
                        ret = Os__comm_recv(ihandle_uart,ucSendBuf,&len,30);
                        printf("uart recv,result = %d\n",ret);
                        printf("uart recieve data: %s\n",ucSendBuf);

                        ret = Os__comm_close(ihandle_uart);
                        printf("uart close,result = %d\n",ret);
                }
        }
	//gsm test
        else if(*argv[1] == '2')
        {
                activeDev(DEV_COMM_GSM);
                ret = Os__comm_init(&param_gsm);
                printf("init,gsm ret = %d\n",ret);
                ihandle_gsm = Os_comm_connect();
                printf("connect,gsm ihandle = %d\r\n",ihandle_gsm);
                memcpy(ucSendBuf,"12345678abcd\r\n",22);
                if(ihandle_gsm > 0)
                {
                        ret=Os__comm_send(ihandle_gsm,ucSendBuf,12);
                        printf("gsm send,result = %d\n",ret);

                        //memset(ucSendBuf,0,sizeof(ucSendBuf));
                        //ret=Os__comm_recv(ihandle_gsm,ucSendBuf,12,30);
                        //printf("gsm recv,result = %d\n",ret);
                        //printf("gsm recieve data: %s\n",ucSendBuf);
                        ret = Os__comm_close(ihandle_gsm);
                        printf("gsm close,result = %d\n",ret);
                }
        }
	//wifi test
        else if(*argv[1] == '3')
        {
                activeDev(DEV_COMM_WIFI);
                ret = Os__comm_init(&param_wifi);
                printf("init,wifi ret = %d\n",ret);
                ihandle_wifi= Os_comm_connect();
                printf("connect,wifi ihandle = %d\r\n",ihandle_wifi);
                if(ihandle_wifi> 0)
                {
                        ret=Os__comm_send(ihandle_wifi,ucSendBuf,12);
                        printf("wifi send,result = %d\n",ret);

                        memset(ucSendBuf,0,sizeof(ucSendBuf));
                        ret=Os__comm_recv(ihandle_wifi,ucSendBuf,&len,30);
                        printf("wifi recv,result = %d\n",ret);
                        printf("wifi recieve data: %s\n",ucSendBuf);

                        ret = Os__comm_close(ihandle_wifi);
                        printf("wifi close,result = %d\n",ret);
                }
        }
	return 1;
}
#endif

#endif

