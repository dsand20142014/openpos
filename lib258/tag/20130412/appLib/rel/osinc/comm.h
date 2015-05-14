#ifndef COMM_H_INCLUDED
#define COMM_H_INCLUDED


#define COMM_OK        0

//通讯类型不存在
#define COMM_MODE      -1

//设备不可重复使用，针对串口
#define COMM_INUSE      -2

//设备打开失败
#define COMM_ERR        -3

//需要把当前连接关闭才可切换通讯方式
#define COMM_REENTRANT  -4

//发送失败
#define COMM_SEND  -5

//连接失败
#define COMM_CONN  -6

/*
  1-UART
  2-TCP/IP
  3-GPRS
  4-WIFI
  5-CDMA
  6-MODEN
  ..
  其他返回错误
*/
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
   unsigned char ip[16];
   unsigned int port;
}P_TCPIP;


//打开设备，并连接
unsigned char Os__comm_open(int *iHandle,COMM_PARAM cp);

//发送数据
unsigned char Os__comm_send(int iHandle,unsigned char *pucInbuf,long lInLen);

//接收数据
unsigned char Os__comm_recv(int iHandle,unsigned char *pucInbuf,long lInLen,int iTimeout);

//关闭连接
unsigned char Os__comm_close(int iHandle);

//检查连接是否成功
unsigned char Os__comm_check();

#endif