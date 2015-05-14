/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  halicc.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-02-22		Xiaoxi Jiang
    Last modified :	2003-02-22		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __LIBICC_H__
#define __LIBICC_H__


#ifndef LIBICCDATA
#define LIBICCDATA extern
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define LIBICC_DEBUG




//#define true			1
#define flase			0

#define Test
#ifndef SUCCESS
#define SUCCESS							0x00
#endif
#define CARDCMD_READER00				0x00
#define CARDCMD_READER01				0x01
#define CARDCMD_READER02				0x02

#define CARDCMD_Invoice					0x01
#define CARDCMD_Return					0x02
#define CARDCMD_Invalid					0x03

#define KeyTypeErr						0x4F		// 密钥类型错误

#define CARDRETURNERR					0xFF

#define LIBICCERR_SUCCESS               0x00
#define LIBICCERR_NOICC                 0x01
#define LIBICCERR_NOREADER              0x02
#define LIBICCERR_OVERRECVBUF           0x03
#define LIBICCERR_BADID                 0x04
#define LIBICCERR_POWERFAIL				0x05
#define LIBICCERR_CMDNOTIMPLEMENT       0x0B
#define LIBICCERR_BADCMDOUTPARAM        0x0C
#define LIBICCERR_WAITCAPDU				0x0F
#define LIBICCERR_BADTS					0x3C
#define LIBICCERR_ATRTOOLONG            0x12
#define LIBICCERR_CARDFAULT             0x15
#define LIBICCERR_BADCMD                0x16
#define LIBICCERR_BADCMDINPARAM         0x17
#define LIBICCERR_UNKNOWNPROTOCOL       0x18
#define LIBICCERR_PROTOCOLPARAM         0x19
#define LIBICCERR_APDUSENDTIMEOUT       0x1A
#define LIBICCERR_APDURECVPROCTIMEOUT   0x1B
#define LIBICCERR_APDURECVTIMEOUT1      0x1C
#define LIBICCERR_APDURECVTIMEOUT2      0x1D
#define LIBICCERR_APDUEXCHUNKNOWN       0x1E
#define LIBICCERR_UNKNOWNSTATUS         0x1F
#define LIBICCERR_NOTPOWERED            0x20
#define LIBICCERR_ATRPARITYERROR		0x21
#define LIBICCERR_APDUPARITYERROR		0x22
#define LIBICCERR_APDUTIMEOUT			0x23
#define LIBICCERR_BADTA1				0x24
#define LIBICCERR_BADTB1                0x25
#define LIBICCERR_BADTD1                0x26
#define LIBICCERR_BADTA2                0x27
#define LIBICCERR_BADTB2                0x28
#define LIBICCERR_BADTC2				0x3D
#define LIBICCERR_BADTD2                0x2A
#define LIBICCERR_BADTA3                0x2B
#define LIBICCERR_BADTB3				0x2C
#define LIBICCERR_BADTC3                0x2D
#define LIBICCERR_BADTCK                0x3E
#define LIBICCERR_BADPROCBYTE			0x41

#define LIBICCERR_EXECUTEOK             0x30
#define LIBICCERR_CARDREMOVE            0x31
#define LIBICCERR_VPPPROBLEM            0x32
#define LIBICCERR_VCCPROBLEM            0x33
#define LIBICCERR_ISASYNCCARD           0x35
#define LIBICCERR_ISSYNCCARD			0x44
#define LIBICCERR_CARDMUTE              0x3F
#define LIBICCERR_NEEDWARMRESET         0x39
#define LIBICCERR_NEEDCOLDRESET			0x3A
#define	LIBICCERR_NEEDSELECTCARD		0x3B
#define LIBICCERR_STANDARDERR			0x3C

#define LIBICCERR_BADSYNCCMD			0x40
#define LIBICCERR_SYNCEOC				0x41
#define LIBICCERR_SYNCSETDATA			0x42
#define LIBICCERR_SYNCMACRO				0x43

#define LIBICCERR_T1BADLEN				0x4F
#define LIBICCERR_T1BADEDC				0x50
#define LIBICCERR_T1BADPCB				0x51
#define LIBICCERR_T1BADRECVSEQ			0x52
#define LIBICCERR_T1UNKNOWNERR			0x53
#define LIBICCERR_T1BADNAD              0x54
#define LIBICCERR_T1SABORTREQUEST       0x55
#define LIBICCERR_T1BADIFRAMELEN        0x56
#define LIBICCERR_T1BADRFRAMELEN        0x57
#define LIBICCERR_T1BADSFRAMELEN        0x58
#define LIBICCERR_T1BADPROTOCOL         0x59
#define LIBICCERR_T1INVALIDSEQ          0x5A
#define LIBICCERR_T1RFRAMEERR           0x5B
#define LIBICCERR_T1SFRAMEERR           0x5C
#define LIBICCERR_T1ILINKERR            0x5D
#define LIBICCERR_T1RFRAMEBADEDC		0x5E
#define LIBICCERR_T1INVALIDBLOCK        0x5F

