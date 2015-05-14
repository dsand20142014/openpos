#include <oslib.h>
#include <include.h>
#include <global.h>
#include <xdata.h>

extern const SINGLEERRMSG ErrMsg[];

unsigned char rs232_ComRecvByte(unsigned char* ucCh,unsigned int uiTimeOut)
{

	unsigned short uiResult;
	unsigned char aucBuf[3];
#if 1
	switch(RunData.ucComType)
	{
	case 2:
		uiResult = Os__rxcar3(uiTimeOut);
		break;
	default:
		uiResult = Os__rxcar(uiTimeOut);
		break;
	}
#else
       uiResult = Os__rxcar(uiTimeOut);
#endif
	switch	( uiResult / 256 )
	{
	case OK :
		*ucCh = uiResult % 256;
		return(OK);
	case COM_ERR_TO:
		return(ERR_COMMS_RECVTIMEOUT);
	default :
		return(ERR_COMMS_RECVCHAR);
	}

}

int rs232_GetLength(unsigned char *aucData)
{
	
	unsigned long	Length;
       Length = bcd_long(aucData,4);
      
	Uart_Printf("\n Length = %d ",Length);
	return Length;
}	

unsigned char rs232_InceptData(unsigned char* pt)
{
	unsigned char ucResult,ucChar;
	unsigned char LRC;
	unsigned int  uiLen;
	int Length;
	unsigned char ucI;
	unsigned char aucBuf[6];
	int i;
	LRC = 0;
	uiLen = 0;

  memset(aucBuf,0,sizeof(aucBuf));
 #if 0
	for(ucI=0;ucI<DataSave0.ConstantParamData.Rs232Config.uiLengthOffset;ucI++)
#else
	for(ucI=0;ucI<2;ucI++)
#endif	
	{
		if(!(ucResult = rs232_ComRecvByte(&ucChar,500)))
		{
			*pt = ucChar;
			aucBuf[ucI] = ucChar;
			LRC ^= ucChar;
			pt++;
			Uart_Printf("\naucBuf[%02x]=%02x\n",ucI,aucBuf[ucI]);
		}
		else 
		{
			return ucResult;
		}
	}

  Length = rs232_GetLength(aucBuf);
  
  if(Length == -2)
  	 return ERR_CANCEL;
 
	Uart_Printf("Trans Data Len = %d\n",Length);
	
	for(i=0;i<Length;i++)
	{
		if(!(ucResult = rs232_ComRecvByte(&ucChar,500)))
		{
			*pt = ucChar;
			LRC ^= ucChar;
			pt++;
			Uart_Printf("\naucBuf[%d]=%02x\n",i,ucChar);
		}
		else
		{
			return ucResult;
		}
	}
    
	if(!(ucResult = rs232_ComRecvByte(&ucChar,500)))
	{
		if(ucChar == ETX_CHAR)
		{
			*pt = ucChar;
			LRC ^= ucChar;
			pt++;
		}
		else
		{
			return ERR_COMMS_PROTOCOL;
		}
	}
	else
	{
		return ucResult;
	}
	
	if(!(ucResult = rs232_ComRecvByte(&ucChar,500)))
	{
		Uart_Printf("LRC = %02x\n",LRC);
		if(LRC != ucChar)
		{
			return ERR_COMMS_LRC;
		}
		else
		{
			*pt = ucChar;
			pt++;
			return SUCCESS;
		}
	}
	else
	{
		return ucResult;
	}
}

unsigned char rs232_InitCOM(void)
{
	COM_PARAM ComParameter;
	unsigned char ucResult;
	
	switch(RunData.ucComType)
	{
	case 2:
		while(1)
		{
			OSUART_Close2();
			if(Os__init_com3(0x0303,0x0c00,0x0c) != OK)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,( uchar *) "˿ڳʼ");
				Os__xdelay(500);
			}else
				break;
		}
		return SUCCESS;
	default:
		ComParameter.stop_bits = 1;
		ComParameter.speed = 9600;
		ComParameter.com_nr = 1;
		ComParameter.data_bits = 8;
		ComParameter.parity = 'N';
		if( Os__init_com(&ComParameter) == OK )
		{
			return SUCCESS;
		}
		else
		{
			return ERR_COMMS_SENDCHAR;
		}
	}
}

unsigned char rs232_SendData(unsigned char *pSendData,unsigned int uiLen)
{
	unsigned char ucResult=SUCCESS;
	unsigned int  uiI;
	unsigned char ucPrintBuf[40];

	switch(RunData.ucComType)
	{
		case 2:
			for(uiI = 0;uiI < uiLen;uiI++)
			{
				Os__txcar3(pSendData[uiI]);
				//Os__xdelay(2);
			}
			return SUCCESS;
		default:
			for(uiI = 0;uiI < uiLen;uiI++)
			{
				ucResult = Os__txcar(pSendData[uiI]);
				if(ucResult != OK)
				{
					return ERR_COMMS_SENDCHAR;
				}
			}
			return SUCCESS;
	}
}

