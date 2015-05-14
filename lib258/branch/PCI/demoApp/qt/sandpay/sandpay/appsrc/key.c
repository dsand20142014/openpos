#include <include.h>
#include <global.h>
#include <xdata.h>


unsigned char KEY_DownloadKeysFromCpuCard(void)
{
	unsigned char ucResult=SUCCESS;
	
#if 1
	ucResult=LOADKEY_LoadWeiKangTransKey(0xab,KEYARRAY_PURSECARD0,KEYINDEX_PURSECARD_PROTECTKEY,20);
	
#else	
	RunData.ucComType=2;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"ȴ");	
	ucResult=DownProtectKey(KEYARRAY_PURSECARD0,KEYINDEX_PURSECARD_PROTECTKEY,20);
#endif
#ifdef TEST
	Uart_Printf("ucResult of Load trans key is %02x \n",ucResult);
#endif
	//chenzx 2009-10-18 05:25
	//Ϊ0δ˳ýʱѡͨѶĹܣͨѶģʽѡ
	//³ȻRS232ʽͨѶ
	RunData.ucComType = 0;
	OSUART_Close2();
	//Os__end_com3();
	return ucResult;
}




unsigned char KEY_StoreNewKey(unsigned char *aucKeyData)
{	
	unsigned char ucResult=SUCCESS;
	unsigned char aucKey[8];
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"Կ...");	
	//pin key	
	memset(aucKey,0,sizeof(aucKey));
	memcpy(aucKey,aucKeyData,8);
	ucResult=PINPAD_47_Crypt8ByteSingleKey(KEYARRAY_PURSECARD2,KEYINDEX_PURSECARD_MASTERKEY,
											aucKey,aucKey);
	if(ucResult!=SUCCESS)
		return ucResult;
	ucResult=PINPAD_47_Encrypt8ByteSingleKey(KEYARRAY_PURSECARD2,KEYINDEX_PURSECARD_MASTERKEY+1,
											aucKey,aucKey);
	if(ucResult!=SUCCESS)
		return ucResult;	
	ucResult=PINPAD_42_LoadSingleKeyUseSingleKey(KEYARRAY_PURSECARD2,KEYINDEX_PURSECARD_MASTERKEY,
										KEYINDEX_PURSECARD2_PINKEY,aucKey);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(DataSave0.ConstantParamData.PinKey,0,sizeof(DataSave0.ConstantParamData.PinKey));
	
	desm1(aucKey,aucKey,DataSave0.ConstantParamData.MacMasterUseKey[0]);
	des(aucKey,aucKey,DataSave0.ConstantParamData.MacMasterUseKey[1]);
	desm1(aucKey,DataSave0.ConstantParamData.PinKey,DataSave0.ConstantParamData.MacMasterUseKey[0]);


Uart_Printf("ǩõԱPINKEY:\n");
{
int i;
for(i=0;i<8;i++)
Uart_Printf("%02x \n",DataSave0.ConstantParamData.PinKey[i]);
Uart_Printf("\n");
}

	if(ucResult!=SUCCESS)
		return ucResult;
		
	//MAC KEY
	memset(aucKey,0,sizeof(aucKey));
	memcpy(aucKey,aucKeyData+8,8);

	{
	int i;
	Uart_Printf("\n**********ܺMAC**********\n");

	for(i=0;i<8;i++)
	{
	Uart_Printf("%02x ",aucKey[i]);
	}
	Uart_Printf("\n");
	}
	desm1(aucKey,aucKey,DataSave0.ConstantParamData.MacMasterUseKey[0]);
	des(aucKey,aucKey,DataSave0.ConstantParamData.MacMasterUseKey[1]);
	desm1(aucKey,DataSave0.ConstantParamData.MacKey,DataSave0.ConstantParamData.MacMasterUseKey[0]);

	ucResult=XDATA_Write_Vaild_File(DataSaveConstant);

	return ucResult;	
}
unsigned char  LoadKey(unsigned char *inbuf,unsigned char keyarry,unsigned char enkeyindex,unsigned char dekeyindex)
{
	unsigned char aucBuf[16];
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"Կ...");	
	PINPAD_47_Crypt8ByteSingleKey(keyarry, enkeyindex, inbuf, aucBuf);
	PINPAD_47_Encrypt8ByteSingleKey(keyarry, enkeyindex+1, aucBuf, aucBuf);
	PINPAD_42_LoadSingleKeyUseSingleKey(keyarry, enkeyindex, dekeyindex, aucBuf);
	return SUCCESS;
}
unsigned char CheckKeyVeriCode(unsigned char *inbuf,unsigned char keyarry,unsigned char keyindex)
{
	unsigned char aucBuf[16];
	memset(aucBuf,0,sizeof(aucBuf));
	PINPAD_47_Encrypt8ByteSingleKey(keyarry, keyindex, aucBuf, aucBuf);
	PINPAD_47_Crypt8ByteSingleKey(keyarry, keyindex+1, aucBuf, aucBuf);
	PINPAD_47_Encrypt8ByteSingleKey(keyarry, keyindex, aucBuf, aucBuf);
	if(memcmp(aucBuf,inbuf,2))
		return ERR_CODE;
	return SUCCESS;
}