#define LIBICCERR_T1IFRAMERESEND        0xFA
#define LIBICCERR_APDUNONEXTCOMMAND     0xFB
#define LIBICCERR_ATRHASLASTCHAR		0xFC
#define LIBICCERR_ATRNOTFINISHED        0xFD
#define LIBICCERR_PROCESSEND            0xFE
#define LIBICCERR_PROCESSPENDING        0xFF

#define LIBICCERR_I2CNACK				0x70

#define ICCMETHOD_ASYNC					0x00
#define ICCMETHOD_SYNC					0x01

#define ICCSTANDARD_EMV                 0x00
#define ICCSTANDARD_ISO7816             0x01
#define ICCSTANDARD_MPEV5               0x02
#define ICCSTANDARD_MONDEX              0x03



#define ICCPROTOCOL_T1                  0x01
#define ICCPROTOCOL_T0                  0x00

#define ICCAPDU_CASE0                   0x00
#define ICCAPDU_CASE1                   0x01
#define ICCAPDU_CASE2                   0x02
#define ICCAPDU_CASE3                   0x03
#define ICCAPDU_CASE4                   0x04
#define ICCAPDU_CASE5                   0x05
#define ICCAPDU_CASE6                   0x06
#define ICCAPDU_CASE7                   0x07

#define ICCSTATUS_INSERT                0x01
#define ICCSTATUS_REMOVE                0x02
#define ICCSTATUS_POWERED               0x03
#define ICCSTATUS_ISSYNCCARD			0x04

#define ICCTYPE_CPUCARD					0x00
#define ICCTYPE_SLE4442CARD				0x01

#define ICCSESSION_IDLE                 0x00
#define ICCSESSION_COLDRESET            0x01
#define ICCSESSION_WARMRESET            0x02
#define ICCSESSION_DATAEXCH             0x03

#define ICCSESSIONSTEP_IDLE             0x00
#define ICCSESSIONSTEP_COLDRESET1       0x01
#define ICCSESSIONSTEP_COLDRESET2       0x02
#define ICCSESSIONSTEP_COLDRESET3       0x03

#define ICCSESSIONSTEP_DATAEXCHSEND1    0x04
#define ICCSESSIONSTEP_DATAEXCHRECV1    0x05
#define ICCSESSIONSTEP_DATAEXCHSEND2    0x06
#define ICCSESSIONSTEP_DATAEXCHRECV2    0x07
#define ICCSESSIONSTEP_DATAEXCHSEND3    0x08
#define ICCSESSIONSTEP_DATAEXCHRECV3    0x09
#define ICCSESSIONSTEP_DATAEXCHSEND4    0x0A
#define ICCSESSIONSTEP_DATAEXCHRECV4	0x0B

#define ICCCMD_SUCCESS					0x00
#define ICCCMD_BADPACKET				0x90
#define ICCCMD_NODATASEND				0x91

#define ICCCMD_MAXNB                    0x0C
#define ICCCMD_MAXBUF					256
#define LIBICC_MAXATRBUF                100
#define LIBICC_MAXCOMMBUF	            255	//zy 270

#define LIBICC_COLDRESET                0x00
#define LIBICC_WARMRESET                0x01

#define LIBICC_READERNB         		4

#define LIBICC_WWTOUT					0x00
#define LIBICC_BWTOUT					0x01
#define LIBICC_CWTOUT					0x02

#define	REV_TIME_OUT					100


    /******************************************************************************/
