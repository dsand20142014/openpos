#include <osmain.h>

#define GUI_DELAY_TIMEOUT 300
#define GUI_COMM_TIMEOUT 50
#define GUI_PROJECT
#define GUI_TTS_DATA


#define CURRENT_PRJ		"M172-MSG-MPT4K-001"
#define CURRENT_VER		"1.0.0"
#define CURRENT_DATE		"120329"
#define COM_VERSION		(uchar *)"00001"
#define APPID_MEMBERCARD		"M094012F"
#define APPID_SMARTCARD		"M066010B"
#define APPID_WKCARD		"M107011E"
#define CASH_PROTOCOL_VER	"100"

#define APP_ID	194


#define BaseSize	(5*1024)
/********************************************前台错误信息显示**********************/
#define SUCCESS						0x00
#define SUCCESS_TRACKDATA			0x00
#define ERR_END						0x01
#define ERR_CANCEL					0x02
#define ERR_DRIVER					0x03
//#define ERR_DATASAVE				0x03
#define ERR_READWRITECARD			0x04
#define ERR_OSFUNC					0x05
#define ERR_NOTPROC					0x06
#define ERR_AMOUNT					0x07
#define ERR_SYSPASS					0x08
#define ERR_DIAL					0x09
#define ERR_HOSTCODE				0x0A
#define ERR_DATAINVALID				0x0B
#define ERR_INVALIDXDATAPAGE		0x0C
//#define ERR_INVALIDTRANS			0x0D
#define ERR_INITCARD				0X0D
#define ERR_TRANSFILEFULL			0x0E
//#define ERR_NOPSAM					0x0F
#define ERR_NOTVALIDPSAM			0x10
#define ERR_NOTLOGON				0x11
#define ERR_NOTINIT					0x12
#define ERR_INBLACKLIST				0x13
#define ERR_NOTINZONE				0x14
#define ERR_NOTRANS					0x15
#define ERR_TERMINALID				0x16
#define ERR_CARDNOFUN				0x17
#define ERR_OVERAMOUNT				0x18
#define ERR_ZONECODE				0x19
#define ERR_TRACENO					0x1A
#define ERR_CARDUNTREAT				0x1B
#define ERR_LOCKEDCARD				0x1C
#define ERR_CARDRECYCLE				0x1D
#define ERR_EXPIREDATE				0x1E
#define ERR_CODE					0x1F
#define ERR_CARDWRITE				0x20
#define ERR_CARD					0x21
#define ERR_CARDSTATUS				0x22
#define ERR_NOTCASHIERLOGON		0x23
#define ERR_ALREADYVOID			0x24
#define ERR_CARDSUPPORT					0x25
#define ERR_DIFPT					0x26
#define ERR_ONLINECARD				0x27
#define ERR_CARDTYPE				0x28
#define ERR_AMOUNTNOTZERO					0x29
#define ERR_DISCOUNT					0x2A
#define ERR_CARDMACERROR				0x2B
#define ERR_HANDNOPROC				0x2C
#define ERR_NOPSAM					0x2D
#define ERR_DIFFCARDTYPE					0x2E
#define ERR_AMOUNTERR					0x2F
#define ERR_TRANSAGAIN					0x30
#define ERR_TRANSTIMEOUT			0x31
#define ERR_EXPNOFEE				0x32
#define ERR_EXPNOPARAM				0x33
#define ERR_OFFLINECARD				0x34

#define ERR_WK_MIFARE				0x35//读巍康卡返回的错

//#define ERR_POWERFAIL				0x40
#define ERR_READMEMORY				0x41
#define ERR_WRITEMEMORY				0x42
#define ERR_CHECKSECRET				0x43
#define ERR_MODIFYSECRET			0x44
#define ERR_CARDLOCK				0x45
#define ERR_CARDCRC					0x46
#define ERR_MAXBLACK				0x47
#define ERR_MAXCARDTABLE			0x48
#define ERR_MAXZONE					0x49
#define ERR_MAXRATE				0x4A
#define ERR_CARDAMOUNTCRC			0x4B
#define ERR_DIFFCARDNO			0x4C
#define ERR_NOTSAFT					0x50
#define ERR_FILENOTFOUND			0x51
#define ERR_RECORDNOTFOUND			0x52
#define ERR_NOTSUCHAPP				0x53
#define ERR_FULLCOUNT				0x54
#define ERR_NOKEYINDEX				0x55
#define ERR_NETCOMMS_INDEXERROR     0x55           /*密钥索引错*/
#define ERR_MACERROR				0x56
#define ERR_CASENOTFULL				0x57
#define ERR_UNKNOWOPERATCODE				0x58
#define ERR_DETAILDIF				0x59
#define ERR_MEDIADIFFRENT				0x5A
#define ERR_NOTDRIVERCARD				0x5B
#define ERR_PACKERROR				0x5C
#define ERR_DEVALCARD				0x5D
#define ERR_TRANSERROR				0x5E
#define ERR_NOTINSYS				0x5F
#define ERR_DISTRUST				0x60
#define ERR_KEYVERIFY				0x61
#define ERR_NOTUNACT					0x62
#define ERR_CASH_EXIST				0x63
#define ERR_CASH_NOTEXIST			0x64
#define ERR_CASH_PASS				0x65
#define ERR_CASH_FILEFULL			0x66
#define ERR_CASH_NOTLOGON			0x67
#define ERR_CASH_ALREADYLOGON		0x68
#define ERR_UNKNOWTRANSTYPE	0x69
#define ERR_PINPADVERSION	0x6A
#define ERR_MAXTESTAMOUNT	0x6B
#define ERR_TESTTRANSFER	0x6C
#define ERR_TRANSCARDDIF				0x6D

#define ERR_CREDITFAIL				0x71
#define ERR_LESSBALANCE				0x72

#define ERR_APP_TIMEOUT					0xFF

#define ERR_ISO83						0x80
#define ERR_ISO8583_INVALIDVALUE		0x81
#define ERR_ISO8583_INVALIDBIT			0x82
#define ERR_ISO8583_INVALIDPACKLEN		0x83
#define ERR_ISO8583_OVERBITBUFLIMIT		0x84
#define ERR_ISO8583_BITNOTEXIST			0x85
#define ERR_ISO8583_INVALIDPARAM		0x86
#define ERR_ISO8583_NODATAPACK			0x87
#define ERR_ISO8583_OVERBUFLIMIT		0x88
#define ERR_NETCOMMS_OVERBUFLIMIT   	0x88            /*缓存超限*/
#define ERR_ISO8583_INVALIDBITATTR		0x89
#define ERR_ISO8583_UNPACKDATA			0x8A
#define ERR_ISO8583_SETBITLEN			0x8B
#define ERR_ISO8583_COMPARE				0x8C
#define ERR_ISO8583_MACERROR			0x8D
#define ERR_NETCOMMS_MACERROR       	0x8D            /*MAC错*/
#define ERR_ISO8583_INVALIDLEN			0x8E
#define ERR_VOIDDEPOSITAMOUNT	    		0x8F
#define ERR_NOFUNCTION    		0x90
#define ERR_DIFFERENT_AMOUNT	0x91
#define ERR_UTIL_OVERBUFLIMIT			0xA0

#define ERR_MSG_INVALIDINDEX			0xB0
#define ERR_MSG_INVALIDVALUE			0xB1
#define ERR_RECYCLE			0xB2

#define ERR_MAG_TRACKDATA				0xC0

//#define ERR_TRANS_UNKNOWNACCTYPE		0xC8
//#define ERR_TRANS_SETTLE				0xC9
#define ERR_TRANS_CARDTYPE				0xCA
//#define ERR_TRANS_CANNOTADJUST			0xCB
#define ERR_COMMS_SENDCHAR				0xD0
#define ERR_COMMS_RECVCHAR				0xD1
#define ERR_COMMS_PROTOCOL				0xD2
#define ERR_COMMS_RECVBUFFOVERLIMIT		0xD3
#define ERR_COMMS_RECVTIMEOUT			0xD4
#define ERR_COMMS_LRC					0xD5
#define ERR_COMMS_ERRTPDU				0xD6
#define ERR_COMMS_NOMAC					0xD7
#define ERR_WORKSWITCH					0xD8
#define ERR_MUSTRETURN					0xD9
#define ERR_SAMECARD				0xDA
//#define ERR_CASH_EXIST				0xE0
//#define ERR_CASH_NOTEXIST			0xE1
//#define ERR_CASH_PASS				0xE2
//#define ERR_CASH_FILEFULL			0xE3
//#define ERR_CASH_NOTLOGON			0xE4
//#define ERR_CASH_ALREADYLOGON		0xE5
//#define ERR_ADMIN_CMD				0xE8

#define ERR_TRANSEMPTY				0xE6		/*交易流水为空*/
#define ERR_SETTLE_FIRST			0xE7

#define ERR_DIFFERENT_TERM			0xE9
#define ERR_DIFFERENT_TIME			0xEA
#define ERR_DIFFERENT_TRACE		0xEB
#define ERR_DIFFERENT_MONEY		0xEC
#define ERR_DIFFERENT_BALANCE		0xED
#define ERR_BAR_REPEAT		0xEF
#define ERR_CARDNOACT		0xF0

/********************************************前台错误信息显示end**********************/

/***************************后台主机错误信息显示****************************/
#define ERR_HOSTCODE_01				1	
#define ERR_HOSTCODE_02				2
#define ERR_HOSTCODE_03				3
#define ERR_HOSTCODE_04				4
#define ERR_HOSTCODE_05				5
#define ERR_HOSTCODE_06				6
#define ERR_HOSTCODE_07				7
#define ERR_HOSTCODE_08				8
#define ERR_HOSTCODE_09				9
#define ERR_HOSTCODE_12				12
#define ERR_HOSTCODE_13				13
#define ERR_HOSTCODE_14				14
#define ERR_HOSTCODE_15				15
#define ERR_HOSTCODE_19				19
#define ERR_HOSTCODE_20				20
#define ERR_HOSTCODE_21				21
#define ERR_HOSTCODE_22				22
#define ERR_HOSTCODE_23				23
#define ERR_HOSTCODE_25				25
#define ERR_HOSTCODE_30				30
#define ERR_HOSTCODE_31				31
#define ERR_HOSTCODE_33				33
#define ERR_HOSTCODE_34				34
#define ERR_HOSTCODE_35				35
#define ERR_HOSTCODE_36				36
#define ERR_HOSTCODE_37				37
#define ERR_HOSTCODE_38				38
#define ERR_HOSTCODE_39				39
#define ERR_HOSTCODE_40				40
#define ERR_HOSTCODE_41				41
#define ERR_HOSTCODE_42				42
#define ERR_HOSTCODE_43				43
#define ERR_HOSTCODE_44				44
#define ERR_HOSTCODE_51				51
#define ERR_HOSTCODE_52				52
#define ERR_HOSTCODE_53				53
#define ERR_HOSTCODE_54				54
#define ERR_HOSTCODE_55				55
#define ERR_HOSTCODE_56				56
#define ERR_HOSTCODE_57				57
#define ERR_HOSTCODE_58				58
#define ERR_HOSTCODE_59				59
#define ERR_HOSTCODE_60				60
#define ERR_HOSTCODE_61				61
#define ERR_HOSTCODE_62				62
#define ERR_HOSTCODE_63				63
#define ERR_HOSTCODE_64				64
#define ERR_HOSTCODE_65				65
#define ERR_HOSTCODE_66				66
#define ERR_HOSTCODE_67				67
#define ERR_HOSTCODE_68				68
#define ERR_HOSTCODE_69				69
#define ERR_HOSTCODE_70				70
#define ERR_HOSTCODE_71				71
#define ERR_HOSTCODE_72				72
#define ERR_HOSTCODE_73				73
#define ERR_HOSTCODE_74				74
#define ERR_HOSTCODE_75				75
#define ERR_HOSTCODE_76				76
#define ERR_HOSTCODE_77				77
#define ERR_HOSTCODE_78				78
#define ERR_HOSTCODE_79				79
#define ERR_HOSTCODE_90				90
#define ERR_HOSTCODE_91				91
#define ERR_HOSTCODE_92				92
#define ERR_HOSTCODE_93				93
#define ERR_HOSTCODE_94				94
#define ERR_HOSTCODE_95				95
#define ERR_HOSTCODE_96				96
#define ERR_HOSTCODE_97				97
#define ERR_HOSTCODE_98				98
#define ERR_HOSTCODE_99				99
#define ERR_HOSTCODE_A0				0xA0
#define ERR_HOSTCODE_Z0				0xZ0
#define ERR_HOSTCODE_Z1				0xZ1
#define ERR_HOSTCODE_Z2				0xZ2
#define ERR_HOSTCODE_Z3				0xZ3
#define ERR_HOSTCODE_Z4				0xZ4
#define ERR_HOSTCODE_Z5				0xZ5
#define ERR_HOSTCODE_Z6				0xZ6
#define ERR_HOSTCODE_Z7				0xZ7
/***************************后台主机错误信息显示end****************************/

/****************************************显示提示信息***********************/
       
