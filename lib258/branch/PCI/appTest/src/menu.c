#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <fsync_drvs.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sand_pci.h>
#include <sys/mount.h>

#define KEY_PATH1	"/etc/pswd1"
#define KEY_PATH2	"/etc/pswd2"

static unsigned char get_nbyte_random(void* buf,int nbyte)
{
	FILE *fd  = NULL;
	if(NULL == buf)
		return 1;
	fd = fopen("/dev/urandom","r");
	fgets(buf,nbyte+1,fd);
	fclose(fd);
	return 0;
}
	

void clear_display(void)
{
	Os__clr_display(0);
	Os__clr_display(1);
	Os__clr_display(2);
	Os__clr_display(3);
	Os__clr_display(4);
	Os__clr_display(5);
	Os__clr_display(6);
	Os__clr_display(7);
	Os__clr_display(8);
}

unsigned char comparePasswd(unsigned char index,unsigned char* key)
{
	unsigned char path[20];
	unsigned char ucBuf[20];
	FILE *fd = NULL;

        if(index < 1 || index > 2 || NULL == key)
                return 1;
	if(1 == index)
		strcpy(path,KEY_PATH1);
	else if(2 == index)
		strcpy(path,KEY_PATH2);

	if(access(path,F_OK) == -1)	
		return 2;
	
	fd = fopen(path,"r");
	fgets(ucBuf,9,fd);
	fclose(fd);
	
	if(strcmp(ucBuf,key) == 0)
		return 0;
	else 
		return 3;
}