//税控宏定义
#define  SIMcardResetErr  	 0x01 	/*税控卡上电错误	   01H ( WatchCore错误 )*/
#define  CPUcardResetErr  	 0x02	/*用户卡上电错误	   02H*/
#define  SPVcardResetErr  	 0x03 	/*税务管理卡上电错误 03H*/
#define  NoCPUcardErr     	 	 0x04	/*无用户卡	   04H*/
#define  NoSIMcardErr	   	 0x05	/*无税控卡	   05H*/
#define  NoSPVcardErr	   	 0x06 	/*无税务管理卡	   06H*/
#define  ReadSIMcardErr   	 0x07 	/*读税控卡错误	   07H*/
#define  ReadCPUcardErr   	 0x08 	/*读用户卡错误 	   08H*/
#define  ReadSPVcardErr   	 0x09 	/*读税务管理卡错误   09H*/
#define  WriteSIMcardErr  	 0x0a 	/*写税控卡错误	   0AH*/
#define  WriteCPUcardErr  	 0x0b	/*写用户卡错误 	   0BH*/
#define  WriteSPVcardErr     	 0x0c	/*写税务管理卡错误   0CH*/
#define  InvalidSIMcardErr 	 0x0d	/*非法税控卡	   0DH*/
#define  InvalidCPUcardErr	 0x0e	/*非法用户卡	   0EH*/
#define  InvalidSPVcardErr	 0x0f	/*非法税务管理卡	   0FH*/
#define  UnTaxInitStatus		 0x10	/*未初始化状态	   10H*/
#define  TaxDateLimitOver	 0x11	/*已过截止开票日期   11H*/
#define  TaxValLimitOver     	 0x12	/*已超过单张发票开票金额限额  12H*/
#define  TaxTotalLimitOver	 0x13	/*已超过开票累计金额限额	   13H*/
#define  TaxReturnValLimitOver 0x14	/*已超过退票累计金额限额	   14H*/
#define  RegistID_NoMatch	 0x15	/*税控卡的注册号与税控器的注册号不一致  15H*/
#define  CPUcardMACchkErr	 0x16	/*用户卡校验MAC错误  16H*/
#define  SIMcardMACchkErr	 0x17	/*税控卡校验MAC错误  17H*/
#define  PINchkErr		        0x18	/*检查PIN失效	   18H*/
#define  InvalidTaxTypeErr	 0x19	/*非法税种税目	   19H*/
#define  RptDateCurDate	     	 0x1a	/*申报截止日期大于当前日期	 1AH (申报时判断)*/
#define  RptDateLastRptDateErr 0x1b	/*申报截止日期小于上次申报日期  1BH (报税回插出错判断)*/
#define  CPUcardLocked		 0x1c	/*用户卡已锁	   1CH*/

#define  TaxCtrlCardLocked      	0x1d 	/*税控卡已锁*/
#define  TaxDeviceInited         		 0x1e 	/*税控器已初始化不能再次初始化*/
#define  CheckSumByteErr       	 0x1f	 /*校验位不正确*/
#define  CmdTypeErr             		 0x20 	/*命令参数错误*/
#define  FrameHeadFormatErr 	 0x21	 /*包头错误*/
#define  FrameLengthErr         		  0x22 	/*包长度错误*/
#define  InvoicePaperEmpty    	  0x23 	/*发票已用完,请分发发票*/
#define  InvoiceMemoryErr      	  0x24 	/*发票存储器异常*/
#define  FiscalMemoryErr        		  0x25	 /*税控存储器异常*/
#define  DataOverflow           		0x26 	/*时间、日期、数据溢出*/
#define  TaxCtrlCardRegisted   	 0x27 	/*税控卡已注册*/
#define  CardSpaceFull          		0x28 	/*卡已满，换卡操作*/
#define  NoInvoicePaperCanBeUse 	0x29 /*没有已分发但未使用的发票号*/
#define  NoInvoiceRollCanBeUse  		0x2a /*用户卡中无发票分发*/
#define  InvalidUserNo          			0x2b /*非法分户编号*/
#define  InvoiceNo_DataNotFind  		0x2c /*此发票号无相关数据*/
#define  InvoiceNo_UseToReturn  		0x2d /*此发票已作退票处理*/
#define  InvoiceNo_UseToBlankOut 	0x2e /*此发票已作废票处理*/
#define  BLCurrDateLiSysDate	 	0x2f /*系统日期小于税控当前日期*/
#define  BLCollectMust			0x30 /*请汇总发票卷*/
#define  CustPinError				 0x31 /*个人密码错误*/
#define  CustDeclare				 0x32 /*请先作申报*/
#define  CustReload			 	0x33 /*请先作申报*/
#define  NoStatisticsData			0x34 /*无统计数据*/
#define  CanNotDeclare			0x36
#define  TaxNameNoChange		0x37