#define MSG_CHANGECD			0		//换卡      
#define MSG_LOADAB				1		//新主密钥下载 
#define MSG_QUERY				2		//查询
#define MSG_TRANSKEYDOWN				3		//新主密钥下载 
#define MSG_RETRANSFER		4	//重发移资
#define MSG_PURCHASE			7       //消费                
#define MSG_ONLINEPURCHASE			8       //消费                
#define MSG_RESETCARD			9       //洗卡
#define MSG_PRELOAD				10      //预充值
#define MSG_REPSETTLE			11		//重打结算单             
#define MSG_LOCK			12		//M1锁定             
#define MSG_DETAIL			13		//M1锁定             
#define MSG_PREAUTH				14      //预授权
#define MSG_PREAUTHVOID				15      //预授权
#define MSG_PREAUTHFINISH				16      //预授权
#define MSG_PREAUTHFINISHVOID				17      //预授权
#define MSG_PREAUTHPROCESS				18     //预授权管理
#define MSG_ONLINEPURCHASE_P			19       //消费                
#define MSG_VOID_P			20      //消费                
#define MSG_LOADONLINE			21      //联机充值  
#define MSG_TRANSFER		22	//移资
#define MSG_AMOUNTINPUT		23	//输金额模式设置
#define MSG_CASHLOAD			24      //现金充值  
#define MSG_LOADBYCREDIT			25      //银行卡充值  
#define MSG_EDDEPOSIT			29      //存折存款    
#define MSG_SYSTEMINIT			33      //系统初始化                                        
#define MSG_ADDCASHIER			34     /*柜员增� */                                                                     
#define MSG_DELCASHIER			35     /*柜员� 除*/                                                                      
#define MSG_MODICASHIER			36     /*柜员修改*/
#define MSG_CASHIERNO			37     /*柜员号:*/       
#define MSG_CASHIERPASS			38     /*柜员密� �:*/     
#define MSG_CASHIERNEWPASS		39     /*柜员新密� �:*/   
#define MSG_SUPERPASS			40     /*主管密� �:*/     
#define MSG_GOON			41      /*继续?*/       
#define MSG_CANCEL			42      /*取消?*/                                                                      
#define MSG_CASH_EXIST			43      /*柜员已存在*/
#define MSG_NOTEXIST			44     /*柜员不存在*/
#define MSG_CASHIERMANAGE	45
#define MSG_CHANGESUPERPASS	46
#define MSG_WKADMIN	47
#define MSG_SMTADMIN	48
#define MSG_HYADMIN	49
#define MSG_WKCHPIN	50
#define MSG_SMTCHPIN	51
#define MSG_HYCHPIN	52
#define MSG_PTCPURCHASE		53
#define MSG_CONFIGBASEDATA		65     	//参数设置   
#define MSG_SALE				66      //卡激活
#define MSG_SALEONLINE				67     //卡激活
#define MSG_FIXEDPURCHASE				68     //定额消费
                                                            						                   
#define MSG_SETTLE				69      //结算
#define MSG_HAND				70      //手工
#define MSG_MULTSET				71      //扣款基数设置
#define MSG_COMSET				72      //串口设置
#define MSG_INPUTPIN			73		//输入个人密� �                                                                        
#define MSG_ONLINE				75		//在线操作管理    
#define MSG_INIONLIE			76		//在线初始化
#define MSG_SURFING			77		//天翼
#define MSG_POSLOGON			81     /*签到*/      
 #define MSG_POSLOGOFF			82     /*签退*/      
 #define MSG_ONLINEREFUND		83       
 #define MSG_BATCHLOAD		84           
 
    
#define MSG_TERMINALID			86     /*终端号:*/       
#define MSG_MERCHANTID			87     /*商户号:*/       
#define MSG_COMMMODE			88     /*M 1-HDLC,2-232*/   
#define MSG_DIALRETRY			89     /*"Dial Retry Time */
#define MSG_DIALWAITTIME		90       /*Dial Wait Times*/
#define MSG_COMMSPEED			91       /*Comm Speed:*/
#define MSG_RECEIVETIMEOUT		92       /*Receive Timeout*/
#define MSG_REFUND		93      
                                                 
#define MSG_TRACENUM			100      /*流水号:*/     
                                                                   
#define MSG_MERCHANTNAME		109      /*商户名称:*/
#define MSG_DIALING			110      	/*拨号中．．．*/
#define MSG_SENDING			111      	/*发送中．．．*/ 
#define MSG_RECEIVING			112      /*接收中．．．*/                                               
        
                                                                      

#define MSG_NOTRANS				123     	/*交易不存在*/
#define MSG_REPRINT				124     	/*重打交易*/
#define MSG_REPBYTRACE		125			//按流水号打印

                                                                   
#define MSG_LISTPRINT			135      /*流水明细打印*/                                                                                             
#define MSG_SYSPASS				137	  	/*系统密� �*/
                                            
#define MSG_CONFIRM			141	   /*请确认*/            

#define MSG_SWITCHBOARD			156		/*SWITCHBOARD=*/
#define MSG_TERMINALTOTAL		157		/*终端统计打印*/
#define MSG_NETTEST 					158		/*通讯线路检测*/

#define MSG_INPUT_MERCHANT_NAME 159		/*输入商户名*/
#define MSG_CLEARREVERSAL 		160	/*� 除冲正� �志*/
#define MSG_LOADDEFUALTKEY		161		/*下载默认密钥*/

#define MSG_MOBILERESEND			162		/*手机充值重发*/
#define MSG_CLEARTRANSKEY			163		/*清除密钥下载� �志*/
#define MSG_ENQURY_LIST			165		/*查询流水*/    

#define MSG_CLEARSETTLE			167		/*清结算� �志*/
#define MSG_VOID			168		/*撤销*/
#define MSG_PRINTTABLES  					169 		/*打印表*/
#define MSG_PAY			170		/*缴费*/
#define MSG_MOBILECHARGE			171		/*手机充值*/
#define MSG_PAYONLINE			172	/*联机缴费*/
#define MSG_MOBILECHARGEONLINE			173		/*联机手机充值*/
#define MSG_PAYADMIN			174		/*缴费管理*/
#define MSG_MOBILECHARGEADMIN			175		/*手机充值管理*/

#define SET_DATETIME				176   	/*日期时间设置*/	       
#define SET_PARAM  					177 		/*参数设置 */
#define SET_COMTYPE 					178 		/*数据采集 */
#define MSG_VERSION				179			//版本查询
#define SET_PRINTER					180 		/* 打印机设置 */		       
#define SET_PINPAD					181   	/*密� �键盘设置*/      
#define MSG_MOBILEADMIN			182	/*手机充值管理*/

#define MSG_TERMINALMANAG     	183   	/*终端统计*/     
#define MSG_CARDMEDIA                   184       /*卡介质选择*/
#define MSG_RATETYPE                   185       
#define MSG_REFAPL                  186       
#define MSG_VOIDREFAPL                   187      
#define MSG_SETCOMMMODE                 188      
#define MSG_SETCOM					189		/*串口设置 */
#define MSG_PAYNOBAR			190		/*� 条� �缴费*/
#define	MSG_FinComm     193       //签退 

#define	MSG_RESETPIN     194      //重置密� �
#define	MSG_CHANEEXP     195      //续期
#define MSG_CASHIERPOSLOGON			196     /*柜员签到*/      
#define MSG_CASHIERCHANGEPIN			197     /*柜员改密*/ 
#define	MSG_CHANEPIN     198      //改密

/****************************************显示提示信息end***********************/
//PBOC KEY DEFINE
#define SMCKEYINDEX_EDLOAD			0x00 

/* Global define */
#define ONESECOND						100
#define MAXMENUITEM						20
#define DATASAVEVALIDFLAG				0x55
#define DATASAVEPAGE_NORMAL				0
#define DATASAVEPAGE_TRANS				1
#define DATASAVEPAGE_BLACKLIST			2
#define EMV_MAXTAGNB					20
#define CFG_MAXDISPCHAR					16
#define CFG_MAXAMOUNTLEN				10
#define PARAM_SWITCHLEN					20
/* Waits for the end of the background dialling during 'TimeOut' x 10 ms */
#define CFG_CHECKDIALTIMEOUT			100 

/* PINPAD Key define */				
#define KEYINDEX_PURSECARD_MASTERKEY			0x00
#define KEYINDEX_PURSECARD_PROTECTKEY			36

#define KEYARRAY_PURSECARD0					14//巍康
#define KEYARRAY_PURSECARD1					14
#define KEYARRAY_PURSECARD2					14
#define KEYARRAY_PURSECARD3					15//斯玛特
#define KEYARRAY_PURSECARD4					15
#define KEYARRAY_PURSECARD8					15
#define KEYARRAY_PURSECARD5					16//+会员卡
#define KEYARRAY_PURSECARD9					16

#define KEYINDEX_PURSECARD0_PURCHASEKEY			2
#define KEYINDEX_PURSECARD0_APPCTRLKEY			4
#define KEYINDEX_PURSECARD0_APPMACKEY			6	
#define KEYINDEX_PURSECARD0_UPDATEKEY		8

#define KEYINDEX_PURSECARD1_CRCKEY			10	/*巍康新卡� �验密钥*/
#define KEYINDEX_PURSECARD1_MESSAGEKEYA			12	/*新卡交易密钥*/
#define KEYINDEX_PURSECARD1_PURCHASEKEY		14     /*新卡消费密钥*/
#define KEYINDEX_PURSECARD1_LOADMACKEY			16	/*新卡充资MAC密钥*/

#define KEYINDEX_PURSECARD2_PINKEY			18
#define KEYINDEX_WKCARD1_CRCKEY1		              20       /*卡� �验密钥1*/
#define KEYINDEX_WKCARD1_MESSAGEKEYA		22        /*信息扇区KeyA密钥*/
#define KEYINDEX_WKCARD1_PURCHASEKEY		24     /*消费密钥*/

#define KEYINDEX_PURSECARD3_PURCHASEKEY		18	/*消费密钥*/
#define KEYINDEX_PURSECARD3_LOADKEY			20	/*充值密钥*/
#define KEYINDEX_PURSECARD3_AMOUNTCHECK			22	/*斯玛特老卡金额� �验*/
#define KEYINDEX_PURSECARD3_CARDCHECK			24	/*斯玛特老卡卡� �验*/

#define KEYINDEX_PURSECARD4_CRCKEYS			10	/*斯玛特新卡� �验密钥*/
#define KEYINDEX_PURSECARD4_MESSAGEKEYAS			12	/*新卡交易密钥*/
#define KEYINDEX_PURSECARD4_PURCHASEKEY		14     /*新卡消费密钥*/
#define KEYINDEX_PURSECARD4_LOADMACKEY			16	/*新卡充资MAC密钥*/

#define KEYINDEX_PURSECARD5_CRCKEYM			10	/*会员卡新卡� �验密钥*/
#define KEYINDEX_PURSECARD5_MESSAGEKEYAM			12	/*新卡交易密钥*/
#define KEYINDEX_PURSECARD5_PURCHASEKEY		14     /*新卡消费密钥*/
#define KEYINDEX_PURSECARD5_LOADMACKEY			16	/*新卡充资MAC密钥*/

#define KEYINDEX_PURSECARD8_PURCHASEKEY			2
#define KEYINDEX_PURSECARD8_APPCTRLKEY			4
#define KEYINDEX_PURSECARD8_APPMACKEY			6	
#define KEYINDEX_PURSECARD8_UPDATEKEY		8

#define KEYINDEX_PURSECARD9_PURCHASEKEY			2
#define KEYINDEX_PURSECARD9_APPCTRLKEY			4
#define KEYINDEX_PURSECARD9_APPMACKEY			6	
#define KEYINDEX_PURSECARD9_UPDATEKEY		8

//for commsEncrypt.c
#define NETCOMMS_VERSION       "1020"
#define KEYARRAY_NETCOMMS       0x04
#define KEYINDEX_NETCOMMS_WORKKEY    0x08
#define KEYINDEX_NETCOMMS_TEMPKEY    0x09

#define NETWORK_MAXDATALEN     512
/*
#define KEYARRAY_PURSECARD					0x00
#define KEYINDEX_PURSECARD_MASTERKEY		0x00
#define KEYINDEX_PURSECARD_CASHIERKEY		0x01
#define KEYINDEX_PURSECARD_PINKEY			0x02
#define KEYINDEX_PURSECARD_MACKEY			0x03
#define KEYINDEX_PURSECARD_ISSKEY			0x04
#define KEYINDEX_PURSECARD_WRITEKEY			0x06
#define KEYINDEX_PURSECARD_CRCKEY			0x08
*/
//Flag defiine
#define FLAG_INIT_UNDO					0
#define FLAG_INIT_OK					1
#define FLAG_LOGON_UNDO					0
#define FLAG_LOGON_OK					1
#define FLAG_SETTLE_UNDO				0
#define FLAG_SETTLE_FAIL				1

/* Trans info define */
#define TRANS_DATELEN					4
#define TRANS_TIMELEN					3
#define PARAM_DATETIMELEN				7
#define TRANS_COMMLEN					40
#define TRANS_PINDATALEN				8
#define TRANS_BATCHNUMLEN				6
#define TRANS_TRACENUMLEN				6
#define BATCH_LEN						12

