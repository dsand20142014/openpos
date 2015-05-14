#ifndef SYSPARAM_H_INCLUDE
#define SYSPARAM_H_INCLUDE

/**
 * @defgroup 提供系统setting.ini,daemon.sh,drivers.sh信息的设定和获取
 * @author 熊剑
 * @brief 提供对驱动,守护进程,配置文件修改
 * @{
 */

#define	DRIVERS_SHELL	0
#define DAEMON_SHELL	1
#define BRIEF_LIST_MAX	10

typedef struct
{
	char name[50];//驱动或后台进程名称
	char version[20];//驱动或后台进程版本号
	int insmod;//是否加载驱动或后台进程,1加载,0不加载
	int dbg;//是否打印驱动或后台进程调试信息,1打印,0不打印
}PROCESS_INIT_INFO;


typedef struct
{
	char section[50];//需要获取的配置文件的节
	char key[20];//需要获取的键
	char value[50];//值
	int type;//值类型
	char briefList[BRIEF_LIST_MAX][50];//对值的简介列表
}SYS_SETTING;

/**
 * @fn int PARAM_setting_get(SYS_SETTING *setting)
 * @brief 获取系统setting.ini文件中setting->section下键值为setting->key的值,同时返回键值后的说明类型和简介列表
 * @param setting	SYS_SETTING结构体指针
 *
 * @return 成功返回简介实际读到的条数,失败返回-1
 * @code
 *	int main(int argc,char *argv[])
 *	{
 *		SYS_SETTING setting;
 *		int i,cn;
 *		if(argc<3)
 *			return -1;
 *		memset(&setting,0,sizeof(SYS_SETTING));
 *		strcpy(setting.section,argv[1]);
 *		strcpy(setting.key,argv[2]);
 *		if((cn = PARAM_setting_get(&setting)) >= 0)
 *		{
 *			printf("section is %s,key is %s,value is %s,type is %d\n",setting.section,setting.key,setting.value,setting.type);
 *			for(i=0;i<cn;i++)
 *				printf("briefList[%d] is %s\n",i,setting.briefList[i]);
 *		}
 *		
 *		if(argc<4)
 *			return -1;
 *		strcpy(setting.value,argv[3]);
 *		if(PARAM_setting_set(&setting) == 0)
 *			printf("set new value OK\n");
 *		return 0;
 *	}
 *	执行输出如下
 *	SAND# ./param GSM_SETTING 06.DEFAULT_BAND  7
 *	section is GSM_SETTING,key is 06.DEFAULT_BAND,value is 1,type is 2
 *	briefList[0] is 850
 *	briefList[1] is 900
 *	briefList[2] is 1800
 *	briefList[3] is 1900
 *	briefList[4] is 850/1900
 *	briefList[5] is 900/1800
 *	briefList[6] is 900/1900
 *	set new value OK
 * @endcode
 */
int PARAM_setting_get(SYS_SETTING *setting);


/**
 * @fn int PARAM_setting_set(SYS_SETTING *setting)
 * @brief 设置系统setting.ini文件中setting->section下键值为setting->key的值
 * @param setting	SYS_SETTING结构体指针
 *
 * @return 成功返回0,失败返回-1
 */
int PARAM_setting_set(SYS_SETTING *setting);

/**
 * @fn int PARAM_shell_get(PROCESS_INIT_INFO proInitList[], int proMaxNum, int proMode)
 * @brief 获取daemon.sh或drivers.sh中的列表信息
 * @param proInitList	程序信息列表,包涵程序名称,是否加载和是否打印调试信息
 * @param proMaxNum		最大程序信息条数
 * @param proMode		DRIVERS_SHELL表示获取驱动程序信息,DAEMON_SHELL表示获取守护进程信息
 *
 * @return 成功返回获取到的程序信息的条数,失败返回-1
 * @code
 *	int main(int argc,char *argv[])
 *	{
 *		int i,cn, mode = 0, dbg = 0;
 *		PROCESS_INIT_INFO proInitList[50];
 *		
 *		memset(proInitList,0,sizeof(PROCESS_INIT_INFO)*50);
 *		
 *		if(argc > 1)
 *			mode = atoi(argv[1]);
 *		cn = PARAM_shell_get(proInitList,50,mode);
 *		
 *		for(i=0;i<cn;i++)
 *		{
 *			printf("NAME:%s,VERSION:%s,INSMOD:%d,DEBUG:%d\n",proInitList[i].name,proInitList[i].version,proInitList[i].insmod,proInitList[i].dbg);
 *		}
 *		
 *		if(argc > 2)
 *			dbg = atoi(argv[2]);
 *		else
 *			return -1;
 *		
 *		for(i=0;i<cn;i++)
 *			proInitList[i].dbg = dbg;
 *		
 *		if(PARAM_shell_set(proInitList,cn,mode) == 0)
 *			printf("############set ok##############\n");
 *		
 *		return 0;
 *	}
 *	执行输出如下
 *	SAND# ./param 0 1
 *	NAME:mxc_scc.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:fuseblow_data.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:rng.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:dryice.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:rtic.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:keycrypt.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:bar_scanner_triger.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:camflash.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:gps.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:gprs.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:TDA8007_ic.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:imx258_mag.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:mifare.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:8192cu.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:printer.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	NAME:jtagio.ko,VERSION:130122,INSMOD:1,DEBUG:0
 *	############set ok##############
 * @endcode
 */
int PARAM_shell_get(PROCESS_INIT_INFO proInitList[], int proMaxNum, int proMode);

/**
 * @fn int PARAM_shell_set(PROCESS_INIT_INFO proInitList[], int tolProNum, int proMode)
 * @brief 设置daemon.sh或drivers.sh中的程序信息
 * @param proInitList	程序信息列表,包涵程序名称,是否加载和是否打印调试信息
 * @param proMaxNum		需要设置的程序信息条数
 * @param proMode		DRIVERS_SHELL表示获取驱动程序信息,DAEMON_SHELL表示获取守护进程信息
 *
 * @return 成功返回0,失败返回-1
 */
int PARAM_shell_set(PROCESS_INIT_INFO proInitList[], int tolProNum, int proMode);

#endif // SYSPARAM_H
/** @} end of etc_fns */
