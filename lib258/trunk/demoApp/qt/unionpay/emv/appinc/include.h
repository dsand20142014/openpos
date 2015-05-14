
#ifndef		_INCLUDE_H
#define		_INCLUDE_H

#include	<oslib.h>
#include	<EMV41.h>
#include  <new_drv.h>

#include  <rs232yth.h>
#define SANDREADER 1
#define WKPINPAD 1
#if SANDREADER
#include    <QPBOC.h>
#endif


#define ERR_TRANS_NOK      0x0F
#define ERR_TRANS_OK       0xF0

#define GUI_TTS_DATA
#define GUI_PROJECT
#define GUI_DELAY_TIMEOUT	1500
#define GUI_GETCARD_TIMES   1000

/*2009-10-12 16:08*/
/********************************************前台错误信息显示**********************/
//  1 为当前机型，同时只可有一个宏为1。
#define PS100	    0
#define PS300	    0			// REMOVED
#define PS400	    1

#if PS100
#define UpData				"YYYYMMDD"				//版本更新日期
#define CURRENT_PRJ		"FFFFFFFFFFFFFFF"				//项目编号－正常的项目号
#define SOFTWARE_VER		"x.x.x"					//软件版本－
#endif

#if PS300
#define CURRENT_PRJ		"FFFFFFFFFFFFFFF"		//项目编号
#define SOFTWARE_VER		"x.x.x"			//软件版本
#endif

#if PS400
#define AppSoftId				"\x00\x68"
#define CURRENT_PRJ		"B000-MEGP-MPT4-001"				//项目编号
#define UpData				"20111013"						//版本更新日期
#define SOFTWARE_VER		"5.0.9"							//软件版本
#endif

#define HARDWARE_VER		"v2.72"				//硬件版本
#define HARDWARE_NAME		"中国银联PBOC"		//项目名称

#define USEINSERTCARD				//该宏打开即可使用插入IC卡功能
//#define USEUPDATADATATIME	//直联非税控需要打开该宏更新日期时间，税控不允许更改日期时时。如是税控直联请关闭该宏
//#define EMVTEST						//使用该宏即为EMV测试状态

#ifndef _TEST
#define _TEST
#endif

 #define util_Printf  printf

#define READER_HONGBAO 1
#define READER_SAND    2


#define COM1		0x31
#define COM2		0x32

#ifndef TMS
//#define TMS
#endif

#define UCHAR   unsigned char
#define UINT    unsigned int
#define ULONG   unsigned long

#define uchar   unsigned char
#define uint    unsigned int
#define ulong   unsigned long					        //该宏为中国银联EMV2.0程序中是否支持TMS模块
#define ushort    unsigned short

//#define UNTOUCH					//该宏为是否支持手机移动支付交易模块

#define FILEWRITENUMBER	3
/*东方CJ参数定义*/

#define TRANS_MAXSENDNB				500
//#define TRANS_SAVEMAXNB				500
#define TRANS_SENDDATAONLINE            0x90
#define TRANS_SENDDATAPORT            	0x91
/*End*/
#define UnTest							//用于手机移动支付模块调试
/********************************************前台错误信息显示**********************/
#define SUCCESS								0x00
#define SUCCESS_TRACKDATA					0x00
#define ERR_END								0x01
#define ERR_CANCEL							0x02
#define ERR_DATASAVE							0x03
#define ERR_DRIVER							0x04
#define ERR_OSFUNC							0x05
#define ERR_NOTPROC							0x06
#define ERR_UNKNOWNTRANTYPE			       0x07
#define ERR_RESETDATASAVEDATA			       0x08
#define ERR_DIAL								0x09
#define ERR_HOSTCODE						0x0A
#define ERR_DATAINVALID						0x0B
#define ERR_INVALIDXDATAPAGE			0x0C
#define ERR_INVALIDTRANS					0x0D
#define ERR_TRANSFILEFULL					0x0E
#define ERR_NOPSAM							0x0F
#define ERR_NOTVALIDPSAM					0x10
#define ERR_NOOLDTRANS						0x11
#define ERR_PRINT							0x12
#define ERR_NOPIN							0x13
#define ERR_ALREADYVOID						0x14
#define ERR_NOTRANS							0x15
#define ERR_CommPortOpenErr				       0x16
#define ERR_TRANS_UNKNOWOPERTYPE			0x17
#define ERR_TRANS_UNKNOWTRANSTYPE			0x18
#define ERR_NOTPHONECARD                                0x19
#define ERR_VERIFY_PINONE						0x20
#define ERR_VERIFY_PINTOW						0x21
#define ERR_VERIFY_PINTHREE						0x22
#define ERR_NOFUNCT								0x30
#define ERR_NOZEOR									0x31
#define ERR_AMOUNTERR							0x41
#define ERR_WRITEMEMORY						0x42
#define ERR_CHECKSECRET							0x43
#define ERR_MODIFYSECRET						0x44
#define ERR_CARDLOCK								0x45
#define ERR_CARDCRC									0x46
#define ERR_TAGNOTEXIST							0x47
#define ERR_NOTSUPEAPP							0x48
#define ERR_INVALDDATE							0x49
#define ERR_PLSLOADPARAM						0x50
#define ERR_INTERNOTTIP							0x51
#define ERR_TRANSNOTSUP						0x52
#define ERR_INVAILDAMOUNT			0x53
#define ERR_TRANSADJUSTED				0x54
#define ERR_USEICCARDFIRST				0x70
#define ERR_EMV_MAGTRANS				0x71
#define ERR_EMV_ICTRANS					0x72
#define ERR_NOTRETURN                                0x7E
#define ERR_TRANSTIEMOUT				0x55
#define ERR_TRANSNotADJUST				0x56
#define ERR_PROUNTOUCH					0x85
#define ERR_OLDNOTVOID					0xFB
/********ERROR RETURN VALUE*********/

#define EMV_ERRDATALEN	        0x61
#define	EMV_ERRTAG					0x62
#define	EMV_ERRLENGTH				0x63
#define	EMV_ERRAPPNUM		    0x64
#define	EMV_ERRDATA					0x65
#define EMV_SETTLEFIRST			0x66
#define EMV_NOSUCHTRACE      0x67
#define EMV_DATAOVERFLOW    0x68
#define EMV_TRANSFAIL				0x69
#define ERR_APP_TIMEOUT			0xFF

#define ERR_ISO83										0x80
#define ERR_ISO8583_INVALIDVALUE			0x81
#define ERR_ISO8583_INVALIDBIT				0x82
#define ERR_ISO8583_INVALIDPACKLEN		0x83
#define ERR_ISO8583_OVERBITBUFLIMIT		0x84
#define ERR_ISO8583_BITNOTEXIST				0x85
#define ERR_ISO8583_INVALIDPARAM		0x86
#define ERR_ISO8583_NODATAPACK			0x87
#define ERR_ISO8583_OVERBUFLIMIT			0x88
#define ERR_ISO8583_INVALIDBITATTR		0x89
#define ERR_ISO8583_UNPACKDATA			0x8A
#define ERR_ISO8583_SETBITLEN					0x8B
#define ERR_ISO8583_COMPARE					0x8C
#define ERR_ISO8583_MACERROR				0x8D
#define ERR_ISO8583_INVALIDLEN				0x8E

#define ERR_UTIL_OVERBUFLIMIT				0xA0

#define ERR_MSG_INVALIDINDEX				0xB0
#define ERR_MSG_INVALIDVALUE				0xB1

#define ERR_MAG_TRACKDATA					0xC0

#define ERR_TRANS_UNKNOWNACCTYPE	0xC8
#define ERR_TRANS_SETTLE							0xC9
#define ERR_TRANS_SAVEINDEX					0xCA
#define ERR_TRANS_CANNOTADJUST			0xCB
#define ERR_COMMS_SENDCHAR				0xD0
#define ERR_COMMS_RECVCHAR				0xD1
#define ERR_COMMS_PROTOCOL				0xD2
#define ERR_COMMS_RECVBUFFOVERLIMIT	0xD3
#define ERR_COMMS_RECVTIMEOUT				0xD4
#define ERR_COMMS_LRC								0xD5
#define ERR_COMMS_ERRTPDU						0xD6
#define ERR_COMMS_NOMAC						0xD7
#define ERR_SANDMAGCARD                      		0xD8

#define ERR_CASH_EXIST				0xE0
#define ERR_CASH_NOTEXIST		0xE1
#define ERR_CASH_PASS				0xE2
#define ERR_CASH_FILEFULL		0xE3
#define ERR_CASH_NOTLOGON				0xE4
#define ERR_CASH_ALREADYLOGON		0xE5
#define ERR_ADMIN_CMD						0xE8
#define ERR_OFFLINE								0xE9
#define ERR_SYS_CASHIERNO					0xEA
#define ERR_READANDWRITEICCARD 		0xEB
#define ERR_EXPIREDATECARD					0xEC
#define ERR_ICCARD								0xED
#define ERR_CHECKAOUNMT					0xEE
#define ERR_CAPKCHECK							0xEF
#define ERR_TIMEOUTOFF						0xF0
#define ERR_SUPERKEY								0xE2

//----------------------------------------
#define ERR_RECVMAXLEN 					0xF1
#define ERR_TAGLEN 							0xF2
#define ERR_TAGVAL 							0xF3
#define ERR_COMMANDBYTE				0xF4
#define ERR_COMMINIT						0xF5
#define ERR_TAGDATA							0xF6
#define ERR_NOLASTSETTLE     			0xF7
#define ERR_CARDNO							0xF8
#define ERR_BALANCE							0xF9
#define ERR_MULTICARD						0xFA
#define ERR_TRANSEMPTY					0xe6		/*交易流水为空*/
#define ERR_SETTLE_FIRST					0xe7
#define ERR_FALLBACK              			0xe8
#define ERR_FORMAT							0x60
#define ERR_NOOLDCARD					0x8F
#define ERR_WRITEFILE					        0xAA
#define ERR_CHECKKEY        				    0xA0
/********************************************前台错误信息显示end**********************/

//该位置为1
#define SETBIT8 		0x80
#define SETBIT7 		0x40
#define SETBIT6 		0x20
#define SETBIT5 		0x10
#define SETBIT4 		0x08
#define SETBIT3 		0x04
#define SETBIT2 		0x02
#define SETBIT1 		0x01
//该位置为0
#define UNBIT8 		0x7F
#define UNBIT7 		0xBF
#define UNBIT6 		0xDF
#define UNBIT5 		0xEF
#define UNBIT4 		0xF7
#define UNBIT3 		0xFB
#define UNBIT2 		0xFD
#define UNBIT1 		0xFE


#define Display_MAXNB					18
#define ACQUERELEN						11
#define ISSUERIDLEN						11
#define MAXLINECHARNUM			16

#define MAXGBCHARNUM				12
/* Global define */
#define ONESECOND						100
#define MAXMENUITEM					20
#define DATASAVEVALIDFLAG			0x55
#define DATASAVEPAGE_NORMAL			0
#define DATASAVEPAGE_TRANS				1
#define DATASAVEPAGE_BLACKLIST		2
#define EMV_MAXTAGNB						20
//#define EMV_MAXTRANSNUM                 300
#define CFG_MAXDISPCHAR					16
#define DISPCOLFIRST								0
#define CFG_MAXAMOUNTLEN				10
#define PARAM_SWITCHLEN					20
#define PARAM_APN								30
/* Waits for the end of the background dialling during 'TimeOut' x 10 ms */
#define CFG_CHECKDIALTIMEOUT			100

/* PINPAD Key define */
#if PS100
#define PINPAD     /*if define PINPAD ,there is external pinpad*/
#endif
#define KEYARRAY_GOLDENCARDSH		0x01

#define KEYINDEX_GOLDENCARDSH_PINKEY			0x07
#define KEYINDEX_GOLDENCARDSH_PINKEY2		0x08
#define KEYINDEX_GOLDENCARDSH_MACKEY		0x09

#define KEYINDEX_GOLDENCARDSH_CASHIERKEY	0x01
#define KEYARRAY_PURSECARD								0x00
#define KEYINDEX_PURSECARD_MASTERKEY			0x00
#define KEYINDEX_PURSECARD_CASHIERKEY			0x01
#define KEYINDEX_PURSECARD_PINKEY					0x02
#define KEYINDEX_PURSECARD_MACKEY				0x03
#define KEYINDEX_PURSECARD_ISSKEY					0x04
#define KEYINDEX_PURSECARD_WRITEKEY				0x06
#define KEYINDEX_PURSECARD_CRCKEY					0x08

#define KEYINDEX_GOLDENCARDSH_KEK			0x24

/* Trans info define */
#define TRANS_DATELEN					4
#define TRANS_TIMELEN						3
#define TRANS_ACCLEN						10
#define TRANS_GROUPLEN				3
#define TRANS_UintCodeLEN			11
#define	TRANS_REMARK						60
#define TRANS_AUTHCODELEN		6
#define TRANS_AUTHCODEMIN		2
#define TRANS_BANKIDLEN				6
#define TRANS_REFNUMLEN				12
#define TRANS_CASHIERNOLEN		4
#define TRANS_COMMLEN					41
#define TRANS_ISO2MINLEN				12
#define TRANS_ISO2LEN						37
#define TRANS_ISO3LEN						104
#define TRANS_PINDATALEN				8
#define TRANS_BATCHNUMLEN		6
#define TRANS_TRACENUMLEN		6
#define TRANS_TICKETNUMLEN		6
#define  TRANS_ORDERSNO			10 //订单编号
#define TRANS_INPUTMODE_SWIPECARD				0x01
#define TRANS_INPUTMODE_MANUALINPUT			0x02
#define TRANS_INPUTMODE_INSERTCARD 				0x04
#define TRANS_INPUTMODE_FULLBACK         			0x08
#define TRANS_INPUTMODE_PUTCARD						0x10
#define TRANS_INPUTMODE_HB_PUTCARD      		0x11
#define TRANS_INPUTMODE_SAND_PUTCARD 		0x12

#define TRANS_TERMINALTYPE_NORMAL		0x00
#define TRANS_TERMINALTYPE_FOREIGN		0x01

#define TRANS_ACCTYPE_DEFAULT			0
#define TRANS_ACCTYPE_ED					4
#define TRANS_ACCTYPE_EP					5

#define TRANS_CARDTYPE_INTERNAL		0
#define TRANS_CARDTYPE_VISA				1
#define TRANS_CARDTYPE_MASTER			2
#define TRANS_CARDTYPE_JCB				3
#define TRANS_CARDTYPE_DINNER			4
#define TRANS_CARDTYPE_AE					5
#define TRANS_CARDTYPE_FOREIGN		6