#define MAX_BLACK_NUM      				2000
#define BLACK_LEN						7
#define BLACKFEILD_LEN					14
#define MAX_CARDTABLE_NUM				1500
#define CARDBIN_LEN						8
#define MAX_ZONE_NUM					500
#define ZONE_LEN						3
#define ZONE_NAME_LEN				16
#define LOADTRACE_LEN					16
#define MAX_RATE_NUM					  200
#define TRANS_ISO2LEN					37
#define TRANS_ISO3LEN					104
#define MAXTESTAMOUNT					100

#define SMART_ED						0x31	//电子存折
#define SMART_EP						0x32	//电子钱包
#define CARD_CPU						0x30
#define CARD_M1						0x31
#define CARD_MAG						0x32
#define CARD_MEM						0x35

#define FLAG_EDLOAD						0x00
#define FLAG_PRELOAD					0x01	

#define TRANS_INPUTMODE_SWIPECARD		0x01
#define TRANS_INPUTMODE_MANUALINPUT		0x02
#define TRANS_INPUTMODE_INSERTCARD 		0x05

#define TRANS_TERMINALTYPE_NORMAL		0x00
#define TRANS_TERMINALTYPE_FOREIGN		0x01

#define TRANS_ACCTYPE_DEFAULT			0
#define TRANS_ACCTYPE_ED				4
#define TRANS_ACCTYPE_EP				5

#define TRANS_CARDTYPE_INTERNAL			0
#define TRANS_CARDTYPE_VISA				1
#define TRANS_CARDTYPE_MASTER			2
#define TRANS_CARDTYPE_JCB				3
#define TRANS_CARDTYPE_DINNER			4
#define TRANS_CARDTYPE_AE				5

#define TRANS_CARDTYPE_INTERNALSMART	10

#define TRANS_BATCHSEND_INTERNAL		0x01
#define TRANS_BATCHSEND_FOREIGN			0x02

#define SETTLE_TWICEBATCH				0x31
#define SETTLE_UNBALANCE				0x32

#define TRANS_S_INIONLINE				0x01
#define TRANS_S_LOGONON					0x02
#define TRANS_S_SETTLE_ONE				0x03
#define TRANS_S_PURCHASE				0x04
#define TRANS_S_EDLOAD					0x05	
#define TRANS_S_SETTLE_TWO				0x06
#define TRANS_S_RESETCARD				0x07
#define TRANS_S_PRELOAD					0x08
#define TRANS_S_TERMINPRT				0x09
#define TRANS_S_CHANGECD				0x0A
#define TRANS_S_RESETPIN				0x0B
#define TRANS_S_CHANGEEXP				0x0C
#define TRANS_ADDEXPIRE				0x0D
#define TRANS_S_QUERY			0x0E
#define TRANS_S_CASHIERLOGONON		0x10
#define TRANS_S_ONLINEPURCHASE		0x11
#define TRANS_S_VOID		0x12
#define TRANS_S_PREAUTH		0x13
#define TRANS_S_PREAUTHVOID		0x14
#define TRANS_S_PREAUTHFINISH		0x15
#define TRANS_S_PREAUTHFINISHVOID		0x16
#define TRANS_S_RETURNCARD	0x17
#define TRANS_S_PAY		0x18
#define TRANS_S_MOBILECHARGE	0x19
#define TRANS_S_PAYONLINE	0x1A
#define TRANS_S_MOBILECHARGEONLINE	0x1B
#define TRANS_S_BARCODE	0x1C
#define TRANS_S_VOIDPAYONLINE	0x1D
#define TRANS_S_ZSHPURCHASE				0x1E
#define TRANS_S_ZSHONLINEPURCHASE				0x1F
#define TRANS_S_SALE				0x20
#define TRANS_S_PURCHASE_P				0x21
#define TRANS_S_ONLINEPURCHASE_P		0x22
#define TRANS_S_VOID_P		0x23
#define TRANS_S_RETURNCARD_P	0x24
#define TRANS_S_REFUND			0x25
#define TRANS_S_TRANSFER	0x26
#define TRANS_S_TRANSPURCHASE		0x27
#define TRANS_S_LOADONLINE	0x28
#define TRANS_S_EXPPURCHASE		0x29
#define TRANS_S_REFAPL			0x2A
#define TRANS_S_VOIDREFAPL			0x2B
#define TRANS_S_PAYNOBAR		0x2C
#define TRANS_S_SALEONLINE				0x2D
#define TRANS_S_CASHLOAD	0x2E
#define TRANS_S_JSZSHONLINEPURCHASE				0x2F
#define TRANS_S_BATCHLOAD			0x30
#define TRANS_S_ONLINEREFUND		0x31
#define TRANS_S_PTCPURCHASE				0x32
#define TRANS_S_NETTEST				0x33
#define TRANS_S_TRANSFERRETURN	0x34
#define TRANS_S_POINTQUERY		0x35//????
#define TRANS_S_POINTGIFT			0x36//????
#define TRANS_S_CONVERT				0x37//????
#define TRANS_S_VOIDCONVERT		0x38//??????

#define TRANS_S_BATCHUP					0xF4
#define TRANS_S_REVERSAL				0xF5

/* comms.c */                                                           
#define COMMS_RECVTIMEOUT		100        

//PBOC
#define PBOC_CARD				0x01
#define MAX_CARD_APP			10
#define MAXMENUDISPLINE				10

/* Terminal Parameter */
#define PARAM_PASSWORDLEN				6
#define PARAM_TERMINALIDLEN				8
#define PARAM_MERCHANTIDLEN				15
#define PARAM_MERCHANTNAMELEN			40
#define PARAM_TELNUMBERLEN				30
#define PARAM_FUNCODE					60
#define PARAM_DATELEN					3
#define PARAM_DISPLAYMINLINE			2
#define PARAM_DISPLAYMAXLINE			4
#define PARAM_COMMMODE_MODEM			0
#define PARAM_COMMMODE_RS232			1
#define PARAM_COMMMODE_HDLC				2
#define PARAM_COMMMODE_GPRS				3
#define PARAM_COMMMODE_CDMA				4
#define PARAM_COMMMODE_TCPIP            5
/* Printer Parameter */
#define PRN_MAXCHAR		24

/* comms.c */
#define CHAR_STX				0x02
#define CHAR_ETX				0x03

//define cardstatus
#define CARD_NORMAL				0x01
#define CARD_LOCKED				0x02
#define CARD_RECYCLED			0x03

/* ISO8583 Define */
#define ISO8583_MSGIDLEN		2
#define ISO8583_BITMAPLEN		16
#define ISO8583_MAXBITNUM		128
#define ISO8583_MAXBITBUFLEN	1024	/* Max unpacked ISO8583 buffer */
#define ISO8583_MAXCOMMBUFLEN	512		/* Max ISO8583 communication buffer */
#define ISO8583_BITNOTEXISTFLAG 0x00
#define ISO8583_BITEXISTFLAG	0xff
#define ISO8583_BIT48NUM		9

#define ATTR_N			0x01
#define ATTR_AN			0x02
#define ATTR_NS			0x03
#define ATTR_XN			0x04
#define ATTR_ANS		0x05
#define ATTR_LLVARN		0x06
#define ATTR_LLVARNS	0x07
#define ATTR_LLVARAN	0x08
#define ATTR_LLVARANS	0x09
#define ATTR_LLLVARANS	0x0A
#define ATTR_LLVARZ		0x0B
#define ATTR_LLLVARZ	0x0C
#define ATTR_BIN		0x0D
#define ATTR_LLLVARN	0x0E

/* cash.c */
#define CASH_MAXSUPERPASSWDLEN		12
#define CASH_MAXCASHIER				10
#define CASH_CASHIERNOLEN			6
#define CASH_CASHIERPASSLEN			6
//#define CASH_LOGONFLAG				0x55

/* MSG.c */
#define MSG_TYPEZHCN	'0'
#define MSG_TYPEEN		'1'

/* serv.c*/
#define SETTLE_OK	0x00
#define SETTLE_OK_BUT_UNBALANCE  0x01

/*msg.c*/
#define ERR_APP_TIMEOUT					0xFF
#define MSG_MAXERRMSG			100                                     
#define MSG_MAXNB				200                                     
#define MSG_MAXHOSTMSG			1100
#define MSG_MAXCHAR			24+1                                      
#define MSG_MAXTYPE				2  
#define MSG_DEFAULTMSGEN		"Default\0"			       
#define MSG_DEFAULTMSGZHCN		"缺省\0"                      
#define MSG_INVALIDMSGEN		"Invalid Index\0"             
#define MSG_INVALIDMSGZHCN		"� 效\0"   
#define Display_MAXNB					18

#define TRANS_MAXNB						250

#define MSG_NULL			0

#define BLACKFEILD_LEN					14
#define ZONE_NAME_LEN				    16

/* ISO8583 Define */
#define ISO8583_MSGIDLEN		2
#define ISO8583_BITMAPLEN		16
#define ISO8583_MAXBITNUM		128
#define ISO8583_MAXBITBUFLEN	1024	
#define ISO8583_MAXCOMMBUFLEN	512		
#define ISO8583_BITNOTEXISTFLAG 0x00
#define ISO8583_BITEXISTFLAG	0xff
#define ISO8583_BIT48NUM		9

/* comms.c */                                                           
#define SMC_COMMS_RECVTIMEOUT		100                             
                                                                        
                                                                        
/* pboc.c */                                                            
#define PBOC_MAXTERMINALAPP		10                              
#define PBOC_MAXCARDAPP			5                               
#define PBOC_MAXAPPIDLEN		16                              
#define PBOC_ISSUERIDLEN		8                               
#define PBOC_APPLINOLEN			10                              
                                                                        
/* admin.c */                                                           
#define ADMIN_END				'0'                     
#define ADMIN_DOWNLOADMSG			'1'                     
#define ADMIN_DOWNLOADERRMSG		'2'                             
#define ADMIN_DOWNLOADISO8583ATT	'3'                             
                                                                        
#define ADMIN_COMMANDOFFSET			  0                     
#define ADMIN_DATAOFFSET			  1                     
                                                                        
/* print.c */                                                           
#define PRINT_MAXCHAR			50                              
#define PRINT_MAXRETRYTIME		3
#define RE_PRINT_FLAG			0x01
#define NORMAL_PRINT_FLAG		0x00
#define PRINT_LIST_FLAG			0x02
#define PRINT_TOTAL_FLAG		0x03
/* Macro */
#define min(a, b)           (((a) < (b)) ? (a) : (b))
#define MAXCHINESEINPUT		33

/* 	Types  */

#define ULONG_C51 unsigned long
#define UINT_C51 unsigned int

#define ICC1			0		
#define STX_CHAR	0x02
#define ETX_CHAR	0x03
typedef struct
{
	unsigned char ucCla;
	unsigned char ucIns;
	unsigned char ucP1;
	unsigned char ucP2;
	unsigned char ucP3;
	unsigned char aucDataIn[128];
}ISO7816IN;

typedef struct
{
	unsigned short	uiLen;
	unsigned char	ucStatus;
	unsigned char	ucSW1;
	unsigned char	ucSW2;
	unsigned char	aucData[255];
}ISO7816OUT;

typedef struct
{
	unsigned char ucTransType;
	unsigned short uiDispIndex;
	unsigned char (*pfnProc)(void);
}DISPMENU;

typedef struct
{
	DISPMENU DispMenu[MAXMENUDISPLINE];
	unsigned char ucMenuCnt;
}SELMENU;

typedef struct
{
	unsigned char	ucAppIDLen;
	unsigned char	aucAppID[16];
	unsigned char	aucAppLabel[17];
}CARDAPP;

typedef struct
{
	unsigned char 	ucAppNumber;
	CARDAPP		CardApp[MAX_CARD_APP];
	unsigned char	ucCurrAppNumber;
	unsigned char	ucTermAppNumber;
}CARDAPPFILE;

typedef struct
{
	unsigned char  ucSourceAcc[10];
	unsigned char  ucAuthCode[4];/*认证� �*/
	unsigned char  ucCardVersion[2];
	unsigned char  ucCardType;
	unsigned char  ucAreaLimitCode[4];
	unsigned char  ucIssueCode[3];/*发卡方代� �*/
	unsigned char  ucDepositTrace[8];
	unsigned char  ucMakeDate[4];
	unsigned char  ucDepositDate[4];
	unsigned char  ucExperiDate[4];
	unsigned char  ucMainAccFlag;/*主帐户� �志*/
	unsigned char  ucMemberFlag;/*会员卡� �志*/
	unsigned char  ucStatus;
	unsigned char  ucCount[4];/*计数器*/
	unsigned char  ucBakup[6];/*保留字节*/
	unsigned char  ucAmount[4];
	unsigned char  ucCheckCrc[4];
}SLE4442DATA;	
typedef struct
{
	unsigned char  aucTime[6];
	unsigned char  ucType;
	unsigned char  aucAmount[4];
	unsigned char  aucPreBalance[4];
	unsigned char  aucTid[4];
	unsigned char  aucTrace[3];
}SLE4442TRANS;

