
#ifndef		_GLOBAL_H
#define		_GLOBAL_H

#ifndef DATAGLO
#define DATAGLO extern
#endif

#include	<EMV41.h>
#include	<include.h>
#include	<interfaceparam.h>
#include <semaphore.h>
#if SANDREADER
#include    <QPBOC.h>
#endif
#ifdef  TURBOC
extern DRV_IN		drvin_s;
extern DRV_OUT		drvout_s;
#define DrvIn		drvin_s
#define DrvOut		drvout_s
#else
DATAGLO DRV_IN  	DrvIn;
DATAGLO DRV_OUT	DrvOut;
#endif
DATAGLO DRIVER  Drv;


DATAGLO ISO7816IN			Iso7816In;
DATAGLO ISO7816OUT		Iso7816Out;
DATAGLO CARDAPP				CardApp;
DATAGLO NORMALTRANS	NormalTransData;
DATAGLO EXTRATRANS 		ExtraTransData;
DATAGLO SLE4442DATA 		SleTransData;
DATAGLO RUNDATA			RunData;
DATAGLO ISO8583				ISO8583Data;
DATAGLO ISO8583MBFE		ISO8583_MBFE;

DATAGLO TEMPFILE_DATA     xDATA_TempFileData;

DATAGLO DIAL_PARAM		DialParam;
DATAGLO unsigned char 	ucTestGroup;
DATAGLO unsigned int ucBatchSandTransNum;
DATAGLO	VISAPK		 	TermVISAPK[MAXTERMCAPKNUMS];//读卡器的公钥数据(非接卡和POS公钥结构体不一样)
DATAGLO	INTERFACESUPPORTAPP	TermUntouchAID[MAXTERMAIDNUMS]; //读卡器的AID数据(非接卡和POS的AID结构体不一样)
//--
DATAGLO PROUIINTERFACE ProUiFace;
DATAGLO unsigned char ProgressFlag;
DATAGLO unsigned char g_ThreadFlag;
DATAGLO unsigned char g_ProgressBarFlag;
DATAGLO unsigned char jiaoyiCnt;

DATAGLO sem_t binSem1;
DATAGLO sem_t binSem2;

#define G_NORMALTRANS_ulStages								NormalTransData.ulStages
#define G_NORMALTRANS_ucStagesType						NormalTransData.ucStagesType
#define G_NORMALTRANS_ulFCurrency							NormalTransData.ulFCurrency
#define G_NORMALTRANS_ulNCurrency						NormalTransData.ulNCurrency

#define G_NORMALTRANS_ulRepayAmount						NormalTransData.ulRepayAmount
#define G_NORMALTRANS_ulRepayFeeAmt						NormalTransData.ulRepayFeeAmt
#define G_NORMALTRANS_aucCBuf								NormalTransData.aucCBuf

#define G_NORMALTRANS_bInstallments						NormalTransData.bInstallments
#define G_NORMALTRANS_aucNumProject					NormalTransData.aucProjectNum
#define G_NORMALTRANS_aucNumStages					NormalTransData.aucNumStages

#define G_NORMALTRANS_EMVTransReqInfo				NormalTransData.TransReqInfo
#define G_NORMALTRANS_ucPTicketFlag                      NormalTransData.ucPTicket
#define G_NORMALTRANS_ucCardTableIndex				NormalTransData.ucCardTableIndex
#define G_NORMALTRANS_euCardSpecies             		NormalTransData.euCardSpecies
#define G_NORMALTRANS_ucPANSeq                  			NormalTransData.ucPANSeq
#define G_NORMALTRANS_aucUntouchSeq					NormalTransData.aucUntouchSeq

