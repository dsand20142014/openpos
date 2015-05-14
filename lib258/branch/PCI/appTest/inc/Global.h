
#ifndef		_GLOBAL_H
#define		_GLOBAL_H

#ifndef DATAGLO
#define DATAGLO extern
#endif

#include	<include.h>


DATAGLO DRV_IN  	DrvIn;
DATAGLO DRV_OUT	DrvOut;
DATAGLO DRIVER  Drv;



DATAGLO NORMALTRANS	NormalTransData;
DATAGLO RUNDATA		RunData;
DATAGLO ISO8583			ISO8583Data;
DATAGLO EXTRATRANS 		ExtraTransData;


#define G_NORMALTRANS_ucTransType    			NormalTransData.ucTransType
#define G_NORMALTRANS_ulTraceNumber  			NormalTransData.ulTraceNumber
#define G_NORMALTRANS_aucISO2  				NormalTransData.aucISO2
#define G_NORMALTRANS_ucISO2Len  				NormalTransData.ucISO2Len
#define G_NORMALTRANS_ucDispSourceAcc   		NormalTransData.ucDispSourceAcc
#define G_NORMALTRANS_ucDispSourceAccLen 		NormalTransData.ucDispSourceAccLen
#define G_NORMALTRANS_aucISO3  				NormalTransData.aucISO3
#define G_NORMALTRANS_ucISO3Len  				NormalTransData.ucISO3Len
#define G_NORMALTRANS_aucCardPan  			NormalTransData.aucCardPan
#define G_NORMALTRANS_ucSourceAccType		NormalTransData.ucSourceAccType
#define G_NORMALTRANS_ucExpiredDate  			NormalTransData.ucExpiredDate
#define G_NORMALTRANS_ulAmount       			NormalTransData.ulAmount
#define G_NORMALTRANS_aucDate                	   	NormalTransData.aucDate
#define G_NORMALTRANS_aucTime					NormalTransData.aucTime
#define G_NORMALTRANS_aucSourceAcc   			NormalTransData.aucSourceAcc
#define G_NORMALTRANS_ucSourceAccLen 			NormalTransData.ucSourceAccLen

#define G_NORMALTRANS_aucAuthCode    			NormalTransData.aucAuthCode
#define G_NORMALTRANS_aucBankID    				NormalTransData.aucBankID
#define G_NORMALTRANS_aucRefNum      				NormalTransData.aucRefNum

#define G_EXTRATRANS_ucISO2Status										ExtraTransData.ucISO2Status
#define G_EXTRATRANS_uiISO2Len                  		ExtraTransData.uiISO2Len
#define G_EXTRATRANS_aucISO2                    		ExtraTransData.aucISO2
#define G_EXTRATRANS_ucISO3Status               		ExtraTransData.ucISO3Status
#define G_EXTRATRANS_uiISO3Len                  		ExtraTransData.uiISO3Len
#define G_EXTRATRANS_aucISO3                    		ExtraTransData.aucISO3
#define G_EXTRATRANS_ucInputPINLen              		ExtraTransData.ucInputPINLen
#define G_EXTRATRANS_aucPINData                 		ExtraTransData.aucPINData


#define G_RUNDATA_uiTransIndex					RunData.uiTransIndex
#define G_RUNDATA_aucTerminalID				RunData.aucTerminalID
#define G_RUNDATA_aucMerchantID				RunData.aucMerchantID
#define G_RUNDATA_ucErrorExtCode				RunData.ucErrorExtCode


#define FONT									DataSave0.ConstantParamData.font
#define PRINTTIMES							DataSave0.ConstantParamData.printtimes




#endif