unsigned char rs232_CloseCOM(void)
{
	switch(RunData.ucComType)
	{
		case 2:
			Os__end_com3();
			break;
		default:
			Os__hang_up();
			break;
	  }
	  return SUCCESS;
}



int rs232_PackData_Send(unsigned char * aucInData,unsigned char *aucOutData)
{
          unsigned int i,j;
	   unsigned char ucLRC;
	   unsigned char ucResult = SUCCESS; 
	   int uiOutLength;
      
	   i =0;
	   ucLRC = 0;
          aucOutData[i] = STX_CHAR;
	   i++;
	   uiOutLength = rs232_GetLength(aucInData);
#if 1
	    uiOutLength = uiOutLength + 2;
#else
          uiOutLength = uiOutLength + DataSave0.ConstantParamData.Rs232Config.uiLengthOffset;
#endif
	   for(j=0;j<uiOutLength;j++)
	   {
	          aucOutData[i+j] = *aucInData;
		   ucLRC ^=   aucOutData[i+j];
		   aucInData++;
	   }
	   i += uiOutLength;
	   aucOutData[i] = ETX_CHAR;
	   ucLRC ^=   aucOutData[i];
	   i++;
	   aucOutData[i] = ucLRC;
	   uiOutLength += 3;
	  return uiOutLength;
}

unsigned char rs232_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
                    unsigned char *pucOutData,unsigned short *puiOutLen)
{
	int uiLen;
	unsigned char ucResult=SUCCESS,aucBuf[600],aucComVersion[10],aucCommuBuf[600],ucChar;
	unsigned char 	aucTPDU[]={ 0x60,0x00,0x03,0x80,0x00,0xAA,0xBB,0x31,0x31};
	int i;
	DRV_OUT *pxKey;

	rs232_InitCOM();
	memset(aucComVersion,0,sizeof(aucComVersion));	
	memcpy(aucComVersion,RunData.aucComVersion,5);
	aucComVersion[5]='L';
	uiLen=uiInLen+9+9;
	Uart_Printf("uiLen=%d\n",uiLen);
	int_bcd(&aucCommuBuf[0],2,&uiLen);
	memcpy(&aucCommuBuf[2],aucTPDU,9);
	memcpy(&aucCommuBuf[11],aucComVersion,9);
	memcpy(&aucCommuBuf[20],pucInData,uiInLen);

	uiLen=rs232_PackData_Send(aucCommuBuf,aucBuf);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0, "");
	ucResult=rs232_SendData(aucBuf, uiLen);
	ucChar=0;
	//Os__GB2312_display(0,0, "");
	do
	{
		pxKey = Os__get_key();
		ucResult = rs232_ComRecvByte(&ucChar,100);
	}
	while(pxKey->xxdata[1]!=KEY_CLEAR&&ucChar!=STX_CHAR);
	if(!ucResult)
	{
		OSDRV_Abort(drv_mmi);
		while(ucResult == OK)
		{
			if(ucChar == STX_CHAR)
				break;
			else
			      Os__xdelay(50);
		      ucResult = rs232_ComRecvByte(&ucChar,1);
		}
		if(ucResult == SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult = rs232_InceptData(aucBuf);
		}
		else if(ucResult == ERR_COMMS_RECVTIMEOUT)
		{
			ucResult = ERR_COMMS_PROTOCOL;
		}		
		//ն
	      /*  while(1)
	   	{
	   		if(rs232_ComRecvByte(&ucChar,10) != SUCCESS)
	   			break;
	   	}*/
		Uart_Printf("recv data:\n");
		for(i=0;i<bcd_long(aucBuf,4);i++)
		{
			Uart_Printf("%02x ",aucBuf[i]);
		}
		Uart_Printf("\n");
		if(ucResult == SUCCESS)
		{
			*puiOutLen=bcd_long(aucBuf,4)-18;
			memcpy(pucOutData,&aucBuf[20],*puiOutLen);
		}
		return ucResult;
	}
	return ucResult;
}