#define G_NORMALTRANS_ucPANSeqExist                  	NormalTransData.ucPANSeqExist
#define G_NORMALTRANS_aucAID                     				NormalTransData.aucAID
#define G_NORMALTRANS_ucAIDLen                 			NormalTransData.ucAIDLen
#define G_NORMALTRANS_AppCrypt                 				NormalTransData.AppCrypt
#define G_NORMALTRANS_TVR										NormalTransData.TVR
#define G_NORMALTRANS_TSI											NormalTransData.TSI
#define G_NORMALTRNAS_ATC						NormalTransData.ATC
#define G_NORMALTRNAS_CVM						NormalTransData.CVM
#define G_NORMALTRANS_ucAppLabelLen			NormalTransData.ucAppLabelLen
#define G_NORMALTRANS_aucAppLabel				NormalTransData.aucAppLabel
#define G_NORMALTRANS_ucAppPreferNameLen    	NormalTransData.ucAppPreferNameLen
#define G_NORMALTRANS_aucAppPreferName      	NormalTransData.aucAppPreferName
#define G_NORMALTRANS_ucCardType				NormalTransData.ucCardType
#define G_NORMALTRANS_aucOldDate                		NormalTransData.aucOldDate
#define G_NORMALTRANS_aucDate                	   	NormalTransData.aucDate
#define G_NORMALTRANS_aucTime					NormalTransData.aucTime
#define G_NORMALTRANS_ucSourceAccType		NormalTransData.ucSourceAccType
#define G_NORMALTRANS_aucSourceAcc   			NormalTransData.aucSourceAcc
#define G_NORMALTRANS_ucSourceAccLen 			NormalTransData.ucSourceAccLen
#define G_NORMALTRANS_aucAuthGroup   			NormalTransData.aucAuthGroup
#define G_NORMALTRANS_aucCardRemark   		NormalTransData.aucCardRemark
#define G_NORMALTRANS_ucCardRemarkFlag 		NormalTransData.ucCardRemarkFlag
#define G_NORMALTRANS_ucTIPAdjustFlag 			NormalTransData.ucTIPAdjustFlag
#define G_NORMALTRANS_ucOldAuthType 			NormalTransData.ucOldAuthType
#define G_NORMALTRANS_ucTransType    			NormalTransData.ucTransType
#define G_NORMALTRANS_ucOldTransType 			NormalTransData.ucOldTransType
#define G_NORMALTRANS_ucOldOldTransType 		NormalTransData.ucOldOldTransType
#define G_NORMALTRANS_ucVoidTrans				NormalTransData.ucTIPCode

#define G_NORMALTRANS_ucInputMode    			NormalTransData.ucInputMode
#define G_NORMALTRANS_ucExpiredDate  			NormalTransData.ucExpiredDate
#define G_NORMALTRANS_ucBatchSendExpiredDate	NormalTransData.ucBatchSendExpiredDate
#define G_NORMALTRANS_aucSettleDate  			NormalTransData.aucSettleDate
#define G_NORMALTRANS_aucISO2  				NormalTransData.aucISO2
#define G_NORMALTRANS_ucISO2Len  				NormalTransData.ucISO2Len
#define G_NORMALTRANS_ucDispSourceAcc   		NormalTransData.ucDispSourceAcc
#define G_NORMALTRANS_ucDispSourceAccLen 		NormalTransData.ucDispSourceAccLen
#define G_NORMALTRANS_aucISO3  				NormalTransData.aucISO3
#define G_NORMALTRANS_ucISO3Len  				NormalTransData.ucISO3Len
#define G_NORMALTRANS_aucCardPan  			NormalTransData.aucCardPan
#define G_NORMALTRANS_aucCardPan_UnAssign  			NormalTransData.aucCardPan_UnAssign
#define G_NORMALTRANS_ucNotSendOkFlag 		NormalTransData.ucNotSendOkFlag
#define G_NORMALTRANS_ulOldAmount       			NormalTransData.ulOldAmount
#define G_NORMALTRANS_ulAmount       			NormalTransData.ulAmount
#define G_NORMALTRANS_ulAuthAddTotalAmount	NormalTransData.ulAuthAddTotalAmount
#define G_NORMALTRANS_ulPrevEDBalance			NormalTransData.ulPrevEDBalance
#define G_NORMALTRANS_ulPrevEPBalance			NormalTransData.ulPrevEPBalance
#define G_NORMALTRANS_ulFeeAmount    			NormalTransData.ulFeeAmount
#define G_NORMALTRANS_ulTicketNumber 			NormalTransData.ulTicketNumber
#define G_NORMALTRANS_ulTraceNumber  			NormalTransData.ulTraceNumber
#define G_NORMALTRANS_aucAuthCode    			NormalTransData.aucAuthCode
#define G_NORMALTRANS_aaucBatchSendAuthCode  NormalTransData.aucBatchSendAuthCode
#define G_NORMALTRANS_aucOldAuthCode    		NormalTransData.aucOldAuthCode
#define G_NORMALTRANS_aucBankID    				NormalTransData.aucBankID
#define G_NORMALTRANS_aucAuthUintCode    		NormalTransData.aucAuthUintCode
#define G_NORMALTRANS_aucCashierNo   			NormalTransData.aucCashierNo
#define G_NORMALTRANS_aucRefNum      				NormalTransData.aucRefNum
#define G_NORMALTRANS_ulOldTraceNumber      	NormalTransData.ulOldTraceNumber

