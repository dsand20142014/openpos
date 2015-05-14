#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "network.h"
#include "sand_network.h"
#include "devSignal.h"


unsigned int  _NETWORK_check_net(char *dev)
{
    unsigned int fd, intrface,i = 1;
    struct ifreq buf[16];
    struct ifconf ifc;

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) 
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        
        if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) 
        {
          intrface = ifc.ifc_len / sizeof (struct ifreq);
          while (intrface-- > 0)
          {
            if (strcmp(dev,buf[intrface].ifr_name)==0) 
            {        
              i=0;
              break;
            }
          }       
        }  
    } 
    close (fd);
    return i;
}

int Os__check_network(unsigned short uiTimeout, int netType)
{
    unsigned short Timeout=uiTimeout;
    unsigned int  result;
    char devName[50];
    int devType = 0;
    
    memset(devName,0,sizeof(devName));
    //根据不同的设备类型选择设备名称
    if(netType == DEV_COMM_GSM)
    {
    	devType = DEV_GPRS_SIGNALL;
    	strcpy(devName,"ppp0");
    }
    else if(netType == DEV_COMM_WIFI)
    {
    	devType = DEV_WIFI;
    	strcpy(devName,"wlan0");
    }
    else
    	return -1;
    //如果GSM无卡或者wifi没有打开直接退出
    result = Os__get_device_signal(devType);
	if(result == 0)
		return -1;
	
    result = _NETWORK_check_net(devName);
    if (result != 0) 
    {
		do
		{
			result = _NETWORK_check_net(devName);
		  	if (result == 0)
		  		return 0;
		  	Os__xdelay(1);
		  	Timeout --;
		
		} while (Timeout != 0);
		return -1;
    }
   
    return 0;
}
