/*
 * netlink_cn.h
 *
 *  Created on: Apr 23, 2012
 *      Author: root
 */
#pragma once

#ifndef NETLINK_CN_H_
#define NETLINK_CN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
//#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <asm/fcntl.h>
//#include <fcntl.h>

#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/connector.h>
#include <linux/ioctl.h>
#include <linux/socket.h>

#include <osdriver.h>


#define MAX_EVENTS 4



#define NETLINK_GXDICC		21
#define MAX_PAYLOAD 400

struct iovec iov;
struct msghdr msg;


#define ICC_WATCHER_NUM   0
#define KEY_WATCHER_NUM   1



#define GETSTATUS   0x15   //add by GXD

static unsigned int seq = 0;


#define SOL_NETLINK 270
#define CN_NETLINK_USERS1 8
#define CN_TEST_IDX CN_NETLINK_USERS1 + 3
#define CN_TEST_VAL  0x456



struct myevent_s
{
    int fd;
    int sender_fd;
    void (*call_back)(int fd, void *arg);
    int events;
    void *arg;
    int status; // 1: in epoll wait list, 0 not in
 //   char buff[128]; // recv data buffer
    int len;
    long last_active; // last active time
}  g_Events[MAX_EVENTS+1];

int g_epollFd;
//myevent_s g_Events[MAX_EVENTS+1]; // g_Events[MAX_EVENTS] is used by listen fd


struct epoll_event ev, events[10];

#pragma pack(push)
#pragma pack(4)

typedef struct {
	unsigned char ucType;
	unsigned char ucLen;
	unsigned char pad;
	unsigned char pad2;
	unsigned int  ioctl_cmd;
}DRVIN258;

typedef struct{
	unsigned char ucReqNR;
	unsigned char ucGenStatus;
	unsigned char ucDrvStatus;
	unsigned char pad;
}DRVOUT258;

typedef struct
{
	unsigned char ucDrvID;
	unsigned char ucDrvCMD;
	unsigned char pad;
	unsigned char pad2;
	DRVIN258      pDrvIn;
	DRVOUT258  	  pDrvOut;
	unsigned char sand_data[400];
//	unsigned char* sand_data;
}  DRV258;

#pragma pack(pop)

struct nlsock_CN
{
	unsigned char           watch_name;
	struct sockaddr_nl		s_local;
	struct sockaddr_nl		s_peer;
	struct sockaddr_nl      s_dest;
	int						s_fd;
	int                     sender_fd;
	int						s_proto;
	unsigned int			s_seq_next;
	unsigned int			s_seq_expect;
	int						s_flags;


	int(*cb_func)(int fd, int events, void *arg);
/////////////////func////////////////////
	struct cn_msg*      		cnmsg;
}  arr_nlsock[3];;




//struct nlsock_CN  arr_nlsock[3];


#endif /* NETLINK_CN_H_ */