unsigned char SetUserKey(unsigned char index)
{
	NEW_DRV_TYPE new_drv;
	unsigned char ucResult;
	unsigned char ucBuf1[20];
	unsigned char ucBuf2[20];
	unsigned char pathBuf[20];
	FILE *fd = NULL;
	int cnt = 0;
	memset(ucBuf1,0,sizeof(ucBuf1));
	memset(ucBuf2,0,sizeof(ucBuf2));
	memset(pathBuf,0,sizeof(pathBuf));

	if('1' == index)
		strcpy(pathBuf,KEY_PATH1);
	else if('2' == index)
		strcpy(pathBuf,KEY_PATH2);

	if(access(pathBuf,F_OK) == -1)		//first set
	{
		while(1)
		{
			ucResult = SUCCESS;
			memset(ucBuf1,0,sizeof(ucBuf1));
			memset(ucBuf2,0,sizeof(ucBuf2));
			clear_display();
        		Os__GB2312_display(0, 0, (uchar *)"首次设定密码");
			if('1' == index)
        			Os__GB2312_display(1, 0, (uchar *)"请输入用户一密码：");
			else if('2' == index)
        			Os__GB2312_display(1, 0, (uchar *)"请输入用户二密码：");
			ucResult = UTIL_Input(2,true,8,8,'P',ucBuf1,NULL);
			if(ucResult  == KEY_ENTER)
			{
				clear_display();
				if('1' == index)
        				Os__GB2312_display(1, 0, (uchar *)"请再次输入用户一密码：");
				else if('2' == index)
        				Os__GB2312_display(1, 0, (uchar *)"请再次输入用户二密码：");

				ucResult = UTIL_Input(2,true,8,8,'P',ucBuf2,NULL);
				if(ucResult == KEY_ENTER)
				{
					if(memcmp(ucBuf1,ucBuf2,8) == 0)
					{
						fd = fopen(pathBuf,"w");
						ucResult = fwrite(ucBuf1,8,1,fd);
						fclose(fd);
						clear_display();
        					Os__GB2312_display(4, 0, (uchar *)"密码设定成功！");
						Os_Wait_Event(KEY_DRV, &new_drv, 100000);
						return 0;
					}
					else
					{
						clear_display();
						Os__GB2312_display(3, 2, (uchar *)"两次输入不一致");
						Os__GB2312_display(4, 2, (uchar *)"请重新输入！");
						Os_Wait_Event(KEY_DRV, &new_drv, 100000);
						continue;
					}
				}
				else
					return 1;
			}
			else
				return 1;		//user cancel
		}
	}
	else
	{
		while(1)
		{
			ucResult = SUCCESS;
			memset(ucBuf1,0,sizeof(ucBuf1));
			memset(ucBuf2,0,sizeof(ucBuf2));
			cnt ++;
			clear_display();
			if('1' == index)
        			Os__GB2312_display(1, 0, (uchar *)"请输入用户一原密码：");
			else if('2' == index)
        			Os__GB2312_display(1, 0, (uchar *)"请输入用户二原密码：");
			ucResult = UTIL_Input(2,true,8,8,'P',ucBuf1,NULL);
			if(ucResult == KEY_ENTER)
			{
				fd = fopen(pathBuf,"r");
				fgets(ucBuf2,9,fd);
				fclose(fd);
				Uart_Printf("ucBuf1 = %s\r\n",ucBuf1);
				Uart_Printf("ucBuf2 = %s\r\n",ucBuf2);
				if(memcmp(ucBuf1,ucBuf2,8) != 0)
				{
					if(cnt < 3)
					{
						Os__GB2312_display(4, 0, (uchar *)"原密码输入错误！");
						Os__GB2312_display(5, 0, (uchar *)"请重新输入");
						Os_Wait_Event(KEY_DRV, &new_drv, 100000);
						continue;
					}
					else
					{
						Os__GB2312_display(4, 0, (uchar *)"原密码输入错误超过3次！");
						Os_Wait_Event(KEY_DRV, &new_drv, 100000);
						return 2;
					}
				}
			}

			clear_display();
			memset(ucBuf1,0,sizeof(ucBuf1));
			if('1' == index)
        			Os__GB2312_display(1, 0, (uchar *)"请输入用户一新密码：");
			else if('2' == index)
        			Os__GB2312_display(1, 0, (uchar *)"请输入用户二新密码：");
			ucResult = UTIL_Input(2,true,8,8,'P',ucBuf1,NULL);
			if(ucResult == KEY_ENTER)
			{
				clear_display();
				if('1' == index)
        				Os__GB2312_display(1, 0, (uchar *)"请再次输入用户一新密码：");
				else if('2' == index)
        				Os__GB2312_display(1, 0, (uchar *)"请再次输入用户二新密码：");
					
				memset(ucBuf2,0,sizeof(ucBuf2));
				ucResult = UTIL_Input(2,true,8,8,'P',ucBuf2,NULL);
				if(ucResult == KEY_ENTER)
				{
					if(memcmp(ucBuf1,ucBuf2,8) == 0)
					{
						fd = fopen(pathBuf,"w");
						ucResult = fwrite(ucBuf1,8,1,fd);
						fclose(fd);
						clear_display();
        					Os__GB2312_display(4, 0, (uchar *)"密码设定成功！");
						Os_Wait_Event(KEY_DRV, &new_drv, 100000);
						return 0;
					}
					else
					{
						clear_display();
						Os__GB2312_display(3, 2, (uchar *)"两次输入不一致");
						Os__GB2312_display(4, 2, (uchar *)"请重新输入！");
						Os_Wait_Event(KEY_DRV, &new_drv, 100000);
						continue;
					}
				}
				else
					return 1;
			}
			else
				return 1;		//user cancel
		}
	}

}