#define TRANS_CARDTYPE_INTERNALSMART	10

#define TRANS_BATCHSEND_INTERNAL	0x01
#define TRANS_BATCHSEND_FOREIGN		0x02


// 多线程使用交易类型来处理原先POS交易

#define TRANS_AID_PARAM	0xA3
#define TRANS_RID_PARAM	0xA4
#define TRANS_PRINT_BYTRACE	0xA1
#define TRANS_PRINT_LASTTRANS	0xA2
#define TRANS_PRINT_LASTSETTLE		0xA5
#define TRANS_PRINT_TRANSDETAIL	0xA6	
#define TRANS_PRINT_STATIC			0xA7



#define TRANS_SHOW_STATIC			0xA8
#define TRANS_SHOW_VERSION		0xA9

#define TRANS_SHOW_TRNSDETAIL		0xAA

#define TRANS_PRINT_TTS			0xAB

#define TRANS_QUERYBAL						0x01	//余额查询
#define TRANS_PURCHASE						0x02	//消费
#define TRANS_CASHOUT						0x03
#define TRANS_REFUND						0x04	// 退货
#define TRANS_DEPOSIT						0x05
#define TRANS_TRANSFER						0x06
#define TRANS_AUTH							0x07
#define TRANS_PREAUTH						0x08	//预授权
#define TRANS_PREAUTHFINISH				       0x09	//预授权完成联机
#define TRANS_OFFPURCHASE					0x0A
#define TRANS_OFFCASHOUT					0x0B
#define TRANS_TIPADJUST						0x0C	//离线调整
#define TRANS_OFFPREAUTHFINISH		              0x0D	//离线结算
#define TRANS_TIPADJUSTOK					0x0E
#define TRANS_OFFPREAUTHFINISHOK	              0x0F
#define TRANS_PREAUTHSETTLE                              0x10	//预授权完成离线
#define TRANS_PREAUTHADD                                  0x11	//追加预授权
#define TRANS_UNDOOFF						 			0x12	//离线撤销
///////////////////////////////////////////////////////
#define TRANS_EC_PURCHASE				    		0x13  //电子现金消费2009-9-4 17:25 cbf
#define TRANS_EC_ASSIGNLOAD				    	0x14  //指定账户圈存
#define TRANS_EC_UNASSIGNLOAD			    0x15  //非指定账户圈存
#define TRANS_EC_CASHLOAD 				    	0x16  //现金充值
#define TRANS_Q_PURCHASE				    		0x17	//快速消费,非接触
#define TRANS_OPENINGCREDITS				    0x18	//信用额度开通
#define TRANS_CREDITSALE							    0x19	//信用消费
#define TRANS_CREDITSVOID						 0x24	//信用消费撤消
#define TRANS_CREDITSQUERY					    0x25	//信用余额查询
#define TRANS_CREDITSREFUND					    0x23	//信用退货
/////////////////////////////////////////////////////
#define TRANS_REPRINT							0x79
#define TRANS_VOID								0x80
#define TRANS_VOIDPURCHASE				       0x82	//消费撤销
#define TRANS_VOIDCASHOUT					0x83
#define TRANS_VOIDREFUND					0x84
#define TRANS_VOIDDEPOSIT					0x85
#define TRANS_VOIDTRANSFER				0x86
#define TRANS_VOIDAUTH						0x87
#define TRANS_VOIDPREAUTH					0x88	//预授权撤销
#define TRANS_VOIDPREAUTHFINISH		              0x89	//预授权完成撤销
#define TRANS_VOIDOFFPURCHASE				0x8A
#define TRANS_LOGONON						0xF0
#define TRANS_LOGONOFF						0xF1
#define TRANS_CHANGESUPERPASS				0xF2
#define TRANS_BATCHUP						0xF3
#define TRANS_SETTLE							0xF4
#define TRANS_REVERSAL						0xF5
#define TRANS_CASHIERLOGON					0xF6
#define TRANS_CASHIERLOGOFF					0xF7
#define TRANS_LOADPARAM					0xF8
#define TRANS_ONLINETEST					0xF9
#define TRANS_SENDSTATS						0xFA
#define TRANS_SCRIPTINFORM              			0xFB
#define TRANS_CUPMOBILE						0x1A //移动支付消费
#define TRANS_VOIDCUPMOBILE					0x1B //移动支付消费撤销
#define TRANS_SENDCOLLECTDATA				0x1C
#define TRANS_OPERATOR						0x1E //操作员认证类型
#define TRANS_UPTMKEY						0x1F //密钥下载类型

#define TRANS_MOBILEAUTH					0x20 //手机无卡预约消费
#define TRANS_VOIDMOBILEAUTH				0x21 //手机无卡预约消费撤销
#define TRANS_MOBILEREFUND					0x22 //手机无卡预约退货

#define TRANS_SAVEMAXNB					500

#define TRANS_NIINORMAL					0x000F	/*普通交易*/
#define TRANS_NIISENDFINISH				0x00F0	/*上送完成交易的离线或调帐交易*/
#define TRANS_NIIVOID					0x0F00	/*取消交易*/
#define TRANS_BATCHSEND					0xF000	/*批上送交易*/

#define PARAM_DOWNKEYCOMM_HDLC		1
#define PARAM_DOWNKEYCOMM_GPRS		2
#define PARAM_DOWNKEYCOMM_TCPIP		3
#define PARAM_DOWNKEYCOMM_COMM		4

/* Terminal Parameter */
#define PARAM_PASSWORDLEN				6
#define PARAM_TERMINALIDLEN			8
#define PARAM_MERCHANTIDLEN			15
#define PARAM_MERCHANTNAMELEN	40
#define PARAM_TERMINALAPP				2
#define PARAM_TELNUMBERLEN				30
#define PARAM_HOSTNUMBERLEN			14
#define PARAM_GLOBALTRANSENABLELEN	2
#define PARAM_DATELEN						3
#define PARAM_CONFIGLEN					10
#define PARAM_DISPLAYMINLINE			2
#define PARAM_DISPLAYMAXLINE			4

#define PARAM_COMMMODE_MODEM			0
#define PARAM_COMMMODE_RS232				1
#define PARAM_COMMMODE_HDLC				2
#define PARAM_COMMMODE_GPRS				3
#define PARAM_COMMMODE_CDMA				4
#define PARAM_COMMMODE_TCPIP				5


#define PARAM_ISSUEIDLEN							8
/* Printer Parameter */
#define PRN_MAXCHAR									24

/* comms.c */
#define CHAR_STX				0x02
#define CHAR_ETX				0x03

/* Mirror Space Define */
#define MIRROR_MAXLEN			3072
#define MIRROR_NOTEXISTFLAG 	0x00
#define MIRROR_EXISTFLAG		0x55
#define MIRROR_INCLUDECHKFLAG	0xff
#define MIRROR_CLRFLAG			0xff

/* ISO8583 Define */
#define ISO8583_MSGIDLEN			2
#define ISO8583_BITMAPLEN			16
#define ISO8583_MAXBITNUM			128
#define ISO8583_MAXBITBUFLEN		1024	/* Max unpacked ISO8583 buffer */
#define ISO8583_MAXCOMMBUFLEN	1024    //512		/* Max Iso8583 communication buffer */
#define ISO8583_BITNOTEXISTFLAG 	0x00
#define ISO8583_BITEXISTFLAG		0xff
#define ISO8583_BIT48NUM			8
#define ISO8583_ICTRANSINFOLEN  	0xff

#define ATTR_N					0x01
#define ATTR_AN					0x02
#define ATTR_NS					0x03
#define ATTR_XN					0x04
#define ATTR_ANS				0x05
#define ATTR_LLVARN			0x06
#define ATTR_LLVARNS		0x07
#define ATTR_LLVARAN		0x08
#define ATTR_LLVARANS		0x09
#define ATTR_LLLVARANS	0x0A
#define ATTR_LLVARZ			0x0B
#define ATTR_LLLVARZ			0x0C
#define ATTR_BIN					0x0D
#define ATTR_LLLVARN		0x0E
#define ATTR_LLVARANZS	0x10
#define ATTR_LLLVARANZS 0x11

/* cash.c */
#define CASH_MAXSUPERNO             			2
#define CASH_MAXSUPERPASSWDLEN		6
#define CASH_MAXCASHIER						25
#define CASH_CASHIERNOLEN			    	2
#define CASH_CASHIERPASSLEN				4
#define CASH_SYSCASHIERNOLEN			2
#define CASH_SYSCASHIERPASSLEN		6

#define CASH_MAXTime					4
#define CASH_MINTime					4

#define CASH_LOGONFLAG			0x55
#define TRANS_NumStages			2
#define TRANS_NumProject			30
/* msg.c */
#define MSG_TYPEZHCN	'0'
#define MSG_TYPEEN		'1'

/* serv.c*/
#define SETTLE_OK		0x00
#define SETTLE_FAIL  	0x01
#define SETTLE_ERROR	0x02

/* sev.c*/
#define OldAuthType_POS		0x30
#define OldAuthType_TEL		0x31
#define OldAuthType_OTHER	0x32

#define EMVFIRSTMAG		0x11
#define EMVFIRSTICC			0x22

#define TYPE_HOTEL	0X32
#define TYPE_SHOP   0x31
#define RESTAURANT	0x33

/****************************************显示提示信息***********************/

#define MSG_NAME						0		/*上海金卡*/
#define MSG_QUERYBAL				1       /*余额查询*/
#define MSG_INPUTNEWPIN		2       /*输新密码*/
#define MSG_BALANCE				3       /*"余额*/
#define MSG_MASTERLOAD		4       /*主密钥下装*/
#define MSG_EDQUERYBAL			5       /*电子存折余额*/
#define MSG_EPQUERYBAL			6       /*电子钱包余额*/
#define MSG_PURCHASE				7       /*消费*/
#define MSG_SENDSTATS			8       /*状态上传*/
#define MSG_LOADPARAM			9       /*参数下载*/
#define MSG_ONLINETEST			10      /*回响测试*/
#define MSG_SETDATETIME			11      /*设置系统时间*/
#define MSG_FPHONENO			12      /*分机设置*/
#define MSG_INQUIRYVER			13      /*版本查询*/
#define MSG_CLEARTRANS			14      /*请交易流水*/
#define MSG_CHANGESTATUS	15      /*状态切换*/
#define MSG_KEYBOARDLOCK		16      /*借记卡取款*/
#define MSG_PRINTTEST				17      /*打印测试*/
#define MSG_EPCASHOUT			18      /*钱包取款*/
#define MSG_REFUND					19      /*退货*/
#define MSG_MAGREFUND			20      /*磁卡退货*/
#define MSG_CREDITREFUND		21      /*信用卡退货*/
#define MSG_DEBITREFUND		22      /*借记卡退货*/
#define MSG_EDREFUND				23      /*存折退货*/
#define MSG_EPREFUND				24      /*钱包退货*/
#define MSG_DEPOSIT					25      /*存款*/
#define MSG_MAGDEPOSIT			26      /*磁卡存款*/
#define MSG_CREDITDEPOSIT		27      /*信用卡存款*/
#define MSG_DEBITDEPOSIT		28      /*借记卡存款*/
#define MSG_EDDEPOSIT				29      /*存折存款*/
#define MSG_EPDEPOSIT				30      /*钱包存款*/
#define MSG_TRANSFER				31      /*转帐*/
#define MSG_MAGTRANSFER		32      /*磁卡转帐*/
#define MSG_SYSTEMINIT			33      /*系统初始化*/
#define MSG_DEBITTRANSFER		34      /*借记卡转帐*/
#define MSG_EDTRANSFER			35      /*存折转帐*/
#define MSG_EPTRANSFER			36      /*钱包转帐*/
#define MSG_AUTH						37      /*授权*/
#define MSG_MAGAUTH				38      /*磁卡授权*/
#define MSG_CREDITAUTH			39      /*信用卡授权*/
#define MSG_DEBITAUTH				40      /*借记卡授权*/
#define MSG_EDAUTH					41      /*存折授权*/
#define MSG_EPAUTH					42      /*钱包授权*/
#define MSG_PREAUTH				43      /*预授权*/
#define MSG_PREAUTHFINISH		44      /*预授权完成*/
#define MSG_CREDITPREAUTH		45     	/*信用卡预授权*/
#define MSG_DEBITPREAUTH		46      /*借记卡预授权*/
#define MSG_EDPREAUTH			47      /*存折联机认证*/
#define MSG_EPPREAUTH			48      /*钱包联机认证*/
#define MSG_VOID						49      /*撤销*/
#define MSG_VOIDPURCHASE		50      /*消费撤销*/
#define MSG_VOIDCASHOUT		51      /*取款撤销*/
#define MSG_VOIDREFUND			52      /*退货撤销*/
#define MSG_VOIDDEPOSIT		53      /*存款撤销*/
#define MSG_VOIDTRANSFER		54      /*转帐撤销*/
#define MSG_VOIDAUTH				55      /*授权撤销*/
#define MSG_VOIDPREAUTH				56      /*预授权撤销*/
#define MSG_VOIDPREAUTHFINISH	57      /*授权完成撤销*/

#define MSG_DATETIME				58      /*时间*/
#define MSG_REFERENCE				59      /*备注:*/
#define MSG_HOLDERSIGN			60      /*持卡人签名*/
#define MSG_REVERSALFAIL		61      /*冲正失败*/
#define MSG_REVERSALSUCC		62      /*冲正成功*/
#define MSG_HOLDERCONFIRM			63      /*本人确认该交易*/
#define MSG_HOLDERCONFIRM2		64      /*同意记入本帐户*/

#define MSG_CONFIGBASEDATA			65     	/*参数设置*/
//#define MSG_CONFIGBASEDATA	65     /*终端号商户号*/
#define MSG_CONFIGPBOCDATA		66      /*互通卡设置*/
#define MSG_DOAPPSELECT				67      /*应用是否选择*/
#define MSG_PSAMREADER					68      /*密钥卡卡槽*/

#define MSG_SETTLE				69      /*结算*/

