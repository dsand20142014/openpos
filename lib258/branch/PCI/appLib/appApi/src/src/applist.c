#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "applist.h"
#include "updateService.h"
#include "param.h"

#define	SYS_APP_MAX_NUM		10	//系统保留的最大应用号为10

#define APP_MAX_SIZE		30	//允许存储的最多的应用数量
#define APP_SHM_ID		8866
#define APP_SHM_SIZE		APP_MAX_SIZE*sizeof(APP_PID_LIST)

typedef struct
{
	unsigned short appID;
	int pID;
}APP_PID_LIST;

static int curAppCun = 0; 						//当前已经注册的APP数量

/*
void print_applist_info(void)
{
	int shmid = 0;
	int i = 0;
	APP_PID_LIST* app_pid_list;
	shmid = shmget(APP_SHM_ID,APP_SHM_SIZE,0666|IPC_CREAT);
	if(-1 == shmid)
	{	
		Uart_Printf("APPLIST_get_appid,shmget error\r\n");
		return ;
	}
	Uart_Printf("in APPLIST_get_appid,shmid = %d\r\n",shmid);
	app_pid_list = (APP_PID_LIST*)shmat(shmid,NULL,0);
	if(-1 == app_pid_list)
	{	
		Uart_Printf("APPLIST_get_appid,shmat error\r\n");
		return ;
	}
	Uart_Printf("in APPLIST_get_appid,shmaddr = %04x\r\n",app_pid_list);
	for(i=0;i<30;i++)
	{
		Uart_Printf("app_pid_list.appID[%d] = %d\r\n",i,app_pid_list[i].appID);
		Uart_Printf("app_pid_list.pID[%d] = %d\r\n",i,app_pid_list[i].pID);
	}
	return ;
}
*/

void OSSPD_smt_update(int _f)
{
	int etcFile,etcSection;
	unsigned char ucPath[50];
	
	memset(ucPath,0,sizeof(ucPath));
	sprintf(ucPath,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_DAEMONSET);
	etcFile=Os__param_loadFullPath(ucPath);
	etcSection=Os__param_findSection(etcFile,"MODULE_ACTIVE",0);
	if(_f==1)
  		Os__param_set(etcSection,"SMT_OPEN","1");
  	else
  		Os__param_set(etcSection,"SMT_OPEN","0");
  		
  	Os__param_saveFullPath(etcFile,ucPath); 
  	
  	Os__param_unload(etcFile);	
}




/*
卸载并删除挂载目录
*/
void OSSPD_mt_unmount()
{
	unsigned char ucParam[100];
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"umount %s",SPD_MT);
	if(system(ucParam) == 0)
	{
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"rm -rf %s",SPD_MT);
		system(ucParam);
	}
}

/*
创建挂载目录并挂载
*/
void OSSPD_mt_mount(unsigned char * _dev)
{
	unsigned char ucParam[100];
	
	//挂载前先卸载
	if(access(SPD_MT,F_OK) == 0)
	{
		OSSPD_mt_unmount();
	}
	
	if(access(SPD_MT,F_OK) != 0)
	{
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"mkdir %s",SPD_MT);
		system(ucParam);
	}
		
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"mount %s %s",_dev,SPD_MT);
	system(ucParam);
}

int Os__mt_active(int _active,int _dev)
{
	unsigned char ucParam[100];
	
	if(_active)
	{
		if(_dev==2)
		{
			if(access(SPD_DEV_USB,F_OK)==0)
				OSSPD_mt_mount(SPD_DEV_USB);
			else
				return -1;
		}
		else
		{
			if(access(SPD_DEV_TF,F_OK)==0)
				OSSPD_mt_mount(SPD_DEV_TF);
			else if(access(SPD_DEV_TF_1,F_OK)==0)
				OSSPD_mt_mount(SPD_DEV_TF_1);
			else
				return -1;
		}
	}
	else
		OSSPD_mt_unmount();
	return 0;
}

