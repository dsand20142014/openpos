/*
	SAND China
--------------------------------------------------------------------------
	FILE  LoadKey.c								(Copyright SAND 2009)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2009-03-02		Lote
    Last modified :	2009-03-02		Lote
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file for LoadKey.

    List of routines in file :

    File history :
*/

#include <include.h>
#include <global.h>
#include <loadkey.h>

unsigned char ucComm;
unsigned char car_status;

unsigned char LOADKEY_DisERRMsg(unsigned char ucErrorMsg)
{
	unsigned char ucResult;
	unsigned short usIndex;
	unsigned char 	ucType;
	unsigned char aucBuf[MSG_MAXCHAR+1];	
	unsigned char aucDispBuf[MSG_MAXCHAR+1];
	
	util_Printf("串口失败错误码%02x\n",ucErrorMsg);
	Os__GB2312_xprint(ucErrorMsg,0x01);
	for(usIndex=0;COMMErr[usIndex].ucErrorNo<0xFF;usIndex++)
		{
			util_Printf("%02x ",COMMErr[usIndex].ucErrorNo);
		}
	util_Printf("\n");
	for(usIndex=0;COMMErr[usIndex].ucErrorNo<0xFF;usIndex++)
		{
			if (ucErrorMsg == COMMErr[usIndex].ucErrorNo)			
				{
					util_Printf("111111111111111\n");
					ucType = COMMErr[usIndex].ucErrorType;
					memcpy(aucBuf,COMMErr[usIndex].aucErrorName,MSG_MAXCHAR);
					break;
				}
				if(COMMErr[usIndex].ucErrorNo == 0xFF)				
				{
					util_Printf("2222222222\n");
					ucType = COMMErr[usIndex].ucErrorType;
					memcpy(aucBuf,COMMErr[usIndex].aucErrorName,MSG_MAXCHAR);
					break;
				}
		}
	//Os__clr_display(255);
    switch( ucType)
    {
	    case '0':
			//memcpy( aucDispBuf, &ucErrorMsg, 2);
			//memcpy(&aucDispBuf[2]," " ,1);
			memcpy(&aucDispBuf[3],aucBuf , strlen((char*)aucBuf));
			//Os__GB2312_display(1, 0, (uchar *)aucDispBuf);
	    	break;
	    default:
	    	Os__display(0,0,aucBuf);
	    	break;
    	}
	MSG_WaitKey(3);
	return (SUCCESS);
}

unsigned char LOADKEY_PORT_SendData(unsigned char *pucInData,unsigned short uiInLen)
{
	uint uiI;
	uchar aucSndBuf[512];

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"发送中...");

	memset(aucSndBuf,0,sizeof(aucSndBuf));
	//长度超限
	if(uiInLen <= sizeof(aucSndBuf))
		memcpy(aucSndBuf,pucInData,uiInLen);
	else
		return ERR_COMMS_SANDLEN;

	util_Printf("\n=COMMS_PORT_SendData Begin:=\n");
	for(uiI=0; uiI<uiInLen;uiI++)
	{
		util_Printf("%02x ",aucSndBuf[uiI]);
		if(ucComm == COM_ONE)
			Os__txcar(aucSndBuf[uiI]);
		else if (ucComm == COM_TWO)
			Os__txcar3(aucSndBuf[uiI]);
	}
	return(SUCCESS);
}

unsigned char LOADKEY_Single(unsigned char ucKeyIndex, unsigned char * pIndata)
{
	unsigned char aucAscData[33];
	unsigned char aucAscData1[17];
	unsigned char aucAscData2[17];
	unsigned char aucHexData[9];
	unsigned char aucHexData1[9];
	unsigned char ucResult,ucI;
	unsigned char ucArrayIndex;

#if 0
	//厦门银联要求单倍长密钥
	if(ucKeyIndex < 5)
	{
		ucArrayIndex = KEYARRAY_GOLDENCARDSH;
	}
	else
	{
		ucArrayIndex = KEYARRAY_GOLDENCARDSH+1;
		ucKeyIndex -= 5;
	}
#endif

	ucArrayIndex = KEYARRAY_GOLDENCARDSH;
	memset(aucAscData,0,sizeof(aucAscData));
	memset(aucAscData1,0,sizeof(aucAscData1));
	memset(aucAscData2,0,sizeof(aucAscData2));
	memset(aucHexData,0,sizeof(aucHexData));
	memset(aucHexData1,0,sizeof(aucHexData1));
    util_Printf("aucAscData=%s\n",aucAscData);
	memcpy(aucAscData1 , pIndata , 8);

	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,ucKeyIndex,aucAscData1,aucHexData);

	if(ucResult != SUCCESS)
	{
		return(ucResult);
	}

	ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
					ucArrayIndex,ucKeyIndex,ucKeyIndex,aucHexData);
	if(ucResult != SUCCESS)
	{
		return(ucResult);
	}
	return(ucResult);
}