typedef struct
{
	unsigned char  ucSourceAcc[8];
	unsigned char  ucDepositTrace[7];
	unsigned char  ucCardVersion;
	unsigned char  ucCardType;
	unsigned char  ucAreaLimitCode[4];
	unsigned char  ucBakup[2];
	unsigned char  ucDepositDate[4];
	unsigned char  ucExperiDate[4];
	unsigned char  ucStatus;
	unsigned char  ucAmount[4];
	unsigned char  ucCheckCrc[4];
	SLE4442TRANS	TransRecord;
}SLE4442DATASMT;	
typedef struct
{
	unsigned char  ucSourceAcc[10];
	unsigned char  ucAuthCode[4];/*认证� �*/
	unsigned char  ucCardVersion[2];
	unsigned char  ucCardType;
	unsigned char  ucAreaLimitCode[4];
	unsigned char  ucIssueCode[3];/*发卡方代� �*/
	unsigned char  ucDepositTrace[8];
	unsigned char  ucMakeDate[4];
	unsigned char  ucDepositDate[4];
	unsigned char  ucExperiDate[4];
	unsigned char  ucMainAccFlag;/*主帐户� �志*/
	unsigned char  ucMemberFlag;/*会员卡� �志*/
	unsigned char  ucStatus;
	unsigned char  ucCount[4];/*计数器*/
	unsigned char  ucDiscount[2];/*保留字节*/
	unsigned char  ucPin[3];
	unsigned char  ucPinAct[1];
	unsigned char  ucAmount[4];
	unsigned char  ucCheckCrc[4];
	unsigned char  ucPoint[4];
	unsigned char  ucPointCheck[4];
	unsigned char  ucBakup1[7];/*保留字节*/
	SLE4442TRANS	TransRecord;
	SLE4442TRANS	LoadRecord;
}SLE4442DATASP;	
typedef struct
{
	unsigned char  ucCardVer;
	unsigned char  ucCardVer1;
	unsigned char  ucAppFlag;
	unsigned char  aucArea[4];
	unsigned char  aucIssueBatch[4];
	unsigned char  aucVerifyCode[4];
	unsigned char  aucIssueDate[3];
	unsigned char  aucCardNo[10];
	unsigned char  aucIssuerCode[3];
	unsigned char  aucCardVer[2];
	unsigned char  ucFlatFlag;
	unsigned char  aucUID[4];
}M1DATA18;	
typedef struct
{
	unsigned char  aucExpDate[4];
	unsigned char  aucCount[2];
	unsigned char  ucPoint;
	unsigned char  ucState;
	unsigned char  ucMemberFlag;
	unsigned char  ucAccountFlag;
	unsigned char  aucLoadTrace[8];
	unsigned char  aucBalance[4];
	unsigned char  aucLoadAmount[4];
	unsigned char  aucLoadMac[4];
	unsigned char  aucCrc[2];
}M1DATA19;	
/* Normal Transaction Information */
typedef struct
{	
	unsigned char       aucDateTime[7];			//交易日期时间
	unsigned char		ucCardType;				//卡类型
	unsigned char 	ucTransType;			//交易类型
	unsigned char       aucHostDateTime[7];			//主机下发时间用来� �验MAC2      
	unsigned char       aucUserCardNo[10];		//卡号	(也称应用序列号)
	unsigned char       aucUserName[20];		//姓名
   	unsigned char       ucBonusflag;			//返利� �志
	unsigned char       aucBakup[19];		//预留
   	unsigned char       ucYhflag;			//一嗨� �志
	unsigned char		aucOldCardNo[10];		//旧卡卡号
	unsigned char		ucUserCardLen;			//卡号长度
	ULONG_C51		ulMaxAmount;
	ULONG_C51		ulTrueBalance;
	unsigned char     	aucExpiredDate[4];		//应用有效期
	unsigned char     	aucNewExpiredDate[4];		//应用新有效期
	ULONG_C51  		ulAmount;				//交易金额 
	ULONG_C51		ulPrevEDBalance;		//交易前余额
	ULONG_C51		ulAfterEDBalance;		//交易后余额
	ULONG_C51		ulYaAmount;				//押金
	ULONG_C51		ulTraceNumber;			//交易流水号  
	unsigned char       ucYaJinFun;				//押金可用判断� �志'0':可用,'1':不可用
	unsigned char       ucReturnflag;
	unsigned char 	ucIssuerKeyIndex;		//PSAM卡密钥索引(� �识符)
	unsigned char		ucIssuerKeyVersion;		//用户卡密钥版本
	unsigned char		aucOfflineCnt[2];		//脱机交易序号
	unsigned char		aucOnlineCnt[2];		//连机交易序号
	unsigned char		aucCardRandom[4];		//用户卡随机数
	unsigned char		aucPsamTerminal[6];		//PSAM卡终端号
  	unsigned char		aucTraceTac[4];			//用户卡产生的交易TAC
	unsigned char       aucLoadTrace[16];		//充值流水号
   	unsigned char       aucSamTace[4];				//计数器
   	unsigned char       ucHandFlag;			//手工类型	
   	unsigned char   	aucZoneCode[4];
   	unsigned char   	aucZoneName[ZONE_NAME_LEN+1];
   	unsigned char   	aucZoneName1[ZONE_NAME_LEN+1];
   	unsigned char   	ucMemFlag;
   	unsigned char   	ucHopassFlag;
   	unsigned char       ucBalanceFlag;			//余额符号
	unsigned char     	aucIssueDate[4];		//发卡日期
	unsigned char     	aucCardSerial[4];		//卡序列号
	unsigned char 	aucVerifyCode[4];		//卡认证� �
	unsigned char 	ucTransPoint;		//交易明细指针
	unsigned char 	ucKYFlag;		//可疑� �志
	unsigned char 	aucAuthCode[14];		//授权� �
	unsigned char 	ucPtCode;		//平台
	unsigned char 	ucNewOrOld;		//旧零新一
	ULONG_C51		ulRateAmount;			
	ULONG_C51		ulTicketNumber;
	unsigned char 	aucBarName[14];		//条� �名称
	unsigned char  	aucPayType[2];				//缴费类别
	unsigned char  	aucPayDate[2];				//帐单日期
	unsigned char  	ucBarLenth;				//lenth of 条形� �
	unsigned char 	aucBarCode[30];				
	unsigned char 	ucSuccessflag;
	unsigned char 	aucBank[4];					//机构编� �
	unsigned char		ucDriverCard;				//司机卡使用
	unsigned char 	aucCashierNo[6];				
	unsigned char	       aucOldTrace[6];//原流水号
	unsigned char	       aucCashTicket[6];//收银机流水号
   	unsigned char       ucCardLoadFlag;			//写卡� �志
   	unsigned char       ucPackFlag;
	unsigned char		aucDiscount[2];		
   	unsigned char       ucZeroFlag;
	ULONG_C51		ulNeedCash;		//还需支付
    unsigned char   aucFunFlag[32];//卡功能� �志(卡表)
    unsigned char aucProCode[30];//产品编� �
}NORMALTRANS;
/* Extra Transaction Information */
typedef struct
{
	unsigned char	ucISO2Status;
	unsigned short	uiISO2Len;
	unsigned char	aucISO2[TRANS_ISO2LEN];
	unsigned char	ucISO3Status;
	unsigned short	uiISO3Len;
	unsigned char	aucISO3[TRANS_ISO3LEN];
}EXTRATRANS;
typedef struct
{
	unsigned char	aucCardNo[8];
	unsigned long	ulAmount;
}ZSHDETAIL;

typedef struct
{
	unsigned short	ucErrorExtCode;
	unsigned char	ucCardType;
	unsigned char	ucDialRetry;
	unsigned char	ucHostConnectFlag;
	unsigned short	uiTransIndex;
	unsigned short	uiOldTransIndex;
	ULONG_C51		ulOldTraceNumber;
	ULONG_C51		ulOldFeeAmount;
	ULONG_C51		ulPoint;
	ULONG_C51		ulValueble;
	ULONG_C51		ulCredit;
	unsigned char	ucPINRetry;
	unsigned char	aucTerminalID[PARAM_TERMINALIDLEN+4];
	unsigned char	aucMerchantID[PARAM_MERCHANTIDLEN];
    unsigned char   ucReprintFlag;
    unsigned char   ucSettleStatus[4];
	unsigned char   aucCashierNo[6];
  	unsigned char   aucPin[8];			//密� �
	unsigned char   aucNewPin[8];			//新密� �
  
  	unsigned char		ucExpireFlag;			//是否判断有效期
  	unsigned char		ucExpire1Flag;			//过期
  	unsigned char       ucCalculateFlag;		//算法� �识
	unsigned char       aucZoneCode[4];			//区域限制
	unsigned char       aucPbocMac[4];			//存放过程MAC1、MAC2
	unsigned char       aucRamdon[4];			//算线路保护MAC用的随机数
	unsigned char       aucWriteMAC[4];			//线路保护MAC
	unsigned char       aucWriteKey[6];			//冲值密钥
	unsigned char       aucWriteKey1[6];			//KEYB密钥
	unsigned char       aucWriteKey2[6];			//KEYB密钥
    unsigned char   aucFunFlag[32];//卡功能� �志(卡表)
    unsigned char   aucWDFunFlag[16];//网点功能� �志(发卡方)
    unsigned char   aucZDFunFlag[64];//终端功能� �志(发卡方)
    unsigned char   aucExFunFlag[32];//其他功能� �志(发卡方)
    unsigned char   aucLoadKey[6];
    unsigned char   aucPreBalance[4];
    unsigned char   aucTrace[2];
    unsigned char   ucKeyVersion;
    unsigned char   ucArithId;
    unsigned char   aucRandom[4];
    unsigned char   aucMAC1[4];
    unsigned char   aucMAC2[4];
    unsigned char   aucKeyA[6];
    unsigned char   aucPurchaseKey[6];
    unsigned char   aucErrorExplain[16];
    unsigned char   aucCheckCrc[4];
	unsigned char       ucRateType;				//费率类别
	unsigned char       ucRateType1;				//费率类别
	unsigned char       ucRateType2;				//费率类别
	unsigned char      ucRateTransType;          /*扣率交易类型*/
	unsigned long       ulRate;					//费率
	unsigned long       ulMaxRate;					//最大费率
	unsigned long       ulMinRate;					//最小费率
	unsigned char       ucWeiKFlag;				/*巍康� �志*/
	unsigned char       ucWanTFlag;				/*万通� �志*/
    unsigned char   aucTotalLoad[4];
    unsigned char   aucLoadMac[4];
    unsigned char   ucTagetStatus;
    unsigned char   aucDownKey[10];
    unsigned int uiBarNum;
	unsigned long      ulTotalAmount;
	unsigned long      ulTotalRate;
	ULONG_C51		ulNeedAmount[4];
	ULONG_C51		ulNeedRate[4];
	unsigned char       aucBarInfo[4][20];
	unsigned char       ucBcksp;
	unsigned int uiSaleCardNb;
	unsigned int uiSaleCardDetail;
	unsigned long ulSaleCardAmount;
	unsigned long ulSaleCountAmount;
	unsigned char       ucPtcode;
	unsigned char       ucCardVer1;
    unsigned char   ucKeyArrey;
	int iTCPHandle;
	unsigned char	ucTCPConnectFlag;
	unsigned char ucInputmode;
	ULONG_C51 ulBalance;
	ULONG_C51 ulFreeze;
    unsigned char   ucZfFlag;
    unsigned char   ucComType;
    unsigned char   aucAppId[8];
       unsigned char   ucPCCOMMFlag;
	unsigned char  aucPCCOMMErrCode[2];
	unsigned char   aucPCCOMMErrData[40];
	unsigned char       ucMustReturn;
	unsigned char       ucNeedRedo;
	unsigned char       aucCashID[6];
	unsigned char       ucGdflag;
	ZSHDETAIL	ZshDetails[6];
	unsigned char       ucHave6F;
	unsigned char       ucDxMobile;
	unsigned char      ucPt;
	unsigned char      uckeyi;
	unsigned int      uiTimesOut;
	unsigned char     aucComVersion[5];
	unsigned char      aucBakBits[16];
	unsigned char      ucInputKeyFlag;
	unsigned char   ucNewLgFlag;
    unsigned char   uc1stIpTye;
    unsigned char   ucWVFlag;
    unsigned char	aucSettleDateTime[7];//上次结算时间
}RUNDATA;
typedef struct
{
       NORMALTRANS Tran;
	unsigned char	ucRedoFlag;	
}REDOTRANS;

typedef struct
{
	unsigned char ucInLen;
	unsigned char aucCommandIn[255];
	unsigned char ucOutLen;
	unsigned char aucCommandOut[255];
}MIFARECOMMANDDATA;
#pragma pack (push)
#pragma pack (1)
typedef struct
{
	unsigned char 	ucPtCode;				//平台� �识
	unsigned char  	aucCardTrans[2];				//卡序号
	unsigned char 	aucCardZone[3];				//发卡方
	unsigned char 	ucCardMedia;				//卡介质
	unsigned char 	aucCardBin[6];	//卡bin [bcd]
	unsigned char  	ucCardBinLen;				//卡bin长度
	unsigned char 	ucCardNoPos;				//卡号位置
	unsigned char 	ucCardNoLen;				//卡号长度
	unsigned char 	ucCardoffset;				//卡号偏移量
	unsigned char 	ucYaAmount;				//卡押金
	unsigned char   aucCardValue[2];					//卡面值
	unsigned char   aucMaxValue[3];			//卡最大余额
	unsigned char   aucFunFlag[4];				//卡� �志
	//unsigned char 	ucOpFlag;				//增� � 除� �志
	unsigned char 	ucTableStatus;				//卡表状态'0'有效'1'� 效
	unsigned char 	ucCountType;				//账户属性'1'单账户'2'双账户
}CARDTABLE;

