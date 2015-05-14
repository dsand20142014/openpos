#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "comm.h"
#include "sand_network.h"

void manager(){}

int main(int argc,char** argv)
{
	int i = 0;
	int gprsHandle;
	unsigned short len;
	unsigned char ucResult;
	unsigned char buff[500];

	Uart_Printf("before activeDev\r\n");		
	activeDev(DEV_GSM);
	Uart_Printf("after activeDev\r\n");		
	//for(i=0;i<20000;i++)
	for(i=1;i<2;i++)
	//while(1)
	{

		ucResult = OSGSM_GprsSetAPN("CMNET");
		printf("OSGSM_GprsSetAPN result = %d\r\n",ucResult);

		ucResult = OSGSM_GprsDial();
		printf("OSGSM_GprsDial result = %d\r\n",ucResult);
	
		ucResult = OSGSM_GprsCheckDial();
		printf("OSGSM_GprsCheckDial result = %d\r\n",ucResult);

		ucResult = TCP_Open(&gprsHandle);
		printf("TCP_Open result = %d\r\n",ucResult);

		//116.236.224.54
		ucResult = TCP_Connect(gprsHandle,0x74ece036,10003);
		printf("TCP_Connect result = %d\r\n",ucResult);
		if(ucResult != 0)
		{
			TCP_Close(gprsHandle);
			continue;
		}

		ucResult = TCP_Send(gprsHandle,"11111111112222222222333333333344\r\n",33);
		printf("TCP_Send result = %d\r\n",ucResult);
		
		ucResult = TCP_Recv(gprsHandle,buff,&len,30);
		printf("TCP_Recv result = %d\r\n",ucResult);
		printf("receive data:%s\r\n\r\n",buff);

		TCP_Close(gprsHandle);
		OSGSM_GprsHangupDial();
		//TCP_Close(gprsHandle);
	}

	return 0;
}