unsigned char LOADKEY_PORT_RevData(unsigned char *pucOutData,unsigned short *puiOutLen)
{
	uint    uiTimesOut,length,uiI,uiOutLen;
	uchar 	LRC,Info_rec,aucLenBuf[10];
    uchar   *pucData;
    uchar   ucFlag;
    uchar  unRevChar;

	util_Printf("\n=COMMS_PORT_RevData Begin=\n");

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"接收中...");

    uiOutLen=0;
	pucData = pucOutData;

	// 1.接收起始字节:CHAR_STX

	util_Printf("=1.COMMS_PORT_RevData CHAR_STX:0X02=\n");

	car_status = 0;
	uiTimesOut = 20*100;
	Os__timer_start(&uiTimesOut);
	do
	{
		if(ucComm == COM_ONE)
	 		car_status = Os__rxcar(uiTimesOut);
		else if (ucComm == COM_TWO)
			car_status = Os__rxcar3(uiTimesOut);

	}while((uiTimesOut)
		 &&(car_status%256 != 0x02)
		  );
	Os__timer_stop(&uiTimesOut);
	if(uiTimesOut==0)
	{
		util_Printf("=COMMS_PORT_RevData ERR:STX=RECVTIMEOUT=\n");
		return(ERR_RECVTIMEOUT);
	}
	if( (car_status%256) != 0x02)
	{
		util_Printf("=COMMS_PORT_RevData ERR:uiTimesOut=[%d]\n",uiTimesOut);
		util_Printf("=COMMS_PORT_RevData ERR:uiTimesOut=[%02x]\n",car_status%256);
		util_Printf("=COMMS_PORT_RevData ERR:STX!=0X02=[%02x]\n",car_status/256);
		return(ERR_COMMS_STX);
	}
	*pucData = car_status%256;
	pucData++;
	uiOutLen++;

	//接收数据包长度:CHAR_LEN

	util_Printf("=2.COMMS_PORT_RevData LENLEN=\n");

	uiTimesOut = 10*100;
	memset(aucLenBuf,0,10);
	for (uiI=0;  uiI<2;  uiI++)
	{
		if(ucComm == COM_ONE)
			car_status = Os__rxcar(uiTimesOut);
		else if (ucComm == COM_TWO)
			car_status = Os__rxcar3(uiTimesOut);

		if (car_status/256 == COM_ERR_TO)
		{
			util_Printf("=COMMS_PORT_RevData ERR:LEN=RECVTIMEOUT=\n");
			return(ERR_COMMS_DATA);
		}
		aucLenBuf[uiI] = car_status%256;
		*pucData = car_status%256;
		pucData++;
		uiOutLen++;
		util_Printf("%02x ",aucLenBuf[uiI]);
	}
	util_Printf("\n");

	length = tab_long(aucLenBuf,2);
	if(length > 1024)
	{
		util_Printf("=COMMS_PORT_RevData ERR:LEN=OVER=\n");
		return ERR_COMMS_DATALEN;
	}

	//根据数据长度, 接收数据:CHAR_DATA  +0x03+LRC
	util_Printf("=3.COMMS_PORT_RevData DATA len=[%d]\n",length);

	uiTimesOut = 10*100;
	for(uiI= 0; uiI<length+2; uiI++)
	{

		if(ucComm == COM_ONE)
			Info_rec = Os__rxcar(uiTimesOut);
		else if (ucComm == COM_TWO)
			Info_rec = Os__rxcar3(uiTimesOut);

		ucFlag = 0;
		switch( Info_rec / 256 )
		{
			case COM_OK :
				*pucData = Info_rec%256;
				pucData++;
				uiOutLen++;
				break ;
			case COM_ERR_TO :
				ucFlag = 1;
				break;
			default :
				util_Printf("=COMMS_PORT_RevData ERR:DATA=RECVTIMEOUT=\n");
				return(ERR_COMMS_DATA);
		}
		if (ucFlag == 1)
		{
			break;
		}
	}
	if(uiI != (length+2))
	{
		return ERR_REVDATA_LEN;
	}
	*puiOutLen = uiOutLen;

	util_Printf("=COMMS_PORT_RevData DATA_Begin=\n");
	for(uiI =0; uiI < uiOutLen; uiI++)
		util_Printf("%02x ", pucOutData[uiI]);
	util_Printf("=COMMS_PORT_RevData DATA_END=\n");

	//校验结束字符:CHAR_ETX
	if(pucOutData[uiOutLen-2] !=  CHAR_ETX)
	{
		util_Printf("=COMMS_PORT_RevData ERR:ETX!=0X03=\n");
		return(ERR_COMMS_DATAETX);
	}
	util_Printf("=COMMS_PORT_RevData CHAR_ETX=SUCCESS=\n");
	//校验LRC
	LRC = 0;
	for(uiI=1;  uiI<uiOutLen-2; uiI++ )
	{
		LRC ^= pucOutData[uiI];
	}
	if(LRC != pucOutData[uiOutLen-1])
	{
		util_Printf("=COMMS_PORT_RevData ERR:ERR_LRC=[%02x]\n",LRC);
		//return(ERR_DATA_LRC);
	}
	util_Printf("=COMMS_PORT_RevData LRC=SUCCESS=\n");
	return(SUCCESS);
}
unsigned char LOADKEY_SelectCOM(void)
{
	unsigned char KEY;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"请选择通讯串口：");
	//Os__GB2312_display(1,0,(unsigned char *)"[1]---串口1 ");
	//Os__GB2312_display(2,0,(unsigned char *)"[2]---串口2");

	while(!(KEY=='1' ||KEY=='2'||KEY==KEY_CLEAR))
		KEY = Os__xget_key();

	ucComm = 0;
	if(KEY == '1')
	{
		ucComm = COM_ONE;
	}
	else if(KEY == '2')
	{
		ucComm = COM_TWO;
	}
	else if(KEY == KEY_CLEAR)
	{
		return ERR_CANCEL;
	}
	return SUCCESS;
}
unsigned char LOADKEY_CommInit(void)
{
	COM_PARAM CommParam =
	{
		1,					/* Stop bit */
		9600,			/* Speed */
		1,					/* MODEM 0 RS232 1 HDLC 2 */
		8,					/* Data bit */
	   	'N'				/* NO parity */
	};

	if(ucComm == COM_ONE)
	{
		if(!OSUART_Init1(&CommParam))
			return SUCCESS;
		else
			return ERR_COMMINIT;
	}
	else
	if(ucComm == COM_TWO)
	{
		Os__end_com3();
		if( !OSUART_Init2(0x0303,0x0C00,0x0C))	//COM2 speed:9600
		{
			return SUCCESS;
		}
		else
		{
			return ERR_COMMINIT;
		}
	}
	return(SUCCESS);
}

