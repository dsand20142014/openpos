#include <include.h>
#include <Global.h>
#include <xdata.h>


void OnEve_power_on(void)
{
       unsigned char ucResult;

	ucResult = SUCCESS;

	ucResult = CFG_CheckConstantParamData();

	if(!ucResult)
		ucResult = CFG_CheckChangeParamData();

	if(!ucResult)
		ucResult = CFG_CheckSaveTrans();
	if(!ucResult)
		ucResult = CFG_CheckTransInfoData();
	
}