#define  FileNotFound				0x40 /*未找到文件*/
#define  VerifyFail				0x41 /*认证失败*/
#define  KeyLockedErr			0x42 /*密钥被锁*/
#define  UseConditionErr			0x43 /*使用状态不满足*/
#define  FileTypeErr				0x44 /*文件类型错误*/
#define  SafeConditionErr			0x45 /*安全状态不满足*/
#define  MessageDataErr			0x46 /*安全报文数据错*/
#define  DataParamErr			0x47 /*数据参数错*/
#define  FunctionNotSupported	0x48 /*不支持此功能*/
#define  FileSpaceInsufficient		0x49 /*文件存储空间不够*/
#define  KeyNotFound				0x4A /*密钥未找到*/
#define  CardNoDataErr			0x4B /*卡中无数据返回*/
#define  MacErr					0x4C /*MAC错*/
#define  ApplicationLocked		0x4D /*应用被永久锁定*/
#define  KeyVersionNotSupported	0x4E /*密钥版本不支持*/
#define  KeyTypeErr				0x4F /*密钥类型错误*/

#define  USER_CANCEL			0x50
#define  WriteRecordEnd			0x51	/* 发票记录文件已满*/
#define  TaxInitStatus			 0x80	/*已初始化状态	   80H*/
#define  NoInvoiceRoll          		0x81   /*用户卡内无发票可分发*/
#define  IssInvoiceValLimitOver	 0x82   /*金额超限*/
//#define  CmdParaErr				 0x83   /*卡返回说:命令参数错*/
#define  CommPortOpenErr      	  0x84   /*打开串口失败*/
#define  Eeprom_RW_err        		  0x85   /*Eeprom读写失败*/
#define  FlashRom_RW_err      		  0x86   /*Flash页写失败*/
#define  TaxCtrlCardResetErr    	0x87   /*税控卡复位错误*/
#define  TaxCtrlCardFileReadErr1 	0x88   /*读税控卡基本信息文件错误*/
#define  TaxCtrlCardFileReadErr2 	0x89  /*读税控卡监控管理数据文件错误*/
#define  TaxCtrlCardFileReadErr3	 0x8A  /*读税控卡发票存储文件错误*/
#define  UserCardResetErr 	 	0x8B   /*用户卡复位错误*/
#define  UserCardFileReadErr1	 0x8C   /*读用户卡基本信息文件错误*/
#define  UserCardFileReadErr2   	0x8D   /*读用户卡税种税目索引文件错误*/
#define  WorkStatusShakeErr    	 0x8E   /*工作状态握手失败*/
#define  DiagStatusSHakeErr     	0x8F   /*测试状态握手失败*/
#define  TaxReportFinished     		 0x90   /*已经报过税*/
#define  ReadRecFileErr			 0x91   /*读 定长记录 失败*/
#define  TaxReportNotEnd       	 	0x92   /*卡未完税*/
#define  DateParaErr		 		0x93   /*税控初始化时日期参数错*/
#define  CheckDataDoNotNeed     	0x94   /*无核查数据*/
#define  TaxCommunicationErr    	0x95   /*税控器通讯错误*/
#define  NowInvoiceNotEndErr       0x96   /*当前发票卷未用完错误*/
#define  SellParameterErr       		0x97   /*销售参数错误*/
#define  SevenYearNoteDateErr   	0x98   /*Flash中5年记录日期错误*/
#define  InvoiceTypeParameterErr   	0x99   /*'发票类型'参数错误*/
#define  DeclareParamErr 		0x9a  /*申报类型错误*/
#define  InvROllNotEmpty        		0x9b   /*发票卷未用完,不需生成发票卷汇总区*/
#define  ChkCardTreateErr_0     	0x9c   /*核查类别为 调整日期,不需生成核查记录*/
#define  CannotDistriboteInvoice 	0x9d  /*税控卡内发票存储文件 发票未用完*/
#define  RecordNotFound          	0x9e  /*未找到记录*/
#define  ReportDateParaErr       	0x9f  /*纳税申报时日期错*/
#define  ChkNeedContinure        	0xa0  /*核查 需要 再插卡*/
#define  ChkTypeErr		        	0xa1  /*核查类别错*/
#define  InvNoErr        			0xa2  /*分发发票号错误*/
#define  CheckDataErr        	0xa3  /*输入核查日期错误*/
#define  COM_TRANS_ERR        	0xa4  /*串口数据通讯出错1*/
#define  COM_TRANS_ERR1        	0xa5  /*串口数据通讯出错1*/
#define  COM_TRANS_ERR2       	0xa6  /*串口数据通讯出错2*/
#define  COM_TRANS_ERR3        	0xa7  /*串口数据通讯出错3*/
#define  COM_TRANS_ERR4        	0xa8  /*串口数据通讯出错4*/
#define  COM_TRANS_ERR5       	0xa9  /*串口数据通讯出错5*/
#define  TCKERR			       	0xaa  /*串口数据通讯出错5*/
#define  TIMEOUTERR			       	0xAB  /*串口数据通讯出错5*/
#define  TSERR			       	0xAC  /*串口数据通讯出错5*/
#define  TC2ERR			       	0xAD  /*串口数据通讯出错5*/
#define  UNKNOWERR			    0xAE  /*未知错误*/
#define  IC_ID_NoMatch	 	0xAF	/*用户卡注册号与税控机注册号不符*/
#define  OneInvAmtLimitOver	 0xb0   /*单张发票金额超限*/
#define  TotInvAmtLimitOver	 0xb1   /*累计开票金额超限*/
#define  TotRefAmtLimitOver	 0xb2   /*累计退票金额超限*/
#define  TimeLimitOver	 0xb3   /*机器已到使用期限*/
#define  ModifyWareList		0xb4	/* 修改商品销售表 */

    /*--------------------------------------------------------------------------------------------------*/

    typedef struct
    {
        unsigned char aucBuf[256];
    }UARTSENDBUF;

    typedef struct
    {
        unsigned char (*pfnProc)(void);
    }ICCCMD;