#define G_NORMALTRANS_aucOldOrdersNo      		NormalTransData.aucOldOrdersNo

#define G_NORMALTRANS_uiOldOldTransIndex		NormalTransData.uiOldTraceIndex
#define G_NORMALTRANS_ulOldOldTraceNumber    NormalTransData.ulOldOldTraceNumber

#define G_NORMALTRANS_aucOrdersNo 			NormalTransData.aucOrdersNo
#define G_NORMALTRANS_aucOldRefNumber  	NormalTransData.aucOldRefNumber
#define G_NORMALTRANS_aucComm        			NormalTransData.aucComm
#define G_NORMALTRANS_aucIssuerID				NormalTransData.aucComm
#define G_NORMALTRANS_aucAcquereID			&NormalTransData.aucComm[11]
#define G_NORMALTRANS_ucPSAMKeyIndex			NormalTransData.aucComm[22]
#define G_NORMALTRANS_ucCardKeyIndex			NormalTransData.aucComm[23]
#define G_NORMALTRANS_aucOfflineCount			&NormalTransData.aucComm[24]
#define G_NORMALTRANS_aucPrevOverDraw		&NormalTransData.aucComm[26]
#define G_NORMALTRANS_aucRandom				&NormalTransData.aucComm[29]
#define G_NORMALTRANS_aucOffTicket				&NormalTransData.aucComm[33]
#define G_NORMALTRANS_aucTAC					&NormalTransData.aucComm[37]
#define G_NORMALTRANS_ucFallBackFlag             	NormalTransData.ucFallBackFlag
#define G_NORMALTRANS_ucICEPKeyVer             	NormalTransData.ucICEPKeyVer
#define G_NORMALTRANS_ucICEPMatchFlag             	NormalTransData.ucICEPMatchFlag
#define G_NORMALTRANS_ucMac1			             	NormalTransData.ucMac1
#define G_NORMALTRANS_ucField55Data					NormalTransData.ucField55Data
#define G_NORMALTRANS_uiField55Len		       			NormalTransData.uiField55Len
#define G_NORMALTRANS_ulTrueAmount     				NormalTransData.ulTrueAmount
#define G_NORMALTRANS_ulOldTrueAmount     		NormalTransData.ulOldTrueAmount

#define G_NORMALTRANS_ucECTrans               NormalTransData.ucECTrans
#define G_NORMALTRANS_PECTransInfo            NormalTransData.PECTransInfo
#define G_NORMALTRANS_aucUnAssignAccountISO2  NormalTransData.aucUnAssignAccountISO2
#define G_NORMALTRANS_aucUnAssignAccountISO3  NormalTransData.aucUnAssignAccountISO3
#define G_NORMALTRANS_ucUAAISOLen2					  NormalTransData.ucUAAISOLen2
#define G_NORMALTRANS_ucUAAISOLen3					  NormalTransData.ucUAAISOLen3
#define G_NORMALTRANS_ucMainAccountLen        NormalTransData.ucMainAccountLen
#define G_NORMALTRANS_aucMobileNUM		NormalTransData.aucMobileNUM
#define G_NORMALTRANS_aucMobileCHK			NormalTransData.aucMobileCHK

#define G_EXTRATRANS_ucISO2Status										ExtraTransData.ucISO2Status
#define G_EXTRATRANS_uiISO2Len                  		ExtraTransData.uiISO2Len
#define G_EXTRATRANS_aucISO2                    		ExtraTransData.aucISO2
#define G_EXTRATRANS_ucISO3Status               		ExtraTransData.ucISO3Status
#define G_EXTRATRANS_uiISO3Len                  		ExtraTransData.uiISO3Len
#define G_EXTRATRANS_aucISO3                    		ExtraTransData.aucISO3
#define G_EXTRATRANS_ucInputPINLen              		ExtraTransData.ucInputPINLen
#define G_EXTRATRANS_aucPINData                 		ExtraTransData.aucPINData
#define G_EXTRATRANS_ucInputPINLen_UnAssign        	ExtraTransData.ucInputPINLen_UnAssign//非指定账户用
#define G_EXTRATRANS_aucPINData_UnAssign            ExtraTransData.aucPINData_UnAssign
#define G_EXTRATRANS_aucHostRespCode			ExtraTransData.aucHostRespCode
#define G_EXTRATRANS_aucSessionKey				ExtraTransData.aucSessionKey
#define G_EXTRATRANS_aucSessionKeyLen			ExtraTransData.ucSessionKeyLen
#define G_EXTRATRANS_aucReadCardTime			ExtraTransData.aucReadCardTime

