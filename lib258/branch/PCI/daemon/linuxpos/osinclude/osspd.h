/*********************
create by liuyifeng	2010/5/14 11:50:55
**********************/
#ifndef __OS_SPD_H
#define __OS_SPD_H

#define SPD_FLASHSIZE_MAX		10485760		//下装文件总大小1M,包含down和app

#define SPD_TLV		          20		//TLV的属性个数

#define SPD_SUCCESS		   		 0		
#define SPD_OPENFAILED   		-1		//串口打开失败
#define SPD_CANCEL		   		-2		//取消
#define SPD_CONTINUE	  		-3		//继续
#define SPD_FILEERR		  		-5		//文件不符合大小
#define SPD_LOADCONFIG		  -6		//配置文件读取失败
#define SPD_ERRFILE				  -7		//文件错误
#define SPD_BADCRC				  -8		//crc校验错
#define SPD_OPENFILE			  -9		//程序文件打开失败
#define SPD_WRITEFILE			  -10		//程序文件写失败
#define SPD_ZIP						  -11		//解压失败
#define SPD_RELIST					-12		//排序错误

//文件路径
#define SPD_LIB_PATH					"/lib/"
#define SPD_DRIVER_PATH				"/driver/"
#define SPD_FONT_PATH					"/fonts/"


#define SPD_LINUXPOS_INFO_FILE	SPD_CONFIG_PATH"linuxpos.dat"
#define SPD_APP_INFO_FILE     	SPD_CONFIG_PATH"appinfo.dat"
#define SPD_FONT_INFO_FILE			SPD_CONFIG_PATH"font.dat"
#define SPD_LIB_INFO_FILE				SPD_CONFIG_PATH"lib.dat"
#define SPD_DRIVER_INFO_FILE		SPD_CONFIG_PATH"driver.dat"
#define SPD_CONFIG							SPD_CONFIG_PATH"spd.dat"

void OSSPD_tool_reboot(void);
void OSSPD_tool_disBufSize(void);
int OSSPD_transmit(void);
void OSSPD_file_delete(void);
void OSSPD_file_formatFileSys(void);
void OSSPD_setting(void);
int OSSPD_file_readDWNDirect(void);
unsigned char OSSPD_file_set(void);

#endif

