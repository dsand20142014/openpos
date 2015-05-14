#ifndef COMMS_ENCRYPT
#define COMMS_ENCRYPT

//#include "oslib.h"

//#define NET_DEBUG
 
#define NETCOMMS_KEYNUM			10
#define NETWORK_MAXDATALEN      1024


#define NETCOMMS_SUCCESS            0x00         
#define ERR_NETCOMMS_VERSION		0x01			/*Í¨Ñ¶°æ±¾´í*/
#define ERR_NETCOMMS_OVERBUFLIMIT   0x02            /*»º´æ³¬ÏÞ*/
#define ERR_NETCOMMS_INDEXERROR     0x03            /*ÃÜÔ¿Ë÷Òý´í*/
#define ERR_NETCOMMS_MACERROR       0x8D            /*MAC´í*/

unsigned char GetWorkKey(unsigned char aucRanNum[8]);
unsigned char CalcNetworkMAC(unsigned char *pucInData,
				             unsigned short uiInLen,
				             unsigned char *pucOutMAC);
unsigned char DataEncrypt(unsigned char *pucInData, unsigned short usInDataLen, 
						  unsigned char aucTerminalID[8],
                          unsigned char *pucOutData, unsigned short *pusOutDataLen);
unsigned char DataDecrypt(unsigned char *pucInData, //unsigned short usInDataLen,
                          unsigned char *pucOutData, unsigned short *pusOutDataLen) ;
void COMMSENCRYPT_ErrMsg(unsigned char ucResult);  
unsigned char COMMSENCRYPT_SetVersion(unsigned char *pucInData);
#endif                        
