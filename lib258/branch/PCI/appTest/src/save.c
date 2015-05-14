#include <include.h>
#include <Global.h>
#include <xdata.h>



unsigned char SAV_CheckTransMaxNb(void)
{
	unsigned short uiIndex;

	for(uiIndex=0;uiIndex<TRANS_SAVEMAXNB;uiIndex++)
	{
		if( DataSave0.TransInfoData.auiTransIndex[uiIndex] == 0)
			break;
	}
	if( uiIndex == TRANS_SAVEMAXNB)
	{
		return(ERR_TRANSFILEFULL);
	}
	G_RUNDATA_uiTransIndex = uiIndex;
	return(SUCCESS);
}

unsigned char SAV_TransSave(void)
{
	 unsigned short uiNII;
	 unsigned short uiIndex;
	
	 
	 Os__clr_display(255);
	 Os__GB2312_display(1, 0, (uchar *) "保存交易中");
	  memcpy(&DataSave1.SaveTrans,&NormalTransData,sizeof(NORMALTRANS));
	  XDATA_Write_SaveTrans_File(G_RUNDATA_uiTransIndex);
	  DataSave0.TransInfoData.auiTransIndex[G_RUNDATA_uiTransIndex] = 0x55;
	  XDATA_Write_Vaild_File(DataSaveChange);
		
	return SUCCESS;
}