#if 0 //0714
    typedef struct
    {
        volatile unsigned char ucStatus;
        volatile unsigned char ucSession;
        volatile unsigned char ucSessionStep;
        volatile unsigned char ucLastError;
        volatile unsigned char ucPowerType;
        volatile unsigned char ucResetType;
        volatile unsigned char ucProtocol;
        unsigned char ucFi;
        unsigned char ucDi;
        unsigned char ucRate;
        unsigned char ucGTR;
        unsigned long ulWWT;
        unsigned short uiCWT;
        unsigned char ucWTX;
        unsigned long ulBWT;
        unsigned char ucT1IFSReqFlag;
        unsigned char ucT1IFSC;
        unsigned char ucT1SendSeq;
        unsigned char ucT1RecvSeq;
        unsigned char ucT1EDCMode;
        unsigned char ucCardType;				// 卡的类型
    }ICCINFO;
#else
    typedef struct
    {
        volatile unsigned char ucStatus;
        volatile unsigned char ucSession;
        volatile unsigned char ucSessionStep;
        volatile unsigned char ucLastError;
        volatile unsigned char ucStandard;
        volatile unsigned char ucPowerType;
        volatile unsigned char ucResetType;
        volatile unsigned char ucAutoPower;
        volatile unsigned char ucATRLen;
        unsigned char aucATR[LIBICC_MAXATRBUF];
        volatile unsigned char ucProtocol;
        volatile unsigned char ucAPDUCase;
        volatile unsigned short uiRecvLen;
        volatile unsigned short uiRecvedLen;
        unsigned char aucRecvBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned char ucAPDUOneByteFlag;
        volatile unsigned short uiAPDUSendLen;
        unsigned char aucAPDUSendBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiAPDUSentLen;
        volatile unsigned short uiAPDURecvLen;
        unsigned char aucAPDURecvBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiAPDURecvedLen;
        volatile unsigned short uiT1FrameSendLen;
        unsigned char aucT1FrameSendBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiT1FrameSentLen;
        volatile unsigned short uiT1FrameLastSendLen;
        unsigned char aucT1FrameLastSendBuf[LIBICC_MAXCOMMBUF];
        unsigned char aucSYNCSendBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiSYNCSendLen;
        unsigned char ucSYNCSendByte;
        unsigned char ucSYNCSentBit;
        unsigned char aucSYNCRecvBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiSYNCRecvedLen;
        unsigned char ucSYNCRecvedByte;
        unsigned char ucSYNCRecvedBit;
        unsigned char ucFi;
        unsigned char ucDi;
        unsigned char ucGTR;
        unsigned long ulWWT;
        unsigned short uiCWT;
        unsigned char ucWTX;
        unsigned long ulBWT;
        unsigned char ucT1Status;
        unsigned char ucT1ErrorCount;
        unsigned char ucT1IFSReqFlag;
        unsigned char ucT1IFSC;
        unsigned char ucT1NAD;
        unsigned char ucT1SendSeq;
        unsigned char ucT1RecvSeq;
        unsigned char ucT1EDCMode;
    }ICCINFO;
