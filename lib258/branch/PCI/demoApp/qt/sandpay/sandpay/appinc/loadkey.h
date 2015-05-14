#ifndef LOADKEY_ABKEY
#define LOADKEY_ABKEY


#include <oslib.h>
#include <osicc.h>
//#include <typelib.h>

#define LOADKEY_MAX_KEY_NUMBER  		15
#define LOADKEY_ONESECOND				100

#define LOADKEY_SUCCESS 					0x00
#define LOADKEY_ERR_CANCEL					0x02
#define LOADKEY_ERR_DRIVER					0x04
#define LOADKEY_ERR_LOAD        			0x0B
#define LOADKEY_ERR_NOPIN					0x13
#define LOADKEY_ERR_CARDLOCK				0x45
#define LOADKEY_ERR_NOTSAFT					0x47
#define LOADKEY_ERR_FILENOTFOUND			0x48
#define LOADKEY_ERR_RECORDNOTFOUND			0x49
#define LOADKEY_ERR_NOTUSEFUNC				0x4A
#define LOADKEY_ERR_LESSBALANCE				0x4B
#define LOADKEY_ERR_FULLCOUNT				0x4C
#define LOADKEY_ERR_NOKEYINDEX				0x4D
#define LOADKEY_ERR_MACERROR				0x4F
#define LOADKEY_ERR_CASENOTFULL				0x50
#define LOADKEY_ERR_KNOWERROR				0x51

#define LOADKEY_DRV_MMI								0xab
#define LOADKEY_DRV_PAD								0xba
/* Smart Card */
#define LOADKEY_SMART_OK				0		/* Return Code Ok  0x00 */
#define LOADKEY_SMART_ASYNC				1       /* Synchronous Ok  0x01 */
#define LOADKEY_SMART_SYNC				2       /* Asynchronous Ok 0x02 */
#define LOADKEY_SMART_DRIVER_ERROR		3       /* Driver Error    0x03 */
#define LOADKEY_SMART_CARD_ERROR		4       /* Card Error      0x04 */
#define LOADKEY_SMART_OWERFLOW			5       /* Owerflow Buffer 0x05 */
#define LOADKEY_SMART_ABORT				6       /* Abort Key       0x06 */
#define LOADKEY_SMART_ABSENT			7		/* Smart Absent    0x07 */
#define LOADKEY_SMART_PRESENT           8		/* Smart Present   0x08 */

/* Smart Async Order Type */
#define LOADKEY_TYPE0					0		/* Order Type0 LgIn,BufIn                     */
#define LOADKEY_TYPE1					1		/* Order Type1 CLA,INS,P1,P2                  */
#define LOADKEY_TYPE2					2		/* Order Type2 CLA,INS,P1,P2,LgOut            */
#define LOADKEY_TYPE3					3		/* Order Type3 CLA,INS,P1,P2,LgIn,BufIn       */
#define LOADKEY_TYPE4					4		/* Order Type4 CLA,INS,P1,P2,LgIn,BufIn,LgOut */

typedef struct
{
	unsigned char ucCla;
	unsigned char ucIns;
	unsigned char ucP1;
	unsigned char ucP2;
	unsigned char ucP3;
	unsigned char aucDataIn[128];
}LOADKEY_ISO7816IN;

typedef struct
{
	unsigned short	uiLen;
	unsigned char	ucStatus;
	unsigned char	ucSW1;
	unsigned char	ucSW2;
	unsigned char	aucData[255];
}LOADKEY_ISO7816OUT;

typedef struct
{
	int  			PBOCErrCode;                        /*错误代码信息*/
	unsigned char 	ErrCode;
	unsigned char 	ErrType;
}LOADKEYERRINF;


typedef struct
{
	unsigned char iKeyNumber;					/*当前存放密钥的组数*/
	unsigned char iKeyType;					/*当前密钥属性：单DES、三DES*/
	unsigned char uc3DKey[LOADKEY_MAX_KEY_NUMBER][16];   /*存放3DES的密钥*/
	unsigned char ucRandom[LOADKEY_MAX_KEY_NUMBER][8];	/*存放随机数*/
	unsigned char ucMAC[LOADKEY_MAX_KEY_NUMBER][8];     /*存放随机数加密*/
}LOADKEY_ABCARDDATA;	


unsigned char LOADKEY_CheckCardErr(unsigned char Sw1,unsigned char Sw2);
unsigned char LOADKEY_CheckKeyMac(unsigned char ucKeyarray,
								  unsigned char ucKeyindex,
								  unsigned char ucStartAddr,
								  unsigned char ucKeyNumber);
unsigned char LOADKEY_ISOCheckReturn(unsigned char ucReader);
unsigned char LOADKEY_ISOGetResponse(unsigned char ucReader,
									 unsigned char ucLen);
unsigned char LOADKEY_ISOReadBinarybySFI(unsigned char ucReader,
										 unsigned char ucSFI,
										 unsigned char ucOffset,
										 unsigned char ucInLen);
unsigned char LOADKEY_ISOReadCard(unsigned char ucCardType);
unsigned char LOADKEY_ISOSelectFile(unsigned char ucReader,
									unsigned char *pucFileName,
									unsigned char ucFileNameLen);
unsigned char LOADKEY_LoadABKey(unsigned char ucDevice,
//								unsigned char ucPinpadFlag,
								unsigned char ucKeyArray,
								unsigned char ucKeyIndex,
								unsigned char ucStartAddr,
								unsigned char ucKeyNumber);
void LOADKEY_SetIso7816Out(void);
unsigned char LOADKEY_SMART_ISO(
			unsigned char ucReader,
			unsigned char ucCardType,
			unsigned char* pucIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut,
            unsigned char ucOrderType);
unsigned char LOADKEY_UTIL_Input(unsigned char ucLine,unsigned char ucClrFlag,
								 unsigned char ucMin, unsigned char ucMax,
								 unsigned char ucType,
								 unsigned char *pucBuf,
								 unsigned char *pucMask);
unsigned char LOADKEY_UTIL_Input_pp(unsigned char ucLine,unsigned char ucClrFlag,
									unsigned char ucMin, unsigned char ucMax,
									unsigned char ucType,
									unsigned char *pucBuf);
unsigned char LOADKEY_VerifyPin(unsigned char ucReader,unsigned char ucLen,unsigned char *Pin_Data);
unsigned int LOADKEY_WaitReadCard(void);

unsigned char LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucDecryptKeyIndex,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData);
unsigned char LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData);
unsigned char LOADKEY_PINPAD_SendReceivePinpad(void);

unsigned char LOADKEY_LoadTransKey(unsigned char ucDevice,
                                   unsigned char ucKeyarray,
                                   unsigned char ucKeyindex);
unsigned char LOADKEY_LoadingKey(unsigned char *pucInData,
                              unsigned char ucKeyarray,
                              unsigned char ucKeyindex);
unsigned char LOADKEY_ReadTransKeyCard(unsigned char *pucOutData);   

unsigned char LOADKEY_LoadWeiKangTransKey(unsigned char ucDevice,
                                   unsigned char ucKeyarray,
                                   unsigned char ucKeyindex,
                                   unsigned char ucArrayNum);  
unsigned char LOADKEY_LoadWeiKangTransKeyEx(void);
unsigned char LOADKEY_GetTransKey(unsigned char *pucKeyData,unsigned char *pucTransKey);                                                                 
#endif