#define MSG_APPSELECT					70    /*应用选择*/
#define MSG_INPUTMASTERKEY		71    /*输入主密钥*/
#define MSG_INPUTKEYINDEX			72    /*输入密钥索引*/
#define MSG_INPUTPIN					73	/*输入个人密码*/
#define MSG_AMOUNT					74     /*"金额:*/
#define MSG_ADDCASHIER				75     /*柜员增加*/
#define MSG_DELCASHIER				76     /*柜员删除*/
#define MSG_MODICASHIER			77     /*柜员修改*/
#define MSG_CASHIERNO				78     /*柜员号:*/
#define MSG_CASHIERPASS				79     /*柜员密码:*/
#define MSG_CASHIERNEWPASS		80     /*柜员新密码:*/
#define MSG_POSLOGON			81     /*设备签到*/
#define MSG_POSLOGOFF			82     /*设备签退*/
#define MSG_SUPERPASS			83     /*系统管理员密码:*/
#define MSG_LOGONSUCCESS	84     /*签到成功*/
#define MSG_LOGONFAIL			85     /*签到失败*/
#define MSG_TERMINALID			86     /*终端号:*/
#define MSG_MERCHANTID			87     /*商户号:*/
#define MSG_COMMMODE			88     /*M 1-HDLC,2-232*/
#define MSG_DIALRETRY				89     /*"Dial Retry Time */
#define MSG_DIALWAITTIME		90       /*Dial Wait Times*/
#define MSG_COMMSPEED			91       /*Comm Speed:*/
#define MSG_RECEIVETIMEOUT		92       /*Receive Timeout*/
#define MSG_HOSTTEL1			93       /*中心号１:*/
#define MSG_HOSTTEL2			94      /*中心号２:*/
#define MSG_HOSTTEL3			95       /*I中心号码3 */
#define MSG_HOSTTEL4			96       /*管理号码:*/

#define MSG_REFNUM				97      /*参考号:*/
#define MSG_AUTHCODE			98      /*授权号:*/
#define MSG_BATCHNUM			99      /*批次号:*/
#define MSG_TRACENUM			100      /*凭证号:*/
#define MSG_TRANSDATE			101      /*交易日期(MMDD)*/
#define MSG_CASHIERPINRELOAD	102      /*柜员密码置零*/
#define MSG_CHANGESUPERPASS		103      /*修改主管密码*/
#define MSG_GOON				104      /*继续?*/
#define MSG_CANCEL				105      /*取消?*/
#define MSG_CASH_EXIST			106      /*柜员已存在*/
#define MSG_NOTEXIST			107      /*柜员不存在*/
#define MSG_CASHIERNOTLOGON		108      /*柜员未签到*/
#define MSG_MERCHANTNAME		109      /*商户名称:*/
#define MSG_DIALING				110      	/*拨号中．．．*/
#define MSG_SENDING				111      	/*发送中．．．*/
#define MSG_RECEIVING			112      	/*接收中．．．*/
#define MSG_SAVING				113     	/*保存*/
#define MSG_TPDU            	114    	 /*TPDU:*/
#define MSG_REVERSAL			115    	 /*冲正．．．*/

#define MSG_DATE				116			/*日期:*/
#define MSG_TIME				117     	/*时间:*/
#define MSG_CARDNO				118     	/*请输入卡号:*/
#define MSG_ISSUERID			119     	/*发卡行号:*/
#define MSG_TRANSTYPE			120     	/*交易类型:*/
#define MSG_SPDB				121     	/*建设银行上海分行*/
#define MSG_ALREADYVOID			122     	/*交易已撤销*/
#define MSG_NOTRANS				123     	/*交易不存在*/
#define MSG_REPRINT				124     	/*重打最后一笔*/
#define MSG_PRINTNUMBER			125     	/*打印第几笔?*/
#define MSG_CASHIERLOGON		126     	/*柜员签到*/
#define MSG_CASHIERLOGOFF		127		/*柜员签退*/
#define MSG_LIST				128     	/*结算统计单*/
#define MSG_TOTALNB				129     	/*笔数总计*/
#define MSG_TOTALAMOUNT			130     	/*金额总计*/
#define MSG_REFUNDNB			131     	/*退货总笔数*/
#define MSG_PURCHASEAMOUNT		132     	/*消费总金额*/
#define MSG_AUTHFINISHAMOUNT	133 		/*授权总金额*/
#define MSG_REFUNDAMOUNT		134      	/*退货总金额*/
#define MSG_LISTPRINT			135      	/*打印交易明细*/
#define MSG_LOGOFFSUCCESS		136      	/*签退成功*/
#define MSG_SYSPASS				137	  	/*系统密码*/
#define MSG_EXPDATE				138	  	/*卡有效期(MMYY):*/
#define MSG_SETTLEOK			139       	/*结算成功!*/
#define MSG_SETTLEFAIL			140       	/*结算失败!*/
#define MSG_CONFIRM				141	  	/*请确认*/
#define MSG_PRINTLASTERONE		142        	/*打印最后一笔?*/

#define MSG_SPECIALTRANS		143        	/*刷转出卡*/
#define MSG_PRINTING			144	  	/*正在打印...*/
#define MSG_CARDLOKE        	145	  	/*卡片被锁*/
#define MSG_TIPADJUST			146	  	/*离线调整*/
#define MSG_FEEAMOUNT			147	  	/*小费*/
#define MSG_OFFPREAUTHFINISH	148        	/*离线结算*/
#define MSG_IFINPUTPIN			149	   	/*输密码请确认*/
#define MSG_INPUTDT				150		/*系统时间设置*/
/********************电子消费卡测试*****************************************/
#define MSG_READCARD			151		/*读同步卡*/
#define MSG_WRITECARD			152		/*写同步卡*/
#define MSG_SECRETCARD			153		/*卡密钥认证*/
#define MSG_MODIFYCARD			154		/*改卡密钥*/
#define MSG_DIAL_T				155		/*DAIL T=*/
#define MSG_SWITCHBOARD			156		/*SWITCHBOARD=*/
#define MSG_TERMINALTOTAL		157		/*终端统计*/
/******************************************************************************/
#define MSG_INIT_SUPER_PASSWORD 	158		/*初始化主管密码*/
#define MSG_INPUT_MERCHANT_NAME 	159		/*输入商户名*/
#define MSG_DELETE_REVERSAL_FLAG 	160		/*删除冲正标志*/
#define MSG_LOADDEFUALTKEY		161		/*下载默认密钥*/
#define MSG_SETTLE_UNBALANCE 		162 		/*结算不平*/
#define MSG_TRANS_LIST				163		/*流水明细查询*/
#define MSG_WELCOME				164		/*欢迎使用*/
#define MSG_CLEAR_TRANS 			165		/*清除交易流水*/

#define MSG_SYS_CHANGEPASS 		166		/*系统管理员改密*/
#define MSG_ADD_SUPVCASHIERNO 	167		/*主管操作员增加*/
#define MSG_DEL_SUPVCASHIERNO 	168		/*主管操作员删除*/
#define MSG_CHG_SUPVCASHIERNO 	169		/*主管操作员改密*/
#define MSG_SYS_CASHIERNO 			170		/*输主管操作员号*/
#define MSG_SYS_CASHIERPASS 		171		/*输主管密码*/
#define MSG_SET_SYSTRANS	 		172		/*系统参数设定*/
#define MSG_PREAUTHSETTLE     		173    	/*预授权结算*/
#define MSG_OFFPURCHASE       		174    	/*脱机消费*/
#define MSG_VOIDOFFPURCHASE   		175    	/*脱机消费撤消*/
#define MSG_DELETE_SCRIPT_FLAG 	176 		/*删除脚本通知标志*/
#define MSG_PKDOWNLOAD          		177   	/*公钥下载*/
#define MSG_EMVPARAMDOWNLOAD   	178   	/*EMV参数下载*/
#define SET_PRINTER					179 		/* 打印机设置 */
#define MSG_REPRINTBYTRACE			180    	/* 重打任意一笔*/
#define MSG_TANSTOTAL				181    	/* 打印交易汇总*/
#define MSG_PREAUTHADD            		182     	/* 预授权追加*/
#define MSG_CLEARMEMERY			183		/* 清内存 */
#define MSG_SETPINPAD				184		/*设置密码键盘*/
#define MSG_CASHIERQUERY			185		/*柜员查询*/
#define MSG_AUTHADDAMOUNT 		186 		/*累计金额*/
#define MSG_READINGCARD			187 		/*正在读卡*/
#define MSG_SETREADER				188		/*外部读卡器设置*/
#define MSG_REPRINTSETTLE			189		/*重打结算单*/
#define MSG_RATESET 				190		/*扣率设置*/
#define MSG_DEFAULTKEY				191		/*设置银行主密钥*/
#define MSG_MUNUALSETKEY			192		/*手工输入密钥*/

/****************************************显示BANKIC***********************/
#define BANK_TRAFFICBANK			193   	/*交通银行上海分行*/
#define BANK_CONSTRUCT  			194 		/*建设银行上海分行*/
#define BANK_INDUSCOMMERCE		195   	/*工商银行上海分行*/
#define BANK_XINGYE					196    	/*兴业银行上海分行*/
#define BANK_HUAXIA					197 		/*华夏银行上海分行*/
#define BANK_AGRICULTURE		198   	/*农业银行上海分行*/
#define BANK_SHENGDEVELOP	199       /*深发展上海分行*/
#define BANK_GUANGDA     		200       /*光大银行上海分行*/
#define BANK_CHINABANK   		201       /*中国银行上海分行*/
#define BANK_MINSHENG			202       /*民生银行上海分行*/
#define BANK_SHANGHAI			203		/*上海银行*/
#define BANK_ZHAOSHANG		204		/*招商银行上海分行*/
#define BANK_GUANGFA				205		/*广发银行上海分行*/
#define BANK_ZHONGXIN			206		/*中信银行上海分行*/
#define BANK_OTHERS					207		/*其他银行*/
#define BANK_PUFA						208		/*浦发银行*/
#define BANK_COUNTRY				209		/*农村信用社*///原农村信用社，现农村商业银行
#define BANK_POSTSAVE            	210		/*邮政储蓄*///ADD BY JIANG.GS
#define MSG_SetOutoSettleTime	211		/*设置自动结算时间*/
#define MSG_SUPERKEY				212		/*主管密码恢复*/
#define MSG_SUPERMOD			213		/*超级密码*/
#define BANK_NINGBO				214		/*宁波银行上海分行*/
#define MSG_SwitchComm			215		/*切换通讯*/
#define MSG_QuanCun				216		/*圈存*/
#define MSG_EnqBalance				217		/*余额查询*/
#define MSG_TestSignal				218		/*回响测试*/
#define MSG_ModManKey			219		/*修改主管密码*/
#define MSG_DOWNKEY				220		/*从母POS下发主密钥*/
#define MSG_INITDATA				221		/*清除数据*/
#define MSG_VOIDCUP				222		/*移动支付撤销*/
#define MSG_KEYLOAD				223		/*特殊密钥下载*/
#define MSG_SETKEYPARA			224		/*设置远程密钥下载参数*/
#define MSG_UCSETREADER           225  /*读卡器设置*/
#define MSG_CLEARCOLLECT           226  /*清除采集数据*/
#define MSG_DKFROMC           227  /*从密钥卡下密钥*/
/****************************************显示提示信息end***********************/

#define ERR_MERSET              0x90

#define MSG_DEFAULTMSGEN		"Default\0"
#define MSG_DEFAULTMSGZHCN	"缺省\0"
#define MSG_INVALIDMSGEN		"Invalid Index\0"
#define MSG_INVALIDMSGZHCN	"无效\0"

#define MSG_MAXERRMSG			100
#define MSG_MAXNB				255
#define MSG_MAXHOSTMSG		100
#define MSG_MAXCHAR			26
#define MSG_MAXTYPE			2
/* comms.c */
#define COMMS_RECVTIMEOUT		2   //changed 08-09-16 Before 200
/*UTIL.c*/
#define PAPER_KM			0x01
#define PAPER_PE				0x02
#define PAPER_XSBN		0x03

/* pboc.c */
#define PBOC_MAXTERMINALAPP	10
#define PBOC_MAXCARDAPP		5
#define PBOC_MAXAPPIDLEN		16
#define PBOC_ISSUERIDLEN		8
#define PBOC_APPLINOLEN		10

/* admin.c */
#define ADMIN_END						'0'
#define ADMIN_DOWNLOADMSG			'1'
#define ADMIN_DOWNLOADERRMSG			'2'
#define ADMIN_DOWNLOADISO8583ATT	'3'

#define ADMIN_COMMANDOFFSET			  0
#define ADMIN_DATAOFFSET			  	1
#define MAXMENUDISPLINE				10

/* print.c */
#define PRINT_MAXCHAR			50
#define PRINT_MAXRETRYTIME		3
#define RE_PRINT_FLAG			0x01
#define NORMAL_PRINT_FLAG		0x02
#define PRINT_LIST_FLAG			0x04
#define PRINT_TOTAL_FLAG		0x08
#define PRINT_SETTLE_FLAG       	0x10
/* Macro */
#define min(a, b)           (((a) < (b)) ? (a) : (b))

#define EMVAIDLEN              17
#define SCRIPTINFORMMAX       300

#define ONLYMAGTRANS

/* 	Types  */

#define ULONG_C51 unsigned long
#define UINT_C51 unsigned int

#define PRINTER_HEAT_FORMAT_1	0x01	//热敏格式一
#define PRINTER_HEAT_FORMAT_2	0x02	//热敏格式二
#define PRINTER_HEAT_FORMAT_3	0x03	//热敏格式三
#define PRINTER_HEAT_FORMAT_4	0x04	//热敏格式三

#define MobileNUMLen 			12
#define MobileCHKLen 			7

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

/* Smart Card struction define *
typedef struct
{
	unsigned short	uiTag;
	unsigned char	ucChildExist;
	unsigned char	ucLen;
}EMVTAG;

typedef struct
{
	unsigned char	ucNumber;
	unsigned short	auiTag[EMV_MAXTAGNB];
	unsigned char	aucLen[EMV_MAXTAGNB];
	unsigned char	*pucOffset[EMV_MAXTAGNB];
}EMVTAGBuf;
*/
typedef struct
{
	unsigned char	ucAppNumber;
	unsigned char	aucAppType[PBOC_MAXTERMINALAPP];
	unsigned char	aucAppID[PBOC_MAXTERMINALAPP][PBOC_MAXAPPIDLEN];
}TERMINALAPP;

typedef struct
{
	unsigned char	ucAppNumber;
	unsigned char	aucAppIDLen[PBOC_MAXCARDAPP];
	unsigned char	aucAppID[PBOC_MAXCARDAPP][PBOC_MAXAPPIDLEN];
	unsigned char	aucAppLabel[PBOC_MAXCARDAPP][PBOC_MAXAPPIDLEN+1];
	unsigned char	ucCurrAppNumber;
	unsigned char	ucTermAppNumber;
}CARDAPP;

typedef struct
{
	unsigned short uiSendIndex;
}SAVESENDINDEX;

