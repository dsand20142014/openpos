#ifndef __INTERFACE_H__
#define __INTERFACE_H__


#ifndef DATAINTERFACE
#define DATAINTERFACE extern
#endif

#include <interfaceparam.h>
#include	<oslib.h>

#ifdef		__cplusplus
extern	"C"
{
#endif




#define  IsRefNextByte(x)  	((x & 0x1F) ==0x1F)
#define  TagByteNums(x)	    IsRefNextByte(x)? 2 : 1
	
	
#define INTERFACE_STX_CHAR		0x02
#define INTERFACE_ETX_CHAR		0x03
#define INTERFACE_MSG_MAX_LEN     1024
#define INTERFACE_KEY_SUPER           "918273"
#define INTERFACEONESECOND		100
#define INTERFACE_COMTYPE_COM1    	0x01
#define INTERFACE_COMTYPE_COM2    	0x02
#define INTERFACE_MSG_TYPEZHCN	'0'
#define INTERFACE_MSG_TYPEEN		'1'
#define INTERFACE_TRYTIMES    3
#define INTERFACE_CFG_MAXDISPCHAR					16
#define INTERFACE_CFG_MAXAMOUNTLEN				10
#define INTERFACE_TAG_BUFFMAX     1024




#define INTERFACE_MAX_LANGUAGELEN      33
#define INTERFACE_MAX_SENDERLEN     2
#define INTERFACE_MAX_DATALEN   2
#define INTERFACE_MAX_RNDBLEN    8
#define INTERFACE_MAX_TEMPKEYLEN   16
#define INTERFACE_MAX_DATETIMELEN   7
#define INTERFACE_MAX_TRACK1LEN  76
#define INTERFACE_MAX_TRACK2LEN  19
#define INTERFACE_MAX_MANUFACTURER  8
#define INTERFACE_MAX_FWVERSION  4
#define INTERFACE_MAX_RFU  4
#define INTERFACE_MAX_TAGLEN   2
#define INTERFACE_MAX_DEALDATALEN   256

#define INTERFACE_MAX_INPUTOVERTIME     60




#define INTERFACE_MSG_MAXERRMSG		100                                         
#define INTERFACE_MAX_PARAMDEF            16
#define INTERFACE_MAXTERMCAPKNUMS     60


#define INTERFACE_MAX_VISAPK             100





#define ID_POLL	                                  0x07
#define ID_ECHO	                                  0x08
#define ID_SET_DEBUG_OPT	             0x10
#define ID_DEBUG_OPT_RFU	             0x11  //
#define ID_SET_PARAMETERS	             0x12
#define ID_INITIALIZE_COMM	             0x20
#define ID_MUTUAL_AUTH	                    0x21
#define ID_GEN_KEYS	                           0x22
#define ID_INVALIDATE_READER          0x23
#define ID_READY_TRANS               	      0x30  //
#define ID_RESET                     	             0x31  //
#define ID_SHOW_STATUS                    0x32  //
#define ID_SWITCH_ADMIN	             0x40
#define ID_GET_CAPAB         	             0x41
#define ID_SET_CAPAB                	      0x42
#define ID_GET_DATE_TIME                 0x43
#define ID_SET_DATE_TIME                 0x44
#define ID_GET_PARAMETERS              0x45
#define ID_GET_VISA_PK                     0x50
#define ID_SET_VISA_PK                      0x51
#define ID_GET_BAUD_RATE                0x52
#define ID_SET_BAUD_RATE                0x53
#define ID_RESET_ACQUIRER_KEY       0x54
#define ID_READER_RECOVERY             0x55
#define ID_GET_EMV_TAGS                  0x56
#define ID_SET_EMV_TAGS                  0x57
#define ID_GET_DISPLAY_MSG             0x58
#define ID_SET_DISPLAY_MSG             0x59
#define ID_GET_CVM_CAPAB               0x5A
#define ID_SET_CVM_CAPAB               0x5B
#define ID_OHTER_MSG                        0x70
#define ID_GET_JCB_PK                  	    0xC0
#define ID_SET_JCB_PK                       0xC1
#define ID_LIST_PUBLIC_KEY              0xEF
#define ID_PUBLIC_KEY                        0xED



#define RC_SUCCESS                               0x00
#define RC_DATA                        		0x01
#define RC_POLL_A                      		0x02
#define RC_POLL_P                      		0x03
#define RC_SCHEME_SUPPORTED            0x04
#define RC_SIGNATURE                   		0x05
#define RC_ONLINE_PIN                  		0x06
#define RC_OFFLINE_PIN                 	0x07
#define RC_SECOND_APPLICATION         0x08
#define RC_FAILURE                     		0xFF
#define RC_ACCESS_NOT_PERFORMED   0xFE
#define RC_ACCESS_FAILURE              	0xFD
#define RC_AUTH_FAILURE                	0xFC
#define RC_AUTH_NOT_PERFORMED       0xFB
#define RC_DDA_AUTH_FAILURE            0xFA
#define RC_INVALID_COMMAND             0xF9
#define RC_INVALID_DATA                	0xF8
#define RC_INVALID_RARAM               	0xF7
#define RC_INVALID_KEYINDEX             0xF6
#define RC_INVALID_SCHEME              	0xF5
#define RC_INVALID_VISA_CA_KEY        0xF4
#define RC_MORE_CARDS                  	0xF3
#define RC_NO_CARD                     		0xF2
#define RC_NO_EMV_TAGS                 	0xF1
#define RC_NO_PARAMETER                	0xF0
#define RC_POLL_N                      		0xEF
#define RC_Other_AP_CARDS              	0xEE
#define RC_US_CARDS                    		0xED
#define RC_NO_PIN                      		0xEC
#define RC_NO_SIG                      		0xEB
#define RC_INVALID_JCB_CA_KEY          0xEA



#define INTERFACE_SUCCESS                                 	       0x00
#define INTERFACE_ERR_CANCEL                           					0x02
#define INTERFACE_ERR_DATASEND                      					0x03
#define INTERFACE_ERR_DATARECV                       					0x04
#define INTERFACE_ERR_SEQNUMBER                    					0x05
#define INTERFACE_ERR_SAMESEQNUMBER            					0x06
#define INTERFACE_ERR_DATARECVLEN           						0x07
#define INTERFACE_ERR_DATAECHO           							0x08
#define INTERFACE_ERR_DRIVER                                                           0x09
#define INTERFACE_ERR_ucKeyType                                                     0x0A
#define INTERFACE_ERR_ucKeyIndex                                                    0x0B
#define INTERFACE_ERR_INITIALIZEKEY                                               0x0C
#define INTERFACE_ERR_KEYTYPE									0x0D
#define INTERFACE_ERR_COMMS_SENDCHAR						0x10
#define INTERFACE_ERR_COMMS_RECVCHAR						0x11
#define INTERFACE_ERR_COMMS_PROTOCOL						0x12
#define INTERFACE_ERR_COMMS_RECVTIMEOUT					       0x14
#define INTERFACE_ERR_COMMS_LRC			    					0x15
#define INTERFACE_ERR_COMMS_INITCOM							0x16
#define INTERFACE_ERR_COMMS_COMTYPE							0x17
#define INTERFACE_ERR_COMMS_LENGTH							0x18
#define INTERFACE_ERR_COMMS_BAUDRATE	    					0x1A
#define INTERFACE_ERR_CASH_PASS                     					0x20
#define INTERFACE_ERR_CASH_CHECKPASS				  		0x21
#define INTERFACE_ERR_NOSUPPORTMODE                                           0x22
#define INTERFACE_ERR_MODULUSLEN                                                  0x23
#define INTERFACE_ERR_EXPONENTLEN                                                 0x24
#define INTERFACE_ERR_VISAPKLEN                                                 	0x25
#define INTERFACE_ERR_MAXMESSAGENUM                                           0x26
#define INTERFACE_ERR_MESSAGELEN								0x27
#define INTERFACE_ERR_MAXMESSAGELEN  						        0x28
#define INTERFACE_ERR_MESSAGENOEXIST					                0x29
#define INTERFACE_ERR_TIMEOUT					       			0x2A
#define INTERFACE_ERR_MAXCVMNUM								0x30
#define INTERFACE_ERR_INPUTMAX                                                      0x31
#define INTERFACE_ERR_MAXTAGNUM								0x32
#define INTERFACE_ERR_MAXTAGBUF                                                      0x33
#define INTERFACE_ERR_TAGDATA                                                           0x34
#define INTERFACE_ERR_ADDORDELFLAG                                                          0x35
#define INTERFACE_ERR_PAINDEX                                                            0x36
#define INTERFACE_ERR_INVALIDMSGID                                                       0x37
#define INTERFACE_ERR_CONTACT                                                                0x38
#define INTERFACE_ERR_INPUTDATE                                                                0x39
#define INTERFACE_ERR_INPUTTIME                                                                0x3A
#define INTERFACE_ERR_BAUDRATE                                                                0x3B
#define INTERFACE_ERR_CAPABILITY                                                                0x3C
#define INTERFACE_ERR_MESSAGEID                                                                0x3D
#define INTERFACE_ERR_CVMID                                                                0x3E
#define INTERFACE_ERR_TAGPARAM                                                         0x3F
#define INTERFACE_ERR_TAGEXIST                                                   0x40
#define INTERFACE_ERR_TAGNOTEXIST                                                   0x41
#define INTERFACE_ERR_RECVOVERLEN                                                   0x42



#define INTERFACE_DEBUG       0x00
#define INTERFACE_NORMAL     0x01
#define INTERFACE_ADMIN       0x02

#define INTERFACE_FIRSTPOWERUP      0x00
#define INTERFACE_FIRSTCONNECT      0x01
#define INTERFACE_SUBSEQUENT        0x02

#define INTERFACE_KEYINITINDEX      0x00




#define KEYARRAY_InitialKey					      0x00
#define KEYINDEX_InitialIMEKKey	       		0x00
#define KEYINDEX_InitialIMEKSessionKey	  0x02
#define KEYINDEX_InitialIAEKKey	      		0x04
#define KEYINDEX_InitialIAEKSessionKey	  0x06

#define KEYARRAY_IMEKKey			          0x01
#define KEYINDEX_IMEKKey	              0x00
#define KEYINDEX_IMEKSessionKey	        0x02
#define KEYINDEX_MEKKey	                0x04
#define KEYINDEX_MEKSessionKey	        0x06
#define KEYINDEX_MsessionKey	          0x08

#define KEYARRAY_IAEKKey			          0x02
#define KEYINDEX_IAEKKey	              0x00
#define KEYINDEX_IAEKSessionKey	        0x02
#define KEYINDEX_AEKKey	                0x04
#define KEYINDEX_AEKSessionKey	        0x06
#define KEYINDEX_AsessionKey	          0x08

#define KEYARRAY_IMEKKey1	       	      0x03
#define KEYARRAY_IAEKKey1       		    0x04


#define INTERFACE_TRANSTIMEOUT             30
#define INTERFACE_TRANSTRYTIMES             3



#define KEYTYPE_IMEKMDK	                0x00
#define KEYTYPE_IMEK   	                0x01
#define KEYTYPE_MEK	             		    0x02
#define KEYTYPE_MSESSION	              0x03
#define KEYTYPE_IAEKMDK	                0x04
#define KEYTYPE_IAEK	             	    0x05
#define KEYTYPE_AEK	                    0x06
#define KEYTYPE_ASESSION	              0x07

#define min(a, b)           (((a) < (b)) ? (a) : (b))
#define max(a, b)           (((a) > (b)) ? (a) : (b))





typedef struct
{
	unsigned short usMsgTimeout;
	unsigned short usSaleTimeout;
	unsigned short usPollMsg;
	unsigned short usBufTimeout;
	unsigned char ucEncryption;
	unsigned char ucDisplay;
	unsigned short usMaxBufZize;
	unsigned short usDoubleDip;
	unsigned char  aucReaderIndex[INTERFACE_MAX_SENDERLEN];
	unsigned short usDisplaySmsg;
	unsigned short usDisplayLmsg;
	unsigned short usDisplaySSmsg;
	unsigned short usDisplaySRmsg;
	unsigned short usDisplayPINmsg;
	unsigned short usDisplayEmsg;
	unsigned char aucLanguage[INTERFACE_MAX_LANGUAGELEN];
}INTERFACEPARAMETERDATA;






typedef struct
{
	unsigned char ucSTX;
	unsigned char ucSeqNumber;
	unsigned char aucSenderIndex[INTERFACE_MAX_SENDERLEN];
	unsigned char aucReceiverIndex[INTERFACE_MAX_SENDERLEN];
	unsigned char ucInstructionID;
	unsigned char aucDataLength[INTERFACE_MAX_DATALEN];
}FIXEDHEADERDATA;

typedef struct
{
	unsigned char ucReaderResponseCode; 
	unsigned char ucDebugMode;
	unsigned char ucAdminMode;
	unsigned char aucRNDB[INTERFACE_MAX_RNDBLEN];
	unsigned char aucRNDR[INTERFACE_MAX_RNDBLEN];
	unsigned char ucKeyType;
	unsigned char ucKeyIndex;
	unsigned char aucTempKey[INTERFACE_MAX_TEMPKEYLEN];
	unsigned long ulAmount;
	unsigned char aucManufacturer[INTERFACE_MAX_MANUFACTURER];
	unsigned char aucFWVersion[INTERFACE_MAX_FWVERSION];
	unsigned char aucRFU[INTERFACE_MAX_RFU];
	unsigned char ucGetVISAPKNoFlag; 
	unsigned char ucAlreadySendTimes;
	unsigned char ucTagDisplayFlag;
	unsigned char ucSendRecoveryFlag;
	unsigned char ucPOLLSendTimes;
	unsigned char ucPOLLTimeFlag;
	unsigned long ulTraceNumber;
	unsigned long ulBatchNumber;
	unsigned long ulsrandTraceNumber;
	unsigned char ucAutoFlag;
	unsigned char ucGlobalSeqNumber;
	unsigned char ucSeqNumberAddFlag;
	unsigned char ucVISAPKNum;
	unsigned char aucVISAPKIndex[INTERFACE_MAX_VISAPK];
	unsigned char ucClearTagBufFlag;
	unsigned char ucPublicPKFlag;
	unsigned char ucRIDNumber;
	INTERFACEPUBLICPKLIST * pInterfacepublicList;
}INTERFACERUNDATA;



typedef	struct
{
  unsigned char		        ucIndex;
	unsigned long           ulBaudRate;
}BAUDRATE;







DATAINTERFACE  INTERFACERUNDATA InterfaceRunData;
DATAINTERFACE  INTERFACEPARAMETERDATA   InterfaceParamData; 









#define       PARAMDATA_usMsgTimeout           InterfaceParamData.usMsgTimeout
#define       PARAMDATA_usSaleTimeout          InterfaceParamData.usSaleTimeout
#define       PARAMDATA_usPollMsg          	    InterfaceParamData.usPollMsg
#define       PARAMDATA_usBufTimeout            InterfaceParamData.usBufTimeout
#define       PARAMDATA_ucEncryption             InterfaceParamData.ucEncryption
#define       PARAMDATA_ucDisplay                   InterfaceParamData.ucDisplay
#define       PARAMDATA_usMaxBufZize            InterfaceParamData.usMaxBufZize
#define       PARAMDATA_usDoubleDip              InterfaceParamData.usDoubleDip
#define       PARAMDATA_aucReaderIndex         InterfaceParamData.aucReaderIndex
#define       PARAMDATA_usDisplaySmsg           InterfaceParamData.usDisplaySmsg
#define       PARAMDATA_usDisplayLmsg           InterfaceParamData.usDisplayLmsg
#define       PARAMDATA_usDisplaySSmsg         InterfaceParamData.usDisplaySSmsg
#define       PARAMDATA_usDisplaySRmsg         InterfaceParamData.usDisplaySRmsg
#define       PARAMDATA_usDisplayPINmsg       InterfaceParamData.usDisplayPINmsg
#define       PARAMDATA_usDisplayEmsg           InterfaceParamData.usDisplayEmsg
#define       PARAMDATA_aucLanguage              InterfaceParamData.aucLanguage



#define       INTERFACERUNDATA_ucReaderResponseCode           InterfaceRunData.ucReaderResponseCode
#define       INTERFACERUNDATA_ucDebugMode           		        InterfaceRunData.ucDebugMode
#define       INTERFACERUNDATA_ucAdminMode           		        InterfaceRunData.ucAdminMode
#define       INTERFACERUNDATA_aucRNDB           		 		 InterfaceRunData.aucRNDB
#define       INTERFACERUNDATA_aucRNDR           		               InterfaceRunData.aucRNDR
#define       INTERFACERUNDATA_ucKeyType           		        InterfaceRunData.ucKeyType
#define       INTERFACERUNDATA_ucKeyIndex           		        InterfaceRunData.ucKeyIndex
#define       INTERFACERUNDATA_aucTempKey           		        InterfaceRunData.aucTempKey
#define       INTERFACERUNDATA_ulAmount           		               InterfaceRunData.ulAmount
#define       INTERFACERUNDATA_aucManufacturer                      InterfaceRunData.aucManufacturer
#define       INTERFACERUNDATA_aucFWVersion               		InterfaceRunData.aucFWVersion
#define       INTERFACERUNDATA_aucRFU                			InterfaceRunData.aucRFU
#define       INTERFACERUNDATA_ucGetVISAPKNoFlag                	InterfaceRunData.ucGetVISAPKNoFlag
#define       INTERFACERUNDATA_ucAlreadySendTimes              InterfaceRunData.ucAlreadySendTimes
#define       INTERFACERUNDATA_ucTagDisplayFlag                    InterfaceRunData.ucTagDisplayFlag
#define       INTERFACERUNDATA_ucSendRecoveryFlag              InterfaceRunData.ucSendRecoveryFlag
#define       INTERFACERUNDATA_ucPOLLSendTimes                  InterfaceRunData.ucPOLLSendTimes
#define       INTERFACERUNDATA_ucPOLLTimeFlag                     InterfaceRunData.ucPOLLTimeFlag
#define       INTERFACERUNDATA_ulTraceNumber                     InterfaceRunData.ulTraceNumber
#define       INTERFACERUNDATA_ulBatchNumber                     InterfaceRunData.ulBatchNumber
#define       INTERFACERUNDATA_ulsrandTraceNumber                     InterfaceRunData.ulsrandTraceNumber
#define       INTERFACERUNDATA_ucAutoFlag                    InterfaceRunData.ucAutoFlag
#define       INTERFACERUNDATA_ucGlobalSeqNumber                     InterfaceRunData.ucGlobalSeqNumber
#define       INTERFACERUNDATA_ucSeqNumberAddFlag                   InterfaceRunData.ucSeqNumberAddFlag
#define       INTERFACERUNDATA_ucVISAPKNum                     InterfaceRunData.ucVISAPKNum
#define       INTERFACERUNDATA_aucVISAPKIndex                   InterfaceRunData.aucVISAPKIndex
#define       INTERFACERUNDATA_ucClearTagBufFlag                  InterfaceRunData.ucClearTagBufFlag
#define       INTERFACERUNDATA_ucPublicPKFlag                  InterfaceRunData.ucPublicPKFlag
#define       INTERFACERUNDATA_ucRIDNumber                  InterfaceRunData.ucRIDNumber
#define       INTERFACERUNDATA_pInterfacepublicList                  InterfaceRunData.pInterfacepublicList



unsigned char INTERFACE_ConfigBasicData(BASICCONFIG *pBasicConfig); // 修改串口,波特率
unsigned char INTERFACE_ConfigKeyData(unsigned char ucKeyIndex,unsigned char *aucInData);// 01 IMEK_MDK 02 IAEK_MDK  aucInData 16 byte  03 IMEK 04IAEK
unsigned char INTERFACE_TermInit(INTERFACEINITDATA *pInterfaceInit);  // 强制，开机事件里面，传指针给INTERFACE
unsigned char INTERFACE_ChangeConnectState(void);  // only for err connect  00 INTERFACE_FIRSTPOWERUP 01 INTERFACE_FIRSTCONNECT 02 INTERFACE_SUBSEQUENT
unsigned char INTERFACE_SetPublicPKToReader(unsigned char ucAddorDelFlag,unsigned char ucAddNum,VISAPK*aucInData);// ucAddorDelFlag 0是del,1是add ; ucAddNum 是add or del的公钥个数;aucInData 是公钥数据,按公钥的标准结构
unsigned char	INTERFACE_ConfigParamTagsToReader(unsigned char *aucInData,unsigned short usInDataLen);  //  修改READER的配置TAG

void INTERFACE_TransToReader(unsigned long ulAmount,INTERFACETRANSINFO * InterfaceTransInfoData,DRV_OUT * aucEventOutData,unsigned char *ucEventFlag); //交易 pInterfaceTransData   ucEventFlag 00 超时 01 IC  02 mag  04 interface
unsigned char INTERFACE_ShowStatusToReader(unsigned char ucResultData);
unsigned char INTERFACE_ClearReaderAllPublicPK(void);
unsigned char INTERFACE_Set_SupportAPToReader(INTERFACESUPPORTAPP *pINTERFACESupportap,unsigned char ucAPNum);
unsigned char INTERFACE_SetDateTimeProcess(void);
unsigned char INTERFACE_GetListPublicKeyProcess(unsigned char *ucRIDNumber,INTERFACEPUBLICPKLIST * pInterfacePublicPKData);
unsigned char INTERFACE_GetTagValue(unsigned char *paucTag,unsigned char *aucOutData,unsigned short *usOutLen);//TLV格式
unsigned char INTERFACE_GetTransTagValue(unsigned char *paucTag,
								unsigned char *paucTagVaule,unsigned short *uiTagVauleLen);// 这个是取具体TAG值数据
unsigned char INTERFACE_RefurbishReaderLCD(unsigned char ucDisplayIndex);


								
#ifdef		__cplusplus
}
#endif


#endif