#define G_EXTRATRANS_ulRefundBatchNumber	ExtraTransData.ulRefundBatchNumber

#define G_RUNDATA_ucReversalFlag    		RunData.ucReversalFlag
#define G_RUNDATA_ucOrderFlag             RunData.ucOrderFlag
#define G_RUNDATA_ucPTicketFlag            RunData.ucPTicketFlag
#define G_RUNDATA_aucMBFEBuf				RunData.aucMBFEBuf
#define G_RUNDATA_ucErrorFileCode				RunData.ucErrorFileCode
#define G_RUNDATA_ucErrorExtCode				RunData.ucErrorExtCode
#define G_RUNDATA_ucTerminalType				RunData.ucTerminalType
#define G_RUNDATA_ucDialRetry               			RunData.ucDialRetry
#define G_RUNDATA_ucHostConnectFlag			RunData.ucHostConnectFlag
#define G_RUNDATA_uiTransIndex					RunData.uiTransIndex
#define G_RUNDATA_uiOldTransIndex				RunData.uiOldTransIndex
#define G_RUNDATA_ulOldTraceNumber			RunData.ulOldTraceNumber
#define G_RUNDATA_ulOldFeeAmount				RunData.ulOldFeeAmount
#define G_RUNDATA_ucPINRetry					RunData.ucPINRetry
#define G_RUNDATA_aucTerminalID				RunData.aucTerminalID
#define G_RUNDATA_aucMerchantID				RunData.aucMerchantID

#define G_RUNDATA_aucVoidRefNum				RunData.aucVoidRefNum
#define G_RUNDATA_aucVoidAuthCode				RunData.aucVoidAuthCode
#define G_RUNDATA_aucOrignalDate				RunData.aucOrignalDate
#define G_RUNDATA_aucOriginalBatchNum			RunData.aucOriginalBatchNum
#define G_RUNDATA_aucOriginalTraceNum			RunData.aucOriginalTraceNum
#define G_RUNDATA_aucOriginalTicketNum			RunData.aucOriginalTicketNum
#define G_RUNDATA_PBOCBankData				RunData.PBOCBankData
#define G_RUNDATA_aucNewSuperPass			RunData.aucNewSuperPass
#define G_RUNDATA_aucOriginalTerminalId		RunData.aucOriginalTerminalId
#define G_RUNDATA_ucReprintFlag				RunData.ucReprintFlag
#define G_RUNDATA_ucTIPSaving					RunData.ucTIPSaving
#define G_RUNDATA_ucTransType					RunData.ucTransType
#define G_RUNDATA_ucAdjustFlag					RunData.ucAdjustAmoutFlag
#define G_RUNDATA_ucFiledErrNo					RunData.ucFiledErrNo
#define G_RUNDATA_ReadCardFlag				RunData.ReadCardFlag
#define G_RUNDATA_RevChack_ucProcessCode		RunData.ucProcessCode
#define G_RUNDATA_RevChack_ucAmount			RunData.ucAmount
#define G_RUNDATA_RevChak_ucTransNum		RunData.ucTransNum
#define G_RUNDATA_RevChack_ucTerminerID	RunData.ucTerminerID
#define G_RUNDATA_RevChack_ucMarchID		RunData.ucMarchID
#define G_RUNDATA_ucDefOperation			RunData.ucDefOperation
//#define G_RUNDATA_MEMBankData			RunData.MEMBankData
#define G_RUNDATA_ucEPReadCardFlag       	RunData.ucEPReadCardFlag// do not wait read card :1 ; wait read card :0
#define G_RUNDATA_ucDialFlag		           	RunData.ucDialFlag
#define G_RUNDATA_ucReadCardFlag 		RunData.ucReadCardFlag
#define G_RUNDATA_ucSettleFlag	           		RunData.ucSettleFlag // 1为平,0为不平
#define G_RUNDATA_ucPreDialFalg       		RunData.ucPreDialFalg
#define G_RUNDATA_aucOldCardPan      		RunData.aucOldCardPan
#define G_RUNDATA_aucAID					RunData.aucAID
#define G_RUNDATA_ucAIDLen                         RunData.ucAIDLen
#define G_RUNDATA_ucQInputPinFlag			RunData.ucQInputPinFlag
#define G_RUNDATA_aucOperaPan      			RunData.aucOperaBuf
#define G_RUNDATA_aucOperaPanKey   		RunData.aucOperaBufKey
#define G_RUNDATA_bConectFlag   				RunData.bConectFlag
#define G_MEMDATA_aucMarketNo					SleTransData.aucMarketNo
#define G_MEMDATA_aucSerial						SleTransData.aucSerial
#define G_MEMDATA_aucAccountNo					SleTransData.aucAccountNo
#define G_MEMDATA_aucBIN						SleTransData.aucBIN
#define G_MEMDATA_aucCardMap					SleTransData.aucCardMap
#define G_MEMDATA_aucKeyVer						SleTransData.aucKeyVer
#define G_MEMDATA_aucRFU						SleTransData.aucRFU
#define G_MEMDATA_aucOpenDate					SleTransData.aucOpenDate
#define G_MEMDATA_aucSaveDate					SleTransData.aucSaveDate
#define G_MEMDATA_Amount						SleTransData.Amount
#define G_MEMDATA_aucCRC						SleTransData.aucCRC