typedef struct
{
	unsigned char	aucIssuerID[PBOC_ISSUERIDLEN];
	unsigned char	ucAppliID;
	unsigned char	ucAppliVersion;
	unsigned char	aucAppliNo[PBOC_APPLINOLEN];
	unsigned char	aucAppliOpenDate[4];
	unsigned char	aucAppliExpiredDate[4];
	unsigned char	aucIssuerPrivateDate[2];
}PBOCBANKDATA;
/* Memory Card */
typedef struct
{
	unsigned char	aucMarketNo[8];
	unsigned char	aucSerial[2];
	unsigned char 	aucAccountNo[8];  	/*卡号*/
	unsigned char   aucBIN;			/*银行号*/
	unsigned char   aucCardMap;
	unsigned char	aucKeyVer;
    unsigned char 	aucRFU[3];
	unsigned char   aucOpenDate[4];		/*制卡日期*/
	unsigned char 	aucSaveDate[4];		/*最近批充值日期*/
	unsigned int   Amount;
    unsigned char	aucCRC[4];
}SLE4442DATA;

enum CARDSPECIES {CARDSPECIES_MAG,CARDSPECIES_EMV,CARDSPECIES_UNTOUCH,CARDSPECIES_HB_UNTOUCH};
enum LOADPARAMTYPE {LOADPARAMTYPE_MAG,
						LOADPARAMTYPE_PKQUERY,
						LOADPARAMTYPE_PKLOAD,
						LOADPARAMTYPE_PKLOADEND,
						LOADPARAMTYPE_RID,
						LOADPARAMTYPE_AIDQUERY,
						LOADPARAMTYPE_AIDLOAD,
						LOADPARAMTYPE_AIDLOADEND};
enum ENCRYPTTYPE {ENCRYPTTYPE_SINGLE,ENCRYPTTYPE_DOUBLE};

// 杉德Qt应用新加接口
typedef struct
{
	//调试窗口: progress 到UI界面
	unsigned char ucClsFlag;	// 0xAA 清除屏幕
	
	
	unsigned int uiLines;		//显示行数
	unsigned char aucLine1[64];
	unsigned char aucLine2[64];
	unsigned char aucLine3[64];
	unsigned char aucLine4[64];	
	unsigned char aucLine5[64];	
	unsigned char aucLine6[64];	
	unsigned char aucLine7[64];	
	unsigned char aucLine8[64];		
	unsigned char aucLine9[64];
	
	unsigned char aucLine10[64];
	unsigned char aucLine11[64];
	unsigned char aucLine12[64];
	unsigned char aucLine13[64];
	unsigned char aucLine14[64];	
	unsigned char aucLine15[64];	
	unsigned char aucLine16[64];	
	unsigned char aucLine17[64];	
	unsigned char aucLine18[64];		
	unsigned char aucLine19[64];
		
	unsigned char aucLine20[64];
	unsigned char aucLine21[64];
	unsigned char aucLine22[64];
	unsigned char aucLine23[64];
	unsigned char aucLine24[64];	
	unsigned char aucLine25[64];	
	unsigned char aucLine26[64];	
	unsigned char aucLine27[64];	
	unsigned char aucLine28[64];		
	unsigned char aucLine29[64];
		
	unsigned char aucLine30[64];
	unsigned char aucLine31[64];
	unsigned char aucLine32[64];
	unsigned char aucLine33[64];
	unsigned char aucLine34[64];	
	unsigned char aucLine35[64];	
	unsigned char aucLine36[64];	
	unsigned char aucLine37[64];	
	unsigned char aucLine38[64];		
	unsigned char aucLine39[64];
	unsigned char aucLine40[64];
	
}FACEPROTOUI;
typedef struct
{
	unsigned char ucOweFlag;	// 刷卡成功

	unsigned char ucInputMode;	//刷卡方式
	unsigned char aucCardAcc[512];

	unsigned int uiLenIso1;
	unsigned char aucIso1[76];		// 76

	unsigned int uiLenIso2;
	unsigned char aucIso2[38];		// 37	

	unsigned int uiLenIso3;
	unsigned char aucIso3[110];	// 104

	unsigned char aucMagInfo[20]; //用于显示
}OWECARDINFO;
typedef struct
{ 
	//交易接口: ui界面传递到 progress
	unsigned char ucTransType;	//交易类型
	unsigned char ucEPTrans;  // 使用电子钱包
	unsigned char ucPrintDetailFlag;// settle print detail

	unsigned char aucOldTrace[7];	// trace Nb

	unsigned char aucTransDate[5];	
	unsigned char aucRefNum[13];	
	unsigned char aucAuthNo[7];	

	
	OWECARDINFO CardInfo;
	
	unsigned long ulAmount;		//金额
	unsigned int uiPwLen; 			// 密码长度
	unsigned char aucPasswd[24]; 	// 密码
	unsigned int  uiIndex;// 查询交易索引
}FACEUITOPRO;

typedef struct
{
	unsigned char ucProBar;//是否有进度条
	unsigned int  uiProgress;//进度条进度

	unsigned int  uiTimes;//倒计时

	NEW_DRV_TYPE OweDrv;
	FACEPROTOUI ProToUi;
	FACEUITOPRO UiToPro;
}PROUIINTERFACE;

// 杉德Qt应用新加接口 end.
#if 0
typedef struct
{
	UCHAR		 	 ucTermCAPKNum;
	CAPK		 	 TermCAPK;
	UCHAR		 	 ucTermAIDNum;
	TERMSUPPORTAPP	 TermAID;
	UCHAR			 ucIPKRevokeNum;
	IPKREVOKE		 IPKRevoke;
	UCHAR			 ucExceptFileNum;
	EXCEPTPAN		 ExceptFile;
	EMV_CONSTPARAM ConstParam;
	TRANSREQINFO 	 TransReqInfo;

}EMVTRANSINFO;
#endif

/* Normal Transaction Information */
typedef struct
{
	unsigned char		ucCardTableIndex;
	enum CARDSPECIES     euCardSpecies;
	unsigned char   ucPANSeqExist;
	unsigned char   ucPANSeq;
	unsigned char	aucUntouchSeq[20];
	unsigned char   aucAID[EMVAIDLEN];
	unsigned char   ucAIDLen;
	unsigned char   AppCrypt[8];
	unsigned char 	TVR[5];
	unsigned char  TSI[2];
	unsigned char	ATC[2];
	unsigned char 	CVM[3];
	unsigned char   ucAppLabelLen;
	unsigned char   aucAppLabel[16];
	unsigned char   ucAppPreferNameLen;
	unsigned char 	aucAppPreferName[16];
	unsigned char  ucFallBackFlag;
	unsigned char	ucCardType;
	unsigned char	aucOldDate[TRANS_DATELEN];
	unsigned char	aucDate[TRANS_DATELEN];
	unsigned char	aucTime[TRANS_TIMELEN];
	unsigned char	ucSourceAccType;
	unsigned char	aucSourceAcc[TRANS_ACCLEN];
	unsigned char	ucSourceAccLen;
	unsigned char	ucOldAuthType;
	unsigned char	aucAuthGroup[TRANS_GROUPLEN];
	unsigned char	aucCardRemark[TRANS_REMARK];
	unsigned char	aucOldRefNumber[TRANS_REFNUMLEN];
	unsigned char	ucCardRemarkFlag;
	unsigned char	ucTIPAdjustFlag;
	unsigned char	ucTransType;
	unsigned char	ucOldTransType;
	unsigned char	ucOldOldTransType;
	unsigned char ucInputMode;
	unsigned char	ucExpiredDate[TRANS_DATELEN];
	unsigned char ucBatchSendExpiredDate[TRANS_DATELEN];
	unsigned char	aucSettleDate[TRANS_DATELEN];
	unsigned char	aucISO2[20];
	unsigned char	ucISO2Len;
	unsigned char	ucISO3Len;
	unsigned char ucDispSourceAcc[TRANS_ACCLEN*2];
	unsigned char	ucDispSourceAccLen;
	unsigned char	aucISO3[53];
	unsigned char   	aucCardPan[6];
	unsigned char aucCardPan_UnAssign[6];
	unsigned char   	ucNotSendOkFlag;
	ULONG_C51		ulOldAmount;
	ULONG_C51		ulAmount;
	ULONG_C51		ulAuthAddTotalAmount;
	ULONG_C51		ulPrevEDBalance;
	ULONG_C51		ulPrevEPBalance;
	ULONG_C51		ulFeeAmount;
	ULONG_C51		ulTicketNumber;
	ULONG_C51		ulTraceNumber;
	ULONG_C51		ulOldTraceNumber;
	ULONG_C51		ulOldOldTraceNumber; //最初流水号
	ULONG_C51		ulTrueAmount;	/*净金额*/
	ULONG_C51		ulOldTrueAmount;
	unsigned char   	aucBankID[TRANS_BANKIDLEN*2];
	unsigned char		aucAuthCode[TRANS_AUTHCODELEN+1];
	unsigned char 		aucBatchSendAuthCode[TRANS_AUTHCODELEN];
	unsigned char		aucOldAuthCode[TRANS_AUTHCODELEN];
	unsigned char 		aucAuthCodeHost[TRANS_AUTHCODELEN];
	unsigned char		aucAuthUintCode[TRANS_UintCodeLEN];
	unsigned char		aucCashierNo[TRANS_CASHIERNOLEN];
	unsigned char		aucRefNum[TRANS_REFNUMLEN];
	unsigned char		aucComm[TRANS_COMMLEN];
	unsigned char 	ucICEPKeyVer;
	unsigned char 	ucICEPMatchFlag;
	unsigned char 	ucMac1[4];
	unsigned int 		uiField55Len;
	unsigned char 	ucField55Data[256];
	unsigned short	uiOldTraceIndex;
	unsigned char 	ucICOnlineOrOfflineFlag;
	unsigned char	aucCashTicket[CASHIERTRACE_LEN];
	unsigned char	ucTIPCode;
	unsigned char  ucPrintFee;
	ULONG_C51	ulFeeNumber;
	unsigned char aucOrdersNo[TRANS_ORDERSNO];  //订单编号
	unsigned char aucOldOrdersNo[TRANS_ORDERSNO]; //撤销时用的原订单号
	unsigned char ucECTrans;
	ECTRANSINFO PECTransInfo;
	unsigned char aucUnAssignAccountISO2[TRANS_ISO2LEN];
	unsigned char aucUnAssignAccountISO3[TRANS_ISO2LEN];
	unsigned char ucUAAISOLen2;
	unsigned char ucUAAISOLen3;
	unsigned char ucMainAccountLen;
	unsigned char ucPTicket;
	unsigned char	aucTargetAcc[TRANS_ACCLEN];
	unsigned char	ucVoidFlag;
	unsigned char	aucMobileNUM[MobileNUMLen]; 	//手机号
	unsigned char	aucMobileCHK[MobileCHKLen];		//手机校验号
	unsigned char	aucNumStages[2];
	unsigned char	aucProjectNum[TRANS_NumProject];
	UCHAR bInstallments;									//是否一次性支付或分期支付
	ULONG_C51	ulRepayAmount;			//首期还款金额
	ULONG_C51	ulRepayFeeAmt;				//分期付款手续费
	unsigned char aucCBuf[3];						//还款币种
	ULONG_C51	ulStages;							//奖励积分
	unsigned char	ucStagesType;					//手续费支付方式
	ULONG_C51	ulFCurrency;					//首期手续费
	ULONG_C51	ulNCurrency;					//每期手续费
    unsigned char   aucCustName[20];//客户名字
	unsigned char   aucCustNo[2];//客户编号
}NORMALTRANS;	//保存交易结构，打印使用。文件[savetrans]

/* Extra Transaction Information */
typedef struct
{
	unsigned char	 ucISO2Status;
	unsigned short uiISO2Len;
	unsigned char	aucISO2[TRANS_ISO2LEN];
	unsigned char	ucISO3Status;
	unsigned short uiISO3Len;
	unsigned char	aucISO3[TRANS_ISO3LEN];
	unsigned char	ucInputPINLen;
	unsigned char	aucPINData[TRANS_PINDATALEN];
	unsigned char	ucInputPINLen_UnAssign;//非指定账户用
	unsigned char	aucPINData_UnAssign[TRANS_PINDATALEN];
	unsigned char	aucHostRespCode[2];
	unsigned char	aucSessionKey[10];
	unsigned char	ucSessionKeyLen;
	unsigned char  aucReadCardTime[10];
	ULONG_C51	ulRefundBatchNumber;

}EXTRATRANS;//临时使用，卡结构。

typedef struct
{
	unsigned char ucInquiry;
	unsigned char	ucAuth;
	unsigned char	ucUnAuth;
	unsigned char	ucAuthFin;
	unsigned char	ucUnAuthFin;
	unsigned char	ucSale;
	unsigned char	ucUnSale;
	unsigned char	ucRefund;
	unsigned char	ucOffline;
	unsigned char	ucTipFee;
	unsigned char ucAuthFinishOff;
	unsigned char ucICScript;
	unsigned char ucOffSale;
	unsigned char ucAddAuth;
	unsigned char ucPurseDeposit;

}ENABLETRANS;

typedef struct
{
	unsigned short	ucErrorExtCode;
	unsigned char	ucTerminalType;
	unsigned char	ucCardType;
	unsigned char	ucDialRetry;
	unsigned char	ucHostConnectFlag;
	unsigned short	uiTransIndex;
	unsigned short	uiOldTransIndex;
	ULONG_C51		ulOldTraceNumber;
	ULONG_C51		ulOldFeeAmount;
	unsigned char	ucPINRetry;
	unsigned char	aucTerminalID[PARAM_TERMINALIDLEN+4];
	unsigned char	aucMerchantID[PARAM_MERCHANTIDLEN];
	unsigned char	aucVoidRefNum[TRANS_REFNUMLEN+1];
    unsigned char   aucVoidAuthCode[TRANS_AUTHCODELEN+1];
    unsigned char   aucOrignalDate[TRANS_DATELEN*2+1];
    unsigned char   aucOriginalBatchNum[TRANS_BATCHNUMLEN+1];
    unsigned char	aucOriginalTraceNum[TRANS_TRACENUMLEN+1];
    unsigned char	aucOriginalTicketNum[TRANS_TICKETNUMLEN+1];
    unsigned char	ucTransType;		/*结算批上送使用两个交易类型*/
	PBOCBANKDATA	PBOCBankData;
//	SLE4442DATA		MEMBankData;
    unsigned char	aucNewSuperPass[CASH_MAXSUPERPASSWDLEN+1];
    unsigned char	aucOriginalTerminalId[PARAM_TERMINALIDLEN+1];
    unsigned char   ucReprintFlag;
   	unsigned char ReadCardFlag;
    ENABLETRANS		TransEnable;
    unsigned char	ucTIPSaving;
    unsigned char	ucAdjustAmoutFlag;	/*分为调整，小费。调整金额比原来大还是小*/
    unsigned char	ucFiledErrNo;		/*记录错误的域记录号*/
    unsigned char    ucProcessCode[6];
	unsigned char ucAmount[12];
	unsigned char ucTransNum[6];
	unsigned char ucTerminerID[8];
	unsigned char ucMarchID[15];
	unsigned char ucDefOperation;
	unsigned char ucEPReadCardFlag;
	unsigned char ucDialFlag;
	unsigned char ucSettleFlag;
	unsigned char ucReadCardFlag;
	unsigned char ucSettleLogonflag;
	unsigned char ucPreDialFalg;
	unsigned char aucOldCardPan[20];
	unsigned char   ucAIDLen;			  //2009-9-4 11:31  cbf
	unsigned char   aucAID[EMVAIDLEN];    //2009-9-4 11:31 cbf
	unsigned char ucQInputPinFlag;		//
	unsigned char aucOperaBuf[17];
	unsigned char aucOperaBufKey[9];
	unsigned short ucErrorFileCode;
	unsigned char aucMBFEBuf[10];
	unsigned char ucPTicketFlag;
	unsigned char ucOrderFlag;
	unsigned char aucMerchantName_cust[9][20+1];//客户名称
    unsigned char ucReversalFlag;
	UCHAR bConectFlag;
}RUNDATA;//临时使用.

