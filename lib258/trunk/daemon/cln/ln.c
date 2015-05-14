#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#define DRIVER 0
#define LIB 1
#define DAEMON 2

#define KO_PATH		"' > /drivers/drivers.sh"
#define DM_PATH		"' > /daemon/daemon.sh"


//判断是否为目录
int IS_DIR(const char* path)
{
	struct stat st;
	lstat(path, &st);
	return S_ISDIR(st.st_mode);
}

//遍历文件夹递归函数
void List_Files_Core(int type, int recursive)
{
	DIR *pdir;
	int i;
	char *str1,*str2,*ptr,*p;
	struct dirent *pdirent;
	char temp[256];int l=0;
	unsigned char ucFile[500],ucShell[2000],ucBuf[500],buff[6][50];


	memset(ucShell,0,sizeof(ucShell));
	memset(buff,0,sizeof(buff));
	sprintf(ucShell,"echo '#!/bin/sh\n");

	switch(type)
	{
	case DRIVER:
		str2=".ko.";
		p="/drivers/";
		system("cd /drivers");
		break;
	case LIB:
		str2=".so.";
		p="/lib/";
		system("cd /lib");
		break;
	case DAEMON:
		str2=".dm.";
		p="/daemon/";
		system("cd /daemon");
		break;
	}	


	pdir = opendir(p);
	if(pdir)
	{
		while(pdirent = readdir(pdir))
		{
			//跳过"."和".."
			if(strcmp(pdirent->d_name, ".") == 0 || strcmp(pdirent->d_name, "..") == 0)
				continue;
			sprintf(temp, "%s%s", p, pdirent->d_name);
			str1=temp;
			  
			//printf("%s\n", temp);
			//当temp为目录并且recursive为1的时候递归处理子目录
			if(IS_DIR(temp) && recursive)
			{
				;//List_Files_Core(temp, recursive);
			}
			else
			{
				ptr = strstr(str1, str2); 

				if(ptr!=NULL)
				{
					memset(ucBuf,0,sizeof(ucBuf));
					switch(type)
					{
					case DRIVER:
						if(strstr(str1,"ehci-hcd") != NULL)	//driver g_serial.ko depend on arcotg_udc.ko
						{
							if(strstr(ucShell,"fsl_otg_arc") == NULL)
							{
								strcpy(buff[0],str1);
							}
							else
							{
								l=strlen(ucShell);
								sprintf(&ucShell[l]," insmod %s dbg=0\n",str1);
							}
						}
						else if(strstr(str1,"arcotg_udc") != NULL)	//driver arcotg_udc.ko depend on ehci-hcd.ko
						{
							if(strstr(ucShell,"ehci-hcd") == NULL)
							{
								strcpy(buff[1],str1);
							}
							else
							{
								l=strlen(ucShell);
								sprintf(&ucShell[l]," insmod %s\n dbg=0",str1);
							}
						}
						else if(strstr(str1,"g_serial") != NULL)	//driver g_serial.ko depend on arcotg_udc.ko
						{
							if(strstr(ucShell,"arcotg_udc") == NULL)
							{
								strcpy(buff[2],str1);
							}
							else
							{
								l=strlen(ucShell);
								sprintf(&ucShell[l]," insmod %s dbg=0\n",str1);
							}
						}
						else if(strstr(str1,"keycrypt") != NULL)		//driver keycrypt.ko depend on mxc_scc.ko, rng.ko, dryice.ko
						{
							if(strstr(ucShell,"mxc_scc") == NULL ||\
		   					   strstr(ucShell,"rng") == NULL ||\
		   					   strstr(ucShell,"dryice") == NULL)
							{
								strcpy(buff[3],str1);
							}
							else
							{
								l=strlen(ucShell);
								sprintf(&ucShell[l]," insmod %s dbg=0\n",str1);
							}
						}
						else if(strstr(str1,"rtic") != NULL)		//driver rtic.ko depend on mxc_scc.ko, dryice.ko 
						{
							if(strstr(ucShell,"mxc_scc") == NULL ||\
		   					   strstr(ucShell,"dryice") == NULL)
							{
								strcpy(buff[4],str1);
							}
							else
							{
								l=strlen(ucShell);
								sprintf(&ucShell[l]," insmod %s dbg=0\n",str1);
							}
						}
						else if(strstr(str1,"8192cu") != NULL)		//driver 8192cu.ko depend on ehci-hcd.ko
						{
							if(strstr(ucShell,"ehci-hcd") == NULL)
							{
								strcpy(buff[5],str1);
							}
							else
							{
								l=strlen(ucShell);
								sprintf(&ucShell[l]," insmod %s dbg=0\n",str1);
							}
						}
						else
						{
							sprintf(ucBuf," insmod /drivers/%s dbg=0\n",pdirent->d_name);
							l=strlen(ucShell);
							memcpy(&ucShell[l],ucBuf,strlen(ucBuf));
						}
						break;
					case DAEMON:
						if(strstr(pdirent->d_name,"qLcdDaemon")!=NULL)
						{
							sprintf(ucBuf," /daemon/%s -qws dbg=0 &\n",pdirent->d_name);
						}
				//		else if(strstr(pdirent->d_name,"dial")!=NULL)
				//		{
				//			sprintf(ucBuf," /daemon/%s dbg=0 &\n",pdirent->d_name);
				//		}
						else if(strstr(pdirent->d_name,"sln")!=NULL)//sln程序由rcs调用
						{
							break;	
						}
					//	else if(strstr(pdirent->d_name,"updateService")!=NULL)//不要自动起updateService
					//	{
					//		break;
					//	}
						else
						{

							sprintf(ucBuf," /daemon/%s dbg=0 &\n",pdirent->d_name);
						}

						l=strlen(ucShell);
						memcpy(&ucShell[l],ucBuf,strlen(ucBuf));
						break;
					case LIB:

						//printf("%s\n",pdirent->d_name);
						str1=pdirent->d_name;
						ptr=strstr(str1,str2);
						memset(ucFile,0,sizeof(ucFile));
						memcpy(ucFile,str1,(ptr-str1+3));
						//printf("-->%s\n",ucFile);
						
						memset(temp,0,sizeof(temp));
						sprintf(temp,"cd %s",p);
						//printf("%s\n",temp);
						
						system(temp);
						memset(temp,0,sizeof(temp));
						sprintf(temp,"ln -s %s %s",pdirent->d_name,ucFile);
						system(temp);
						break;
					}
					//printf(" --> %s \n",ucShell);
				}
			}
		}
	}
	else
	{
		printf("opendir error");
	}

	if(type == DRIVER)
	{
		for(i=0;i<6;i++)
		{
			l = strlen(buff[i]);
			if(l != 0)
			{
				sprintf(ucBuf," insmod %s dbg=0\n",buff[i]);
				l=strlen(ucShell);
				memcpy(&ucShell[l],ucBuf,strlen(ucBuf));
			}
		}
	}

	switch(type)
	{
	case DRIVER:

		l=strlen(ucShell);
		memcpy(&ucShell[l],KO_PATH,strlen(KO_PATH));
		
		//printf("--[%s]--\n",ucShell);
		system(ucShell);
		system("chmod 777 /drivers/drivers.sh");
		break;
	case DAEMON:
		l=strlen(ucShell);
		memcpy(&ucShell[l],DM_PATH,strlen(DM_PATH));

		//printf("--[%s]--\n",ucShell);
		system(ucShell);
		system("chmod 777 /daemon/daemon.sh");
		break;
	case LIB:
		system("mv ./*.so /lib");
	}

	closedir(pdir);
}

void List_Files(int recursive)
{
	int len;
	char temp[256];
	//去掉末尾的'/'
	//len = strlen(path);
	//strcpy(temp, path);

	//if(temp[len - 1] == '/') temp[len -1] = '\0';
 
	//if(IS_DIR(temp))
	//{
		//处理目录
		List_Files_Core(DRIVER, recursive);
	//}

	//else   //输出文件
	//{
		//printf("%s\n", path);
	//}
}

int main(int argc, char** argv)
{
	int iret=0;
	iret=access("/etc/lnlock",F_OK);
	if(iret== 0)
	{
//		printf("PLS USE SMT TO INI TERMINAL\n");
		return 0;
	}

	List_Files_Core(0, 0);
//printf("KO #######################3\n");

	List_Files_Core(1, 0);
//printf("SO #######################3\n");


	List_Files_Core(2, 0);
//printf("DM #######################3\n");

	system("touch /etc/lnlock");
	system("sync");

	return 0;
}