#define	G_RUNDATA_TransEnable					RunData.TransEnable
#define	I_Inquiry								G_RUNDATA_TransEnable.ucInquiry
#define	I_Auth									G_RUNDATA_TransEnable.ucAuth
#define	I_UnAuth								G_RUNDATA_TransEnable.ucUnAuth
#define	I_AuthFin								G_RUNDATA_TransEnable.ucAuthFin
#define	I_UnAuthFin								G_RUNDATA_TransEnable.ucUnAuthFin
#define	I_Sale									G_RUNDATA_TransEnable.ucSale
#define	I_UnSale								G_RUNDATA_TransEnable.ucUnSale
#define	I_Refund								G_RUNDATA_TransEnable.ucRefund
#define	I_Offline								G_RUNDATA_TransEnable.ucOffline
#define	I_TipFee								G_RUNDATA_TransEnable.ucTipFee
#define	I_AuthFinishOff					G_RUNDATA_TransEnable.ucAuthFinishOff
#define	I_ICScript								G_RUNDATA_TransEnable.ucICScript
#define	I_OffSale								G_RUNDATA_TransEnable.ucOffSale
#define	I_AddAuth							G_RUNDATA_TransEnable.ucAddAuth
#define	I_PurseDeposit						G_RUNDATA_TransEnable.ucPurseDeposit

#define I_UndoOff							DataSave0.ConstantParamData.ucUndoOff
#define Pinpad_flag							DataSave0.ConstantParamData.Pinpad_flag
#define FONT									DataSave0.ConstantParamData.font
#define LINENUM							DataSave0.ConstantParamData.linenum
#define PRINTTIMES							DataSave0.ConstantParamData.printtimes
#define XorTipFee								DataSave0.ConstantParamData.ucXorTipFee
#define ReaderType							DataSave0.ChangeParamData.ucReaderType
#define ReaderSupport						DataSave0.ChangeParamData.bReaderSupport

DATAGLO	TERMCONFIG	 	EMVConfig;
#if SANDREADER
DATAGLO       QTERMCONFIG	QEMVConfig;
DATAGLO	QTERMSUPPORTAPP QTemAID[MAXTERMAIDNUMS];
#endif
DATAGLO	UCHAR		 			ucTermCAPKNum;
DATAGLO	CAPK		 			TermCAPK[MAXTERMCAPKNUMS];
DATAGLO	UCHAR		 			ucTermAIDNum;
DATAGLO	TERMSUPPORTAPP	TermAID[MAXTERMAIDNUMS];

DATAGLO	UCHAR						ucIPKRevokeNum;
DATAGLO	IPKREVOKE				IPKRevoke[MAXIPKREVOKENUMS];
DATAGLO	UCHAR						ucExceptFileNum;
DATAGLO	EXCEPTPAN				ExceptFile[MAXEXCEPTFILENUMS];
DATAGLO	EMV_CONSTPARAM	ConstParam;
DATAGLO	TRANSREQINFO 			TransReqInfo;
DATAGLO	EMVAIDPARAM 			AIDParam[MAXTERMAIDNUMS];

/*=------------------------------------------------*/
#define TerminalHost							DataSave0.ConstantParamData.ucTerIPType	   //标志选择哪种终端主机IP
/*=------------------------------------------------*/




#endif