typedef struct
{
	unsigned char	ucDispMsg[17];
	unsigned char	(*pFct)();
}SYSMENU;

typedef struct// add dpnew
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

/* Saved Data struct define */
typedef struct
{
	unsigned char	ucSetFlag;
	unsigned char	aucPassword[PARAM_PASSWORDLEN];
	unsigned char	ucTerminalType;
	unsigned char	aucTerminalID[PARAM_TERMINALIDLEN];
	unsigned char	ucMerchantType;
	unsigned char	aucMerchantID[PARAM_MERCHANTIDLEN];
	unsigned char	aucMerchantName[PARAM_MERCHANTNAMELEN];	/*商户名称*/
	unsigned char	ucCommMode;
	unsigned char	ucDialRetry;					/*重拨次数*/
	unsigned short	uiDialWaitTime;			/*等待拨通时间*/
	unsigned int	uiCommSpeed;
	unsigned int	uiReceiveTimeout;		/*接收超时时间*/
	unsigned char 	ucLoadFlag;				/*参数下载标志*/
	unsigned char 	aucTermianlApp[PARAM_TERMINALAPP];		/*终端应用类型*/
	unsigned char 	ucXorTipFee;			/*是否支持小费*/
	unsigned int usTipFeePre;				/*小费百分比*/
	unsigned char	ucInputCardNo;				/*是否支持手输卡号*/
	unsigned char	ucSelfLogoff;				/*自动签退标志*/
	unsigned char ucOFFLineSendType;			/*离线上送方式*/
	unsigned char	ucUseKeyIndex;				/*主密钥索引*/
	unsigned char ucTransRetry;				/*交易重发次数*/
	unsigned char ucPrnErrTicket;				/*打印故障报告单标志*/
	unsigned char	ucUnAuthFinSwipe;			/*预授权完成撤销是否刷卡*/
	unsigned char	ucUnSaleSwipe;				/*消费撤销是否刷卡*/
	unsigned char ucVoidInputPin;				/*撤销交易是否输密码*/
	unsigned char ucUnManagePwd;			/*消费撤销是否输主管密码*/
	unsigned char uICCInfo;					/*撤销交易是否输密码*/
	unsigned char ucUseLogonDisp;				/*签到状态提示功能*/
	unsigned char ucRePAuthority;				/*重打印功能权限*/
	//unsigned char ucSettleAuth;					/*结算功能权限*/
	unsigned char ucPREAUTHFINISHInputPin; 	/*预授权完成联机*/
	unsigned long   ulRefundMaxAmount;		/*最大退货金额*/
	unsigned char	aucPOSDialConnect[25];		/*POS拨通率*/
	unsigned char	aucTpdu[2];
	unsigned char	aucSwitchBoard[PARAM_SWITCHLEN];
	unsigned char	aucHostTelNumber1[PARAM_TELNUMBERLEN];/*终端交易电话*/
	unsigned char	aucHostTelNumber2[PARAM_TELNUMBERLEN];
	unsigned char	aucHostTelNumber3[PARAM_TELNUMBERLEN];
	unsigned char	aucHostTelNumber4[PARAM_TELNUMBERLEN];/*终端管理电话*/
	unsigned char	aucGlobalTransEnable[PARAM_GLOBALTRANSENABLELEN];//support transcation
	unsigned char	ucDispMaxLine;
	unsigned char	ucLogonModiBatch;		/*需签到修改批次*/
	unsigned char	aucSettleFlag;			/*是否产生结算*/
	unsigned char  ucForgienSettleFlag;	/*外卡是否结算成功标志*/
	unsigned char	ucInternalSettleFlag;		/*内卡是否结算成功标志*/
	unsigned char ucFunctStep;			/*结算中断位置*/
	unsigned char	aucConfig[PARAM_CONFIGLEN];
	unsigned char prnflag;
	unsigned char ucPRNPAPER;
	unsigned char Pinpad_flag;
	unsigned char Pinpadflag;
	unsigned char BackLightFlag;
	unsigned char	prnflagNew;
	unsigned char	prnflagOld;
	unsigned char	prnflagZD;
	unsigned char   linenum;
	unsigned char   font;
	unsigned char ucHeatPrintFormat;
	unsigned char ucPrintPageNum;
	unsigned char   printtimes;
	MODEM_PARAM     ModemParam;
	/* For PBOC Smart Card Process */
	unsigned char	ucPBOCAppSelectFlag;
	unsigned char	ucPBOCPSAMReader;
	unsigned char   ucFirstPKDownLoad;                 //第一次公钥下载
	unsigned char   ucFirstAIDParmaDownLoad;    // 第一次参数下载
	unsigned char   ucDefaultTransParam;             //缺省交易类型
	enum ENCRYPTTYPE     ENCRYPTTYPEParam;   //　终端加密方式参数
	unsigned char   ucMagPreauthFinish;               //磁条卡预授权完成实现参数
	unsigned char   aucRecvDate[5];	/*签到时间同机器时间差*/
	unsigned char   aucTermDate[5];
	unsigned char	APP_Project[30];
	unsigned char	SOFTWARE_Version[17];
	unsigned char	HARDWARE_Version[17];

	unsigned long	ulHostIP;
	unsigned long	ulBackIP;
	unsigned long   ulHostIPAddress1;
	unsigned long	ulHostIPAddress2;
	unsigned long	ulHostIPAddress3;
	unsigned long	ulHostIPGPRS1;
	unsigned long	ulHostIPGPRS2;
	unsigned long	ulHostIPGPRS3;

	unsigned long	ulHostIPBack1;
	unsigned long	ulHostIPBack2;
	unsigned long	ulHostIPBack3;
	unsigned long	ulHostIPMIS1;
	unsigned long	ulHostIPMIS2;
	unsigned long	ulHostIPMIS3;

	unsigned char	ucEncryptDataFlag;
	unsigned char	ucTerIPType;
	unsigned int  	uiHostPort;
	unsigned int  	uiHostBackPort;
	unsigned char	aucAPN[30];
	unsigned char ucPrint;
	unsigned char BankID;
	unsigned char   Top[Display_MAXNB];
	unsigned char ucDisAutoSettleFlag;
	unsigned char ucDisAutoTiming;
	unsigned char ucPrintCardNo;  //是否支持隐藏卡号标志
	unsigned char ucPBeforeSix;		//隐藏方式:前段、中间段
	unsigned char ucCardtype;		//卡种隐藏标志  0:内卡、1：外卡、2：内外卡
	unsigned char ucForeignCard;	//外卡标志
	unsigned char ucDisAuth;        //是否支持预授权隐藏标志 0:不支持、1：支持

	/*扣率的一些变量 表示万分比*/
	ULONG_C51		ulRMBRate;
	ULONG_C51		ulVISARate;
	ULONG_C51		ulMASTERRate;
	ULONG_C51		ulJCBRate;
	ULONG_C51		ulDINERRate;
	ULONG_C51		ulAMEXRate;
	unsigned char   ucUndoOff;
	unsigned char ucCashFlag;
	unsigned char ucTMSFlag;
	unsigned char aucPrintType[3];
	unsigned char	aucReFuncation[2];
	unsigned char ucENCFlag;
	unsigned char ucKEYMODE;
	unsigned char ucDisplay;
	unsigned char ucList;
	unsigned char ucCNOREN;
	unsigned char ucQuickUndo;//F3快速消费撤销
	unsigned char ucSelIP;
	unsigned char ucSwiptEvent;
	unsigned char ucORDERFLAG;
	unsigned char ucKEYCOMM;
	unsigned char	aucHostTel1[PARAM_TELNUMBERLEN];/*终端交易电话*/
	unsigned char	aucHostTel2[PARAM_TELNUMBERLEN];/*终端交易电话*/
	unsigned long	ulHostKEYIP;
	unsigned int  	uiHostKEYPort;
	unsigned char	aucKEYAPN[30];
	unsigned char	aucKEYTpdu[5];
	unsigned char aucKEKRow[3];
	unsigned char aucKEKLine[3];
	unsigned char ucMBFEFlag;
	unsigned char ucSendOrder;
	unsigned char ucLogistics;
	unsigned char ucLoadEMVParam;
	unsigned char ucBatchErrTicket;
	unsigned char ucInforFlag;
	unsigned char ucEmptySettle;

	unsigned char ucCollectFlag;					//数据采集标志
	unsigned char ucSendCollectFlag;				//采集成功标志
	unsigned long ulSendIPAddress;	   			//数据采集IP
	unsigned int  uiSendPort;				       //数据采集端口
	unsigned char aucSendAPN[20];
	unsigned char ucMobileFlag;				//手机预约
	unsigned char ucInstallment;    
    unsigned char aucMerchantName_cust[9][20+1];//客户名称
    unsigned char uclujiaomc;
}CONSTANTPARAM;

typedef struct
{
	unsigned char	ucRunStep;
	unsigned char	ucSuperLogonFlag;		/*借用作为测试交易状态*/
	unsigned char ucPackgeHeadEve;			/*报头处理要求*/
	unsigned char	ucCashierLogonFlag;
	unsigned char	ucCashierLogonIndex;
	unsigned char	aucLogonDate[PARAM_DATELEN];
	ULONG_C51 ulBatchNumber;
	ULONG_C51 ulTraceNumber;
	ULONG_C51 ulTicketNumber;
	UINT_C51 uiTotalNumber;			/*累计上送总笔数*/
	UINT_C51 uiMaxTotalNb;			/*保存的交易总笔数*/
	unsigned char	ucSettleTime[TRANS_TIMELEN];
	unsigned char	ucSettleData[TRANS_DATELEN];
	unsigned short uiLastTransIndex;
	unsigned char	ucTicketPrintFlag;
	unsigned char ucTerminalLockFlag;
	unsigned char	ucStoreKeyRight;
	unsigned char ucDownLoadFlag;		/*下载参数标志*/
	unsigned char ucDialStartStamp;		/*拨号累计时间标记*/
	unsigned long	ulDialTimeTotal;		/*拨号累计时间*/
	unsigned long	ulDialTotalCount;		/*拨号总次数*/
	unsigned long	ulDialConnectCnt;		/*拨通次数*/
	unsigned long	ulDialConnectRate;		/*拨通率*/
	unsigned char aucIssuerID[11];
	unsigned char aucAcquereID[11];
	unsigned char ucEMVICTransFailNum;          /*上一笔IC卡交易失败*/
	unsigned short uiEMVICLogNum;                 /*EMV　IClog num*/
	unsigned char ucScriptInformValid;
	enum LOADPARAMTYPE euLoadParamType; /*下载参数类型*/

	unsigned char aucSettleCycle[5];                  /*日结算时间*/
	unsigned char aucSettleDate[17];
	unsigned char ucResult;
	unsigned short	ucErrorExtCode;
	unsigned char ucKeyRight;

	unsigned long   ulUnTouchTransLimit;	 /*非接卡交易最大金额*/
	UCHAR bReaderSupport;                        /*是否支持外接读卡器*/
	UCHAR bECSupport;             	               /*是否支持电子现金*/
	unsigned char ucReaderType;              /*读卡器类型*/
	unsigned char aucMBFEBuf[TRANS_ORDERSNO];

	UINT_C51		uiSendTotalNb;
	unsigned char	uiSendIndex[TRANS_MAXSENDNB];
	unsigned char	aucSendDate[8];

}CHANGEPARAM;

/* Count information */
typedef struct
{
	UINT_C51		uiTotalNb;      			/*总交易笔数*/
	UINT_C51		uiCreditNb;					/*总贷记笔数*/
	ULONG_C51		ulCreditAmount;				/*总贷记金额*/
	UINT_C51		uiDebitNb;					/*总借记笔数*/
	ULONG_C51		ulDebitAmount;				/*总借记金额*/
	UINT_C51		uiAuthNb;					/*总授权笔数*/
	ULONG_C51		ulAuthAmount;				/*总授权金额*/
	UINT_C51		uiPurchaseNb;				/*总消费笔数*/
	ULONG_C51		ulPurchaseAmount;			/*总消费金额*/
	UINT_C51		uiAuthFinishNb;				/*总授权完成笔数*/
	ULONG_C51		ulAuthFinishAmount;			/*总授权完成金额*/
	UINT_C51		uiRefundNb;					/*总退货笔数*/
	ULONG_C51		ulRefundAmount;				/*总退货金额*/
	UINT_C51		uiOfflineNb;				/*离线总笔数*/
	ULONG_C51		ulOfflineAmount;			/*离线总金额*/
	UINT_C51		uiPreauthSettleNb;				/*预授权结算总笔数*/
	ULONG_C51		ulPreauthSettleAmount;			/*预授权结算总金额*/
	UINT_C51		uiUndoOffSum;				/*离线撤消总笔数*/
	ULONG_C51		ulUndoOffAmount;			/*离线撤消总金额*/
	UINT_C51		uiCashLoadNb;      			/*现金圈存总笔数*/
	ULONG_C51		uiCashLoadAmount;					/*现金圈存总金额*/
	unsigned char  	ucSettleFlag;	           /*是否结算成功标志*/
}TRANSTOTAL;

