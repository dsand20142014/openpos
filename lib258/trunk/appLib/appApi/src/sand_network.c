#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <assert.h>
#include <time.h>  
#include <sys/ioctl.h>  
#include <netinet/in.h>   
#include <netinet/if_ether.h>  
#include <net/if_arp.h>  
#include <arpa/inet.h> 
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include "sand_network.h"

#define _PATH_PROCNET_ROUTE		"/proc/net/route"

#define MAX_RCV_LEN 1500
#define bbzero(a, b)             memset(a, 0, b)
void Os__keyled_display(unsigned char status, unsigned char led){}
#define ONLINE_MASK 0x01
#define RX_MASK 0x02
#define TX_MASK 0x04

struct sockaddr_in udp_addr;


/* TCP */
unsigned char TCP_CheckOSVersion(void)
{

}

unsigned char TCP_Open(int *piTCPHandle)
{
    int sockfd;



    Os__keyled_display(0, RX_MASK);
    Os__keyled_display(0, TX_MASK);
    /*---Open socket for streaming---*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Socket");
        Uart_Printf("errno = %d\n",errno);
        //exit(errno);
        return(errno);
    }

    *piTCPHandle = sockfd;
    return(0);
}

//add by liuyifeng 
//modified by lichao 4.7.13
//only support gsm and wifi right now.
int activeDev(int iDev)
{
	FILE *fd;
	int len;
	unsigned char ucbuf[500];
	memset(ucbuf,0,sizeof(ucbuf));
	
	fd = fopen(_PATH_PROCNET_ROUTE,"r");
	if(fd == NULL)
	{
		Uart_Printf("popen error\r\n");
		return -1;
	}
	fread(ucbuf,1,500,fd);
	fclose(fd);
	
	switch(iDev)
	{
		case DEV_COMM_GSM:
			if(strstr(ucbuf,"ppp0") == NULL)//if gsm route is not exist then delete other old route and setup a new gsm route.
			{
				system("route del default");
				if(strstr(ucbuf,"wlan0") != NULL)
					system("/etc/usbwifi/reroute");
				system("route add default ppp0");
				system("cp /etc/ppp/resolv.conf /etc/ &");
			}
			else//if gsm route is already exist,just return.
			{
				if(strstr(ucbuf,"wlan0") != NULL)
				{
					system("route del default wlan0");
					system("/etc/usbwifi/reroute");
					system("cp /etc/ppp/resolv.conf /etc/ &");
				}
				return 0;
			}
			break;
		case DEV_COMM_WIFI:
			if(strstr(ucbuf,"wlan0") == NULL && access("/tmp/dhcpclock",F_OK) != 0)//if wifi route is not exist then delete other old route and setup a new gsm route.
			{
				system("route del default");
				system("ifconfig wlan0 0.0.0.0");
				usleep(500);
				system("udhcpc -q -b -i wlan0 -s /etc/usbwifi/dhcpc.script");
				usleep(500);
			}
			else//if wifi route is already exist,just return.
			{
				if(strstr(ucbuf,"ppp0") != NULL)
				{
					system("route del default ppp0");
				}
				return 0;
			}
			break;
		default:
			return -1;
	}
	return 0;
}




#if 0
unsigned char TCP_Connect(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort)
{

    struct sockaddr_in dest;
    int sockfd;

    /*---Initialize server address/port struct---*/
    bbzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(uiPeerPort);

    //dest.sin_addr.s_addr = htons(ulPeerAddr);
    sockfd = iTCPHandle;
    dest.sin_addr.s_addr = htonl(ulPeerAddr);
    Uart_Printf("s_addr is %x\n", dest.sin_addr.s_addr);

    /*---Connect to server---*/
    if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
    {
        perror("Connect ");
        Uart_Printf("errno = %d\n",errno);
        //exit(errno);
        return(errno);

    }

    return(0);
}