/*
*根据用户选择的pln文件释放资源，并返回conf.ini文件路径
*/
char *Os__pln_unzip(char *plnPath)
{
	static char ucParam[100];
	char *pStr = NULL;
	int i;
	
	if(access(plnPath,F_OK) == 0)
	{
		//删除download下的零时文件夹
		memset(ucParam,0x00,sizeof(ucParam));
		sprintf(ucParam,"rm -rf %s/*",SPD_DOWN_PATH);
		system(ucParam);
		
		//删除tmp下的pln文件夹
		memset(ucParam,0x00,sizeof(ucParam));
		sprintf(ucParam,"rm -rf %s",SPD_PLN_PATH);
		system(ucParam);
		
		//创建pln文件夹
		memset(ucParam,0x00,sizeof(ucParam));
		sprintf(ucParam,"mkdir -p %s",SPD_PLN_PATH);
		system(ucParam);
		
		//解压缩
		memset(ucParam,0x00,sizeof(ucParam));
		//如果文件路径含有空格
		if(strstr(plnPath," ") != NULL)
		{
			pStr = ucParam;
			strcpy(pStr,"/bin/unzip ");
			pStr += 11;
			for(i=0;i<strlen(plnPath);i++)
			{
				if(*(plnPath+i) == ' ')
					*pStr++ = '\\';
				*pStr++ = *(plnPath+i);
			}
			sprintf(pStr," -d %s",SPD_PLN_PATH);
		}
		else
		{
			sprintf(ucParam,"/bin/unzip %s -d %s",plnPath,SPD_PLN_PATH);
		}
		
		if(system(ucParam) == 0)
		{
			memset(ucParam,0x00,sizeof(ucParam));
			sprintf(ucParam,"%s%s",SPD_PLN_PATH,SPD_FILE_CONF);
			if(access(ucParam,F_OK) == 0)
				return ucParam;
		}
	}
	
	return NULL;
}

int Os__msg_open(key_t key)
{
	int msqid = 0;
	msqid = msgget(key, 0777|IPC_CREAT);
	return msqid;
}

int Os__msg_send(int msqid, const void *msgp, size_t msgsz)
{
	return msgsnd(msqid, msgp, msgsz, IPC_NOWAIT);
}

int Os__msg_recv(int msqid, void *msgp, size_t msgsz, long msgtyp)
{
	int ret = 0;
	ret = msgrcv(msqid, msgp, msgsz, msgtyp, IPC_NOWAIT);
	if(ret == -1)
		return 0;
	else
		return ret;
}

int Os__msg_close(int msqid)
{
	return msgctl(msqid,IPC_RMID,0);
}

int Os__applist_get(APPINFOLIST applist[], int appMaxNum)
{
	int i = 1, cn = 0;
	int etcFile,etcSection;
	unsigned char ucParam[200], ucID[6];
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	etcFile = Os__param_loadFullPath(ucParam);
	if(etcFile > 0)
	{
		//计算应用个数并获取应用值
		while(1)
		{
			memset(ucParam,0,sizeof(ucParam));
			sprintf(ucParam,"APP%d",i);
			etcSection = Os__param_findSection(etcFile,ucParam,0);
			if(etcSection <= 0)
				break;
			else if( cn < appMaxNum)
			{
				memset(ucID,0,sizeof(ucID));
				Os__param_get(etcSection,"ID",ucID,sizeof(ucID)-1);
				//只获取应用ID大于10的应用
				if(atoi(ucID) > SYS_APP_MAX_NUM)
				{
					strncpy(applist[cn].id,ucID,sizeof(ucID));
					Os__param_get(etcSection,"NAME",applist[cn].name,sizeof(applist[cn].name)-1);
					Os__param_get(etcSection,"EDITDATE",applist[cn].editDate,sizeof(applist[cn].editDate));
					Os__param_get(etcSection,"HIDDEN",applist[cn].hidden,sizeof(applist[cn].hidden));
					Os__param_get(etcSection,"VERSION",applist[cn].version,sizeof(applist[cn].version));
					Os__param_get(etcSection,"ITEMCODE",applist[cn].itemCode,sizeof(applist[cn].itemCode));
					Os__param_get(etcSection,"OWNER",applist[cn].owner,sizeof(applist[cn].owner));
					Os__param_get(etcSection,"CHECKCODE",applist[cn].checkCode,sizeof(applist[cn].checkCode));
					Os__param_get(etcSection,"EDIT",applist[cn].edit,sizeof(applist[cn].edit)-1);
					cn++;
				}
			}
			i++;
		}
		
		Os__param_unload(etcFile);
		return cn;
	}
	return -1;
}

