#ifndef _RS232YTH_H
#define _RS232YTH_H

//#include <oslib.h>
//#include <typelib.h>

//#define     IDENTIFER       "M1470323"//畅购卡
//#define     IDENTIFER       "M1570526"  // 中国农行
#define     IDENTIFER       "B0000526"  // 中国银联


#define     RSLEN_LEN				2
#define     IDENTIFER_LEN    		8
#define		OBLIGATE_LEN 		 	5
#define 	VER_LEN					3
#define 	OPERTYPE_LEN			2
#define		TRANSTYPE_LEN			2
#define 	CARDTYPE_LEN			2
#define 	CASHREGNO_LEN			6
#define 	CASHIER_LEN				6
#define     SETTLEAMOUNT_LEN		12
#define 	CASHIERTRACE_LEN		6 
#define 	OLDTICKETNUMBER_LEN		6
#define 	RESERVERD_LEN 			48
#define 	RESPCODE_LEN 			2
#define 	RESPINFO_LEN 			40
#define 	MERCHID_LEN 			15
#define 	MERCHANTNAME_LEN 		40
#define 	TERMID_LEN 				8
#define 	CARDID_LEN  			19
#define 	VALIDDATE 				4
#define 	BANKTYPE_LEN 			6
#define 	TRANSDATE_LEN 			8
#define 	TRANSTIME_LEN 			6
#define 	AUTHCODE_LEN  			6
#define 	SYSREFNO_LEN  			12
#define 	TRACENO_LEN 			6
#define 	ORGTRACENO_LEN 			6
#define     BATCHNUM_LEN            6

#define     ERR_RESULT            0x01
#define     SUCCESS_RESULT        0



typedef struct
{
	 unsigned char    aucIdentifer[IDENTIFER_LEN];  //应用标识符
	 unsigned char    aucObligate[OBLIGATE_LEN];   //处理码（主副应用之间约定码）
	 unsigned char    aucVer[VER_LEN];  //版本号
}HEADDATA;//16  消息头


typedef struct
{
	unsigned char	aucOperType[OPERTYPE_LEN]; //操作类型
	unsigned char	aucTransType[TRANSTYPE_LEN];//交易代码
	unsigned char aucCardType[CARDTYPE_LEN];//卡类型
	unsigned char	aucCashRegNo[CASHREGNO_LEN];		/*收银机编号*/
	unsigned char	aucCashier[CASHIER_LEN];					/*柜员号(查询时为流水号)*/
	unsigned char	aucSettleAmount[SETTLEAMOUNT_LEN];				/*交易金额*/
	unsigned char aucCashierTrace[CASHIERTRACE_LEN];				/*收银机流水号*/
	unsigned char	aucOldTicketNumber[OLDTICKETNUMBER_LEN];		/*撤消收银机流水号*/
	unsigned char	aucReserved[RESERVERD_LEN];				/*保留字段*/
}RECEIVETRANS;//66


typedef struct
{
	unsigned char	aucOperType[OPERTYPE_LEN];
	unsigned char	aucTransType[TRANSTYPE_LEN];
	unsigned char aucCardType[CARDTYPE_LEN];
	unsigned char	aucRespCode[RESPCODE_LEN];				/*交易返回码*/
	unsigned char	aucRespInfo[RESPINFO_LEN];				/*交易返回信息*/
	unsigned char	aucCashRegNo[CASHREGNO_LEN];			/*收银机编号*/
	unsigned char	aucCashier[CASHIER_LEN];				/*柜员号*/
	unsigned char aucSettleAmount[SETTLEAMOUNT_LEN];
	unsigned char	aucBatchNum[BATCHNUM_LEN];				/*结算批次号*/
	unsigned char	aucMerchID[MERCHID_LEN];				/*商户号*/
	unsigned char	aucMerchantName[MERCHANTNAME_LEN];		/*商户名称*/
	unsigned char	aucTermID[TERMID_LEN];				/*终端号*/
	unsigned char	aucCardId[CARDID_LEN];				/*卡号*/
	unsigned char	aucValidDate[VALIDDATE];			/*卡有效期*/	
	unsigned char	aucBankType[BANKTYPE_LEN];				/*发卡行编码*/
	unsigned char	aucTransDate[TRANSDATE_LEN];			/*交易日期*/
	unsigned char	aucTransTime[TRANSTIME_LEN];			/*交易时间*/
	unsigned char	aucAuthCode[AUTHCODE_LEN];				/*授权号*/
	unsigned char	aucSysRefNo[SYSREFNO_LEN];			/*系统参照号*/
	unsigned char aucCashTraceNo[CASHIERTRACE_LEN];			/*收银机流水号*/
	unsigned char	aucOrgCashTraceNo[OLDTICKETNUMBER_LEN];		/*原收银机流水号*/
	unsigned char	aucTraceNo[TRACENO_LEN];				/*系统流水号*/
	unsigned char	aucOrgTraceNo[ORGTRACENO_LEN];			/*原系统流水号*/
	unsigned char aucReserved[RESERVERD_LEN];       //保留字段
}SENDTRANS;//251




unsigned char Rs232yth_UnPackPortData(unsigned char* aucInData,RECEIVETRANS *ReceiveData,unsigned char *aucIdentifer);
unsigned char Rs232yth_PackPortData(SENDTRANS *SendTransData,unsigned char *aucOutData,unsigned char ucFlag);



#endif 