#endif
    /*
    EN:
    ZH_CN:
        智能卡接口所用到的变量   */
    LIBICCDATA ICCINFO ICCInfo[LIBICC_READERNB];


    LIBICCDATA char bAbnoFiscalFlag;			// 触点异常标志
    LIBICCDATA char bAbnoUserFlag;
    LIBICCDATA char bAbnoControlFlag;

    LIBICCDATA char bRemoveCardFlag;			// 卡拔出标志
    LIBICCDATA char bRemoveFiscalFlag;

    LIBICCDATA char bTO3;
    LIBICCDATA char bTO2;
    LIBICCDATA char bTO1;
    LIBICCDATA char bFinished;
    LIBICCDATA char bSentByte;
    LIBICCDATA char bRecvByte;

    unsigned char ucCardReader;
    unsigned char ucSW1;
    unsigned char ucSW2;
    unsigned char ucCardRecvLen;
    unsigned char ucCardRecvStatus;
    unsigned char aucCardRecvBuf[256];

    unsigned char ucCardSendLen;
    unsigned char aucCardSendBuf[256];

    unsigned char aucATR[LIBICC_MAXATRBUF];

    unsigned char uiRecvLen;
    unsigned char uiRecvedLen;
    unsigned char ucAPDUCase;
    unsigned char ucAPDUOneByteFlag;
    unsigned char uiAPDUSendLen;
    unsigned char uiAPDUSentLen;
    unsigned char uiAPDURecvLen;
    unsigned char uiAPDURecvedLen;
    unsigned char aucAPDURecvBuf[LIBICC_MAXCOMMBUF];
    LIBICCDATA volatile unsigned char ucATRLen;

    LIBICCDATA volatile unsigned short uiT1FrameLastSendLen;
    LIBICCDATA volatile unsigned short uiT1FrameSendLen;
    LIBICCDATA volatile unsigned short uiT1FrameSentLen;
    unsigned char aucT1FrameSendBuf[LIBICC_MAXCOMMBUF];

    /* timer.c */
    unsigned char aucCtrlTimer[7];

    /* 发票部分信息 */
    LIBICCDATA unsigned short usRefInvSectNo;			/* 退货时原来的发票序号 */
    unsigned char ucRefInvSectNo;			/* 退货时原来的发票序号 */

    unsigned char ucReaderNO;


    /* pboc.c add by zy */
    void PBOC_SetIso7816Out(void);
    unsigned char PBOC_SelectFile(unsigned char ucFlag,unsigned char ucReader,
                                  unsigned char *pucData,unsigned char ucDataLen);
    unsigned char PBOC_ISOGetResponse(unsigned char ucReader,
                                      unsigned char ucLen);
    unsigned char PBOC_ISOReadRecordbySFI(unsigned char ucReader,
                                          unsigned char ucSFI,unsigned char ucRecordNumber);
    unsigned char PBOC_ReadRecord(unsigned char ucReader,unsigned char ucRecordNumber);
    unsigned char PBOC_ISOReadBinarybySFI(unsigned char ucReader,
                                          unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen);
    unsigned PBOC_Get_RegisterNb(unsigned char ucReader);
    unsigned PBOC_Get_RegisterSign(unsigned char ucReader,
                                   unsigned char *pucData,unsigned char ucLen);
    unsigned PBOC_Terminal_Register(unsigned char ucReader,
                                    unsigned char *pucData,unsigned char ucLen);
    unsigned PBOC_Pin_UNBlock(unsigned char ucReader, unsigned char *pucData,unsigned char ucLen);
    unsigned PBOC_VerifyFiscal_Pin(unsigned char ucReader, unsigned char *pucData,unsigned char ucLen);
    unsigned char PBOC_ISOCheckReturn(void);//zy 0716
    unsigned char PBOC_DitributeInvoice(unsigned char ucReader);
    unsigned char PBOC_InputInvoiceNB(unsigned char ucReader,
                                      unsigned char *pucData,unsigned char ucLen);
    unsigned PBOC_VerifyUser_Pin(unsigned char ucReader,
                                 unsigned char *pucData, unsigned char ucPinIndex, unsigned char ucLen);
    unsigned char PBOC_IssueInvoice(unsigned char ucReader,
                                    unsigned char *pucData,unsigned char ucLen);
    unsigned char PBOC_DailyCollectSign(unsigned char ucReader,
                                        unsigned char *pucData,unsigned char ucLen);
    unsigned PBOC_UpdateRecord(unsigned char ucReader,
                               unsigned char ucSFI,unsigned char ucRecordNumber,
                               unsigned char *pucData,unsigned char ucLen,unsigned char ucFlag);
    unsigned char PBOC_UpdateBinary(unsigned char ucReader,unsigned char ucSFI,
                                    unsigned short usOffset,unsigned char *pucData,unsigned char ucLen,unsigned char  ucFlag);
    unsigned PBOC_DeclareDuty(unsigned char ucReader, unsigned char *pucData,unsigned char ucLen);
    unsigned PBOC_DataCollect(unsigned char ucReader, unsigned char *pucData,unsigned char ucLen);
    unsigned PBOC_UpdateControls(unsigned char ucReader, unsigned char *pucData,unsigned char ucLen);
    unsigned char PBOC_GetChallenge(unsigned char ucReader, unsigned char ucLen);
    unsigned PBOC_InternalAuthentication(unsigned char ucReader,
                                         unsigned char *pucData, unsigned char ucPinIndex, unsigned char ucLen);
    unsigned char PBOC_ExternalAuthentication(unsigned char ucReader,
            unsigned char *pucData, unsigned char ucPinIndex, unsigned char ucLen);
    unsigned char PBOC_ISOCheckReturn_test(unsigned char ucReader);

    /* delay.a51 */

    /* i2c.a51 */
    unsigned char I2C_ReadNByte(unsigned char ucWordByte,unsigned char ucLen);
    unsigned char I2C_WriteNByte(unsigned char ucWordByte,unsigned char ucLen);

    /* cardcmd.c */
    unsigned char CARDCMD_DetectCard(unsigned char ucReader);
    unsigned char CARDCMD_GetResponse(unsigned char ucReader,unsigned char ucDataLen);
    unsigned char CARDCMD_VerifyUserPin(unsigned char ucReader,unsigned char *pucData,unsigned char ucPinIndex, unsigned char ucLen);
    unsigned char CARDCMD_Reset(unsigned char ucReader);
    unsigned char CARDCMD_SelectFile(unsigned char ucReader,unsigned char ucFlag,
                                     unsigned char *pucData,unsigned char ucDataLen);
    unsigned char CARDCMD_ReadRecord(unsigned char ucReader,
                                     unsigned char ucSFI,unsigned char ucRecordNumber);
    unsigned char CARDCMD_ReadBinary(unsigned char ucReader,
                                     unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen);
    unsigned char CARDCMD_ExternalAuthentication(unsigned char ucReader,
            unsigned char *pucData, unsigned char ucPinIndex);
    unsigned char CARDCMD_GetRegisterNb(void);
    unsigned char CARDCMD_GetChallenge(unsigned char ucReader,unsigned char ucLen);
    unsigned char CARDCMD_TerminalRegister(unsigned char *pucMAC2);
    unsigned char CARDCMD_IssueInvoice(unsigned char *pucData);
    unsigned char CARDCMD_DeclareDuty(unsigned char *pucData);
    unsigned char CARDCMD_UpdateControls(unsigned char *pucData);
    unsigned char CARDCMD_InputInvoiceNb(unsigned char *pucData);
    unsigned char CARDCMD_VerifyFiscalPin(unsigned char *pucData);
    unsigned char CARDCMD_DailyCollectSign(unsigned char *pucData,unsigned char ucLen);
    unsigned char CARDCMD_RegisterSign(unsigned char *pucData);
    unsigned char CARDCMD_DataCollect(unsigned char *pucData);