int Os__applist_set(APPINFOLIST applist[], int appNum)
{
	int i = 0;
	int cn = 0;
	int etcFile,etcSection;
	unsigned char ucParam[200], ucID[6];
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	etcFile = Os__param_loadFullPath(ucParam);
	if(etcFile > 0)
	{
		for(i=0;i<appNum;i++)
		{
			if(applist[i].edit[0] == 0x33)
			{
				//删除应用
				memset(ucParam,0,sizeof(ucParam));
				sprintf(ucParam,"rm -rf %s%s",SPD_APP_PATH,applist[i].id);
				system(ucParam);
				system("sync");
			}
			else
			{
				//重写conf.ini文件
				while(1)
				{
					cn++;
					memset(ucParam,0,sizeof(ucParam));
					sprintf(ucParam,"APP%d",cn);
					etcSection = Os__param_findSection(etcFile,ucParam,1);
					//查看应用ID是否为大于10的应用
					memset(ucID,0,sizeof(ucID));
					Os__param_get(etcSection,"ID",ucID,sizeof(ucID)-1);
					if(atoi(ucID) > SYS_APP_MAX_NUM || atoi(ucID) <= 0)
					{
						Os__param_set(etcSection,"ID",applist[i].id);
						Os__param_set(etcSection,"NAME",applist[i].name);
						Os__param_set(etcSection,"EDITDATE",applist[i].editDate);
						Os__param_set(etcSection,"HIDDEN",applist[i].hidden);
						Os__param_set(etcSection,"VERSION",applist[i].version);
						Os__param_set(etcSection,"ITEMCODE",applist[i].itemCode);
						Os__param_set(etcSection,"OWNER",applist[i].owner);
						Os__param_set(etcSection,"CHECKCODE",applist[i].checkCode);
						Os__param_set(etcSection,"EDIT",applist[i].edit);
						break;
					}
				}
			}
		}
		
		//删除多余的配置
		i = cn;
		while(1)
		{
			i++;
			memset(ucParam,0,sizeof(ucParam));
			sprintf(ucParam,"APP%d",i);
			etcSection = Os__param_findSection(etcFile,ucParam,0);
			if(etcSection <= 0)
				break;
			else
			{
				memset(ucID,0,sizeof(ucID));
				Os__param_get(etcSection,"ID",ucID,sizeof(ucID)-1);
				if(atoi(ucID) > SYS_APP_MAX_NUM)
					Os__param_rmSection(etcFile,ucParam);
				else
				{
					cn++;
					//对于后面存在的系统APP向前移动
					if(i > cn)
					{
						APPINFOLIST tmpApplist;
						memset(&tmpApplist,0,sizeof(APPINFOLIST));

						strncpy(tmpApplist.id,ucID,sizeof(ucID));
						Os__param_get(etcSection,"NAME",tmpApplist.name,sizeof(tmpApplist.name));
						Os__param_get(etcSection,"EDITDATE",tmpApplist.editDate,sizeof(tmpApplist.editDate));
						Os__param_get(etcSection,"HIDDEN",tmpApplist.hidden,sizeof(tmpApplist.hidden));
						Os__param_get(etcSection,"VERSION",tmpApplist.version,sizeof(tmpApplist.version));
						Os__param_get(etcSection,"ITEMCODE",tmpApplist.itemCode,sizeof(tmpApplist.itemCode));
						Os__param_get(etcSection,"OWNER",tmpApplist.owner,sizeof(tmpApplist.owner));
						Os__param_get(etcSection,"CHECKCODE",tmpApplist.checkCode,sizeof(tmpApplist.checkCode));
						Os__param_get(etcSection,"EDIT",tmpApplist.edit,sizeof(tmpApplist.edit)-1);
						//获取完节中内容后删除
						Os__param_rmSection(etcFile,ucParam);
						
						memset(ucParam,0,sizeof(ucParam));
						sprintf(ucParam,"APP%d",cn);
						etcSection = Os__param_findSection(etcFile,ucParam,1);
						Os__param_set(etcSection,"ID",tmpApplist.id);
						Os__param_set(etcSection,"NAME",tmpApplist.name);
						Os__param_set(etcSection,"EDITDATE",tmpApplist.editDate);
						Os__param_set(etcSection,"HIDDEN",tmpApplist.hidden);
						Os__param_set(etcSection,"VERSION",tmpApplist.version);
						Os__param_set(etcSection,"ITEMCODE",tmpApplist.itemCode);
						Os__param_set(etcSection,"OWNER",tmpApplist.owner);
						Os__param_set(etcSection,"CHECKCODE",tmpApplist.checkCode);
						Os__param_set(etcSection,"EDIT",tmpApplist.edit);
					}
				}
			}
		}

		//保存配置文件
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
		Os__param_saveFullPath(etcFile,ucParam);
		Os__param_unload(etcFile);
		system("sync");
		return 0;
	}
	return -1;
}

