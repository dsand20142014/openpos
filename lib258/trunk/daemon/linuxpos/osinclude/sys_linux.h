#ifndef __SYS_LINUX_H
#define __SYS_LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>

/* extern */
extern void app_timeout_proc(void);

/* process  */
int  sys_execl(char* file,char* p1,char* p2,char* p3,char* p4);

/* share memory  */
int   sys_shm_create(key_t key);
void* sys_shm_attach(int id);
int  sys_shm_detach(void *addr);

/* message queue */
int   sys_msgque_create(key_t key);
int  sys_msgque_send(int msqid,const void *pmsg,int nbytes);
int  sys_msgque_recv(int msqid,void *pmsg,int nbytes,long type);

/* timeout */
void  sys_timeout_clear(void);
void  sys_timeout(unsigned int seconds );
void  sys_timeout_op(int signum,siginfo_t *info,void *myact);
void sys_timeout_run(unsigned int seconds, void (*pfn)(int,siginfo_t *,void *) );
#endif
