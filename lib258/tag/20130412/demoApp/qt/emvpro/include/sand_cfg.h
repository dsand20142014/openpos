/*******************************************************************************
* PS4000 配置参数定义
*
* Note:
*			主要参考PS400的oscfg.h
********************************************************************************/

#ifndef __OSCFG_H_
#define __OSCFG_H_

#ifndef OSCFGDATA
#define OSCFGDATA extern
#endif



#ifdef __cplusplus
extern "C" {
#endif




//磁卡配置参数
typedef struct
{
	unsigned char ucMagDebug;
	unsigned char ucMagVer[5+1]; //格式：mm.nn 最多5个字节
}OSMAGCFG;
OSCFGDATA OSMAGCFG OsMAGCfg;


//IC卡配置参数
typedef struct
{
	unsigned char ucICDebug;
	unsigned char ucICVer[5+1]; //格式：mm.nn 最多5个字节
}OSICCFG;
OSCFGDATA OSICCFG OsICCfg;


#ifdef __cplusplus
}
#endif

#endif
