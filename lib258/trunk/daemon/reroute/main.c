/**
@auther liuyifeng 
重新设置路由
*/

#include<stdio.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <sys/mman.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>



void manager(){}


int main(){
	FILE *file;
  	char _pid[200];
  	char _cmd[100];
  	char _destination[30],_genmask[30];
  	char *p_begin,*p_end;

	memset(_pid,0,sizeof(_pid));
	memset(_cmd,0,sizeof(_cmd));
	memset(_destination,0,sizeof(_destination));
	memset(_genmask,0,sizeof(_genmask));
	file=popen("route |grep wlan0","r");
	
	if (file != NULL) {
		if (fgets(_pid, 200, file) != NULL) {	
			p_begin=_pid;
			p_end=strstr(_pid," ");
			memcpy(_destination,_pid,(p_end-p_begin));
			p_begin=strstr(_pid,"255");
			p_end=strstr(p_begin," ");
			memcpy(_genmask,p_begin,(p_end-p_begin));
			sprintf(_cmd,"route del -net %s netmask %s dev wlan0",_destination,_genmask);
			system(_cmd);
			//system("route add default wlan0> /dev/null 2>&1 &");	
		}
	}
	
	fclose(file);
}

 