typedef struct
{
	unsigned int		uiTotalNb;			/*每种外卡的总笔数*/
	unsigned long		ulAmount;			/* 每种外卡的总金额*/
	unsigned long		ulTipAmount;		/*每种外卡小费总金额*/
	unsigned long		ulTrueAmount;		/*每种外卡净金额*/
}TRANSDETAIL;

typedef struct
{
	unsigned short	auiTransIndex[TRANS_SAVEMAXNB];
	TRANSTOTAL		TransTotal;
	TRANSTOTAL		LastTransTotal;
	TRANSTOTAL		ForeignTransTotal;
	TRANSTOTAL		LastForeignTransTotal;
	TRANSDETAIL		TransDetail[6];
	TRANSDETAIL		LastTransDetail[6];
	ULONG_C51		ulLastBatchNumber;

	unsigned char	aucLastCashierNo[TRANS_CASHIERNOLEN];

	unsigned char	ucLasSettleTime[TRANS_TIMELEN];
	unsigned char	ucLasSettleData[TRANS_DATELEN];
	unsigned char ucLastSettleFlag;

}TRANSINFO;



typedef struct
{
	unsigned char aucSuperNo[CASH_MAXSUPERNO];
	unsigned char	aucSuperPassword[CASH_MAXSUPERPASSWDLEN];
	unsigned char ucSYSCashierExitFlag;
	unsigned char	aucSYSCashierNo[CASH_SYSCASHIERNOLEN];
	unsigned char	aucSYSCashierPass[CASH_SYSCASHIERPASSLEN];

	unsigned char	aucCashierNo[CASH_MAXCASHIER][CASH_CASHIERNOLEN];
	unsigned char	aucCashierPass[CASH_MAXCASHIER][CASH_CASHIERPASSLEN];
	unsigned char	aucSuperPassWord[7];

}SYSCASHIER;


typedef struct
{
	unsigned char	aucMSGType;
	unsigned char	aucMSGTab[MSG_MAXCHAR];
}MSG;


typedef struct
{
	unsigned char	ucType;
	unsigned char	ucErrCode;
	unsigned char	aucErrMsg[MSG_MAXCHAR];
}SINGLEERRMSG;

typedef struct
{
	unsigned char	ucType;
	unsigned char	ucErrCode;
	unsigned char	aucErrMsg[MSG_MAXCHAR];
	unsigned char	aucErrMsg_ENG[MSG_MAXCHAR];
}FILEERRMSG;

typedef struct
{
	unsigned char	ucType;
	unsigned char	ucErrCode;
	unsigned char	aucErrMsg[MSG_MAXCHAR];
	unsigned char	aucErrMsgSub[MSG_MAXCHAR];
}HOSTEERRMSG;

typedef struct
{
	unsigned char	ucExistFlag;
	unsigned short uiOffset;
	unsigned short uiLen;
}ISO8583Bit;

typedef struct
{
	ISO8583Bit Bit[ISO8583_MAXBITNUM];
	unsigned short uiBitBufLen;
	unsigned char	aucMsgID[ISO8583_MSGIDLEN];
	unsigned char	aucBitBuf[ISO8583_MAXBITBUFLEN];
	unsigned short uiCommBufLen;
	unsigned char	aucCommBuf[ISO8583_MAXCOMMBUFLEN];
}ISO8583;

typedef struct
{
	ISO8583Bit Bit[ISO8583_MAXBITNUM];
	unsigned short uiBitBufLen;

	unsigned char	aucBitBuf[ISO8583_MAXBITBUFLEN];
	unsigned short uiCommBufLen;
	unsigned char	aucCommBuf[ISO8583_MAXCOMMBUFLEN];
}ISO8583MBFE;

typedef struct
{
	ISO8583Bit		Bit[ISO8583_MAXBITNUM];
	unsigned short	uiBitBufLen;
	unsigned char		aucMsgID[ISO8583_MSGIDLEN];
	unsigned char		aucBitBuf[ISO8583_MAXBITBUFLEN];

}BACKUPISO8583;



typedef struct
{
	unsigned char	ucValid;
	unsigned char   ucRevealEvent[2];
	unsigned char	ucAuthFlag;
	unsigned char	aucAuthData[6];
	BACKUPISO8583	BackupISO8583Data;
	unsigned short  uiICTransInfoLen;
	unsigned char   aucICTransInfo[ISO8583_ICTRANSINFOLEN];

}REVERSALISO8583;

typedef struct
{
	unsigned char	ucAttr;
	unsigned short uiMaxLen;
}ISO8583BitAttr;

typedef struct
{
	unsigned char	ucValid;
	unsigned char	aucMirror[MIRROR_MAXLEN];
}MIRROR;

/* Notice:
	Don't change the order of Data&chk, some function access fixed address!!
*/
typedef struct
{
	NORMALTRANS	ReversalTrans;
	BACKUPISO8583	SaveISO8583Data;
	BACKUPISO8583	SendISO8583Data;
	REVERSALISO8583	ReversalISO8583Data;
	BACKUPISO8583   ScriptInform8583Data;
}TRANS_8583_DATA;

typedef struct
{
    unsigned char aucTempFile[8500];
    NORMALTRANS ucSTRBackupData;
}TEMPFILE_DATA;

typedef struct
{

	ISO8583BitAttr	ISO8583BitAttrTab[ISO8583_MAXBITNUM];
	TERMINALAPP	TerminalAppData;
	CONSTANTPARAM	ConstantParamData;
	CHANGEPARAM	ChangeParamData;
	SYSCASHIER		Cashier_SysCashier_Data;
	TRANSINFO		TransInfoData;
	TRANS_8583_DATA Trans_8583Data;

	NORMALTRANS		SaveTrans1;

}DATASAVEPAGE0;

typedef struct
{
	NORMALTRANS	SaveTrans;
}DATASAVEPAGE1;

#if 0
typedef struct
{

	TERMCONFIG	 		EMVConfig;
	UCHAR		 		ucTermCAPKNum;
	CAPK		 		TermCAPK[MAXTERMCAPKNUMS];
	UCHAR		 		ucTermAIDNum;
	TERMSUPPORTAPP		TermAID[MAXTERMAIDNUMS];
	UCHAR				ucIPKRevokeNum;
	IPKREVOKE			IPKRevoke[MAXIPKREVOKENUMS];
	UCHAR				ucExceptFileNum;
	EXCEPTPAN			ExceptFile[MAXEXCEPTFILENUMS];
	EMV_CONSTPARAM	ConstParam;
	TRANSREQINFO 		TransReqInfo;

}EMVDATASAVE;
#endif
/////////////////////////////////////////////////////////////////
typedef		struct
{
	UCHAR		aucTerminalID[TERMINALIDLEN];
	UCHAR		aucMerchantID[MERCHANTIDLEN];
	UCHAR		aucMerchantName[MERCHANTNAMELEN];
	UCHAR		ucLanguage;
	UCHAR		bForceOnline;
	UCHAR		bBatchCapture;
	UCHAR		bPBOC20;
	UCHAR		bSelectAccount;
	UCHAR		ucAccountType;
	UCHAR		bShowRandNum;
	UCHAR		bDDOL;
	UCHAR		bDTOL;
	UCHAR		ucTransType;
	UINT		uiTotalAmount;
	USHORT		uiTotalNums;
	UINT		uiTraceNumber;
	UCHAR		ucCommID;
	UCHAR		bPrintTicket;
	UINT		uiBatchNumber;
	UCHAR		ucFlag;
	uchar             ucReaderPort;
	UCHAR 		bDigitalDebug;
	UCHAR   		bAutoRemove;
}EMV_CONSTPARAM;
typedef struct
{
	unsigned char aucAppVer[2];
	unsigned char aucTACDefault[5];
	unsigned char aucTACOnline[5];
	unsigned char aucTACDenial[5];
	unsigned long ulFloorLimit;
	unsigned long ulThreshold;
	unsigned char ucMaxTargetPercent;
	unsigned char ucTargetPercent;
	unsigned char ucDDOLLen;
	unsigned char aucDDOL[MAXDEFAULTDOLLEN];
	unsigned char ucOnlinePINCapab;
	unsigned long ulTelECFloorLimit;  /*终端电子现金(EC)交易限额 */
	unsigned long ulUnTouchFloorLimit;/*非接终端脱机最低限额  */
	unsigned long ulUnTouchTransLimit;/*非接终端交易限额 */
	unsigned long ulTelCVMLimit;      /*非接终端执行CVM限额 	*/
}EMVAIDPARAM;
#if 1
#define		CONSTPARAMFILE			"ConstPara"
#define		EMVCONFIGFILE				"EMVConfig"
#define		CAPKFILE							"CAPK"
#define		TERMSUPPORTAPPFILE	"TermApps"
#define		TRANSLOG						"TransLog"
#define		EXCEPTFILE						"Except"
#define		BATCHRECORD				"BatchRec"
#define		IPKREVOKEFILE					"IPKRevoke"
#define		EMVAIDPARAMFILE			"AIDParam"
#define		ERRICCTRANS					"ErrICC"
//#define          READERPARAMFILE				"Reader"
#endif
typedef		struct
{
	UCHAR   		ucTransType;
	UCHAR   		aucAuthRespCode[2];
	UCHAR		aucAuthCode[AUTHCODELEN];
	unsigned long ulTraceNum;
	UCHAR		ucPANLen;
	UCHAR		aucPAN[MAXPANDATALEN];
	UCHAR		aucExpiredDate[4];
	unsigned long 	ulAmount;
	UINT      		uiOtherAmount;
	UCHAR   		ucPANSeq;
	UCHAR   		aucCurrencyCode[3];
	USHORT  	uiLogBuffLen;
	UCHAR   		aucLogBuff[256];
	USHORT  	uiSendFlag;
	UCHAR		aucTransDate[TRANSDATELEN];
	UCHAR		aucTransTime[TRANSTIMELEN];
	UCHAR		aucAppCrypt[8];
	UCHAR		ucTransFlag;	//0x00 --- Online ; 0x01 --- Offline ; 0x02 --- ERR
}TRANSRECORD;


typedef enum {LEFT_ALIGN,MIDDLE_ALIGN,RIGHT_ALIGN}ALIGN;
typedef struct
{
	UCHAR		ucOptionValue;
	UCHAR		szOptionMsg[30];

}OPTION,*POPTION;

/////////////////////////////////////////////////////////////////
/* Function Description */
void OnEve_power_on(void);
unsigned char EMV_Check_file(void);


/* trans.c */
unsigned char Trans_TestSingal(void);
unsigned char Trans_EnqBalance(void);
unsigned char Trans_Process(unsigned char Flag);
unsigned char Trans_VoidCUP(void);
unsigned char TRANS_Online(void);
unsigned char Trans_OFFLineTrans(void);
unsigned char TRANS_OnlineProcess(void);
unsigned char  Trans_IssuerScriptInformSend(void);
unsigned char  Trans_SendReversalTrans(void);
unsigned char Trans_MagOffLineSettle(unsigned char ucReTryFlag);
unsigned char EMV_PackgeHeadEveProcess(unsigned char ucPackgeHeadEveID);
unsigned char TRANS_SetField55(TAGPHASE enTagPhase,unsigned char ucLogType , unsigned char * SendBuf ,unsigned short * SendBufLen);
unsigned char Trans_SendICOfflineTrans(void);
unsigned char Serv_OnlineTransSend(unsigned char ucSettleFlag,unsigned int *pucTransNum);
unsigned char Trans_SpecInfoSend(unsigned char ucSettleFlag,unsigned int *pucSpecTransNum);
unsigned char SERV_ICInformTransSend(unsigned char ucSettleFlag,unsigned int *pucTransNum);
unsigned char Trans_EMVProcessScript(void);
unsigned char Trans_SendStats(void);
unsigned char SERV_SettleBatchEnd2(unsigned int ucSentTransNum);



/* mag.c */
unsigned char Mag_Menu(unsigned char ucKey);
unsigned char MAG_InputCashierPass(void);
unsigned char Mag_AuthMenu(void);
unsigned char MAG_GetTrackInfo(unsigned char *pucTrack);
unsigned char MAG_Process(void);
void MAG_Read_LoadTransType(void);
unsigned char Trans_Query(void);
unsigned char MAG_SwipeOrHit(unsigned char *aucBuf);
unsigned char MAG_DisplaySwipeCardMode(uchar ucTransType,uchar ucMOde);
unsigned char MAG_UerICCardFirstProcess(unsigned char ucCardSelect,unsigned char ucFullBackFlag,unsigned char ucLastResult);
unsigned char MAG_ManuSelectCard(unsigned char ucTransType);
unsigned char  MAG_DispCardNo(void);
unsigned char Menu_KEYNINE(void);

/* emv.c */
UCHAR EMV_AccessTag(unsigned short uiTag,unsigned char *pucInBuf,unsigned char ucInLen,
		unsigned char *pucOutBuf,unsigned char *pucOutLen);

/* pboc.c */
unsigned char PBOC_Menu(union seve_in *EveIn);
unsigned char PBOC_EDProcess(void);
unsigned char PBOC_EPProcess(void);
unsigned char PBOC_Process(void);
unsigned char PBOC_AppSelect(void);
unsigned char PBOC_UserAppSelect(void);
unsigned char PBOC_CheckTerminalApp(void);
unsigned char PBOC_VerifyPIN(void);
unsigned char PBOC_PSAMActive(void);
unsigned char PBOC_DebitStep1(void);
unsigned char PBOC_DebitStep2(void);
unsigned char PBOC_ISOSelectFilebyAID(unsigned char ucReader,
			unsigned char *pucAID,unsigned char ucAIDLen);
unsigned char PBOC_ISOSelectFilebyFID(unsigned char ucReader,
			unsigned char *pucFID);
unsigned char PBOC_ISOReadBinarybySFI(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen);
unsigned char PBOC_ISOReadRecordbySFI(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucRecordNumber);
unsigned char PBOC_ISOGetBalance(unsigned char ucReader,
			unsigned char ucType);
unsigned char PBOC_ISOInitForPurchase(unsigned char ucReader,
			unsigned char ucType,unsigned char *pucInData);
unsigned char PBOC_ISODebit(unsigned char ucReader,
			unsigned char *pucDebitData,unsigned char ucLen);
unsigned char PBOC_ISOVerifyPIN(unsigned char ucReader,
			unsigned char ucInLen,unsigned char *pucPinData);
unsigned char PBOC_ISOGetResponse(unsigned char ucReader,
			unsigned char ucLen);
unsigned char PBOC_ISOGetChallenge(unsigned char ucReader);
unsigned char PBOC_ISOSAMComputeMAC1(unsigned char ucReader,
			unsigned char *pucInData);
unsigned char PBOC_ISOSAMVerifyMAC2(unsigned char ucReader,
			unsigned char *pucInData);
