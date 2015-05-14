/*********************
create by liuyifeng	2010/5/14 11:50:55
**********************/
#ifndef __OS_LOCALSERVICE_H
#define __OS_LOCALSERVICE_H

//文件路径
#define SPD_APP_PATH		"/app/"
#define SPD_APP_DATA		"/data"
#define SPD_LIB_PATH		"/lib/"
#define SPD_DRIVER_PATH		"/drivers/"
#define SPD_FONT_PATH		"/fonts/"
#define SPD_APPMANAGER_PATH	"/daemon/"
#define SPD_DOWN_PATH		"/download"
#define SPD_PLN_PATH		"/tmp/pln/"
#define SPD_USB_PATH		"/storage/usb/"
#define SPD_TF_PATH			"/storage/tf/"
#define SPD_BIDDY_PATH		"/biddy/"		//作为母pos时用到的下载存放路径
#define SPD_UBOOT_PATH		"/uboot/"		//只在下装目录有，实际目录没有
#define SPD_KERNEL_PATH		"/kernel/"		//只在下装目录有，实际目录没有

#define SPD_FILE_CONF		"conf.ini"		//配置文件名
#define SPD_FILE_DAEMONLST	"list.ini"		//daemon的配置文件名
#define SPD_FILE_DAEMONSET	"setting.ini"	//系统设置
#define SPD_FILE_DRVSH		"drivers.sh"	//驱动加载的配置文件
#define SPD_FILE_DAEMONSH	"daemon.sh"		//进程加载的配置文件
#define SPD_FILE_APP		"*.app"			//应用
#define SPD_FILE_SCRIPT		"*.qss"			//应用UI脚本文件
#define SPD_FILE_ICO		"app.*"			//应用图标文件
#define SPD_FILE_PNG		"/image"		//应用生成的图片文件
#define SPD_FILE_BOOT		"uboot.rsa"		//uboot文件
#define SPD_FILE_KERNEL		"uImage.rsa"	//kernel文件
//这里为了不和下载时候搞混，分开，下装是复用了conf.ini，用路径来区分
#define SPD_FILE_TMPDRV		"driver.ini"	//给系统设置用的文件，包含所有驱动和对应配置
#define SPD_FILE_TMPDMN		"daemon.ini"	//给系统设置用的文件，包含所有进程和对应配置


#define SPD_CONFIG_FILE 	"setting"		//配置文件名称

#define SPD_MTD0			"/dev/mtd0"		//uboot分区
#define SPD_MTD2			"/dev/mtd3"		//kernle分区

#define SPD_DEV_TF			"/dev/mmcblk0p1"	//tf设备结点
#define SPD_DEV_TF_1		"/dev/mmcblk1p1"	//tf设备结点
#define SPD_DEV_USB			"/dev/sda1"			//usb设备结点
#define SPD_MT				"/mnt/mt"			//挂载点

#define SPD_SUCCESS 	0
#define SPD_LRC			-1		//lrc校验错
#define SPD_FILE		-2		//文件错误或者不存在
#define SPD_WRITEFILE	-3		//写文件失败

#define SPD_EPT_APP		 0		//1-加密应用所在文件夹 0-不加密


//以下是在操作时候给界面送的消息
#define SPD_SMG_KEY		3000	//消息队列key值
#define MSG_TYPE_SERVER 1		
#define MSG_TYPE_CLIENT 2
#define MSG_PROCESS_COMPLETE 100

struct update_msg
{
	unsigned char ucProcess;	//更新进度
	unsigned char ucOption;		//操作类型 2更新中,3更新失败,4更新成功
};

typedef struct
{
	long msgtyp;
	struct update_msg msg_data;
}UP_SER_MSG;


#endif