typedef struct
{
	unsigned char 	ucPtCode;				//平台� �识
       unsigned char 	aucCardZone[3];		              /*发卡方*/
	unsigned char 	ucTransType;				/*交易类型*/
	unsigned char        ucRateType[2];					/*费率类别,,,'1':按比数收取;'0:'按交易金额收取
																		'1' 内扣'0'外扣*/
	unsigned char        aucMax[3];					/*最大*/
	unsigned char        aucMin[3];					/*最小*/
	unsigned char        ucRateNb;					/*费率个数*/
	
	unsigned char 	ucSubType1;				/*费率类型*/
	unsigned char        aucRate1[3];					/*费率*/
	unsigned char        aucMin1[3];					/*最大*/
	unsigned char        aucMax1[3];					/*最小*/
	unsigned char 	ucSubType2;				/*费率类型*/
	unsigned char        aucRate2[3];					/*费率*/
	unsigned char        aucMin2[3];					/*最大*/
	unsigned char        aucMax2[3];					/*最小*/
	unsigned char 	ucSubType3;				/*费率类型*/
	unsigned char        aucRate3[3];					/*费率*/
	unsigned char        aucMin3[3];					/*最大*/
	unsigned char        aucMax3[3];					/*最小*/
}RATETABLE;


typedef struct
{
	unsigned char 	ucPtCode;				//平台� �识
	unsigned char 	aucCardZone[3];				//发卡方
	unsigned char 	ucValFlag;				//开通� �志
	unsigned char   aucWaikangName[16];			//巍康名称
	unsigned char   aucHopassName[16];			//万通名称
	//unsigned char 	ucOpFlag;				//增� � 除� �志
	unsigned char   aucCardValue[2];					//网点功能
	unsigned char 	ucVerifyOffset;				//� �验位置
	unsigned char 	aucFunction[8];				//发卡方功能� �志
	unsigned char 	aucExFunction[4];				//发卡方功能
}CARDZONE;

typedef struct
{
	unsigned char 	ucPtCode;				//平台� �识
	unsigned char 	aucCardZone[3];				//发卡方
	unsigned char 	ucType;				//类型'0'loadtrace '1'csn
	unsigned char   aucBeginCard[8];					//起始
	unsigned char   aucEndCard[8];			//终止
	//unsigned char 	ucOpFlag;				//增� � 除� �志
	unsigned char 	ucReason;				//黑名单原� 
}BLACKFIELD;
#pragma pack(pop)
typedef struct
{
	unsigned char	ucType;
	unsigned char	ucErrCode;
	unsigned char	aucErrMsg[MSG_MAXCHAR];
}SINGLEERRMSG;

typedef struct
{
	unsigned char	ucType;
	unsigned short	ucErrCode;
	unsigned char	aucErrMsg[MSG_MAXCHAR];
}HOSTEERRMSG;

typedef struct
{
	 unsigned char	aucMSGType;
	 unsigned char	aucMSGTab[MSG_MAXCHAR];
}MSG;


typedef struct
{
	unsigned char	ucExistFlag;
	unsigned short	uiOffset;
	unsigned short	uiLen;
}ISO8583Bit;

typedef struct
{
	ISO8583Bit		Bit[ISO8583_MAXBITNUM];
	unsigned short	uiBitBufLen;
	unsigned char	aucMsgID[ISO8583_MSGIDLEN];
	unsigned char	aucBitBuf[ISO8583_MAXBITBUFLEN];
	unsigned short	uiCommBufLen;
	unsigned char	aucCommBuf[ISO8583_MAXCOMMBUFLEN];
}ISO8583;

typedef struct
{
	ISO8583Bit		Bit[ISO8583_MAXBITNUM];
	unsigned short	uiBitBufLen;
	unsigned char	aucMsgID[ISO8583_MSGIDLEN];
	unsigned char	aucBitBuf[ISO8583_MAXBITBUFLEN];
}BACKUPISO8583;

typedef struct
{
	unsigned char	ucValid;
	BACKUPISO8583	BackupISO8583Data;
}REVERSALISO8583;

typedef struct
{
	unsigned char	ucAttr;
	unsigned short	uiMaxLen;
}ISO8583BitAttr;

typedef struct
{
	unsigned char	dp;
	unsigned char	chdt;
	unsigned char	dt1;
	unsigned char	dt2;
	unsigned char	ht;
	unsigned char	wt;
	unsigned char	ssetup;
	unsigned char	dtimes;
	unsigned char	timeout;
	unsigned char	asmode;
}MODEM_PARAM;
typedef struct
{
	unsigned char	aucName[6];
	unsigned char	aucPassword[6];
}CASHIERINFO;

typedef struct
{
    unsigned char	aucTerminalID[PARAM_TERMINALIDLEN];//终端号
	unsigned char	ucMerchantType;
    unsigned char	aucMerchantID[PARAM_MERCHANTIDLEN];//商户号
	unsigned char	aucMerchantName[PARAM_MERCHANTNAMELEN];
	unsigned char	ucCommMode;
	unsigned char	ucDialRetry;
	unsigned short	uiDialWaitTime;
	unsigned short	uiCommSpeed;
	unsigned short	uiReceiveTimeout;
	unsigned char	aucSwitchBoard[PARAM_SWITCHLEN];
	unsigned char	aucInitTelNumber1[PARAM_TELNUMBERLEN];
	unsigned char	aucInitTelNumber2[PARAM_TELNUMBERLEN];
	unsigned char	aucHostTelNumber1[PARAM_TELNUMBERLEN];
	unsigned char	aucHostTelNumber2[PARAM_TELNUMBERLEN];
	unsigned char	aucHostTelNumber3[PARAM_TELNUMBERLEN];
	unsigned char	aucHostTelNumber4[PARAM_TELNUMBERLEN];
	unsigned char	ucPBOCAppSelectFlag;
	unsigned char	ucPBOCPSAMReader;	
	unsigned char 	prnflag;
	unsigned char	prnflagS;
	unsigned char	prnflagL;
	unsigned char   linenum;
	unsigned char   Top[Display_MAXNB];
	unsigned char   Pinpadflag;
	unsigned char   font;
	unsigned char   printtimes;	
	unsigned char   Bank_flag;
	unsigned char   Printer_flag;
	unsigned char   Pinpad_flag;
	unsigned char   terminfoFirst;
	unsigned char   ucCommunication;	//通讯模式 '1'拨号 '2'网络
	unsigned long   ulHostIPAdd;	//服务器ip	
	unsigned int  	uiHostPort;	//端口
	unsigned char   ucAPN[50];
	MODEM_PARAM     ModemParam;		
	unsigned char   aucMid[3][8];
	unsigned char	  ucTranskeyflag;
    unsigned char   ucYTJFlag;//一体机� �识
	unsigned char   aucPinpadVersion[14];
	unsigned char   aucTid[3][4];
	unsigned char   Rate_flag;
	unsigned char   ucAmoutInput;
	unsigned long   ulMult;	//公交扣款基数
	unsigned char   ucCom;
	unsigned char   aucbakup[21];
	unsigned char   PinKey[8];
	unsigned char   MacKey[8];
	unsigned char   MacMasterKey[2][8];
	unsigned char   MacMasterUseKey[2][8];
	unsigned char   aucOnlytermcode[8];
	unsigned char   ucCardMd;	//卡介质'1'memory'2'非接
	unsigned char   Cpukey[15][16];
	unsigned char	  ucAccessType;
	unsigned char	  ucConnectType;
	unsigned long   ulHostIPAdd1;	//服务器ip	1
	unsigned int  	uiHostPort1;	//端口1
	unsigned long   ulHostIPAdd2;	//服务器ip	2
	unsigned int  	uiHostPort2;	//端口2
	unsigned char   aucHostDomain[30];
	unsigned long		ulMaxXiaoFeiAmount;
	CASHIERINFO	  Cashier[CASH_MAXCASHIER];
	unsigned char   aucSuperPassword[CASH_MAXSUPERPASSWDLEN];
	unsigned char   aucTreat[10];
	unsigned char   aucPrintType[2];
	unsigned char	  ucKFCFlag;
	unsigned long   ulKFCHostIPAdd;	//KFC服务器ip	
	unsigned int  	uiKFCHostPort;	//KFC端口
}CONSTANTPARAM;

typedef struct
{				
	unsigned char aucKEYA[6];	/*扇区密钥A*/
	unsigned char aucKEYB[6];	/*扇区密钥B*/
	unsigned char aucPurchasekey[6];	/*消费密钥*/
	unsigned char aucLoadkey[6];	/*充值密钥*/
	unsigned char aucContactCtl[4];	/*目录控制*/
	unsigned char aucBaseCtl[4];	/*基本控制*/
	unsigned char aucDetailCtl[4];	/*明细控制*/
	unsigned char aucPackCtl[4];	/*钱包控制*/
	unsigned char aucIssueCtl[4];	/*发卡控制*/
	unsigned char aucKeepCtl[4];	/*保留控制*/
	unsigned char ucCRC;
	unsigned char ucInfo;
	unsigned char aucContactInfo[30];	/*目录信息*/
	unsigned char aucIssueCode[3];	/*发卡方代� �*/
	unsigned char aucCardVersion[2];	/*发卡方代� �*/
	unsigned char ucCardType;
	unsigned char aucArea[4];	/*区域限制� �*/
	unsigned char aucIssueDate[4];	/*发卡日期*/
	unsigned char aucValidDate[4];	/*有效期*/
	unsigned char aucLoadTrans[8];	/*充值流水*/
	unsigned char ucPoint;	/*指针*/
	unsigned char aucTransNum[4];	/*交易数量*/
	unsigned char ucCardStatus;	/*卡状态*/
	unsigned char ucMemberFlag;	/*会员卡� �志*/
	unsigned char ucHopassFlag;	/*万通卡� �志*/
	unsigned char aucPie1Check[4];	/*扇区1� �验*/
	unsigned char aucPackAmount[4];	/*金额*/
	unsigned char aucCardNo[10];	/*卡号*/
	unsigned char aucCardAuth[12];	/*卡认证*/
	unsigned char aucBatch[4];	/*批次*/
	unsigned char ucBackupPackFlag;	/*备份钱包启用� �志*/
}UNTOUCHISSUE;	

typedef struct
{
	UINT_C51		uiTotalNb;      			//总交易笔数
	ULONG_C51		ulTotalAmount;				//总累计金额
	UINT_C51		uiPurchaseNb;				//总消费笔数
	ULONG_C51		ulPurchaseAmount;			//总消费金额
	ULONG_C51		ulDiscountAmount;			//总折扣金额
	UINT_C51		uiOnlinePurchaseNb;				//总消费笔数
	ULONG_C51		ulOnlinePurchaseAmount;			//总消费金额
	UINT_C51		uiLoadNb;				//预充值笔数
	ULONG_C51		ulLoadAmount;			//预充值金额	
	UINT_C51		uiReturnNb;				//总退卡笔数
	ULONG_C51		ulReturnAmount;			//总退卡金额
	ULONG_C51		ulYJAmount;			
	UINT_C51		uiPayNb;				//缴费笔数
	ULONG_C51		ulPayAmount;			//缴费金额	
	UINT_C51		uiOnlinePayNb;				//联机缴费笔数
	ULONG_C51		ulOnlinePayAmount;			//联机缴费金额
	UINT_C51		uiMobileNb;				//手机充值笔数
	ULONG_C51		ulMobileAmount;			//手机充值金额	
	UINT_C51		uiOnlineMobileNb;				//联机手机充值笔数
	ULONG_C51		ulOnlineMobileAmount;			//联机手机充值金额

	UINT_C51		uiPreAuthNb;				//预授权笔数
	ULONG_C51		ulPreAuthAmount;			//预授权金额
	UINT_C51		uiPreAuthVoidNb;				//预授权笔数
	ULONG_C51		ulPreAuthVoidAmount;			//预授权金额
	UINT_C51		uiPreAuthFinishNb;				//预授权完成笔数
	ULONG_C51		ulPreAuthFinishAmount;			//预授权完成金额
	ULONG_C51			ulTotalBarNb;
	UINT_C51		uiZshPurchaseNb;				//总消费笔数
	ULONG_C51		ulZshPurchaseAmount;			//总消费金额
	ULONG_C51		ulZshPurchaseRate;			//总消费金额
	UINT_C51		uiZshOnlinePurchaseNb;				//总消费笔数
	ULONG_C51		ulZshOnlinePurchaseAmount;			//总消费金额
	ULONG_C51		ulZshOnlinePurchaseRate;			//总消费金额
	UINT_C51		uiSaleNb;				//售卡笔数
	ULONG_C51		ulSaleAmount;			//售卡金额	
	UINT_C51		uiReturnpNb;				//总退卡笔数汇点
	ULONG_C51		ulReturnpAmount;			//总退卡金额汇点
	UINT_C51		uiPurchasepNb;				//总消费笔数汇点
	ULONG_C51		ulPurchasepAmount;			//总消费金额汇点
	ULONG_C51		ulYJpAmount;			
	UINT_C51		uiOnlinePurchasepNb;				//总消费笔数
	ULONG_C51		ulOnlinePurchasepAmount;			//总消费金额
	UINT_C51		uiRefundNb;				//总退货笔数
	ULONG_C51		ulRefundAmount;			//总退货金额
	UINT_C51		uiTransferNb;				//移资笔数
	ULONG_C51		ulTransferAmount;			//移资金额
	UINT_C51		uiTransferPurchaseNb;				//移资消费笔数
	ULONG_C51		ulTransferPurchaseAmount;			//移资消费金额
	UINT_C51		uiLoadOnlineNb;				//预充值笔数
	ULONG_C51		ulLoadOnlineAmount;			//预充值金额	
	UINT_C51		uiChangeCardNb;				//换卡笔数
	ULONG_C51		ulChangeCardAmount;			//换卡金额	
	UINT_C51		uiChangeExpNb;				//续期笔数
	ULONG_C51		ulChangeExpAmount;			//续期金额
	UINT_C51		uiExpPurchaseNb;				//过期卡笔数
	ULONG_C51		ulExpPurchaseAmount;			//过期卡金额
	UINT_C51		uiRefAplNb;				//退货申请笔数
	ULONG_C51		ulRefAplAmount;			//退货申请金额
	UINT_C51		uiPayNobarNb;				//总� 条� �缴费笔数
	ULONG_C51		ulPayNobarAmount;			//总� 条� �缴费金额
	UINT_C51		uiSaleOnlineNb;				//售卡笔数
	ULONG_C51		ulSaleOnlineAmount;			//售卡金额	
	UINT_C51		uiCashLoadNb;				//现金充值笔数
	ULONG_C51		ulCashLoadAmount;			//现金充值金额	
	UINT_C51		uiJsZshOnlinePurchaseNb;				//总消费笔数
	ULONG_C51		ulJsZshOnlinePurchaseAmount;			//总消费金额
	ULONG_C51		ulJsZshOnlinePurchaseRate;			//总消费金额
	UINT_C51		uiBatchLoadNb;				//批量充值笔数
	ULONG_C51		ulBatchLoadAmount;			//批量充值金额	
	UINT_C51		uiOnlineRefundNb;				//联机退货笔数
	ULONG_C51		ulOnlineRefundAmount;			//联机退货金额	
	UINT_C51		uiPTCPurchaseNb;				//公交卡充值扣款笔数
	ULONG_C51		ulPTCPurchaseAmount;			//公交卡充值扣款金额
	unsigned char	aucBatchNumber[BATCH_LEN];		/*保存上次交易批次号*/
}TRANSTOTAL;

