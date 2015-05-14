#include <include.h>
#include <global.h>
#include <xdata.h>
#include "interfaceparam.h"

unsigned char INTERFACE_ConfigBasicData(BASICCONFIG *pBasicConfig)
{
	return 0;
}
unsigned char INTERFACE_ConfigKeyData(unsigned char ucKeyIndex,unsigned char *aucInData){return 0;}// 01 IMEK_MDK 02 IAEK_MDK  aucInData 16 byte  03 IMEK 04IAEK
unsigned char INTERFACE_TermInit(INTERFACEINITDATA *pInterfaceInit){return 0;}  // 强制，开机事件里面，传指针给INTERFACE
unsigned char INTERFACE_ChangeConnectState(void){return 0;}  // only for err connect  00 INTERFACE_FIRSTPOWERUP 01 INTERFACE_FIRSTCONNECT 02 INTERFACE_SUBSEQUENT
unsigned char INTERFACE_SetPublicPKToReader(unsigned char ucAddorDelFlag,unsigned char ucAddNum,VISAPK*aucInData){return 0;}// ucAddorDelFlag 0是del,1是add {return 0;} ucAddNum 是add or del的公钥个数{return 0;}aucInData 是公钥数据,按公钥的标准结构
unsigned char	INTERFACE_ConfigParamTagsToReader(unsigned char *aucInData,unsigned short usInDataLen){return 0;}  //  修改READER的配置TAG

void INTERFACE_TransToReader(unsigned long ulAmount,INTERFACETRANSINFO * InterfaceTransInfoData,DRV_OUT * aucEventOutData,unsigned char *ucEventFlag){return 0;} //交易 pInterfaceTransData   ucEventFlag 00 超时 01 IC  02 mag  04 interface
unsigned char INTERFACE_ShowStatusToReader(unsigned char ucResultData){return 0;}
unsigned char INTERFACE_ClearReaderAllPublicPK(void){return 0;}
unsigned char INTERFACE_Set_SupportAPToReader(INTERFACESUPPORTAPP *pINTERFACESupportap,unsigned char ucAPNum){return 0;}
unsigned char INTERFACE_SetDateTimeProcess(void){return 0;}
unsigned char INTERFACE_GetListPublicKeyProcess(unsigned char *ucRIDNumber,INTERFACEPUBLICPKLIST * pInterfacePublicPKData){return 0;}
unsigned char INTERFACE_GetTagValue(unsigned char *paucTag,unsigned char *aucOutData,unsigned short *usOutLen){return 0;}//TLV格式
unsigned char INTERFACE_GetTransTagValue(unsigned char *paucTag,
								unsigned char *paucTagVaule,unsigned short *uiTagVauleLen){return 0;}// 这个是取具体TAG值数据
unsigned char INTERFACE_RefurbishReaderLCD(unsigned char ucDisplayIndex){return 0;}