unsigned char TCP_Connect(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort)
{
    struct sockaddr_in dest;
    struct timeval tv;
    int err_again_cnt = 0;
    
    tv.tv_sec = 20; // modified by liugm, 2010-11-12 
    tv.tv_usec = 0;
    
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(uiPeerPort);
    dest.sin_addr.s_addr = htonl(ulPeerAddr);
    Uart_Printf("s_addr is %x\n", dest.sin_addr.s_addr);
    
    setsockopt(iTCPHandle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));//0726 zy
    /*---Connect to server---*/

    /* modified by liugm, 2010-11-12 */
again:
    if ( connect(iTCPHandle, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
    {
        perror("Connect :");
        Uart_Printf("errno = %d\n",errno);
        if (errno == EINPROGRESS || errno == EAGAIN) {
           if (err_again_cnt++ > 5){
                set_ppp_restart();
                return 1;
            }
            goto again; 
        }
        return(1);
    }
    return(0);
}
#endif

#if 1
int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)
{
    int  flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;

    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    error = 0;
    if ( (n = connect(sockfd, saptr, salen)) < 0)
        if (errno != EINPROGRESS)
            return (-1);

    /* Do whatever we want while the connect is taking place. */

    if (n == 0)
        goto done;               /* connect completed immediately */

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;

    if ( (n = select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL)) == 0) {
		/* can't close fd ,or next connect on this socket will bad fd */
        //close(sockfd);          /* timeout */
        errno = ETIMEDOUT;
        return (-1);
    }

    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return (-1);     /* pending error */
    } else{
        fprintf(stderr,"select error: sockfd not set\n");
        return (-1);
    }

  done:
    fcntl(sockfd, F_SETFL, flags);  /* restore file status flags */

    if (error) {
		/* can't close fd ,or next connect on this socket will bad fd */
        //close(sockfd);     /* just in case */ 
        errno = error;
        return (-1);
    }
    return (0);
}


unsigned char TCP_Connect(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort)
//unsigned char TCP_Connect(int iTCPHandle,unsigned char * ulPeerAddr,unsigned short uiPeerPort)
{
    struct sockaddr_in dest;
    int err_again_cnt = 0;
    volatile unsigned int timeout;
    
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(uiPeerPort);
   dest.sin_addr.s_addr = htonl(ulPeerAddr);
//    dest.sin_addr.s_addr = inet_addr(ulPeerAddr);//change by liuyifeng 20121018

  //  Uart_Printf("s_addr is %x\n", dest.sin_addr.s_addr);

again:
    if (connect_nonb(iTCPHandle, (struct sockaddr*)&dest, sizeof(dest) , 20) < 0){// timeout 20s
        perror("Connect :");
        Uart_Printf("errno = %d\n",errno);

        /* 3/30/2011 3:54p  */
        if (errno == 110 && err_again_cnt++ >=2) { 	// Connection timed out
        //	set_ppp_restart(0xE5); 						// reset PPP
			sleep(5); 								// waiting a while ,and ip address dis-appear
            timeout = 20*100; 						// 20 s
            Os__timer_start(&timeout);
           // while(OSGSM_GprsCheckDial(10) && timeout);
            Os__timer_stop(&timeout);
            if (timeout == 0) {
                return 1;
            }
            goto again;
        }/* end */
        return (1);
    }
    return (0);
}

#endif