typedef struct
{
	unsigned short	auiTransIndex[TRANS_MAXNB];
	//unsigned short	auiZoneCtrlIndex[MAX_ZONE_NUM];
//	unsigned short	auiCardTableIndex[MAX_CARDTABLE_NUM];
	TRANSTOTAL		TransTotal;	
	TRANSTOTAL		LastTransTotal;
	TRANSTOTAL 		PtInfoData[3];
	TRANSTOTAL 		LastPtInfoData[3];
	unsigned char		aucTotalIndex[50];
}TRANSINFO;

typedef struct
{
	unsigned char		ucTranstype;
	unsigned char		ucPtcode;
	unsigned char		aucIssueName[16];
	UINT_C51		uiTransNb;				
	ULONG_C51		ulTransAmount;			
	ULONG_C51		ulYJAmount;			
}ISSUETRANSINFO;

typedef struct
{
	unsigned char	aucLogonDateTime[PARAM_DATETIMELEN];
	unsigned char	aucBlackDateTime[PARAM_DATETIMELEN];
	unsigned char	aucTableDateTime[PARAM_DATETIMELEN];
	unsigned char	aucZoneDateTime[PARAM_DATETIMELEN];
	unsigned char	aucRateRenewDateTime[PARAM_DATETIMELEN];     /*上次扣率更新时间*/
	unsigned char	aucBatchNumber[BATCH_LEN];
	unsigned long	ulTraceNumber;
	unsigned long   aucFirstTraceNum[LOADTRACE_LEN];		//第一笔充值流水号
	unsigned char   aucLastTraceNum[LOADTRACE_LEN];		//上笔成功的充值流水
	unsigned long   ulFixAmount;						//充值金额

	unsigned char	ucSwitchIPFlag;						
	unsigned char	ucCurrentIPFlag;						

	unsigned char	ucExpFlag;						
	unsigned char	ucExpFlag1;						
	unsigned char   ucInilizeFlag;
	unsigned char	ucIsLogonFlag;
	unsigned char   aucCashierNo[6];
	unsigned char ucIsCashierLogonFlag;
	unsigned char	ucSettleFlag;
	unsigned int	uiBlackNum;				//黑名单数
	unsigned int    uiBlackFeildNum;		//黑名单段数	
	unsigned int   uiBlackSrlFeildNum;
	unsigned int	uiCardTableNum;			//卡表数
	unsigned int    uiZoneCodeNum;			//区位� �数
	unsigned int   	uiRateTableNum;
	unsigned char   aucSettleCycle[3];		//结算周期
	unsigned char   aucPrintInfo[40];		//凭条打印信息
	unsigned char   aucFirstZoneCode[4];	//第一� 卡的区域� �	
	unsigned char	ucSpecialTable;
	unsigned char	aucTermFlag[16];
	unsigned long	ulTicketNumber;
	unsigned int   	uiIssueKeyNum;
	unsigned char	aucIssueKeyIndex[18][3];
	unsigned char	aucIssueKeyRenewDateTime[PARAM_DATETIMELEN];    
	unsigned int	uiCTNum[3][8];			//分类卡表数
	unsigned char	aucSettleDateTime[7];//上次结算时间
}CHANGEPARAM;

typedef struct
{
	CONSTANTPARAM	ConstantParamData;
	CHANGEPARAM		ChangeParamData;
	TRANSINFO		TransInfoData;
	BACKUPISO8583	SaveISO8583Data;
	BACKUPISO8583	SendISO8583Data;
	REVERSALISO8583	ReversalISO8583Data;
	NORMALTRANS		SaveTransData;
	REDOTRANS		RedoTrans;
}DATASAVEPAGE0;


typedef struct
{
	unsigned char		ucBatchFlag[TRANS_MAXNB];
	BLACKFIELD		BlackFeild;
	CARDZONE        ZoneCtrl;
	CARDTABLE	      CardTable;	
	RATETABLE	RateTable;	
}DATASAVEPAGE1;

typedef struct
{
	unsigned int	uiBarCodeLen;			//条� �长度
	unsigned char	aucBarCode[50];			//条� �
	unsigned char	aucBarCodeBCD[30];		//条� �LLVAR(BCD)
	unsigned char	aucOrganName[41];		//条� �名称，比如：市南水
	unsigned char	aucOrganNames[120];
	unsigned char	aucOrganCode[16];		//出帐机构代� �
	unsigned char	aucOrganCodes[45];	
	unsigned char	aucAccountPeriod[7];	//期次
	unsigned char aucAccountPeriods[18];
	unsigned char	aucCopyTimes[3];		//抄次
	unsigned char aucCopyTimess[6];
	unsigned char ucNumber;
	unsigned char 	aucAmount[11];			//金额
	unsigned char aucAmounts[30];	
}BAR_CODE_INFO;

typedef struct
{
	unsigned char	ucTransType;			//交易类型
	unsigned long	ulTraceNum;
	unsigned char	aucHostTrace[21];		//主机流水
	unsigned char	aucHostDate[9];				
	unsigned char	aucHostTime[7];				
	unsigned char	ucPayFeeType;			//付费类型
	unsigned char ucAccType;				//卡折类型
	unsigned char aucAccNum[21];			//帐号
	unsigned char	aucIssureID[16];			//支付机构代� �
	unsigned char aucAccName[41];		//帐号户名
	unsigned char	aucAccStatus[3];			//帐户状态
	unsigned char ucAccBalanceFlag;		//余额符号
	unsigned long 	ulAccBalance;			//帐户余额
	unsigned long	ulAccAvailBalance;		//可用余额
	unsigned long 	ulAmount;				//支付金额	
	unsigned char	aucOrderNum[21];		//订单号
	unsigned char	aucValidDate[7];			//有效期YYYYMM
	unsigned char	aucAcceptDate[9];		//受理方日期YYYYMMDD
	unsigned char	aucAcceptCode[16];		//受理机构代� �
	unsigned char	aucMemo[50];
	BAR_CODE_INFO	BarCodeInfo;				//条� �信息
	unsigned char ucCardType;	//卡类型
}PAYMENT_TRANS;
/*条� �解析规则结构体 add by gyc20041111*/
typedef struct
{
	int bill_kind;   	     /* 条� �类别 */
    int bill_type;		     /* 条� �类型 */
    int bar_len;		     /*条� �长度*/
    char bill_name[41];      /* 条� �名称 出帐机构名称 */
    char commpany_code[16];  /* 出帐机构代� � */
	char field_map[17];      /* 位图	*/
	char field1[3];          /* 公司代� � */
	char field2[151];        /* 区号 */
	char field3[151];        /* 代办� � */
	char field4[151];        /* 单位� � */
	int field5;              /* 合同号长度（帐号、用户户号、编号 */
	int field6;              /* 年份  长度 */
	int field7;              /* 月份  长度 */
	int field8;              /* 抄次  长度 */
	int field9;              /* 帐单类型 长度 */
	int fieldA;              /* 金额  长度 */
	int fieldB;
	int fieldF;              /* � �验位 长度 */
	int check_type;          /* � �验规则 */
	char field5_name[21];
	int field5_off; 
}CDBBillInfo;
typedef struct
{
	unsigned char ucOperateType[2];//操作类型
	unsigned char	ucCashID[6];//收银机编号
	unsigned char	ucTestData[8];//测试数据
}TESTDATA;

typedef struct
{
	unsigned char ucOperateType[2];//操作类型
	unsigned char	ucCashID[6];//收银机编号
	unsigned char	ucExtendTime[2];//延时时间
}EXTENDTIMEDATA;

typedef struct
{
	unsigned char 	ucOperateType[2];//操作类型
	unsigned char 	ucTransType[2];//交易类型
	unsigned char 	ucCardType[2];//卡类型
	unsigned char	ucCashID[6];//收银机编号
	unsigned char	ucCashierID[6];//柜员号
	unsigned char	ucAmount[12];//交易金额
	unsigned char	ucCashTicket[6];//收银机流水号
	unsigned char	ucOldCashTicket[6];//原收银机流水号
	unsigned char	aucPreHold[48];	//保留字段
}INCEPTCASHTRANSDATA;
typedef struct
{
	unsigned char 	ucOperateType[2];//操作类型
	unsigned char 	ucTransType[2];//交易类型
	unsigned char 	ucCardType[2];//卡类型
	unsigned char	       ucCashID[6];//收银机编号
	unsigned char	       ucCashTicket[6];//收银机流水号
}DEMANDDATA;

typedef struct
{
	unsigned char 	ucOperateType[2];//操作类型
	unsigned char 	ucTransType[2];//交易类型
	unsigned char 	ucCardType[2];//卡类型
	unsigned char   aucResponseCode[2];//返回� �
	unsigned char   aucExplain[40];//返回信息
	unsigned char	ucCashID[6];//收银机编号
	unsigned char	ucCashierID[6];//柜员号
	unsigned char	ucAmount[12];//交易金额
	unsigned char   ucMidBatch[6];//结算批次
	unsigned char	aucMerchantID[15];//商户号
	unsigned char 	aucMerchantName[40];//商户名称
	unsigned char	aucTerminalID[8];//终端号
	unsigned char   aucCardNo[19];//卡号
	unsigned char	aucExpiredDate[4];//卡有效期
	unsigned char   aucBankNo[6];//发卡行编� �
	unsigned char   ucTransDate[8];//交易日期
	unsigned char   ucTransTime[6];//交易时间
	unsigned char   aucAuthCode[6];//授权号
	unsigned char	  aucSysTrace[12];//系统参照号
	unsigned char	  ucCashTicket[6];//收银机流水号
	unsigned char	  ucOldCashTicket[6];//原收银机流水号
	unsigned char   ucHostTicket[6];//系统流水号
	unsigned char   ucOldHostTicket[6];//原系统流水号
	unsigned char	  aucPreHold[48];	//保留字段
}SENDCASHTRANSDATA;

typedef struct
{
	unsigned char 	ucOperateType[2];//操作类型
	unsigned char 	ucTransType[2];//交易类型
	unsigned char 	ucCardType[2];//卡类型
	unsigned char   aucResponseCode[2];//返回� �
	unsigned char   aucExplain[40];//返回信息
	unsigned char		aucPreHold[48];	//保留字段
}SENDTRYDATA;
typedef struct
{
	unsigned long	ulCommLen; 
	unsigned char	aucCommBuf[512];
}COM1_INFO;
typedef struct
{
	 //   unsigned char aucLength[2];//包长度
      unsigned char aucEncryptNo[3];  //� 密算法编号3
      unsigned char aucMuchAppNo[8];//多应用编号8
      unsigned char aucCardType[2];//卡类型
      unsigned char aucCommuVer[3];//通讯协议版本 3
      unsigned char aucDynamicVer[3];//动态库版本 3
      unsigned char ucMessageGoon;//消息续包 1 0x55 有
      unsigned char aucOvertime[3];//超时时间 3
}HEADDATA;   //23

