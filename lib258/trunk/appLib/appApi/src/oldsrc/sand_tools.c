
#define __SAND_TOOLS__
#include <string.h>
#include <stdio.h>
#include "sand_tools.h"

/*
功能：num转换成str, 字符数为L, 不足部分左补0
*/
void long_to_0_str(unsigned char *str, unsigned int num, int L)
{
    int i,Len;
    char buf[20];

    memset(buf,0,20);
    sprintf(buf,"%d",num);
    Len = strlen((char *)buf);
    for (i=0;i<L-Len;i++) *str ++ = 0x30;
    for (i=0;i<Len;i++) *str ++ = buf[i];
    *str = 0;
}

unsigned char BCD2HEX(unsigned int bcd_data)
{
    unsigned char temp;
    temp=((bcd_data>>8)*100)|((bcd_data>>4)*10)|(bcd_data&0x0f);
    return temp;
}

unsigned int HEX2BCD(unsigned char hex_data)
{
    unsigned int bcd_data;
    unsigned char temp;
    temp=hex_data%100;
    bcd_data=((unsigned int)hex_data)/100<<8;
    bcd_data=bcd_data|temp/10<<4;
    bcd_data=bcd_data|temp%10;
    return bcd_data;
}

//十六进制转换为压缩BCD码
//   convert   {0x4c,0x5e,0x33}   to   "4c5e33"
unsigned   char istoASCs(unsigned   char * desBuffer, unsigned   char * srcBuffer,int   len)
{
    int   i;
    int ch;
    for (i=0;i<len;i++)
    {
        ch   =   srcBuffer[i]   >>   4;

        if ((ch>=0) && (ch<=9))
        {
            desBuffer[2*i]   =   ch   +'0';
        }
        else if ((ch   >=10)&&(ch   <=   15))
        {
            desBuffer[2*i]   =   ch   +   55;
        }
        else
            desBuffer[2*i]   =   '*';

        ch   =   srcBuffer[i]   &   0x0F;
        if ((ch   >=   0)&&(ch   <=   9))
        {
            desBuffer[2*i+1]   =   ch   +'0';
        }
        else   if ((ch   >=10)&&(ch   <=   15))
        {
            desBuffer[2*i+1]   =   ch   +   55;
        }
        else
            desBuffer[2*i+1]   =   '*';
    }
    desBuffer[2*i]   =   '\0';
    return   0;
}



//压缩BCD码转换为十六进制
//89860099--->0x89,0x86,0x00,0x99
# if 0
int   StrToEBCD(char *buf,char   *ucBuffer,int   BufLen)
{
    unsigned   char   temp1,temp2;
    int   Len=BufLen/2,i;

    for   (i=0;i<Len;i++)
    {
        temp1=buf[i*2];
        if   (temp1>='a')
            temp1=temp1   -   'a'   +   10;
        if   (temp1>='A')
            temp1=temp1   -   'A'   +   10;
        if   (temp1>='0')
            temp1=temp1-'0';


        temp2=buf[i*2   +   1];
        if   (temp2>='a')
            temp2=temp2   -   'a'   +   10;
        if   (temp2>='A')
            temp2=temp2   -   'A'   +   10;
        if   (temp2>='0')
            temp2=temp2-'0';
        ucBuffer[i]=((temp1&0x0f)<<4)|(temp2&0x0f);
    }
    return   0;
}
#endif

/*
将接受到的数据转换成字符串
*/
void str_from_bytes(unsigned char *str, unsigned char *rec, int len)
{
    unsigned char i, asc1;
    unsigned char s[3], s1[3], s2[3]={};
    memset(str, 0x0, 513);
    memset(s, 0x0, 3);
    memset(s1, 0x0, 3);

    for (i=0; i<len;i++)
    {
        Uart_Printf("rec=%x ", rec[i]);
    }
    Uart_Printf("\n");
    istoASCs(str, rec, len);
    Uart_Printf("final str=%s len=%d\n", str, strlen(str));

}






