/********************************************************
 * Copyright 2013-2015 Sand, All Right Reserved
 * sand_pci.c
 * sand API for PCI
 * Created by xiangliu 20130604
 ********************************************************/
#include <stdio.h>
#include <string.h>
#include "sand_pci.h"
#include "sysglobal.h"

#define AUTH_LEN 16

/*
 * Create authorization base on serial number <info> and return it to <authData>
 * @info: serial number of POS
 * @authData: authorization code that created.
 */
static void auth(char *info, int infoLen, char *authData)
{
	char tmp, tmpData[8] = {0};
	char random[8];
	int i, j;

	srand(time(NULL));
	for(i=0; i<8; i++)
		random[i] = rand();

    for(i=0, j=0; i<infoLen; i++)
    {
        tmpData[j++] += info[i];
        if(j == 8)
            j = 0;
    }

    tmp = tmpData[0];
    for(i=0; i<7; i++)
    {
        tmpData[i] = tmpData[i] <<4;
        tmpData[i] |= (tmpData[i+1] >> 4) &0x0f;
    }
    tmpData[7] = tmpData[7] <<4;
    tmpData[7] |= (tmp >>4) &0x0f;

    for(i=0; i<8; i++)
        tmpData[i] += random[i];

	memcpy(authData, tmpData, 8);
	memcpy(authData +8, random, 8);    
}

/*
 * Create 16bytes info for authrize
 */
static void create_auth_info(unsigned char *info)
{
	if (!info) {
		printf("<create_auth_info> info is NULL!\n");
		return;
	}

    SPARAM sno = Os_readParam();
    info[0] = SER_ID_GET_AUTH;         //授权指令
    info[1] = AUTHORIZATION_LEN - 2;   //后续数据长度
    memcpy(info+2, (char *)sno.aucSerialNo, AUTH_LEN); //16byte序列号
	//other
}

/*
 * Get authorization code
 * @authData: authorization code returned.
 * @Return: 0 when success or -1 when no authorization or error.
 */
static int get_auth(char *authData)
{
	if (!authData)
		return -1;
	int ret = 0;	
	FILE *fp = NULL;

	if ((fp = fopen ("/daemon/authorization", "r")) == NULL) {
		perror("Can not find authorization code");
		return -1;
	}
	
	int len = fread(authData, 1, AUTH_LEN+1, fp);
	if (len < 0) {
		perror("fread error");
		ret = -1;
	}
	else if (len != AUTH_LEN) {
		printf("get_auth = %d", len);
		ret = -1;
	}
	
	fclose(fp);
	return ret;
}

/*
 * Write authorization code
 * @authData: authorization code.
 * @Return: 0 when success or -1 when error.
 */
static int write_auth(char *authData)
{
	if (!authData)
		return -1;
	int ret = 0;	
	FILE *fp = NULL;

	if ((fp = fopen ("/daemon/authorization", "w")) == NULL) {
		perror("write_auth error");
		return -1;
	}
	
	int len = fwrite(authData, AUTH_LEN, 1, fp);
	if (len < 0) {
		perror("fwrite error");
		ret = -1;
	}
	else {
//		printf("fwrite = %d", len);
		ret = 0;
	}
	
	fclose(fp);
	return ret;
}
	
static void Uint2Bcd(int uintData, unsigned char *bcdData, int bcdDataLen)
{
	int i;
    for(i=bcdDataLen-1; i>=0; i--)
    {
        bcdData[i] = (((uintData%100)/10 ) <<4) |(uintData%10);
        uintData /=100;
    }
}

static int Bcd2Uint(unsigned char *bcdData, int bcdDataLen)
{
	int i,uintData = 0;

    for(i=0; i<bcdDataLen; i++)
    {
		uintData *= 100;
		uintData += (bcdData[i] &0x0f);
	  	uintData += ((bcdData[i] >>4) &0x0f) *10; 	
    }
    return uintData;
}

/*
 * Check authorization base on serial number <info> and authorization code <authData>
 * @info: serial number of POS
 * @authData: authorization code.
 * @Return: 0 when check ok or -1 when check error.
 */
int checkAuth(char *info, int infoLen, char *authData)
{
	char tmp, tmpData[AUTH_LEN] = {0};
	int i, j;

    for(i=0, j=0; i<infoLen; i++)
    {
        tmpData[j++] += info[i];
        if(j == 8)
            j = 0;
    }

    tmp = tmpData[0];
    for(i=0; i<7; i++)
    {
        tmpData[i] = tmpData[i] <<4;
        tmpData[i] |= (tmpData[i+1] >> 4) &0x0f;
    }
    tmpData[7] = tmpData[7] <<4;
    tmpData[7] |= (tmp >>4) &0x0f;

    for(i=0; i<8; i++){
        tmpData[i] += authData[i +8];
	}
	if(memcmp(tmpData, authData, 8)) {
		printf("<checkAuth> failed!\n");
		return -1;
	}
	return 0;
}