typedef struct
{
//	HEADDATA HeadData;
	unsigned char	aucOperType[2];//操作类型
	unsigned char	aucTransType[2];//交易类型
//	unsigned char aucCardType[2];//卡类型
	unsigned char	aucCashierNo[6];		/*收银机编号*/
	unsigned char	aucCashier[6];					/*柜员号(查询时为流水号)*/
	unsigned char	aucAmount[12];				/*交易金额*/
	unsigned char aucCashierTrace[6];				/*收银机流水号*/
	unsigned char aucOldDateTime[8];//原交易日期
	unsigned char	ulOldTicketNumber[6];		/*撤消流水号*/
	unsigned char aucOldRefNum[12];//原系统参照号
	unsigned char	aucHoldData[320];			//保留字段
}RECEIVETRANS;//382 
typedef struct
{
	unsigned char	aucOperType[2];//操作类型
	unsigned char	aucTransType[2];//交易类型
	unsigned char aucCardType[2];//卡类型
	unsigned char	aucCashierNo[6];		/*收银机编号*/
	unsigned char	aucCashier[6];					/*柜员号(查询时为流水号)*/
	unsigned char	aucAmount[12];				/*交易金额*/
	unsigned char aucCashierTrace[6];				/*收银机流水号*/
	unsigned char	ulOldTicketNumber[6];		/*撤消流水号*/
	unsigned char	aucHoldData[48];			//保留字段
}RECEIVETRANS_GD; 


typedef struct
{
	unsigned char	aucOperType[2];  //操作类型
	unsigned char	aucTransType[2];  //交易类型
	unsigned char   CardType[2];   //卡类型
	unsigned char	aucRespCode[2];				/*交易返回� �*/
	unsigned char	aucRespInfo[40];				/*交易返回信息*/
	unsigned char	aucCashRegNo[6];			/*收银机编号*/
	unsigned char	aucCashier[6];				/*柜员号*/
	unsigned char	aucAmount[12];				/*交易金额*/
	unsigned char	aucBatchNum[6];				/*结算批次号*/
	unsigned char	aucMerchID[15];				/*商户号*/
	unsigned char	aucMerchantName[40];		/*商户名称*/
	unsigned char	aucTermID[8];				/*终端号*/
	unsigned char	aucCardId[19];				/*卡号*/
	unsigned char	aucValidDate[4];			/*卡有效期*/	
	unsigned char	aucCardType[6];				/*发卡行编� �*/
	unsigned char	aucTransDate[8];			/*交易日期*/
	unsigned char	aucTransTime[6];			/*交易时间*/
	unsigned char	aucAuthCode[6];				/*授权号*/
	unsigned char	aucSysRefNo[12];			/*系统参照号*/
	unsigned char aucCashTraceNo[6];			/*收银机流水号*/
	unsigned char	aucOrgCashTraceNo[6];		/*原收银机流水号*/
	unsigned char	aucTraceNo[6];				/*系统流水号*/
	unsigned char	aucOrgTraceNo[6];			/*原系统流水号*/
	unsigned char aucHoldData[48];   //保留字段
}SENDTRANS;//267

typedef struct
{
	unsigned long   ulHeadLength;
	unsigned long   ulSendLength;
	unsigned long   ulRecieveLength;
	unsigned char   aucBuf[512];
}PORTDATA;

typedef struct
{
	unsigned char   ucPt;
	unsigned char   aucZone[3];
	unsigned char   aucFee[4];
}EXPFEE;
typedef struct
{
	unsigned char   ucPt;
	unsigned char   aucZone[3];
	unsigned char   aucBegin[2];
	unsigned char   aucEnd[2];
	unsigned char   ucYear;
	unsigned char   aucNew[2];
}EXPPARAM;
typedef struct
{
	EXPFEE ExpFeeData[20];
	EXPPARAM ExpParamData[30];
}EXPDATA;

//============= ɼ��QtӦ���¼ӽӿ�============================
typedef struct
{
	//���Դ���: progress ��UI����
	unsigned char ucClsFlag;	// 0xAA �����Ļ	

	unsigned int uiLines;		//��ʾ����
	unsigned char aucLine1[64];
	unsigned char aucLine2[64];
	unsigned char aucLine3[64];
	unsigned char aucLine4[64];	
	unsigned char aucLine5[64];	
	unsigned char aucLine6[64];	
	unsigned char aucLine7[64];	
	unsigned char aucLine8[64];	
}FACEPROTOUI;

typedef struct
{
	//���׽ӿ�: ui���洫�ݵ� progress
	unsigned char ucTransType;		//��������
	unsigned char ucInputMode;		//ˢ����ʽ
	unsigned char aucCardAcc[512];	
	unsigned long ulAmount;			//���
	unsigned int uiPwLen; 	// ���볤��
	unsigned char aucPasswd[24]; 	// ����
    unsigned long ulTraceNum;//liushui
    unsigned long ulPrevEDBalance;
}FACEUITOPRO;

typedef struct
{
	unsigned char ucProBar;//�Ƿ��н�����
	unsigned int  uiProgress;

	FACEPROTOUI ProToUi;
	FACEUITOPRO UiToPro;	
}PROUIINTERFACE;
//============= ɼ��QtӦ���¼ӽӿ�==========================[END].

/*eve.com*/
unsigned char OnEve_power_on(void);
unsigned char OnEve_KeyPress(unsigned char ucKey);

/*filechk.c*/						
unsigned char FILE_CheckConstantParamData(void);
unsigned char FILE_CheckChangeParamData(void);
unsigned char FILE_CheckTransInfoData(void);
unsigned char FILE_CheckSaveISO8583(void);
unsigned char FILE_CheckSendISO8583(void);
unsigned char FILE_CheckReversalISO8583(void);
unsigned char FILE_CheckBatchFlags(void);
unsigned char FILE_CheckBlackFeilds(void);
unsigned char FILE_CheckSaveTrans(void);
unsigned char FILE_CheckZoneCtrls(void);
unsigned char FILE_CheckCardTables(void);
unsigned char Rate_InsertCode(unsigned char *ZoneCode);
void Rate_MoveBackward(int StartPos,int EndPos);
unsigned char Rate_DeleteCode(unsigned char *ZoneCode);

/*cfg.c*/
unsigned char CFG_ConstantParamVersion(void);
void CFG_GetVersionString(unsigned char ucFlag,unsigned char *pucBufOut);
unsigned char CFG_ConstantParamBaseData(void);
unsigned char CFG_Cardtype_Choice(void);
unsigned char CFG_ConstantParamDateTime(void);
unsigned char CFG_ConstantParamPhoneNo(void);
unsigned char CFG_ClearRom(void);
unsigned char CFG_ClearSettleProcess(void);
unsigned char CFG_ClearReversalProcess(void);
unsigned char CFG_CheckFlag(void);
unsigned char CFG_CheckCardValid(void);
unsigned char CFG_CheckBlackFeild(unsigned char *InData);
unsigned char CFG_CheckBlackFeild(unsigned char *InData);
unsigned char CFG_CheckCardTbl(unsigned char *InData);
unsigned char  CFG_SetBatchNumber(void);
void CFG_GetDateTime(void);
unsigned char CFG_ChkExpireValid(const unsigned char *InDate);
unsigned char CFG_CalcPreLoadTAC(void);
unsigned char CFG_CalcPurhaseTAC(void);
unsigned char CFG_QueryList(void);
unsigned char CFG_SetBatchNumber1(void);
unsigned char MAC_Load_Key(void);
unsigned char CFG_PrintTables(void);
unsigned char CFG_CheckCardZone(unsigned char *InData);
unsigned char CFG_ClearTransKeyFlag(void);
unsigned char CFG_SetAmountInput(void);
unsigned char CFG_NetTest(void);





/*msg.c*/
unsigned char MSG_WaitKey(unsigned short uiTimeout);
unsigned char MSG_GetMsg(unsigned short uiIndex,unsigned char *pucMsg,unsigned short uiMsgLen);
unsigned char MSG_DisplayErrMsgNotWait(unsigned char ucErrorCode);
unsigned char MSG_DisplayMsgKey(unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned short uiIndex,
					unsigned short uiTimeout);
unsigned char MSG_DisplayErrMsg(unsigned char ucErrorCode);
unsigned char MSG_DisplaySINGLEERRMSG(const SINGLEERRMSG ErrMsg[],unsigned char ucErrorCode);

/*sel.c*/
unsigned char SEL_ManaMenu(void);
unsigned char SEL_InitProcess(void);
unsigned char SEL_ChangeCard(void);



/*util.c*/
void UTIL_Beep(void);
void UTIL_ClearGlobalData(void);
void UTIL_GetTerminalInfo(void);
unsigned char  UTIL_GetMenu_Value(unsigned char ucTransType,unsigned short uiIndex,
		unsigned char (*pfnProc)(void),unsigned char *pucFlag,SELMENU *Menu);
unsigned char UTIL_Input(unsigned char ucLine,unsigned char ucClrFlag,
                unsigned char ucMin, unsigned char ucMax,
                unsigned char ucType,
                unsigned char *pucBuf,
                unsigned char *pucMask);
unsigned char UTIL_DisplayMenu(SELMENU *pMenu);
unsigned char UTIL_Input_XY(unsigned char ucLine,unsigned char ucCol,unsigned char ucClrFlag,unsigned char ucMin, 
                unsigned char ucMax,unsigned char ucType,unsigned char *pucBuf,unsigned char *pucMask);
unsigned char UTIL_GetKey(unsigned char wait_sec);
unsigned char UTIL_SetPrinter(void);
unsigned char UTIL_IncreaseTraceNumber(unsigned char aucMode);
void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule);
unsigned char UTIL_InputAmount(unsigned char ucLine,
				unsigned long *pulAmount,
				unsigned long ulMinAmount, unsigned long ulMaxAmount);
void UTIL_Print_PRJ_VER(void);
unsigned char OSUTIL_CheckIPAddrField(
                unsigned char *pucIP,
                unsigned char ucIPLen,
                unsigned char *pucIPField);
unsigned char OSUTIL_InputIPV4Addr(
                unsigned int uiTimeout,
                unsigned char ucFont,
                unsigned char ucRow,
                unsigned char *pucIPV4Addr);
void UTIL_GetDateTime(void);
unsigned char UTIL_SetPinpad(void);
unsigned char UTIL_CalcMemberMac(
				unsigned char *pucInData,
				unsigned short uiInLen,
				unsigned char *pucOutMAC);
unsigned char UTIL_CalcSandLoadTAC(unsigned char *pucInData,
									         unsigned short  uiInLen,
									         unsigned char  *pucOutData);
unsigned char UTIL_CalcSandTAC(unsigned char *pucInData,
									         unsigned short  uiInLen,
									         unsigned char  *pucOutData);
unsigned char UTIL_GetKey_pp(unsigned char wait_sec);
unsigned char UTIL_DisplayTrans(void);
void UTIL_DisplayAmount(unsigned char line,unsigned long amount);
unsigned char UTIL_InputEncryptPIN(void);
unsigned char UTIL_ChangeEncryptPIN(void);
void UTIL_TEST(void);		
void UTIL_TEST1(void);
unsigned char UTIL_BitFlagAnalyze(uchar *aucBitFlag,int iLen,uchar *aucAscFlag);
unsigned char UTIL_SetRateType(void);
unsigned char UTIL_SetMult(void);
unsigned char UTIL_SetCommMode(void);
unsigned char UTIL_SetCOM(void);



/*key.c*/
unsigned char KEY_DownloadKeysFromCpuCard(void);
unsigned char KEY_StoreNewKey(unsigned char *aucKeyData);

/*logon.c*/
unsigned char LOGON_IniProcess(void);
unsigned char LOGON_IniOnline(void);
unsigned char LOGON_LogonProcess(void);
unsigned char LOGON_LogonOnline(void);
unsigned char LOGON_DownZone(void);
unsigned char LOGON_DownTable(void);
unsigned char LOGON_DownBlackList(void);
unsigned char LOGON_LogoffProcess(void);
unsigned char CashierLogonProcess(void);
unsigned char CashierLogonOnline(void);
unsigned char CashierChangepin(void);
unsigned char CashierChangepinOnline(void);
unsigned char LOGON_DownRate(void);
unsigned char LOGON_DownKey(void);
unsigned char LOGON_DownTransKey(void);
unsigned char LOGON_DownTableSps(void);
unsigned char LOGON_DownMemberCardKey(void);
unsigned char ZoneKey_Insert(uchar *aucIndata);
unsigned char LOGON_DownApp(void);
unsigned char LOGON_SyncOnline(void);
unsigned char LOGON_DownTransKeyProcess(void);
unsigned char LOGON_DownWKCardKey(void);
unsigned char CASH_ChangeSuperPasswd1(void);




/*comms.c*/
unsigned char COMMS_PreComm(void);
unsigned char COMMS_CheckPreComm(void);
unsigned char COMMS_TransOnline(void);
unsigned char COMMS_TransSendReceive(void);
unsigned char COMMS_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
                    unsigned char *pucOutData,unsigned short *puiOutLen);
unsigned char COMMS_ReceiveByte(unsigned char *pucByte,unsigned short *puiTimeout);
unsigned char COMMS_FinComm(void);
unsigned char COMMS_SendReceiveNet(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen,
					unsigned long ulHostIPAddress,unsigned int uiHostPort);