unsigned char TCP_Send(int iTCPHandle,unsigned char *pucInBuf,unsigned short uiInLen)
{
    int ret;
    struct timeval tv;    
    int err_again_cnt = 0;

    tv.tv_sec = 20;// modified by liugm, 2010-11-12 
    tv.tv_usec = 0;    

    setsockopt(iTCPHandle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

    /* modified by liugm, 2010-11-12 */

    Os__keyled_display(0, RX_MASK);

again:   
    Os__keyled_display(1, TX_MASK);

    ret = send(iTCPHandle, pucInBuf, uiInLen, 0);

    Os__keyled_display(0, TX_MASK);

    	
    if (ret == -1)
    {
        perror("TCP_Send");
        Uart_Printf("errno = %d\n",errno);

        if (errno == EAGAIN || errno == EWOULDBLOCK || errno==EINTR) {
            if (err_again_cnt++ > 5){
                //set_ppp_restart();
                return errno;
            }
            goto again;
        }

        return(errno);
    }

    return(0);
}

unsigned char TCP_Recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout)
{
    int ret;
    struct timeval tv;
    int err_again_cnt = 0;  // liugm 2010-11-12

    tv.tv_sec = uiTimeout;
    tv.tv_usec = 0;
	/*
    if (setsockopt(iTCPHandle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))>=0)
    {
        ret = recv(iTCPHandle, pucInBuf, MAX_RCV_LEN, 0);
    }*/

    /* modified by liugm, 2010-10-12  */
    if (setsockopt(iTCPHandle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        return -1;

   // Os__keyled_display(0, TX_MASK);

again:
    //Os__keyled_display(1, RX_MASK);

    ret = recv(iTCPHandle, pucInBuf, MAX_RCV_LEN, 0);

    //Os__keyled_display(0, RX_MASK);
	
	  if(ret==-1)
    {*puiInLen=0;return -1;}
    else  
      *puiInLen = ret;
	/*
	if (ret == -1)
    {
        perror("TCP_Recv:");
        Uart_Printf("errno = %d\n",errno);

        if (errno == EAGAIN || errno == EWOULDBLOCK || errno==EINTR) {
            if (err_again_cnt++ > 5){
                //set_ppp_restart();
                return errno;
            }
            goto again;
        }

        return(errno);
    }
    *puiInLen = ret;
    */
    return(0);
}


unsigned char TCP_Recv_old(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout)
{
    int ret;

    ret = recv(iTCPHandle, pucInBuf, *puiInLen, 0);

    if (ret == -1)
    {

        perror("TCP_Send");
        Uart_Printf("errno = %d\n",errno);
        //exit(errno);
        return(errno);
    }

    *puiInLen = ret;
    return(0);
}

unsigned char TCP_Close(int iTCPHandle)
{
    Os__keyled_display(0, RX_MASK);
    Os__keyled_display(0, TX_MASK);
    close(iTCPHandle);
    return(0);
}



/* udp*/
unsigned char UDP_Open(int *piUDPHandle)
{
    int sockfd;

    Os__keyled_display(0, RX_MASK);
    Os__keyled_display(0, TX_MASK);
    /*---Open socket for streaming---*/  
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
        perror("Socket");
        exit(errno);
    }
		
    *piUDPHandle = sockfd;
    return(0);


}

unsigned char UDP_Config(int iUDPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort)
{

    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(uiPeerPort);
    udp_addr.sin_addr.s_addr = htons( ulPeerAddr);

    return(0);
}

unsigned char UDP_Send(int iUDPHandle, unsigned char *pucInBuf, unsigned short uiInLen)
{
    int ret;
    Os__keyled_display(0, RX_MASK);
    Os__keyled_display(1, TX_MASK);	
    ret = sendto(iUDPHandle, pucInBuf, uiInLen, 0, (struct sockaddr*)&udp_addr, sizeof(udp_addr));
    Os__keyled_display(0, TX_MASK);	
    if (ret == -1)
    {
        perror("UDP_Send");
        exit(errno);
    }
    return(0);
}

unsigned char UDP_Recv(int iUDPHandle,unsigned char *pucInBuf,unsigned short *puiInLen, unsigned short uiTimeout)
{

    int ret;
    int addr_temp;
    addr_temp = sizeof(udp_addr);
    Os__keyled_display(1, RX_MASK);
    Os__keyled_display(0, TX_MASK);
    ret = recvfrom(iUDPHandle, pucInBuf, *puiInLen, 0, (struct sockaddr*)&udp_addr, &addr_temp);
    Os__keyled_display(0, RX_MASK);

    if (ret == -1)
    {
        perror("UDP_Recv");
        exit(errno);
    }

    return(0);

}

unsigned char UDP_Close(int iUDPHandle)
{

    Os__keyled_display(0, RX_MASK);
    Os__keyled_display(0, TX_MASK);
    close(iUDPHandle);
    return(0);
}
