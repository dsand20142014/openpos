#ifndef		_INTERFACEPARAM_H
#define		_INTERFACEPARAM_H

#ifndef		INTERFACEPARAM_DATA
#define		INTERFACEPARAM_DATA 		extern
#endif



#ifdef		__cplusplus
extern "C"
{
#endif



#define   INTERFACEBASICCONFIGDATA   "waveconf"
#define   INTERFACEKEYDATA   "wavekey"


#define IMEKMDKDEFAULTVALUE   "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
#define IAEKMDKDEFAULTVALUE   "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
#define IMEKDEFAULTVALUE      "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11"
#define IAEKDEFAULTVALUE      "\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22"



#define  INTERFACE_RIDLEN                   5
#define  INTERFACE_MAX_TAGLEN                       2
#define  INTERFACE_TAG_NUMMAX                       50
#define  INTERFACE_MAX_TAGVALUELEN                  1024
#define  INTERFACE_MESSAGE_MAXCHAR     							50
#define  INTERFACE_MAX_DEFPARAMBUFLEN  							10
#define  INTERFACE_MAX_DISPLAYLEN                   22
#define	 INTERFACE_MAXMODULLEN											248
#define	 INTERFACE_MAXEXPONENTLEN										3
#define	 INTERFACE_HASHDATALEN											20
#define  INTERFACE_MSG_MAXCHAR						          21  
#define  INTERFACE_USHORT_TYPE   									  0x01
#define  INTERFACE_UCAHR_TYPE      									0x02

#define INTERFACE_KEYARRAY_MAX                      3
#define INTERFACE_KEYINDEX_MAX                      10
#define INTERFACE_KEYVALUE_MAX                      8

#define INTERFACE_MAX_INDEXLEN 											2
#define INTERFACE_TVRLEN   5
#define INTERFACE_MAX_AUTHLEN   6
#define INTERFACE_MAXINDEX             20

#define VISA_MSI             0x10   
#define VISA_DDA_MSI         0x11
#define VISA_CDA             0x12
#define VISA_DCVV            0x13
#define VISA_VSDC            0x14
#define VISA_FDDA_VLP        0x15
#define VISA_WAVE2           0x16
#define VISA_WAVE3           0x17
#define VISA_MSD20           0x18
#define JCB_WAVE1            0x60
#define JCB_WAVE2            0x61
#define JCB_WAVE3            0x62 



#define P_MSG_TIMEOUT          0x0001    //0.5s
#define P_SALE_TIMEOUT         0x0002   // 15s
#define P_POLL_MSG             0x0003  // 30s   但是文档上面写的是10s
#define P_BUF_TIMEOUT          0x0004  // 5s
#define P_ENCRYPTION           0x0005  //  1         0x00 – TDEA is disabled, 0x01 – TDEA is enabled
#define P_DISPLAY              0x0006  /*   1        0x00 – The terminal should display all the response code from the reader.
                                                                   This is used mainly for testing and debugging purpose   0x01 – The terminal should convert the response code into meaning
                                                                    statement and display the statement*/
#define P_MAX_BUF_SIZE         0x0007   // 1024b
#define P_DOUBLE_DIP           0x0008   // 5s
#define P_READER_INDEX         0x0009  // Reader Index as defined in Chapter 2    
#define P_LANGUAGE             0x000A  // tag 5F2D + Length + Language definition + state (01: activated, 00: noactivated).
#define P_DISPLAY_S_MSG        0x000B   // 2s
#define P_DISPLAY_L_MSG        0x000C   // 5s
#define P_DISPLAY_SS_MSG       0x000D   // 10 s
#define P_DISPLAY_SR_MSG       0x000E   // 5s
#define P_DISPLAY_PIN_MSG      0x000F   // 10 s
#define P_DISPLAY_E_MSG        0x0010   // 3s

#define		INTERFACE_TRANSDATELEN				8
#define		INTERFACE_TRANSTIMELEN				6
#define INTERFACE_MAX_TRACK1LEN  76
#define INTERFACE_MAX_TRACK2LEN  19
#define INTERFACE_MAX_PINLEN       12
#define INTERFACE_MAX_TCLEN             8
#define INTERFACE_MAXAIDLEN    16




typedef struct
{
	unsigned char ulCOMType;
	unsigned long ulBaudRate;      // 115200
	unsigned char ucDebugMode;     // 00 debug 01normal  02 administrative mode
       unsigned char ucPowerUpFlag;   // 00 first powerup 01 first connect  02 subsequent
       unsigned char ucTerminalIndex[INTERFACE_MAX_INDEXLEN+1];  // \x0B\x01"
       unsigned char ucReaderIndex[INTERFACE_MAX_INDEXLEN+1]; // "\x0E\x01"
       unsigned char ucDisplayDebugDataFlag;//  显示监控信息
       unsigned char ucDisplayErrMsgData;//  显示错误信息
}BASICCONFIG;


typedef struct
{
	unsigned char	       aucTag[INTERFACE_MAX_TAGLEN];
	unsigned short 	     uiLen;
	unsigned char        aucTafValue[INTERFACE_MAX_TAGVALUELEN];
}TAGDEFINFO;


typedef struct
{
	unsigned char	ucType;
	unsigned char	ucErrCode;
	unsigned char	aucErrMsg[INTERFACE_MSG_MAXCHAR];
}EERRMSG;



typedef struct
{
	unsigned char	       aucTag[INTERFACE_MAX_TAGLEN];
	unsigned short 	     uiLen;
	unsigned char        *pTagValue;
}TAGINFO;

typedef struct
{
	unsigned char 		     ucTagNum;
	TAGINFO	                          TagInfo[INTERFACE_TAG_NUMMAX];
	unsigned short		     uiTagBuffLen;
	unsigned char 		     aucTagValue[INTERFACE_MAX_TAGVALUELEN];  
}BASICTAG;



typedef struct
{
	unsigned char	ucMessageID;
	unsigned char	aucMessageData[INTERFACE_MESSAGE_MAXCHAR];
}MESSAGE;



typedef struct
{
	unsigned short usParamIndex;
	unsigned char  ucType;
	unsigned char  ucParamLen;
	unsigned char  aucParamBuf[INTERFACE_MAX_DEFPARAMBUFLEN];
}INTERFACEPARAMDEF;


typedef struct
{
	unsigned char  ucSchemeID;
	unsigned char  aucSchemeName[INTERFACE_MAX_DISPLAYLEN];
}READER_CAPAB;


typedef struct
{
	unsigned char 		aucCVMDisplayData[INTERFACE_MAX_DISPLAYLEN];
	unsigned char     ucCVMID;
	unsigned char     ucCVMState;
}CVMINFO;

typedef	struct
{
       unsigned char               aucRID[INTERFACE_RIDLEN];
       unsigned char		       ucIndex;
	unsigned char 		ucHashInd;
	unsigned char 		ucArithInd;
	unsigned char 		ucModulLen;
	unsigned char 		aucModul[INTERFACE_MAXMODULLEN];
	unsigned char 		ucExponentLen;
	unsigned char 		aucExponent[INTERFACE_MAXEXPONENTLEN];
	unsigned char 		aucCheckSum[INTERFACE_HASHDATALEN];
}VISAPK;


typedef	struct
{
       unsigned char               aucRID[INTERFACE_RIDLEN];
       unsigned char               ucPKNumber;
       unsigned char               aucPKIndex[INTERFACE_MAXINDEX];
}INTERFACEPUBLICPKLIST;



typedef struct
{
	unsigned char aucKeyData[INTERFACE_KEYARRAY_MAX][INTERFACE_KEYINDEX_MAX][INTERFACE_KEYVALUE_MAX];
}INTERFACEKEYFILE;




typedef	struct
{
	unsigned char		EMVTVR[INTERFACE_TVRLEN];   // TVR
	unsigned char		aucTransDate[INTERFACE_TRANSDATELEN]; //YYYYMMDD
	unsigned char		aucTransTime[INTERFACE_TRANSTIMELEN]; //hhmmss
       unsigned char	       bCheckSignature;    //  是否要签名
	unsigned int		uiTransAmount;     //交易金额
	unsigned char	       unTransResult;     //交易结果
	unsigned char		bForceOnline;      //是否强制联机
	unsigned char        ucSchemeIdent;  // 配置ID
	unsigned char        ucTrack1Len;      //一 磁道长度
	unsigned char        aucTrack1Data[INTERFACE_MAX_TRACK1LEN]; //一 磁道数据
	unsigned char        ucTrack2Len;    //二 磁道长度
	unsigned char        aucTrack2Data[INTERFACE_MAX_TRACK2LEN];  //二磁道数据
	unsigned char        bCheckOnlinePin;     //  是否要输入联机PIN
	unsigned char        ucOnlinePinLen;// 联机PIN长度
	unsigned char        aucOnlinePinData[INTERFACE_MAX_PINLEN];    // 如果有数据，说明READER已经输入完，POS直接拿来用
	unsigned char        bSendTCFlag;    //是否联机发送TC
	unsigned char        aucTCData[INTERFACE_MAX_TCLEN];    // TC数据
	unsigned long        ulSpendingAmount;       // 可用的脱机金额
	unsigned char        aucIssuerAuthCode[INTERFACE_MAX_AUTHLEN];     //发卡行授权码
}INTERFACETRANSINFO;

typedef 	struct 
{
	unsigned char 	ucAIDLen;
	unsigned char 	aucAID[INTERFACE_MAXAIDLEN];        	//5-16
	unsigned char	ucASI;            			//Application Selection Indicator.0-needn't match exactly(partial match up to the length);1-match exactly
}INTERFACESUPPORTAPP;






typedef struct
{
	unsigned char   (*pWriteBasicConfigFile)(BASICCONFIG *pBasicConfig);	
	unsigned char   (*pReadBasicConfigFile)(BASICCONFIG *pBasicConfig);	
	unsigned char   (*pWriteInterfaceKeyFile)(INTERFACEKEYFILE *pInterfacekeyFile);	
	unsigned char   (*pReadInterfaceKeyFile)(INTERFACEKEYFILE *pInterfacekeyFile);
	//unsigned char   (*pInterfaceOnlineTrans)(void);
}INTERFACEINITDATA;


INTERFACEPARAM_DATA    INTERFACEINITDATA    INTERFACEINITData;
INTERFACEPARAM_DATA    INTERFACETRANSINFO  InterfaceTransData;



#ifdef		__cplusplus
}
#endif

#endif
