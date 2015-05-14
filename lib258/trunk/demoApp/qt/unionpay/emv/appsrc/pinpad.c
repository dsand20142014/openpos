/*
	SAND China
--------------------------------------------------------------------------
	FILE  pinpad.c									(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file for pinpad interface.

    List of routines in file :

    File history :

*/

#include <include.h>
#include <global.h>
#include <xdata.h>

unsigned char PINPAD_0D_InitScraCode(unsigned char *pucInData)
{
	DrvIn.type      = 0x0d;
	DrvIn.length	= 16;

	hex_asc(&DrvIn.xxdata[0], pucInData, 16);

	return(SendReceivePinpad());
}

unsigned char PINPAD_30_BackLight(unsigned char ucBackLight)
{
	DrvIn.type      = 0x30;
	DrvIn.length	= 1;

	if( ucBackLight )
	{
		DrvIn.xxdata[0] = 0x31;
	}else
	{
		DrvIn.xxdata[0] = 0x30;
	}
	return(SendReceivePinpad());
}

unsigned char PINPAD_42_LoadSingleKeyUseSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucDecryptKeyIndex,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData)
{
	DrvIn.type      = 0x42;
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
	return(ERR_DRIVER);
}
/*************系统DES加密*****************/
unsigned char PINPAD_47_Encrypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
	DrvIn.type      = 0x47;
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
	return(ERR_DRIVER);
}

/*************系统DES解密*****************/
unsigned char PINPAD_47_Crypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
	DrvIn.type      = 0x3F;
	DrvIn.length	= 18;
	DrvIn.xxdata[0] = ucKeyArray+0x30;
	DrvIn.xxdata[1] = ucKeyIndex+0x30;

	hex_asc(&DrvIn.xxdata[2],pucInData, 16);
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			asc_hex(pucOutData,8,&DrvOut.xxdata[1],16);
/*
#ifdef TEST
	util_Printf("DrvOut.xxdata=%s\n",DrvOut.xxdata);
#endif
*/
			return(SUCCESS);
		}
	}
	return(ERR_DRIVER);
}

unsigned char PINPAD_4E_PINInput(
					unsigned char ucRow,unsigned char ucCol,
					unsigned char ucMin,unsigned char ucMax,
					unsigned char ucDispChar,
					unsigned char ucKeyArray,unsigned char ucKeyIndex,
					unsigned char *pucOutKeyNb,
					unsigned char *pucOutData)
{
	DrvIn.type      = 0x4e;
	DrvIn.length	= 9;
	DrvIn.xxdata[0] = ucRow+0x30;
	DrvIn.xxdata[1] = ucCol+0x30;
	DrvIn.xxdata[2] = ucMin+0x30;
	DrvIn.xxdata[3] = ucMax+0x30;
	DrvIn.xxdata[4] = 0x30;
	DrvIn.xxdata[5] = ucKeyArray+0x30;
	DrvIn.xxdata[6] = 0x30;
	DrvIn.xxdata[7] = ucDispChar;
	DrvIn.xxdata[8] = ucKeyIndex+0x30;

	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			//xxdata[1] is number of entered keys
			*pucOutKeyNb = DrvOut.xxdata[1] - '0';
			asc_hex(pucOutData,8,&DrvOut.xxdata[2],16);
			return(SUCCESS);
		}
	}
	return(ERR_DRIVER);
}


unsigned char SendReceivePinpad(void)
{
#ifdef PINPAD
	util_Printf("\nSendReceivePinpad[Pad]");
	util_Printf("DataSave0.ConstantParamData.ucKEYMODE =%02x\n",DataSave0.ConstantParamData.ucKEYMODE);
	if(DataSave0.ConstantParamData.ucKEYMODE ==0x31)
	{
		Drv.drv_nr = drv_pad;
	}
	else if(DataSave0.ConstantParamData.ucKEYMODE ==0x30)
	{
		Drv.drv_nr = drv_mmi;
	}
#else
	util_Printf("\nSendReceivePinpad[mmi]n");
	Drv.drv_nr = drv_mmi;
#endif

	Drv.type = execute;

	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	Os__call_drv(&Drv);
	do{
	}while( DrvOut.gen_status == DRV_RUNNING );

	util_Printf("\n DrvOut.gen_status=%02x\n",DrvOut.gen_status);

	if(  ( DrvOut.gen_status == DRV_ABSENT )
       ||( ( DrvOut.gen_status == DRV_ENDED )
          &&( DrvOut.drv_status == DRV_ABSENT )
         )
      )
	{
		util_Printf("DrvOut.drv_status=%02x\n",DrvOut.drv_status);
		return(ERR_DRIVER);
	}else
	{
		return(SUCCESS);
	}
}
unsigned char PP_In_Store_Key(
					unsigned char ucRow,
					unsigned char ucCol,
					unsigned char ucKeyIndex,
					unsigned char *aucBuf)

{
    unsigned ucResult;

    ucResult = SUCCESS;

	DrvIn.type = 0x44;
	DrvIn.length = 35;

    DrvIn.xxdata[0]  = (ucRow     + 0x30);
    DrvIn.xxdata[1]  = (ucCol     + 0x30);
    DrvIn.xxdata[2]  = (ucKeyIndex   + 0x30);

	bcd_asc(&DrvIn.xxdata[3], aucBuf, (DrvIn.length-3));

	if( SendReceivePinpad() == SUCCESS)
    {
		if(DrvIn.xxdata[0])
	    	return(DrvIn.xxdata[1]);
    }
    return(ucResult);
}