int Os__applist_getAppCount(void)
{
	int cn = 0, etcFile, etcSection, i = 1;
	unsigned char ucFile[100],ucApp[10],ucID[6];

	memset(ucFile,0,sizeof(ucFile));
	sprintf(ucFile,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	etcFile = Os__param_loadFullPath(ucFile);

	if(etcFile>0)
	{
 		while(1)
 		{
			memset(ucApp,0,sizeof(ucApp));
			sprintf(ucApp,"APP%d",i);
			etcSection = Os__param_findSection(etcFile,ucApp,0);
			if(etcSection<=0)
				break;
			else
			{
				memset(ucID,0,sizeof(ucID));
				Os__param_get(etcSection,"ID",ucID,sizeof(ucID)-1);
				//应用ID大于10的应用才计数
				if(atoi(ucID) > SYS_APP_MAX_NUM)
					cn++;
			}
			i++;
		}
		Os__param_unload(etcFile);
	}
	return cn;
}

int APPLIST_add_appid(unsigned short appid, int pid)
{
	int i = 0,ret = 0;
	int shmid = 0;
	APP_PID_LIST* app_pid_list;
	shmid = shmget(APP_SHM_ID,APP_SHM_SIZE,0666|IPC_CREAT);
	if(-1 == shmid)
	{	
		Uart_Printf("APPLIST_add_appid,shmget error\r\n");
		return -1;
	}
	//Uart_Printf("in APPLIST_add_appid,shmid = %d\r\n",shmid);
	app_pid_list = (APP_PID_LIST*)shmat(shmid,NULL,0);
	if(-1 == (int)app_pid_list)
	{	
		Uart_Printf("APPLIST_add_appid,shmat error\r\n");
		return -1;
	}
	//Uart_Printf("in APPLIST_add_appid,shmaddr = %04x\r\n",app_pid_list);
	
	curAppCun = Os__applist_getAppCount();
	if(curAppCun == 0)
		memset(app_pid_list,0,APP_SHM_SIZE);
	
	for(i=0;i<APP_MAX_SIZE;i++)
	{
		if(app_pid_list[i].pID == 0 || app_pid_list[i].appID == appid)
		{
			app_pid_list[i].pID = pid;
			app_pid_list[i].appID = appid;
			ret = shmdt(app_pid_list);
			if(ret != 0)
			{	
				Uart_Printf("APPLIST_add_appid shmdt error\r\n");
				return -1;
			}
			return 0;
		}
	}
	
	return -1;
}

int APPLIST_rmove_appid(int pid)
{
	int i=0,ret = 0;
	int shmid = 0;
	APP_PID_LIST* app_pid_list;
	shmid = shmget(APP_SHM_ID,APP_SHM_SIZE,0666|IPC_CREAT);
	if(-1 == shmid)
	{	
		Uart_Printf("APPLIST_rmove_appid,shmget error\r\n");
		return -1;
	}
	//Uart_Printf("in APPLIST_rmove_appid,shmid = %d\r\n",shmid);
	app_pid_list = (APP_PID_LIST*)shmat(shmid,NULL,0);
	if(-1 == (int)app_pid_list)
	{	
		Uart_Printf("APPLIST_rmove_appid,shmat error\r\n");
		return -1;
	}
	//Uart_Printf("in APPLIST_rmove_appid,shmaddr = %04x\r\n",app_pid_list);
	
	curAppCun = Os__applist_getAppCount();
	if(curAppCun == 0)
		memset(app_pid_list,0,APP_SHM_SIZE);

	for(i=0;i<curAppCun;i++)
	{
		if(app_pid_list[i].pID == pid)
		{
			app_pid_list[i].appID = 0;
			app_pid_list[i].pID = 0;
			curAppCun--;
			break;
		}
	}
	
	ret = shmdt(app_pid_list);
	if(ret != 0)
	{
		Uart_Printf("APPLIST_add_appid shmdt error\r\n");
		return -1;
	}
	return 0;
}


unsigned short APPLIST_get_appid(void)
{
	int i = 0;
	int ret = 0;
	int curPID = getpid();
	int shmid = 0;
	int appid = 0;
	//Uart_Printf("APPLIST_get_appid,curPID = %d\r\n",curPID);
	APP_PID_LIST* app_pid_list;
	shmid = shmget(APP_SHM_ID,APP_SHM_SIZE,0666|IPC_CREAT);
	if(-1 == shmid)
	{	
		Uart_Printf("APPLIST_get_appid,shmget error\r\n");
		return 0;
	}
	//Uart_Printf("in APPLIST_get_appid,shmid = %d\r\n",shmid);
	app_pid_list = (APP_PID_LIST*)shmat(shmid,NULL,0);
	if(-1 == (int)app_pid_list)
	{	
		Uart_Printf("APPLIST_get_appid,shmat error\r\n");
		return 0;
	}
	//Uart_Printf("in APPLIST_get_appid,shmaddr = %04x\r\n",app_pid_list);
	
	curAppCun = Os__applist_getAppCount();
	
	if(curPID <= 0 || curAppCun == 0)
		return 0;

	for(i=0;i<APP_MAX_SIZE;i++)
	{
		//Uart_Printf("app_pid_list[%d].pID = %d\r\n",i,app_pid_list[i].pID);
		if(app_pid_list[i].pID == curPID)
		{
			appid = app_pid_list[i].appID;
			ret = shmdt(app_pid_list);
			if(ret != 0)
			{
				Uart_Printf("APPLIST_add_appid shmdt error\r\n");
				return 0;
			}
			return appid;
		}
	}

	//Uart_Printf("didn't found appid:%d\r\n",curPID);
	ret = shmdt(app_pid_list);
	if(ret != 0)
	{
		Uart_Printf("APPLIST_add_appid shmdt error\r\n");
		return 0;
	}
	return 0;
}