unsigned char PBOC_ISOCheckReturn(void);
void PBOC_SetIso7816Out(void);
unsigned short PBOC_ComputeTerminalDataChk(void);
unsigned char PBOC_CheckTerminalData(void);
void PBOC_WriteTerminalAppDataChk(unsigned char ucClrMirrorFlag);
unsigned char PBOC_PSAM_InitForDescrypt(unsigned char ucReader,unsigned char *pucInData, unsigned char ucInLen);
unsigned char PBOC_PSAM_DESCrypt(unsigned char ucReader,unsigned char *pucInData, unsigned char ucInLen);
unsigned char PBOC_PSAM_ComputeMAC1(void);
unsigned char PBOC_ISOInitForLoad(unsigned char ucReader,
			unsigned char ucType,unsigned char *pucInData);
unsigned char PBOC_TRANSOnline(void);
unsigned char PBOC_TransOnline(void);


/* sel.c */
unsigned char SEL_GetCardMsg(unsigned char ucKey);
unsigned char SEL_LogonMenu(void);
unsigned char SEL_ServiceMenu(void);
unsigned char SEL_ManagementMenu(void);
unsigned char SEL_DisplayEMVInfo(void);
unsigned char SEL_DOWNTTMSPARA(void);
unsigned char SEL_CashierMenu(void);
unsigned char SEL_SLE4442_Menu(union seve_in *EveIn);
unsigned char SEL_UnLockKeyBoard(void);
unsigned char SEL_IncDialTotalCount( void );
unsigned char SEL_IncDialConnectCnt( void );
unsigned char SEL_ClrAccumulatDial( void );
unsigned char SEL_ParamUpdata(void);
unsigned char SEL_SetMasterKey(void);
unsigned char SEL_HostCopyIP(const unsigned char ucIpTY);
unsigned char SEL_HostPorts(void);
unsigned char SEL_HostBackPorts(void);
unsigned char SEL_HostIP(const unsigned char ucTerIp);
unsigned char SEL_SelectIP(const unsigned char ucIpFlag,const unsigned char ucHostIp,unsigned char aucIpBuf[30]);
unsigned char SEL_SelectBackIP(const unsigned char ucIpFlag,const unsigned char ucIp,unsigned char aucIpBuf[30]);
/* cash.c */
unsigned char CASH_AddCashier(void);
unsigned char CASH_DelCashier(void);
unsigned char CASH_ModiCashier(void);
unsigned char CASH_PinReload(void);
unsigned char CASH_CheckCashierNo(unsigned char *pucIndex,unsigned char *pucCashierNo);
void CASH_UpdateCashierData(unsigned char ucNo,
				unsigned char *pucCashierNo,unsigned char *pucCashierPass);
unsigned char CASH_CheckCashierData(void);
void CASH_WriteCashierDataChk(unsigned char ucClrMirrorFlag);
unsigned short CASH_ComputeCashierDataChk(void);
unsigned char CASH_CashierQuery(void);
unsigned char CASH_InputAndChackManagerPass(unsigned char ucFlag);
/* sysinit*/
unsigned char Init_Process(void);
/* serv.c */
unsigned char SERV_Settle(unsigned char ucFlag);
unsigned char SERV_SettleProcess(void);
unsigned char SERV_SettleOnline(void);
unsigned char SERV_SettleBatchSend(unsigned int ucSentTransNum);
unsigned char SERV_SettleOfflineSend(unsigned short  uiSendFlag,unsigned int *pucOfflineNum);
unsigned char SERV_Reprint(void);
unsigned char SERV_ListPrint(unsigned char flag);
unsigned char SERV_TransList(unsigned char flag);

/*e_serv.c*/
unsigned char SettleProcess(void);
unsigned char SettleOnline(void);
unsigned char SettleOfflineBatchSend(void);
unsigned char SERV_SettleBatchEnd(unsigned char ucSettleFlag,unsigned int ucTransNum);
unsigned char SERV_ReprintByTrace(void);
unsigned char SERV_ReprintSettle(void);
unsigned char SERV_DispTotal(void);
unsigned char SERV_BatchSendICOfflineTrans(void);
unsigned char SERV_MAGTransBatchSend(void );

/* logon.c */
unsigned char LOGON_CashierProcess(unsigned char ucFlag);
unsigned char LOGON_Online(void);
void LOGON_WriteSuperLogonFlag(unsigned char ucFlag);
void LOGON_WriteCashierLogonFlag(unsigned char ucFlag);
void LOGON_WriteCashierLogonIndex(unsigned char ucIndex);
unsigned char LOGON_Dispatch_DateTime(unsigned char *pucDate);
//unsigned char LOGON_DispJustLogonFlag(void);


/* sav.c */
unsigned char SAV_TransSave(void);
unsigned char SAV_SavedTransIndex(unsigned char ucFlag);
unsigned short SAV_ComputeTransInfoDataChk(void);
unsigned char SAV_CheckTransInfoData(void);
void SAV_WriteTransInfoDataChk(unsigned char ucClrMirrorFlag);
unsigned char SAV_CheckTransMaxNb(void);
/* comms.c */
unsigned char COMMS_PreComm(void);
unsigned char COMMS_CheckPreComm(void);
unsigned char COMMS_TransOnline(void);
unsigned char COMMS_TransSendReceive(void);
unsigned char COMMS_GenSendReceive(void);
unsigned char COMMS_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen);
unsigned char COMMS_KEYDOWNReceiveByte(unsigned char *pucByte,unsigned short *puiTimeout);
unsigned char COMMS_ReceiveByte(unsigned char *pucByte,unsigned short *puiTimeout);
unsigned char COMMS_FinComm(void);
unsigned char KEY_COMMS_FinComm(void);
unsigned char COMMS_PreCommforInit(void);
unsigned char COMMS_MODEM_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen);
unsigned char COMMS_TCPIP_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
                    unsigned char *pucOutData,unsigned short *puiOutLen);
unsigned char COMMS_TCPIP_SendReceive_Encrypt(unsigned char *pucInData,unsigned short uiInLen,
                    unsigned char *pucOutData,unsigned short *puiOutLen);
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
unsigned short ISO8583_ComputeBitAttrTabChecksum(void);
//void ISO8583_CheckBitAttrTab(void);
unsigned char ISO8583_CheckResponseValid(void);
unsigned char ISO8583_CompareSentBit(short iBitNo);
void ISO8583_SaveISO8583Data(unsigned char *pucSrc,unsigned char *pucDest);
void ISO8583_RestoreISO8583Data(unsigned char *pucSrc,unsigned char *pucDest);
unsigned char ISO8583_SaveReversalISO8583Data(void);
unsigned short ISO8583_ComputeReversalISO8583DataChk(void);

/* util.c */
unsigned char UTIL_DisplayMenu(SELMENU *pMenu);
unsigned char UTIL_InputTelNumber(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask);
unsigned char UTIL_Input(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask);
unsigned char UTIL_InputAmount(unsigned char ucLine,
				unsigned long *pulAmount,
				unsigned long ulMinAmount, unsigned long ucMaxAmount);
unsigned char UTIL_Input_EMV_pp(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf);
unsigned char UTIL_InputEncryptPIN(unsigned char * pucEnPIN,unsigned char * pucPinLen);
unsigned char UTIL_InputClearPIN(unsigned char ucPinpadFlag,unsigned char ucMax,
						unsigned char *pucOutData);
unsigned char UTIL_CalcGoldenCardSHMAC_Single(
				unsigned char *pucInData,
				unsigned short uiInLen,
				unsigned char *pucOutMAC);
unsigned char UTIL_EncryptCashierPass(
				unsigned char *pucInData,
				unsigned char *pucOutData);
unsigned char UTIL_Single_StoreNewKey(void);
unsigned char UTIL_Double_StoreNewKey(void);
void UTIL_IncreaseTraceNumber(void);
void UTIL_GetTerminalInfo(void);
void UTIL_PrintHexMsg(unsigned char *pt,unsigned short Len);
void UTIL_ClearGlobalData(void);
unsigned char UTIL_SaveDataToMirror(
				unsigned char ucPage,
				unsigned char ucClearFlag,
				unsigned char ucValidMirror,
				unsigned char *pucChkAddr,
				unsigned char *pucDest,
				unsigned short uiLen);
void UTIL_CheckSavedXDataMirror(unsigned char ucPage);
unsigned char UTIL_ClearCollectData(void);
unsigned short UTIL_ComputeMirrorChk(MIRROR *pMirror);
unsigned char UTIL_format_time_bcd_str( unsigned char *Ptd, unsigned char *Pts );
unsigned char UTIL_format_date_bcd_str( unsigned char *Ptd, unsigned char *Pts );
void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule);
unsigned char UTIL_Input_Super_Pass(void);
unsigned char UITL_Input_Merchant_Name(void);
unsigned char UITL_Delete_Reversal_Flag(void);
unsigned char UITL_Delete_Script_Flag(void);
unsigned char UTIL_Is_Trans_Empty(void);
unsigned char UITL_LocateTranceByTraceNumber(unsigned short *index,unsigned long traceNumber);
void UTIL_Beep(void);
unsigned char UTIL_displayCardNum(void);
unsigned char UTIL_Input_EMV_PIN(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask);
unsigned char UTIL_Input_pp(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf);
void UTIL_delay(unsigned int timeOut);
void UTIL_display_amount(unsigned char line,unsigned long amount);
unsigned char UTIL_SetPrinter(void);
unsigned char UTIL_GetKey(unsigned char wait_sec);
unsigned char OSUTIL_Input(unsigned int uiTimeout,unsigned char ucLine,
                unsigned char ucMin,unsigned char ucMax,
                unsigned char ucType,unsigned char *pucBuf,
                char *pcMask);
void UTIL_READ_DateTimeAndFormat(unsigned char * aucOutBuf/*YYMMDDhhmm*/);
unsigned char MSG_DisplyTransType(unsigned char ucInTransType ,unsigned char Line ,unsigned char Col ,unsigned char ucClrFlag ,unsigned char ucClrLine);
unsigned char UTIL_ClearMemery(void);
void UTIL_TestDispStr(unsigned char * pucInBuf ,unsigned int uiLen ,unsigned char * DispInfo, unsigned char flag);
void UTIL_DispTransResult(TRANSRESULT	enTransResult);
void UTIL_SavEMVTransDate(void);
unsigned char UTIL_CheckTransAmount(void);
void UTIL_SavEMVTransData(void);
unsigned char UTIL_DisplyTransType(unsigned char ucInTransType ,unsigned char Line ,unsigned char Col ,unsigned char ucClrFlag ,unsigned char ucClrLine);
unsigned char UTIL_SavPrinterParamToPOS(unsigned char ucFlag);

unsigned char GetINCardName(unsigned char ucFlag ,unsigned char *temp,unsigned char *aucPrintBuf);
void UTIL_ClearEMVInterfaceData(void);
unsigned char UTIL_WaitGetKey(unsigned char wait_sec);
unsigned char UTIL_WaitGetKey_AD(unsigned char wait_sec);
unsigned char KEY_InputKey_Double_TEST(unsigned char ucKeyIndex);
unsigned char EMV_UTIL_Input_PP(void);
unsigned char UTIL_SetPinpad(void);
unsigned char UTIL_GetKey_pp(unsigned char wait_sec);
void UTIL_SetDefault_Clear(void);
unsigned char UTIL_CheckProjAndVer(void);
unsigned char UTIL_ClearTrans(void);
unsigned char UTIL_GetDateTime(unsigned char * pucDateTimeBuf );
unsigned char SERV_Print_Detail(void);
unsigned char UTIL_SetInputCardMode(unsigned char * ucReadCardMode);
void UTIL_WaitPullOutICC(void);
unsigned char UTIL_DispBalance(unsigned char * aucBuf , unsigned char ucFlag);
unsigned char UTIL_DispDateAndTime(unsigned char lin,unsigned col );
unsigned char  UTIL_GetMenu_Value(unsigned char ucTransType,unsigned short uiIndex,
		unsigned char *pucFlag,unsigned char (*pfnProc)(void),SELMENU *Menu);
unsigned char UTIL_Check_BasetSet(void);
unsigned char UTIL_AccumulatRunTime( void );
void UTIL_EMVSetTransAmount(unsigned long ulAmount);
unsigned char EMV_UTIL_Input_PP(void);
unsigned char EMV_UTIL_Input(void);
void gz_xor(unsigned char *a, unsigned char *b, char lg);
unsigned char UTIL_Backlight(void);
unsigned char UTIL_SetPrinter_Heat(void);
unsigned char UTIL_InPutMobileNum(void);

/* cfg.c */
unsigned char CFG_TermIDAndMarchID(void);
unsigned char CFG_ConstantParamBaseData(void);
unsigned char CFG_ConstantParamPBOCData(void);
unsigned char CFG_ConstantParamDateTime(void);
unsigned char CFG_ChangeParamData(void);
unsigned short CFG_ComputeConstantParamDataChecksum(void);
unsigned char CFG_CheckConstantParamData(void);
unsigned short CFG_ComputeChangeParamDataChecksum(void);
unsigned char CFG_CheckChangeParamData(void);
unsigned char CFG_ConstantParamSystInit(void);
unsigned char CFG_Smart_Download_Keys(void);
unsigned char CFG_ConstantParamPhoneNo(void);
unsigned char CFG_ConstantParamTotal(void);
unsigned char CFG_ConstantParamVersion(void);
unsigned char UTIL_AmountFormat(unsigned char * ucOutAmount ,unsigned long  ucInAmount ,unsigned char * ucDispLen);
unsigned char CFG_TermTypeConfig(void);
unsigned char CFG_SetMarchName(void);
unsigned char CFG_WriteDateTimetoPOS(unsigned char * ucInBuf);
unsigned char CFG_SwitchComm(void);
unsigned char CFG_SetCommParam(void);
unsigned char CFG_AllInOneMenu(void);
unsigned char CFG_SetParam_All(void);
/* key.c */
unsigned char KEY_DownKeyFromCard(void);
unsigned char KEY_DownKeyFromPOS(void);
unsigned char KEY_InputKeyIndex(void);
unsigned char KEY_InputMasterKey(void);
unsigned char KEY_InputKey_Single(unsigned char ucKeyIndex);
unsigned char KEY_InputKey_Double(unsigned char ucKeyIndex);
unsigned char KEY_Load_Default_Key(void);
unsigned char KEY_Load_Default_Key1(void);
unsigned char KEY_Load_Default_Key2(void);
unsigned char KEY_InputSYSTrans(void);



