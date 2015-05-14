#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"

#define MAX_BUF_SIZE	10240
#define WIFIINFO_FILE_NAME	"/etc/usbwifi/wpa_supplicant.conf"

int Os_checkRouter(void)
{
	int ret;
	FILE *fd;
	unsigned char ucbuf[500];
	
	fd = popen("route -n &","r");
        if(fd == NULL)
        {
                Uart_Printf("popen error\r\n");
                return -1;
        }

        while(fgets(ucbuf,500,fd) != NULL)
        {
		if(strstr(ucbuf,"ppp0") != NULL)
		{
			pclose(fd);
			return 1;
		}
		else if(strstr(ucbuf,"wlan0") != NULL)
		{
			pclose(fd);
			return 2;
		}
        }
        pclose(fd);
	return 0;
}

int Os__setWifi(WifiNetwork wifiInfo)
{
	int ret = 0;
	int cn = 0;
	int c;
	int errno = 1;
	unsigned char *p, *p1;
	unsigned char readBuf[MAX_BUF_SIZE];
	unsigned char tmpBuf[MAX_BUF_SIZE];
	FILE *rfd = NULL, *wfd = NULL;
	//打开要修改的文件
	rfd = fopen(WIFIINFO_FILE_NAME,"r");
	if(rfd == NULL)
	{
		return -1;
	}
	memset(readBuf,0,sizeof(readBuf));
	memset(tmpBuf,0,sizeof(tmpBuf));
	sprintf(tmpBuf,"%s.bak",WIFIINFO_FILE_NAME);
	//把要修改的文件中的内容保存到另一个文件中
	wfd = fopen(tmpBuf,"w");
	if(wfd < 0)
	{
		fclose(rfd);
		return -1;
	}
	//逐行读取文件内容
	while(fgets(readBuf,sizeof(readBuf),rfd) != NULL)
	{
		cn = strlen(readBuf);
		//筛选文件中关键字,确定要修改文件的起始点
		if(cn >= 8 && memcmp(readBuf,"network=",8) == 0)
		{
			if((p = strchr(readBuf,'}')) == NULL)
			{
				//继续读取文件内容直到出现"}"或者文件的结束符
				c = fgetc(rfd);
				while(c != '}' && c != '\0')
				{
					readBuf[cn++] = c;
					c = fgetc(rfd);
				}
			}
			else
				*p = '\0';

			//对内容进行处理
			p = strchr(readBuf,'{');
			memset(tmpBuf,0,sizeof(tmpBuf));
			memcpy(tmpBuf,readBuf,p-readBuf+1);
			tmpBuf[p-readBuf+1] = '\n';
			fwrite(tmpBuf,1,strlen(tmpBuf),wfd);
			//将字符串拆分
			p1 = strtok(p+1,"\n");
			do{
				memset(tmpBuf,0,sizeof(tmpBuf));
				if(memcmp(p1,"ssid",4) == 0)
				{
					sprintf(tmpBuf,"ssid=\"%s\"\n",wifiInfo.ssid);
					fwrite(tmpBuf,1,strlen(tmpBuf),wfd);
				}
				else if(memcmp(p1,"psk",3) == 0)
				{
					sprintf(tmpBuf,"psk=\"%s\"\n",wifiInfo.psk);
					fwrite(tmpBuf,1,strlen(tmpBuf),wfd);
				}
				else
				{
					sprintf(tmpBuf,"%s\n",p1);
					fwrite(tmpBuf,1,strlen(tmpBuf),wfd);
				}
			}while((p1=strtok(NULL,"\n")));
			//最后写入}
			fwrite("}\n",1,2,wfd);
		}
		else
		{
			fwrite(readBuf,1,cn,wfd);
		}
		memset(readBuf,0,sizeof(readBuf));
	}
	
	fclose(rfd);
	fclose(wfd);
	memset(tmpBuf,0,sizeof(tmpBuf));
	sprintf(tmpBuf,"mv %s.bak %s",WIFIINFO_FILE_NAME,WIFIINFO_FILE_NAME);
	system(tmpBuf);
	system("sync");
	return 0;
}