unsigned char LOADKEY_LoadKeyFromPOS(void)
{
	unsigned char aucBuf[1024];
	unsigned char ucResult=SUCCESS;
	unsigned char aucMid[18];
	unsigned short uiOutLen;
	unsigned int uiI,uiOffset;
	unsigned int uiIndex=0;

	if(!(ucResult = UTIL_Is_Trans_Empty()))
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1, 0, (uchar *)"POS存有交易数据");
		//Os__GB2312_display(2, 0, (uchar *)"请先结算");
		//Os__GB2312_display(2, 0, (uchar *)"或清除交易数据");
		Os__xget_key();
		return ERR_CANCEL;
	}
	util_Printf("\n KEY DOWN is:");

	ucResult = LOADKEY_SelectCOM();
	if (ucResult == SUCCESS)
		ucResult = LOADKEY_CommInit()	;
	if (ucResult == SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"输入密钥索引:");
		//Os__GB2312_display(1,0,(unsigned char*)"(0-2)");
		if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char*)"012") != KEY_ENTER )
		{
			return (ucResult);
		}
		uiIndex = aucBuf[0]-'0';
	}
	if (ucResult == SUCCESS)
	{
		uiOutLen = 0;
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = LOADKEY_PORT_RevData(aucBuf,&uiOutLen);//接收密钥
		if(ucResult == SUCCESS)
		{
			//首先保存数据
			//02 00 08 31 31 ...31 03 f3
			if ( aucBuf[2] == 8 )
			{
				LOADKEY_Single(uiIndex,&aucBuf[3]);
			}
			//发送数据接收成功的应答
			memset(aucMid,0,sizeof(aucMid));
			aucMid[0]=0x02;
			aucMid[1]=0x00;
			aucMid[2]=0x02;
			if ( aucBuf[2] == 8 )
			{
				memcpy(&aucMid[3],"00",2);
				ucResult = 0;
			}else
			{
				memcpy(&aucMid[3],"01",2);
				ucResult = 0x02;
			}
			aucMid[5]=0X03;
			aucMid[6]=0X01;//校验位无需判断：edit jianggs
			ucResult = LOADKEY_PORT_SendData(aucMid,7);
		}
	}
	Os__end_com3();
	//if (ucResult != SUCCESS)
		//LOADKEY_DisERRMsg(ucResult);
	if(ucResult == SUCCESS)
	{
		//Os__GB2312_display(2,0,(uchar *)"密钥下载成功！");
	}else
	{
		//Os__GB2312_display(2,0,(uchar *)"密钥下载失败！");
	}
	MSG_WaitKey(20);
	return SUCCESS;
}



