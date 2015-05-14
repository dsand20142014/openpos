/********************************************************
 * Copyright 2013-2015 Sand, All Right Reserved
 * sand_pci.h
 * sand API for PCI
 * Created: 20130604 XiangLiu <xiang.l@sand.com.cn>
 * File history :
 *
 ********************************************************/
 
#ifndef SAND_PCI_H
#define SAND_PCI_H

#define OK 0
#define AUTHORIZATION_LEN   24
#define SER_ID_GET_AUTH 	0x15 //POS获取授权指令
#define SER_ID_AUTH     	0x16 //POS授权指令
#define STX_CHAR            (char)0x02
#define ETX_CHAR            (char)0x03

enum RxStat
{
	S_STX,
	S_DATALEN,
    S_ETX,
    S_CHK,
    S_END
};

/**
 * @fn int check_auth()
 * @brief Check authorization of POS
 * @Param
 * @return Return 0 if authorized ok, or -1 when no authorization or error
 **/
int check_auth();
int checkAuth(char *info, int infoLen, char *authData);

/*
 * @fn int authorize()
 * @brief Get authorization
 * @Return: 0 on success or -1 when error.
 */
int authorize();

/*
 * read data from com3
 * rdata: data received from com3 
 * rdataMaxLen: length of receive data (must <= sizeof(rdata))  example: if want receiving 32bytes, rdataMaxLen > 32+8
 * timeout: multiple of 10ms
 */
int serial_read(unsigned char *rdata, unsigned int rdataMaxLen, unsigned short timeout);

/*
 * send data to com3
 * wdata: data will send to
 * wdataLen: length of data (must <= sizeof(wdata))
 */
int serial_write(unsigned char *wdata, int wdataLen);

#endif