unsigned char RS232_SendTry(unsigned char ucTryType)
{
	unsigned char ucResult = SUCCESS;
	unsigned int offset;
	SENDTRYDATA  SendClearData;

	Uart_Printf("\n99999999999999999999\n");	
	memset(Com1_Info.aucCommBuf,0,sizeof(Com1_Info.aucCommBuf));
	Com1_Info.ulCommLen = 0;
	
	offset=2;
	//Identifier
	memcpy(&Com1_Info.aucCommBuf[offset],RunData.aucAppId,8);
	offset += 8;
	
	//Obligate
	Com1_Info.aucCommBuf[offset] = 0x04;
	offset+= 5;
	
	//TransDataCASH_PROTOCOL_VER
	memcpy(&Com1_Info.aucCommBuf[offset],CASH_PROTOCOL_VER,3);
	offset+= 3;
	
	memset(&SendClearData,0,sizeof(SENDTRYDATA));
	memcpy(SendClearData.ucOperateType,"FF",2);
	//MSG_SetErrMsg(ucTryType);
	MSG_DisplayErrMsg(ucTryType);
	hex_asc(SendClearData.aucResponseCode,&ucTryType,2);
	memcpy(SendClearData.aucExplain,RunData.aucErrorExplain,16);
	memcpy(&Com1_Info.aucCommBuf[offset],(unsigned char*)&SendClearData,sizeof(SENDTRYDATA));
	offset+=sizeof(SENDTRYDATA);	
	Com1_Info.ulCommLen = offset;
	offset-=2;
	long_bcd(Com1_Info.aucCommBuf,2,(ulong *)&offset);

	return SUCCESS;
}


unsigned char RS232_ExtendTime(unsigned long ulTime)
{
	unsigned char ucResult = SUCCESS,aucBuf[100];
	unsigned int offset,uiLen;
	EXTENDTIMEDATA ExtendTimeData;
	
	memset(Com1_Info.aucCommBuf,0,sizeof(Com1_Info.aucCommBuf));
	Com1_Info.ulCommLen = 0;
	
	offset=2;
	//Identifier
	memcpy(&Com1_Info.aucCommBuf[offset],RunData.aucAppId,8);
	offset += 8;
	
	//Obligate
	//Com1_Info.aucCommBuf[offset] = 0x03;
	offset+= 5;
	
	//TransDataCASH_PROTOCOL_VER
	memcpy(&Com1_Info.aucCommBuf[offset],CASH_PROTOCOL_VER,3);
	offset+= 3;
	
	memset(&ExtendTimeData,0,sizeof(ExtendTimeData));
	memcpy(ExtendTimeData.ucOperateType,"C0",2);
	memcpy(ExtendTimeData.ucCashID,RunData.aucCashID,6);
	long_bcd(ExtendTimeData.ucExtendTime,2,&ulTime);
	memcpy(&Com1_Info.aucCommBuf[offset],(unsigned char*)&ExtendTimeData,sizeof(EXTENDTIMEDATA));
	offset+=sizeof(EXTENDTIMEDATA);	
	Com1_Info.ulCommLen = offset;
	offset-=2;
	long_bcd(Com1_Info.aucCommBuf,2,(ulong *)&offset);
	uiLen=rs232_PackData_Send(Com1_Info.aucCommBuf,aucBuf);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0, "");
	ucResult=rs232_SendData(aucBuf, uiLen);

	return ucResult;
}



unsigned char  rs232_Get_FromLength(unsigned char *aucOutData,unsigned long uiLength)
{
	unsigned char 	aucData[30];
	unsigned long	Length;
       long_bcd(aucOutData,2,&uiLength);
	return SUCCESS;
}	
unsigned char rs232_PackData(unsigned long uiLength,unsigned char * aucInData,unsigned char *aucOutData)
{
          unsigned int i,j;
	   unsigned char ucLRC;
	   unsigned char ucResult = SUCCESS; 

	   i =0;
	   ucLRC = 0;
         aucOutData[i] = STX_CHAR;
	   i++;
	   ucResult = rs232_Get_FromLength(&aucOutData[i],uiLength);
	   
	   for(j=0;j<2;j++)
	   {
	   	  ucLRC ^= aucOutData[i];
	         i++;  
	   }
	   for(j=0;j<uiLength;j++)
	   {
	          aucOutData[i+j] = *aucInData;
		   ucLRC ^=   aucOutData[i+j];
		   aucInData++;
	   }
	   i += uiLength;
	   aucOutData[i] = ETX_CHAR;
	   ucLRC ^=   aucOutData[i];
	   i++;
	   aucOutData[i] = ucLRC;
	 return ucResult;
}

void rs232_SendCancelData(void)
{
	   unsigned char aucPrintData[30];
	  unsigned char aucBuf[30];
	  
	
	RunData.ucComType=DataSave0.ConstantParamData.ucCom;
       rs232_InitCOM();	
	memset(aucPrintData,0,sizeof(aucPrintData));
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucPrintData,&HeadData,23);
	memcpy(&aucPrintData[3],"CANCEL  ",8);
	rs232_PackData(23,aucPrintData,aucBuf);
	rs232_SendData(aucBuf,28);
}