/* msg.c */
unsigned short MSG_ComputeMSGDataChecksum(void);
void MSG_CheckMSGData(void);
void MSG_DisplayMsg( unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned int uiIndex);
void MSG_DisplayMenuMsg( unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned int uiIndex);
void MSG_PinpadDisplayMsg( unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned int uiIndex);
unsigned char MSG_DisplayMsgKey(unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned short uiIndex,
					unsigned short uiTimeout);
unsigned char MSG_WaitKey(unsigned short uiTimeout);
unsigned char MSG_GetMsg(unsigned short uiIndex,unsigned char *pucMsg,unsigned short uiMsgLen);
unsigned char MSG_DisplayErrMsg(unsigned char ucErrorCode);
unsigned char MSG_DisplayErrMsgNotWait(unsigned char ucErrorCode);
unsigned char MSG_DisplayErrMsgNotWait_Dail(unsigned char ucErrorCode);
unsigned short MSG_ComputeErrMSGDataChk(void);
void MSG_CheckErrMSGData(void);
void MSG_WriteErrMSGDataChk(unsigned char ucClrMirrorFlag);
unsigned char MSG_DisplayEMVErrMsg(unsigned char ucErrorCode);

/* pinpad.c */
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

/* admin.c */
unsigned char ADMIN_Process(void);
unsigned char ADMIN_SendTerminalData(void);
unsigned char ADMIN_ReceivePackage(void);
unsigned char ADMIN_ProcessPackage(void);
unsigned char ADMIN_DownloadMsg(void);
unsigned char ADMIN_DownloadErrMsg(void);
unsigned char ADMIN_SendPackage(void);

/* print.c */
void PRINT_ConstantParam(void);
void PRINT_OLDData(void);
void PRINT_Ticket(unsigned char flag,unsigned char ucCopy);
void PRINT_Ticket_stylusprinter(unsigned char flag,unsigned char ucSaleCopy);
void PRINT_Ticket_HeatPrinter_1(unsigned char flag,unsigned char ucSaleCopy);
void PRINT_Ticket_HeatPrinter_2(unsigned char flag,unsigned char ucSaleCopy);
void PRINT_Ticket_HeatPrinter_3(unsigned char flag,unsigned char ucSaleCopy);
void PRINT_Ticket_HeatPrinter(unsigned char flag,unsigned char ucSaleCopy);
void PRINT_List(unsigned char flag);
unsigned char PRINT_RePrint_List(void);
unsigned char PRINT_PrintTwoMSG( unsigned char ucFontType,
					unsigned short iIndex1,
					unsigned char *pucBuf1,
					unsigned short uiBufLen1,
//					unsigned short iIndex2,
					short iIndex2,
					unsigned char *pucBuf2,					unsigned short uiBufLen2);
unsigned char PRINT_PrintMsg( unsigned char ucFontType,unsigned short uiIndex);
void PRINT_Print_Local_Date_Time(void);
unsigned char PRINT_Detail_Menu(void);
void PRINT_detail(void);
void PRINT_detail_void(void);
void PRINT_detail_ZD(void);
void PRINT_transType(unsigned char Flag);
void PRINT_SettleTotal(unsigned char flag);
void PRINT_OfflineDetial(void);
unsigned char PRINT_GetReadCardMethord(void);
void PRINT_Ticket_L_Old(unsigned char flag);
void PRINT_Ticket_L_New(unsigned char flag);
void PRINT_Ticket_L_2(unsigned char flag);
void PRINT_Ticket_S(unsigned char flag);
void PRINT_TransType_Stylusprinter(unsigned char Flag);
void PRINT_TransType_L(unsigned char Flag);
void PRINT_version(void);
void PRINT_remark(void);
void PRINT_remark_L(void);
unsigned char PRINT_GB2312_xprint(uchar * pucBuffer, uchar ucFont );
unsigned char PRINT_xprint(uchar * pucBuffer);
unsigned char PRINT_xprint_Big(uchar * pucBuffer, uchar ucFont );
unsigned char PRINT_xlinefeed(uchar ucLineNum);
unsigned char PRINT_XReverseFeed(uchar ucLineNum );
void PRINT_EveryCardDetail(unsigned char printFlag);
unsigned char Print_CheckBase(void);
void PRINT_ext_xlinereverse(uchar nbline);
unsigned char PRINT_ext_GB2312_xprint(unsigned char *pucPtr, unsigned char ucGBFont);
unsigned char PRINT_ext_xlinefeed(uchar nbline);
unsigned char PRINT_ext_xprint(uchar *buffer);
unsigned char PRINT_DFDatalist(void);

/* SEL4442*/
unsigned char SLE4442_ATR(uchar *pBuf);
unsigned char SLE4442_Read_Main_Memory(uchar *pBuf,uchar ucAddr,uchar ucNbr);
unsigned char SLE4442_Read_Security_Memory(uchar *pBuf);
unsigned char SLE4442_Update_Main_Memory(uchar Input_Data,uchar ucAddr);
unsigned char SLE4442_Update_Security_Memory(uchar Input_Data,uchar ucAddr);
unsigned char SLE4442_Compare_Verification_Data(uchar Input_Data,uchar ucAddr);
unsigned char SLE4442_ResetAddress(uchar *code_buff);
unsigned char SLE4442_End_ATR(uchar *code_buff);
unsigned char SLE4442_Enter_CMD_Mode(uchar *code_buff);
unsigned char SLE4442_Leave_CMD_Mode(uchar *code_buff);
unsigned char SLE4442_Enter_Outgoing_Mode(uchar *code_buff);
unsigned char SLE4442_Send_Command(uchar *code_buff,uchar Command_Type,uchar Addr,uchar Input_Data);
unsigned char SLE4442_ReadBits(uchar *code_buff, uchar N_Bits);
unsigned char SLE4442_NReadBits(uchar *code_buff, uchar N_Bits);
unsigned char SLE4442_Erase_And_Write(uchar *code_buff);
unsigned char SLE4442_Compare_Process(uchar *code_buff);
unsigned char SLE4442_FinishCommand(uchar *code_buff);
unsigned char SLE4442_Check_Secret(uchar *pSecret_Code);
unsigned char SLE4442_Modify(void);
unsigned char SLE4442_Write(void);
unsigned char SLE4442_Read(void);
unsigned char SLE4442_Process(void);
unsigned char Check_Card_Valid(unsigned char *key);
unsigned char Smart_Card_Trans(unsigned char *key);

/*e_pub.c*/
unsigned char SEL_SystemMenu(void);
unsigned char Pocket_Read_Binary(unsigned char ucReader,
			unsigned short ucOffset,unsigned char ucInLen);
unsigned char Pocket_Select_File(unsigned char ucReader,
			unsigned short pucAID);
unsigned char Pocket_Input_VerifyPIN(void);
unsigned char Pocket_Verify_PIN(unsigned char ucReader,
			unsigned char ucInLen,unsigned char *pucPinData);
unsigned char Pocket_Update_Binary(unsigned char ucReader,unsigned short ucOffset,
			unsigned char ucInLen,unsigned char *ucBuf);
unsigned char Wait_Read_Card(void);
/*e_logon*/
unsigned char LOGON_Process(void);
unsigned char E_LOGON_Online(void);

/*syscash.c*/
unsigned char SEL_SYSMenu(void);
unsigned char SYSCASH_ChangeSuperPasswd(void);
unsigned char SYSCASH_AddCashier(void);
unsigned char SYSCASH_DelCashier(void);
unsigned char SYSCASH_ModiCashier(void);
unsigned char SYSCASH_SuperKey(void);
unsigned char SYSCASH_SetTransType(void);
unsigned char SYSCASH_CheckCashierData(void);
void SYSCASH_WriteCashierDataChk(unsigned char ucClrMirrorFlag);

/* test.c */
void test_Disp(void);
void Disp_yinglian(void);
void Test_ErrCode(void);

/*appmain.c*/
void DispAD(unsigned char ucFalg);
void DispLogo_yinglian(void);
unsigned char FileSysAPPMain(void);
unsigned char DispDateAndTime(unsigned char lin,unsigned col );
void DispPINPAD_AD(void);
unsigned char OffLineSettle(unsigned char ucReTryFlag);

/*  Menu.c */
unsigned char Menu_EPTrans(void);
unsigned char  MenuSys_TerminalSet(void);
unsigned char  MenuSys_Init(void);
unsigned char OSUTIL_CheckIPAddrField(
                unsigned char *pucIP,
                unsigned char ucIPLen,
                unsigned char *pucIPField);
unsigned char OSUTIL_InputIPV4Addr(
                unsigned int uiTimeout,
                unsigned char ucFont,
                unsigned char ucRow,
                unsigned char *pucIPV4Addr);

unsigned char   Main_MainFunction(unsigned char ucKey);
void Main_PowerOff(void);
unsigned char Main_Logon(void);
unsigned char Main_EMVProcess(unsigned char  ucFlag);
void UTIL_PINPADDispLOGO(void);
unsigned char UITL_Input_Rate(void);
unsigned char UTIL_BANK_Process(void);
unsigned char UTIL_BANK_OTHERS(void);
unsigned char UTIL_OTHERBANK_OTHERS(void);
unsigned char UTIL_Set_BankID(void);
void UTIL_AddTraceNumber(void);

unsigned char UITL_SetAutoSettleTime(void);
void UTIL_AutoSettleOn(void);
void AutoSettle(void);


/*port.c*/
unsigned char Port_Main_Process(RECEIVETRANS *ReceiveData,unsigned char* aucOutData);
unsigned char Port_Card_trans(unsigned char *ucFlag,RECEIVETRANS *ReceiveData/*,SENDTRANS *SendTransData*/);
unsigned char Port_PackResult(RECEIVETRANS *ReceiveData,SENDTRANS *SendTransData,unsigned char ucResult);
unsigned char Port_Card_trans_SecondDeal(unsigned char *ucFlag,RECEIVETRANS *ReceiveData);
unsigned char Port_Query_trans(unsigned char *ucFlag,RECEIVETRANS *ReceiveData);
unsigned char Port_PrintTicket(void);

unsigned char LOADKEY_LoadKeyFromPOS(void);
unsigned char MSG_GetErrMsg(unsigned char ucErrorCode,unsigned char *aucBuf);

#endif
unsigned char TRANS_SetField55_Reversal(TAGPHASE enTagPhase,unsigned char ucLogType , unsigned char * SendBuf ,unsigned short * SendBufLen);
////////////////////////EC///////////
unsigned char Mag_ECMenu(void);
unsigned char MASAPP_SwipeCard(void);
unsigned char MASAPP_GetTrackInfo(unsigned char *pucTrack);
unsigned char UTIL_InputEncryptPIN_EC(unsigned char * pucEnPIN,unsigned char * pucPinLen);
////////////////////////reader hongbao
unsigned char TRANS_SetUntouchField55(unsigned char * SendBuf ,unsigned short * SendBufLen);
unsigned char SEL_DisplayEMVInfo(void);
unsigned char SEL_ReaderMenu_HongBao(void);
unsigned char SEL_ReaderMenu_Sand(void);
unsigned char SEL_TestOnlineTrans(void);
unsigned char CFG_ChangeReaderParamData(void);
UCHAR UTIL_SaveReaderCAPKFile(void);
unsigned char CFG_ChangeReaderParamData_HongBao(void);
unsigned char VerifyReaderCAPK(unsigned char ucFlag);
unsigned char EMV_DisplayReaderPKInfo(void);
UCHAR UTIL_ManualSaveReaderCAPKFile(void);
unsigned char UTIL_SetReader(void);
unsigned char MASAPP_External_Event(DRV_OUT *aucEventOutData,unsigned char InputMode,unsigned char * ucOutKey);
unsigned char MASAPP_UnTouchCard_Event(void);
unsigned char SetHBReaderAIDInfo(void);
unsigned char CFG_ChangeReaderParamData_Sand(void);
unsigned char TRANS_SetUntouchField55_Reversal(unsigned char * SendBuf ,unsigned short * SendBufLen);
/////////////////////////////reader sand///////////////////////////
#if SANDREADER
unsigned char Trans_InputOrder(void);
unsigned char MASAPP_Event_SAND(unsigned char ucDispInfoFlag , unsigned char * ucOutKey);
UCHAR UTIL_WriteReaderParamFile(EMV_CONSTPARAM * pConstParam);
UCHAR	QTransProcess(QTRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount);
void MasApp_QSetEMVTransInfo(void);
unsigned char QMasApp_CARDPAN(unsigned char* pstrCardNo,unsigned char *pucCardNoLen);
UCHAR	QTransCapture(TRANSRESULT	enTransResult);
unsigned char QMasAPP_CheckAID(void);
unsigned char QTRANS_SetField55(TAGPHASE enTagPhase,unsigned char ucLogType , unsigned char * SendBuf ,unsigned short * SendBufLen);
unsigned char QTRANS_SetField55_Reversal(TAGPHASE enTagPhase,unsigned char ucLogType , unsigned char * SendBuf ,unsigned short * SendBufLen);
unsigned char  QMasApp_CopytEMVTransInfoToNormalTrans(void);
void SetSandReaderAIDInfo(void);
UCHAR	QCardHolderConfirmApp(PQCANDIDATELIST pCandidateList,PUCHAR pucAppIndex);
unsigned char QEMV_SetEMVConfig(void);
unsigned char EMV_GetTagFromStr(uchar *aucTagStr,uchar *aucTag);
unsigned char DIGITAL_CheckCard(void);
UCHAR WKICC(QICCIN *pQICCIn ,QICCOUT  *pQICCOut);
unsigned char DIGITAL_SendReceive(unsigned int uiTime);
unsigned char DIGITAL_SendReceive_tmp(void);
unsigned char DIGITAL_RemoveCard(uchar ucDisFlag);
void QAID_Dump(void);
#endif
unsigned char CASH_GetLogonCasherNo(unsigned char *pucIndex);
void PRINT_transType_NEW(unsigned char *paucOutData,unsigned char *paucOutDataEN);
UCHAR QEMVICC(QICCIN *pQICCIn ,QICCOUT  *pQICCOut);

unsigned char EXTrans_ProHead_Ic(struct seven *EventInfo);
unsigned char EXTrans_Process_IC(struct seven *EventInfo);

unsigned char EXTrans_ProHead_Mag(struct seven *EventInfo);

unsigned char EXTrans_Process(unsigned char Flag);

unsigned char EX_KEY_InputKey_Double(unsigned char ucKeyIndex);
//int utf8togb2312(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);



unsigned int OWE_RstErrInfo(int ret);
unsigned char OWE_GetIsoTrack(unsigned char *pucTrack);
unsigned char OWE_GetCardInfo(void);


void PBOC_CPU_TEST(VOID);


void PRINT_tts(void);
