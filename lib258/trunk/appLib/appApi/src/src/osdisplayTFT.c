#include <stdio.h>
#include <string.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <stdlib.h>
#include <sys/msg.h>
#include "osdisplay.h"

#define nbytes 140

//定义消息队列ID
int lcd_message_id = 18888;
//定义消息队列的Key值
key_t lcd_key = 1300;


//定义消息队列数据结构体
struct message_data
{
	int api_id;
	
	int line;
	int colum;
	
	int gprs_d;
	int gprs_s;
	int modem_dial;
	int r1;
	int r2;
	int r3;
	int r4;
			
	unsigned char mtext[34];
};

struct lcd_messaage
{
	long  mtype;
	struct message_data msg_data;
};


void lcd_init()
{
again:
    /*get the lcd icon key*/
    if ((lcd_message_id = msgget(lcd_key ,0777))<0)  /*creat a new message array!!*/
    {
        sleep(1);
        goto again;
    }
}

int Os__GB2312_disp(unsigned char ucRow, unsigned char *pucPtr)
{
	struct lcd_messaage msg;

	memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext); 

	msg.msg_data.api_id = 0;
	msg.msg_data.line = ucRow;
	strncpy(msg.msg_data.mtext, pucPtr, lt-1);
	msg.mtype = 50;

	if (pucPtr == NULL || *pucPtr == 0)
	{
		return OSDISPLAY_TEXTISNULL;
	}

	if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
		return OSDISPLAY_MESSAGESENDERR;
	}
	return OSDISPLAY_OK;
}

int Os__GB2312_display(unsigned char line, unsigned char column, unsigned char *t)
{
	struct lcd_messaage msg;

	if (t == NULL || *t == 0)
	{
		return OSDISPLAY_TEXTISNULL;
	}

	memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext); 

	msg.msg_data.api_id = 1;
	msg.msg_data.line = line;
	msg.msg_data.colum = column;
	strncpy(msg.msg_data.mtext, t, lt-1);
	msg.mtype = 50;

	if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
			return OSDISPLAY_MESSAGESENDERR;
	}

	return OSDISPLAY_OK;
}

int Os__display(unsigned line, unsigned colum, char *t)
{
	struct lcd_messaage msg;

	if (t == NULL || *t == 0)
	{
		return OSDISPLAY_TEXTISNULL;
	}

	memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext); 
	
	msg.msg_data.api_id = 2;
	msg.msg_data.line = line;
	msg.msg_data.colum = colum;
	strncpy(msg.msg_data.mtext, t, lt-1);
	msg.mtype = 50;

	if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
			return OSDISPLAY_MESSAGESENDERR;
	}
	return OSDISPLAY_OK;
}

void Os__clr_display(unsigned char line)
{
	struct lcd_messaage msg;
	msg.msg_data.api_id = 3;
	msg.msg_data.line = line;
	msg.mtype = 50;

	if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
		perror("message send");
	}
}

int Os__graph_display(unsigned line, unsigned char colum, unsigned char *drawing, unsigned char size)
{
	struct lcd_messaage msg;
	char path[50];

	if (drawing == NULL)
	{
		return OSDISPLAY_TEXTISNULL;
	}

	memset(&msg , 0, sizeof(msg));
	memset(path,0,sizeof(path));
    int lt = sizeof(msg.msg_data.mtext); 

	sprintf(path,"/mnt/pos/app/lcd/%s",drawing);
    //检查文件是否存在
    if(access(path,0)!=0)
    {
    	return OSDISPLAY_TEXTISNULL;
    }
	
	msg.msg_data.api_id = 7;
	msg.msg_data.line = line;
	msg.msg_data.colum = colum;
	msg.mtype = 50;
	strncpy(msg.msg_data.mtext, drawing, lt-1);
	
	if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
		return OSDISPLAY_MESSAGESENDERR;
	}
	return OSDISPLAY_OK;
}

int OSMMI_DisplayASCII (unsigned char ucFont, unsigned char line, unsigned char colum, unsigned char * t)
{
	struct lcd_messaage msg;

	if (t == NULL || *t == 0)
	{
		return OSDISPLAY_TEXTISNULL;
	}

	memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext); 

	msg.msg_data.api_id = 8;
	msg.msg_data.line = line;
	msg.msg_data.colum = colum;
	strncpy(msg.msg_data.mtext, t, lt-1);
	msg.mtype = 50;

	if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
		return OSDISPLAY_MESSAGESENDERR;
	}
	return OSDISPLAY_OK;
}


int OSMMI_ClrDisplay(unsigned char ucFont, unsigned char line)
{

	struct lcd_messaage msg;
	msg.msg_data.api_id = 9;
	msg.msg_data.line = line;
	msg.mtype = 50;

	if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
		return OSDISPLAY_MESSAGESENDERR;
	}
	return OSDISPLAY_OK;
}

int Os__lcd_image(int x,int y,char *file)
{
	struct lcd_messaage msg;
	char path[50];
	
	memset(&msg , 0, sizeof(msg));
	memset(path,0,sizeof(path));
    int lt = sizeof(msg.msg_data.mtext); 

    sprintf(path,"/mnt/pos/app/lcd/%s",file);
    //检查文件是否存在
    if(file == NULL || access(path,0)!=0)
    {
    	return OSDISPLAY_TEXTISNULL;
    }

	msg.mtype = 50;
	msg.msg_data.api_id = 13;
	msg.msg_data.colum = y;
	msg.msg_data.line = x;
	strncpy(msg.msg_data.mtext,file,lt-1);
	
	if (msgsnd(lcd_message_id, &msg, 140, 0) < 0 )
	{
		return OSDISPLAY_MESSAGESENDERR;
	}
	return OSDISPLAY_OK;
}

/////////////////////// END //////////////////////

int _Os__displayTFT(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text)
{
	struct lcd_messaage msg;
	int max,length;
	
	//默认定义显示文本内容的长度为最大显示长度
	max = strlen(Text);

	if(0 == max)
	{
		return OSDISPLAY_TEXTISNULL;
	}

	memset(&msg , 0, sizeof(msg));
	length = sizeof(msg.msg_data.mtext);
    max = (length-1)>max ? max : (length-1);
    //小字体,可显示8行,每行最多21个字符
	if(0x30 == ucFont)
	{
		msg.msg_data.api_id = 8;
		if(Line > 7 || Column > 20)
			return OSDISPLAY_DISPLAYBEYOND;	
	}
	//大字体,默认字体,ucFont=0x31,可显示5行,每行最多16个字符
	else
	{
		msg.msg_data.api_id = 2;
		if(Line > 4 || Column > 15)
			return OSDISPLAY_DISPLAYBEYOND;
	}
	msg.msg_data.line = Line;
	msg.msg_data.colum = Column;
	strncpy(msg.msg_data.mtext, Text, max);
	msg.mtype = 50;

	if(msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
		return OSDISPLAY_MESSAGESENDERR;
	}
	return OSDISPLAY_OK;
}

void _Os__clearTFT(unsigned char ucFont, unsigned char Line)
{
	struct lcd_messaage msg;
	
	if(0x30 == ucFont)
	{
		msg.msg_data.api_id = 9;
	}
	else
	{
		msg.msg_data.api_id = 3;
	}
	msg.msg_data.line = Line;
	msg.mtype = 50;

	if(msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
	{
		perror("message send");
	}
}