unsigned char SetMenu(void)
{
	NEW_DRV_TYPE new_drv;
	unsigned char ucResult;

	ucResult = SUCCESS;
	clear_display();
	Os__GB2312_display(0, 0, (uchar *)"1.pos参数设置");
	Os__GB2312_display(1, 0, (uchar *)"2.主密钥下载密码设定");
	Os_Wait_Event(KEY_DRV, &new_drv, 100000);
	switch(new_drv.drv_data.xxdata[1])
	{
		case '1':
			ucResult = CFG_ConstantParamBaseData();
			lcd_exit();
			break;
		case '2':
			clear_display();
			Os__GB2312_display(0, 0, (uchar *)"1.用户一密码设定");
			Os__GB2312_display(1, 0, (uchar *)"2.用户二密码设定");
			Os_Wait_Event(KEY_DRV, &new_drv, 100000);
			switch(new_drv.drv_data.xxdata[1])
			{
				case '1':
				case '2':
					ucResult = SetUserKey(new_drv.drv_data.xxdata[1]);
					lcd_exit();
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return ucResult;
}

unsigned char COMMS_MasterKey_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
	unsigned char	ucResult;
	unsigned char	ucSendBuf[50];
	unsigned char	ucRecvBuf[50];
	unsigned char	ucLRC;
	unsigned short	ucLen;
	unsigned short	TimeOut;
	unsigned char	ucByte;
	unsigned char 	dbgBuf[20];
	int i;
	NEW_DRV_TYPE new_drv;

	ucLen = 0;
	ucLRC = 0x00;
	ucSendBuf[0] = 0x02;				//STX
	short_bcd(&ucSendBuf[1],2,&uiInLen);		//LEN
	memcpy(&ucSendBuf[3],pucInData,uiInLen);	//DATA
	ucSendBuf[3+uiInLen] = 0x03;			//ETX
	ucLen = 3+uiInLen+1;
	for(i=1;i<ucLen;i++)
	{
		ucLRC ^= ucSendBuf[i];
	}
	ucSendBuf[ucLen] = ucLRC;			//LRC
	ucLen ++;

	while(1)					//init com3
        {
		OSUART_Close2();
		if(Os__init_com3(0x0303,0x0c00,0x0c) != OK)
                {
			Os__xdelay(500);
		}else
		{
			break;
		}
	}
	clear_display();
	Os__GB2312_display(1, 0, (uchar *) "已连接收单中心\r\n");
	Os__GB2312_display(2, 0, (uchar *) "处理中...\r\n");
	Os__GB2312_display(3, 0, (uchar *) "数据发送中...\r\n");
	sleep(1);
	for(i=0;i<ucLen;i++)				//send data
	{
		Os__txcar3(ucSendBuf[i]);
	}

	//receive data
	TimeOut = 6000;
	ucLRC = 0;
	ucResult = COMMS_ReceiveByte(&ucByte,&TimeOut);		//receive STX
	if(OK != ucResult)
		return ucResult;
	if(CHAR_STX != ucByte)
		return(ERR_COMMS_PROTOCOL);
	//pucOutData[0] = CHAR_STX;

	TimeOut = 2;
	memset(ucRecvBuf,0,2);
	for(i=0;i<2;i++)					//receive LEN
	{
		ucResult = COMMS_ReceiveByte(&ucByte,&TimeOut);
		if(OK != ucResult)
			return ucResult;
		ucRecvBuf[i] = ucByte;
		ucLRC ^= ucByte;
	}
	ucLen = (unsigned short)bcd_long(ucRecvBuf,4);
	//memcpy(&pucOutData[1],ucRecvBuf,2);

	memset(ucRecvBuf,0,sizeof(ucRecvBuf));
	for(i=0;i<ucLen;i++)					//receive DATA
	{
		ucResult = COMMS_ReceiveByte(&ucByte,&TimeOut);
		if(OK != ucResult)
			return ucResult;
		ucRecvBuf[i] = ucByte;
		ucLRC ^= ucByte;
	}
	memcpy(pucOutData,ucRecvBuf,ucLen);

	ucResult = COMMS_ReceiveByte(&ucByte,&TimeOut);		//receive ETX
	if(OK != ucResult)
		return ucResult;
	//if(CHAR_ETX != ucByte)
	if(0x03 != ucByte)
		return ERR_COMMS_PROTOCOL;
	ucLRC ^= CHAR_ETX;
	//pucOutData[3+ucLen] = CHAR_ETX;

	ucResult = COMMS_ReceiveByte(&ucByte,&TimeOut);         //receive LRC
	if(OK != ucResult)
		return ucResult;
	if(ucLRC != ucByte)
		return ERR_COMMS_LRC;
	//pucOutData[3+ucLen+1] = ucByte;

	*puiOutLen = ucLen;
	//close com3 and recover com2
	Os__end_com3();
        OSUART_Init2(0x0303,0x0c00,0x0c);

	return(SUCCESS);
}

unsigned char DownloadMasterKey(void)
{
	unsigned char ucResult;
	unsigned char ucSendBuf[50];
	unsigned char ucRecvBuf[50];
	unsigned char rand1[10];
	unsigned char rand2[10];
	unsigned char randData[20];
	unsigned char masterKey[20];
	unsigned short inLen = 0;
	unsigned int i = 0;
	int outLen = 0;
	NEW_DRV_TYPE new_drv;
	
	//send random data;
	memset(ucSendBuf,0,sizeof(ucSendBuf));
	inLen = 0x0a;
	ucSendBuf[0] = 0x01;
	ucSendBuf[1] = 0x00;
	get_nbyte_random(&ucSendBuf[2],8);
	memcpy(rand1,&ucSendBuf[2],8);
	//ucResult = COMMS_MasterKey_SendReceive(ucSendBuf,inLen,ucRecvBuf,&outLen);
	outLen = serial_write(ucSendBuf,inLen);
	if(outLen < 0)
		return ucResult;
	outLen = serial_read(ucRecvBuf,500,6000);

	if(outLen < 0)
		return ucResult;
	else
	{
		if(ucRecvBuf[0] != 0x02)
			return 1;
		if(outLen != 0x0a)
			return 2;
		memcpy(rand2,&ucRecvBuf[2],8);
	}
	for(i=0;i<16;i++)
	{
		if(i%2)
			randData[i] = rand2[7-(i/2)];
		else
			randData[i] = rand1[i/2];
	}

	//send terminalID and merchantID
	memset(ucSendBuf,0,sizeof(ucSendBuf));
	memset(ucRecvBuf,0,sizeof(ucRecvBuf));
	ucSendBuf[0] = 0x11;
	ucSendBuf[1] = 0x00;
	inLen = 0x19;
	XDATA_Read_Vaild_File(DataSaveConstant);
	memcpy(&ucSendBuf[2],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	memcpy(&ucSendBuf[2+PARAM_TERMINALIDLEN],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	
	//ucResult = COMMS_MasterKey_SendReceive(ucSendBuf,inLen,ucRecvBuf,&outLen);
	outLen = serial_write(ucSendBuf,inLen);
	if(outLen < 0)
		return ucResult;
	outLen = serial_read(ucRecvBuf,500,6000);
	if(outLen < 0)
		return ucResult;
	else
	{
		if(ucRecvBuf[0] != 0x12)
			return 1;
		if(outLen != 0x12)
			return 2;
	}
	memcpy(masterKey,&ucRecvBuf[2],16);
	//Uart_Printf("MasterKey:\r\n");
	//for(i=0;i<16;i++)	
//		Uart_Printf("%02x ",masterKey[i]);
//	Uart_Printf("\r\n");

	return 0;
}



unsigned char DownloadMasterKeyMenu(void)
{
        unsigned char ucResult = SUCCESS;
        unsigned char aucPIN[13];
        NEW_DRV_TYPE new_drv;

        memset(aucPIN,0,sizeof(aucPIN));
	clear_display();
        Os__GB2312_display(1, 0, (uchar *)"请输入密码1：");
        ucResult = UTIL_Input(2,true,0,8,'P',aucPIN,NULL);
        if(ucResult == KEY_ENTER)
        {
                ucResult = comparePasswd(1,aucPIN);
                if(SUCCESS == ucResult)
                {
                        memset(aucPIN,0,sizeof(aucPIN));
			clear_display();
                        Os__GB2312_display(1, 0, (uchar *)"请输入密码2：");
			memset(aucPIN,0,sizeof(aucPIN));
                        ucResult = UTIL_Input(2,true,0,8,'P',aucPIN,NULL);
                        if(ucResult == KEY_ENTER)
                        {
                                ucResult = comparePasswd(2,aucPIN);
                                if(ucResult == SUCCESS)
                                {
					//clear_display();
					//Os__GB2312_display(4, 3, (uchar *)"now we can anything.");
					ucResult = DownloadMasterKey();
					return ucResult;

                                }
				else if(2 == ucResult)
				{
					clear_display();
					Os__GB2312_display(4, 3, (uchar *)"请先设置用户2密码");
					Os_Wait_Event(KEY_DRV, &new_drv, 100000);
					return ucResult;

				}
                                else if(3 == ucResult)
                                {
					clear_display();
                                        Os__GB2312_display(4, 3, (uchar *)"密码2错误！");
					Os_Wait_Event(KEY_DRV, &new_drv, 100000);
                                        return ucResult;
                                }
                        }
			else
                        {
                                return ucResult;
                        }
                }
		else if(2 == ucResult)
		{
			clear_display();
                        Os__GB2312_display(4, 3, (uchar *)"请先设置用户1密码");
			Os_Wait_Event(KEY_DRV, &new_drv, 100000);
                        return ucResult;
		}
                else if(3 == ucResult)
                {
			clear_display();
                        Os__GB2312_display(4, 3, (uchar *)"密码1错误！");
			Os_Wait_Event(KEY_DRV, &new_drv, 100000);
                        return ucResult;
                }
        }
        else
        {
                return ucResult;
        }
}

unsigned char StoreRandData(int MB)
{ 
	NEW_DRV_TYPE new_drv;
	int bufLen = 1024;
	unsigned char ucResult = 0;
	unsigned char buff[1024]={0};
	FILE * fp=NULL;
	int i=0,j=0;

	if(access("/dev/mmcblk0p1",0) != 0)
	{
		clear_display();
		Os__GB2312_display(4, 3, (uchar *)"请插入TF卡");
		Os_Wait_Event(KEY_DRV, &new_drv, 100000);
		ucResult = 1;
		return ucResult;
	}

	//if(mount("/dev/mmcblk0p1","/tmp","ext2",MS_DIRSYNC,NULL) != 0)
	{
		clear_display();
		Os__GB2312_display(4, 3, (uchar *)"挂载TF卡失败");
		Os_Wait_Event(KEY_DRV, &new_drv, 100000);
		ucResult = 2;
		return ucResult;
	}
	
	fp = fopen("/tmp/randdata.bin","wb+");
	if(NULL == fp)
	{
		clear_display();
		Os__GB2312_display(4, 3, (uchar *)"打开文件失败");
		Os_Wait_Event(KEY_DRV, &new_drv, 100000);
		ucResult = 3;
		return ucResult;
	}

	for(i=0;i<MB;i++)
	{
		for(j=0;j<1024;j++)
		{
			if(get_nbyte_random(buff,bufLen) != 0)
			{
				clear_display();
				Os__GB2312_display(4, 3, (uchar *)"获取随机数失败");
				Os_Wait_Event(KEY_DRV, &new_drv, 100000);
				ucResult = 4;
				return ucResult;

			}		
			ucResult = fwrite(buff,1,bufLen,fp);
			if(ucResult != bufLen)
			{
				clear_display();
				Os__GB2312_display(4, 3, (uchar *)"写文件失败");
				Os_Wait_Event(KEY_DRV, &new_drv, 100000);
				ucResult = 4;
				return ucResult;
			}
		}
	}
	
	if(umount("/dev/mmcblk0p1") != 0)
	{
		clear_display();
		Os__GB2312_display(4, 3, (uchar *)"卸载TF卡失败");
		Os_Wait_Event(KEY_DRV, &new_drv, 100000);
		ucResult = 5;
		return ucResult;
	}
}

unsigned char Main_Menu(unsigned char ucKey)
{
        unsigned char ucResult = SUCCESS,ucFlag;
        unsigned char ucNextKey;

       //Os__light_on();

        switch(ucKey)
        {
                case '1':
                        ucResult = LOGON_Online();
                        if(( ucResult != SUCCESS)&&( ucResult != ERR_CANCEL))
                        {
                                MSG_DisplayErrMsg(ucResult);
                        }
                        //Os_Wait_Event(KEY_DRV, &new_drv, 100000);
                        lcd_exit();
                        break;
                case '2':
                        ucResult = SetMenu();
                        lcd_exit();
                        break;
		case '3':
                        break;
		case '4':
			ucResult = 1;
			lcd_exit();
			break;
                case KEY_CLEAR:
                case KEY_BCKSP:
                        lcd_exit();
                        break;
                default:
                        break;
        }
        //Os__light_off();      
        return(SUCCESS);
}