//unsigned char CARDCMD_EchoplexControls(unsigned char *pucData);
    unsigned char CARDCMD_DistributeInvoiceNb(void);
    unsigned char CARDCMD_InternalAuthentication(unsigned char ucReader,
            unsigned char *pucData, unsigned char ucPinIndex);
    unsigned char CARDCMD_UpdateRecord(unsigned char ucReader,unsigned char ucSFI,
                                       unsigned char ucRecordNumber,unsigned char *pucData, unsigned char ucLen);
    unsigned char CARDCMD_CheckReturn(unsigned char ucReader);
    unsigned char CARDCMD_CheckCardErr(unsigned short usSW);


    /* libicc.c */
    void LIBICC_CallbackCardRemove(unsigned char ucReader);
//void LIBICC_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType);

    /* icccmd.c */
    unsigned char ICCCMD_Detect(void);
    unsigned char ICCCMD_Select(void);
    unsigned char ICCCMD_Insert(unsigned char ucReader);
    unsigned char ICCCMD_Remove(void);
    unsigned char ICCCMD_PowerOff(void);
    unsigned char ICCCMD_WarmReset(void);
    unsigned char ICCCMD_ReadTimer(void);
    unsigned char ICCCMD_WriteTimer(void);
    unsigned char ICCCMD_ChangeProtocol(void);

    /* iso7816.c */
    unsigned char ISO7816_WarmReset(unsigned char ucReader,unsigned char ucRate);
    unsigned char ISO7816_AsyncReset(unsigned char ucReader,unsigned char ucRate);
    unsigned char ISO7816_SetAPDUNextCommand(unsigned char ucReader);
    void ISO7816_ResetRecvCharProc(unsigned char ucReader);
    void ISO7816_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    void ISO7816_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);

    /* atr.c */
    unsigned char ATR_ColdReset(unsigned char ucReader,unsigned char ucRate);
    unsigned char ATR_WarmReset(unsigned char ucReader,unsigned char ucRate);
    unsigned char ATR_Process(unsigned char ucReader);
    unsigned char ATR_FinishedProc(unsigned char ucReader);
    unsigned char MISC_GetCharBitNB(unsigned char ucValue,char ucBit);
    unsigned char ATR_CheckFinished(unsigned char *pucATR,unsigned short uiATRLen);
    void ATR_CallbackCardRemove(unsigned char ucReader);

    /* protocol.c */
    /* protocol.c */
