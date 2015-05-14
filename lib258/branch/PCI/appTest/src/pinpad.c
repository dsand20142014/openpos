#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <encrypt.h>


unsigned char PINPAD_42_LoadSingleKeyUseSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucDecryptKeyIndex,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData)
{
       ENCRYPT_DRV_DATA  EncryptData;

	 memset(&EncryptData,0,sizeof(EncryptData));
	 EncryptData.keytype = ENCRYPT_SAVE_BYTRANSKEY;
	 EncryptData.cryptindex = ucKeyArray*256+ucDecryptKeyIndex;
	 EncryptData.saveindex = ucKeyArray*256+ucDestKeyIndex;
	 EncryptData.crypttype = ENCRYPT_DES_SINGLE_DECRYPT;
	 hex_asc(EncryptData.keyvalue,pucInData, 16);
	 EncryptData.cryptflag = ENCRYPT_FUNCTION_NORMAL;

	return  ENCRYPT_Decrypt_Load_Key(&EncryptData);
	 
	 
/*	DrvIn.type      = 0x42;
	DrvIn.length	= 19;
	DrvIn.xxdata[0] = ucKeyArray+0x30;
	DrvIn.xxdata[1] = ucDecryptKeyIndex+0x30;
	DrvIn.xxdata[2] = ucDestKeyIndex+0x30;

	hex_asc(&DrvIn.xxdata[3],pucInData, 16);
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			if( DrvOut.xxdata[0] == 0x31 )
				return(ERR_DRIVER);
			else
				return(SUCCESS);
		}
	}
	return(ERR_DRIVER);*/
}

unsigned char PINPAD_47_Encrypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
       ENCRYPT_DRV_DATA  EncryptData;
	unsigned char ucResult;

	 memset(&EncryptData,0,sizeof(EncryptData));
	 EncryptData.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;
	 EncryptData.cryptindex = ucKeyArray*256+ucKeyIndex;
	 EncryptData.crypttype = ENCRYPT_DES_SINGLE;
	 hex_asc(EncryptData.keyvalue,pucInData, 16);
	 EncryptData.cryptflag = ENCRYPT_FUNCTION_NORMAL;

	ucResult =  ENCRYPT_Decrypt_Load_Key(&EncryptData);

	if(!ucResult)
	{
	       asc_hex(pucOutData,8,EncryptData.outdata,16);
	}
	return ucResult;

	/*DrvIn.type      = 0x47;
	DrvIn.length	= 18;
	DrvIn.xxdata[0] = ucKeyArray+0x30;
	DrvIn.xxdata[1] = ucKeyIndex+0x30;

	hex_asc(&DrvIn.xxdata[2],pucInData, 16);
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			asc_hex(pucOutData,8,&DrvOut.xxdata[1],16);
			return(SUCCESS);
		}
	}
	return(ERR_DRIVER);*/
}

unsigned char PINPAD_47_Crypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
	/*DrvIn.type      = 0x3F;
	DrvIn.length	= 18;
	DrvIn.xxdata[0] = ucKeyArray+0x30;
	DrvIn.xxdata[1] = ucKeyIndex+0x30;

	hex_asc(&DrvIn.xxdata[2],pucInData, 16);
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			asc_hex(pucOutData,8,&DrvOut.xxdata[1],16);
			return(SUCCESS);
		}
	}
	return(ERR_DRIVER);*/
	 ENCRYPT_DRV_DATA  EncryptData;
	unsigned char ucResult;

	 memset(&EncryptData,0,sizeof(EncryptData));
	 EncryptData.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;
	 EncryptData.cryptindex = ucKeyArray*256+ucKeyIndex;
	 EncryptData.crypttype = ENCRYPT_DES_SINGLE_DECRYPT;
	 hex_asc(EncryptData.keyvalue,pucInData, 16);
	 EncryptData.cryptflag = ENCRYPT_FUNCTION_NORMAL;

	ucResult =  ENCRYPT_Decrypt_Load_Key(&EncryptData);

	if(!ucResult)
	{
	       asc_hex(pucOutData,8,EncryptData.outdata,16);
	}
	return ucResult;
}



#if 0
unsigned char SendReceivePinpad(void)
{
       Drv.drv_nr = drv_mmi;
	Drv.type = execute;

	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	Os__call_drv(&Drv);
	do{
	}while( DrvOut.gen_status == DRV_RUNNING );


	if(  ( DrvOut.gen_status == DRV_ABSENT )
       ||( ( DrvOut.gen_status == DRV_ENDED )
          &&( DrvOut.drv_status == DRV_ABSENT )
         )
      )
	{
		return(ERR_DRIVER);
	}else
	{
		return(SUCCESS);
	}
}
#endif