int check_auth()
{
	// get serial number
	SPARAM sno = Os_readParam();

	// get authorization code
	char aucbuf[AUTH_LEN+1];
	memset(aucbuf, 0, sizeof(aucbuf));
	if (get_auth(aucbuf) < 0) {
		printf("<check_auth> get_auth error!\n");
		return -1; // no authorization
	}
	
	// check
	return checkAuth(sno.aucSerialNo, AUTH_LEN, aucbuf);
}

/*
 * receive data from com3
 * @Return received byte (>=0) on success or -1 when error
 */	
int serial_read(unsigned char *rdata, unsigned int rdataMaxLen, unsigned short timeout)
{
	int i, ret = 0;
	unsigned char dataLen = 0;
   	unsigned char lrc = 0;
	unsigned int rxNum =0;
	unsigned short data;
	enum RxStat stat = S_STX;	

  	if(!Os__init_com3(0x0303,0x0c00,0x0c))
 	{
    	while(stat != S_END)
    	{
			data = Os__rxcar3(timeout);
			if(data >> 8) {
				return -2;
				printf("serial_read error: timeout!\n");
			}
			else
				rdata[rxNum] = data;

       	 	switch(stat)
        	{
				case S_STX:
					if(rdata[rxNum = 0] == STX_CHAR)
					{
						rxNum++;
						stat = S_DATALEN;
					}
					break;
				case S_DATALEN:
					if(rxNum == 2)
					{
						dataLen = Bcd2Uint( rdata +1, 2);
						if(dataLen +5> rdataMaxLen)
						{
							stat = S_STX;
							rxNum = 0;
						}
						else
							stat = S_ETX;
					}	
					rxNum++;	
					break;
				case S_ETX:
					if(rxNum == 3+dataLen)
					{
						if(rdata[rxNum] != ETX_CHAR)
						{
							stat = S_STX;
							rxNum = 0;
						}
						else
							stat = S_CHK;
					}
					rxNum++;
					break;
				case S_CHK:
					if(rxNum == 1+2+dataLen+1)
					{
						for(i=1, lrc=0; i<4+dataLen; i++)
							lrc ^= rdata[i];
						if(lrc != rdata[1+2+dataLen+1])
						{
							stat = S_STX;
							rxNum = 0;
						}
						else
							stat = S_END;
					}
					break;
				default:
					rxNum = 0;
					stat = S_STX;
					break;
			}
		}
		memmove(rdata, rdata +3, dataLen);
		ret = dataLen;
	}
	else {
		ret = -1;	
		printf("serial_read error: COM3 init error!\n");
	}
	Os__end_com3();
	return ret;
}

/*
 * send data to com3
 * @Return 0 on success or -1 when error
 */	
int serial_write(unsigned char *wdata, int wdataLen)
{
	int i;
	unsigned char lenBytes[2] = {0};
   	unsigned char lrc = 0;

  	if(!Os__init_com3(0x0303,0x0c00,0x0c))
 	{
		Uint2Bcd(wdataLen, lenBytes, sizeof(lenBytes));
		
		for(i=0; i<sizeof(lenBytes); i++)
			lrc ^= lenBytes[i];
		for(i=0; i<wdataLen; i++)
			lrc ^= wdata[i];
		lrc ^= ETX_CHAR;

   		Os__txcar3(STX_CHAR);                 // head 0x2
		for(i=0; i<sizeof(lenBytes); i++)     // length
			Os__txcar3(lenBytes[i]);
		for(i=0; i<wdataLen; i++)             // data
			Os__txcar3(wdata[i]);
		Os__txcar3(ETX_CHAR);                 // end 0x3
		Os__txcar3(lrc);					  // check byte
	
		Os__end_com3();
		return 0;
	}
	else
		return -1;
}

/********************* Authorization *********************/
int authorize()
{
	int ret = 0;
    unsigned char sbuf[AUTHORIZATION_LEN], rbuf[AUTHORIZATION_LEN];
    memset(sbuf, 0, AUTHORIZATION_LEN);
    memset(rbuf, 0, AUTHORIZATION_LEN);

	create_auth_info(sbuf);

    if (serial_write(sbuf, sizeof(sbuf)) < 0) {
        printf("----Linuxpos ERROR---- serial_write error!\n");
		ret = -1;
    }
    else { // write ok
        if (serial_read(rbuf, sizeof(rbuf), 6000) < 0) {
            printf("----Linuxpos ERROR---- serial_read error!\n");
            ret = -2;
        }
        else { //read ok
            if (write_auth(rbuf+2) < 0) {
                printf("----Linuxpos ERROR---- write authorization error!\n");
            	ret = -3;
            }
        }
    }
	return ret;
}