unsigned char COMMS_SendReceiveNet_Long(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen,
					unsigned long ulHostIPAddress,unsigned int uiHostPort);
					
/*menu.c*/
unsigned char MenuSys_Appli(void);		
unsigned char MENU_OnlineManage(void);
			


/* ISO8583.c */
void ISO8583_Clear(void);
void ISO8583_ClearBit(void);
void ISO8583_ClearBitBuf(void);
void ISO8583_ClearCommBuf(void);
unsigned char ISO8583_GetMsgID(unsigned short *piMsgID);
unsigned char ISO8583_SetMsgID(unsigned short iMsgID);
unsigned char ISO8583_GetBitValue(short iBitNo, unsigned char *pucOutData, short *piOutDataLen, short iOutBufLen);
unsigned char ISO8583_SetBitValue(short iBitNo,unsigned char *pucInData,ushort uiInLen);
unsigned char ISO8583_SetBitHexValue(short iBitNo,unsigned char *pucInData,ushort uiInLen);
unsigned char ISO8583_CheckBit(short iBitNo);
unsigned char ISO8583_RemoveBit(short iBitNo);
unsigned char ISO8583_PackData(unsigned char *pucOutData, unsigned short *piOutDataLen, short iOutLen);
unsigned char ISO8583_UnpackData(unsigned char *pucInData, short iInDataLen);
void ISO8583_DumpData(void);
//void ISO8583_CheckBitAttrTab(void);
unsigned char ISO8583_CheckResponseValid(void);
unsigned char ISO8583_CompareSentBit(short iBitNo);
void ISO8583_SaveISO8583Data(unsigned char *pucSrc,unsigned char *pucDest);
void ISO8583_RestoreISO8583Data(unsigned char *pucSrc,unsigned char *pucDest);
unsigned char ISO8583_SaveReversalISO8583Data(void);

/*file.c*/
unsigned char SortFile(unsigned char ucFileIndex,unsigned char *pRec,unsigned int uiRecordNum);
unsigned char XDATA_AppendRecords(unsigned char ucFileIndex,/*文件名称*/
								 unsigned char *pRecord,/*下发记录的缓冲区*/
								 unsigned int uiRecordSize,/*每个记录的大小*/
								 unsigned int uiRecordNum/*下发记录的个数*/);
unsigned char Zone_InsertCode(unsigned char *ZoneCode);
int Zone_SearchIns(unsigned char *InData);
void Zone_MoveBackward(int StartPos,int EndPos);
unsigned char Zone_DeleteCode(unsigned char *ZoneCode);
int Zone_SearchDel(unsigned char *InData);
void Zone_MoveForward(int StartPos,int EndPos);
unsigned char CardTable_InsertCode(unsigned char *ZoneCode);
int CardTable_SearchIns(unsigned char *InData);
void CardTable_MoveBackward(int StartPos,int EndPos);
unsigned char CardTable_DeleteCode(unsigned char *ZoneCode);
int CardTable_SearchDel(unsigned char *InData);
void CardTable_MoveForward(int StartPos,int EndPos);
unsigned char Black_InsertCode(unsigned char *ZoneCode);
int Black_SearchIns(unsigned char *InData);
void Black_MoveBackward(int StartPos,int EndPos);
unsigned char Black_DeleteCode(unsigned char *ZoneCode);
int Black_SearchDel(unsigned char *InData);
void Black_MoveForward(int StartPos,int EndPos);

/*memcard.c*/
unsigned char SLE4442_Menu(union seve_in *EveIn);
unsigned char SLE4442_ATR(unsigned char *pBuf);
unsigned char SLE4442_ResetAddress(unsigned char *code_buff);
unsigned char SLE4442_NReadBits(unsigned char *code_buff, unsigned char N_Bits);
unsigned char SLE4442_End_ATR(unsigned char *code_buff);
unsigned char SLE4442_FinishCommand(unsigned char *code_buff);
unsigned char SLE4442_ReadCardData(void);
unsigned char SLE4442_ReadMainMemory(unsigned char *pBuf,unsigned char ucAddr,unsigned char ucNbr);
unsigned char SLE4442_EnterCommandMode(unsigned char *code_buff);
unsigned char SLE4442_SendCommand(unsigned char *code_buff,unsigned char Command_Type,
							unsigned char Addr,unsigned char Input_Data);
unsigned char SLE4442_LeaveCommandMode(unsigned char *code_buff);
unsigned char SLE4442_CheckCardValid(void);
unsigned char SLE4442_CheckSecret(unsigned char *pSecret_Code);
unsigned char SLE4442_ReadSecurityMemory(unsigned char *pBuf);
unsigned char SLE4442_CompareVerificationData(unsigned char Input_Data,unsigned char ucAddr);
unsigned char SLE4442_UpdateSecurityMemory(unsigned char Input_Data,unsigned char ucAddr);
unsigned char SLE4442_Changecard(void);
unsigned char SLE4442_ResetPinProcess(void);
unsigned char SLE4442_QueryChange(void);
char SLE4442_GenCardVerify(char* sCardNo);
unsigned char SLE4442_CardTrans(void);
unsigned char SLE4442_EraseAndWrite(unsigned char *code_buff);
unsigned char SLE4442_CompareProcess(unsigned char *code_buff);
void gz_xor(uchar *a, uchar *b, char lg);
unsigned char SLE4442_Break(unsigned char *code_buff);
unsigned char SLE4442_UpdateMainMemory(unsigned char Input_Data,unsigned char ucAddr);


/*untouch.c*/
unsigned char Untouch_GetTAC(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength);
unsigned char Untouch_Check_Err(unsigned char ucData);
unsigned char Untouch_GetBitDate(uchar *inDate,uchar *outDate);
unsigned char Untouch_Load_Process(void);
unsigned char Untouch_Load_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength);
unsigned char Untouch_Purchase_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength);
unsigned char Untouch_ReadCardNo_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength);
unsigned char Untouch_ReadPie1_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength);
unsigned char Untouch_ReadSierial_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength);
unsigned char Untouch_ChangeStatus_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength);
unsigned char Untouch_ResetCard(void);
unsigned char Untouch_SendReceive_tmp(void);
unsigned char Untouch_SendReceive_Getcard(unsigned int uiTime);
unsigned char Untouch_OnlineRefund(void);

										
unsigned char PBOC_ISOCheckReturn(void);
void PBOC_SetIso7816Out(void);
unsigned char Untouch_Detail(void);
unsigned char Untouch_OnlinePurchase(void);
unsigned char Untouch_Void(void);
unsigned char Untouch_Purchase(void);
unsigned char Untouch_Zsh_Process(void);
unsigned char Untouch_SendReceive_rp(void);
unsigned char Untouch_PayAdmin(void);
unsigned char Untouch_Query(void);
unsigned char Untouch_MobileAdmin(void);
unsigned char Untouch_PreAuthProcess(void);
unsigned char Untouch_Purchase_P(void);
unsigned char Untouch_Refund(void);
unsigned char Untouch_HandChoice(void);
unsigned char Untouch_DX_WaitCard(void);
unsigned char Untouch_PTCPurchase(void);
unsigned char Untouch_ChangePin(void);
unsigned char Untouch_DX_Process(void);


/*pinpad.c*/
unsigned char PINPAD_0D_InitScraCode(unsigned char *pucInData);
unsigned char PINPAD_30_BackLight(unsigned char ucBackLight);
unsigned char PINPAD_42_LoadSingleKeyUseSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucDecryptKeyIndex,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData);
unsigned char PINPAD_47_Encrypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData);
unsigned char PINPAD_47_Crypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData);
unsigned char PINPAD_4E_PINInput(
					unsigned char ucRow,unsigned char ucCol,
					unsigned char ucMin,unsigned char ucMax,
					unsigned char ucDispChar,
					unsigned char ucKeyArray,unsigned char ucKeyIndex,
					unsigned char *pucOutKeyNb,
					unsigned char *pucOutData);
unsigned char PP_In_Store_Key(
					unsigned char ucRow,
					unsigned char ucCol,
					unsigned char ucKeyIndex,
					unsigned char *aucBuf);
unsigned char SendReceivePinpad(void);
unsigned char	Load_Default_Keys(void);
uchar	Single_Encrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar * Pt_crypt);
uchar	Single_Decrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar *Pt_crypt);
uchar	Single_Store_Key(uchar Array, uchar Key_used, uchar Key_stored, uchar *Pt_data);
uchar	Double_Encrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar *Pt_crypt);
uchar	Double_Decrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar *Pt_crypt);
uchar	Double_Store_Key(uchar Array, uchar Key_used, uchar Key_stored, uchar DataType, uchar *Key_data);
uchar	PINPAD_45_LoadKey(uchar *pucInData);
uchar	PINPAD_46_LoadKey1(uchar ucDecryptTransmitKeyType,uint  uiStoringAppNum,
						  uint  uiDecryptTransKeyAppNum, uchar *pucDecryptKeyArray,
						  uchar ucDecryptTye,            uchar ucProctKeyIndex,
					      uchar *pucStoringArray,		 uchar *pucInData);

					      
/*trans.c*/
unsigned char TRS_ResetPin_Online(void);
unsigned char TRS_CheckReversal(void);
unsigned char Trans_Preload(void);




/*sav.c*/
unsigned char SAV_SavedTransIndex(unsigned char ucFlag);
unsigned char SAV_ChangeCDSave(void);
unsigned char SAV_ChangeExpSave(void);


/*print.c*/
void PRT_PrintTicket(void);
void PRT_ConstantParam(void);
void PRT_TransType(void);
void PRT_ChangeParam(void);
void PRT_DateTime(void);
void PRT_PrintBalance(void);
void PRT_PrintADInfo(void);
void PRT_PrintInitData(void);
void PRT_Terminal(void);
void PRT_LocalDateTime(void);
void PRT_TerminalList(void);
void PRT_PrintTAC(void);
void PRT_Total(uchar ucPtcode);
void PRT_PrintSettle(void);
unsigned char PRT_Reprint(void);
unsigned char PRT_ReprintSettle(void);
unsigned char PRT_Detail(void);
unsigned char PRT_TerminalTt(void);
unsigned char PRT_ReprintbyTrace(void);
void PRT_ExpDateTime(void);
unsigned char PrintBar(ulong ulPayTrace);
void PRT_ZSH_Total(void);
void PRT_2ExpDateTime(void);



/*settle.c*/
unsigned char STL_SendOfflineBatch(void);
unsigned char STL_SendOfflineBatch_M(void);
unsigned char STL_Settle(void);
unsigned char STL_SettleProcess(void);
unsigned char STL_OnlineProcess(void);
unsigned char STL_SendBatchTAC(void);
unsigned char STL_SettleOnline(uchar ucPtcode);
unsigned char STL_SendKeyi(void);
unsigned char STL_SendReturn(void);
unsigned char STL_SendPay(void);
unsigned char STL_SendBarCode(void);
unsigned char STL_SendZSHPurchase(void);
unsigned char STL_SendSale(void);
unsigned char STL_SendOfflineBatch_P(void);
unsigned char STL_SendReturn_P(void);
unsigned char STL_ByCom(void);
unsigned char STL_SendTransferPurchase(void);
unsigned char STL_SendChangeExp(void);
unsigned char STL_SendExpPurchaseBatch(void);
unsigned char STL_SendPTCPurchase(void);
//scan.c
unsigned char Scan_Process(void);
unsigned char Scan_BarCode(void);
unsigned char Scan_ShowBarCodeInfo(void);
unsigned char PAYFEE_ReceiveData_COM1(unsigned int uiMaxLen,unsigned char *aucRevData,
							unsigned int *uiRevLen);
int CheckNumber(char* buffer,int buflen);
int CheckBar_X25(char* buffer,int buflen);
int CheckBar_Normal(char* buffer,int buflen,int modType);
int CheckBar_DB31(char* buffer,int buflen,int modType) ;
int AnalyMyString(char *sSour,int iLen,char *sDest);
int GetBillYearsBound(int *iUBound,int *iDBound);
int CheckBillField234(char *sCode,char *sField,int iLenField);
int Scan_BarAnalyze(char *pBarCode ,  CDBBillInfo *pBillInfo);
unsigned char Scan_FormatSearch(int index);
unsigned char CASH_CashierMenu(void);
unsigned char CASH_CheckCashierNo(unsigned char *pucIndex,unsigned char *pucCashierNo);
unsigned char CASH_UpdateCashierData(unsigned char ucNo,
				unsigned char *pucCashierNo,unsigned char *pucCashierPass);								

//port.c
unsigned char Port_Main_Process(void);
unsigned char Port_Card_trans(void);
unsigned char Port_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen);
unsigned char Port_Query_trans(void);
unsigned char Port_SendPortResult(unsigned char ucResp);
unsigned char Port_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen);
unsigned char PackPortResult(unsigned char ucResp);
unsigned char STL_SendPayNobar(void);

unsigned char Untouch_CardAdminSelectPt(void);
unsigned char	Load_Default_Keys_Check(void);

unsigned char Untouch_Menu(void);
unsigned char EXTrans_CheckSandCard(void);

unsigned char Trans_SwitchInform(void);
unsigned char XDATA_Read_SaveTrans_File(unsigned short uiIndex);