#if 0
    unsigned char PROT_AsyncCommand(unsigned char ucReader,
                                    unsigned char *pucInData,
                                    unsigned short uiInLen);
#endif
    unsigned char PROT_CallbackT0SendChar(unsigned char ucReader);
    unsigned char PROT_AsyncCommand(unsigned char ucReader);
    unsigned char PROT_T0Command(unsigned char ucReader);
    unsigned char PROT_T0SendReceive(unsigned char ucReader);
    unsigned char PROT_T0RecvChar(unsigned char ucReader,unsigned char *pucCh);
    unsigned char PROT_T1Command(unsigned char ucReader);
    unsigned char PROT_T1SendReceiveFrame(unsigned char ucReader,unsigned char ucFrameType,unsigned char ucLastFrameType);
    unsigned char PROT_T1ReceiveFrame(unsigned char ucReader);
    unsigned char PROT_T0FinishedProc(unsigned char ucReader);
    unsigned char PROT_T1FinishedProc(unsigned char ucReader);
    unsigned char PROT_T1ComputeEDC(unsigned char ucMode,unsigned char *pucInData,unsigned short uiInLen);
    unsigned char PROT_T1PackIFrame(unsigned char ucReader);
    unsigned char PROT_T1PackRFrame(unsigned char ucReader,unsigned char ucSEQ,unsigned char ucError);
    unsigned char PROT_T1PackSFrameIFSReq(unsigned char ucReader);
    unsigned char PROT_T1PackSFrameSyncReply(unsigned char ucReader);
    unsigned char PROT_T1PackSFrameIFSReply(unsigned char ucReader);
    unsigned char PROT_T1PackSFrameBWTReply(unsigned char ucReader);
    unsigned char PROT_T1PackFrame(unsigned char ucReader,unsigned char ucPCB,
                                   unsigned char *pucInData,
                                   unsigned char ucInLen);
    void PROT_CallbackSendChar(unsigned char ucReader);
    void PROT_CallbackT1SendChar(unsigned char ucReader);
    void PROT_CallbackRecvChar(unsigned char ucReader,unsigned char ucCh);
    void PROT_CallbackT0RecvChar(unsigned char ucReader,unsigned char ucCh);
    void PROT_CallbackT1RecvChar(unsigned char ucReader,unsigned char ucCh);
    void PROT_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void PROT_CallbackT0Timeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void PROT_CallbackT1Timeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void PROT_CallbackCardRemove(unsigned char ucReader);
    void PROT_CallbackT0CardRemove(unsigned char ucReader);
    void PROT_CallbackT1CardRemove(unsigned char ucReader);
    void PROT_CallbackFault(unsigned char ucReader);
    void PROT_CallbackParityError(unsigned char ucReader);
    void PROT_CallbackT0ParityError(unsigned char ucReader);
    void PROT_CallbackT1ParityError(unsigned char ucReader);
    /* timer.c */
    unsigned char TIMER_Read();
    unsigned char TIMER_Step(unsigned char *ptTime);
    unsigned char TIMER_StepTimer(unsigned char *pTime);



#ifdef __cplusplus

#endif

#endif
