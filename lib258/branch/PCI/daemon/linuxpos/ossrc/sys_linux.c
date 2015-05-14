/************************************************************
* LINUX  SYSTEM functions
*************************************************************/
#include <errno.h>
#include "sys_linux.h"
#include "main.h"

extern int lcd_message_id;

static void msgr_interrupt(int s)
{
    printf("msgrcv_interrupt\n");
}

static void sighandle()
{
    struct sigaction siga;
    siga.sa_handler = msgr_interrupt;
    siga.sa_flags  = 0;
    memset (&siga.sa_mask, 0, sizeof(sigset_t));
    sigaction(SIGALRM, &siga, NULL);
}

int sys_execl(char* file,char* p1,char* p2,char* p3,char* p4)
{
	char pathname[64];
	int ret=0;
	
	memset(pathname, 0, 64);
	strcat(pathname,"/app/");
	strcat(pathname,file);
	strcat(pathname,"/");
	strcat(pathname,file);
	
    if(execl(pathname, file, p1, p2,p3, p4,(char*)0 ) < 0	){   //"-geometry","240x300+0+20",
		
		printf("\n************************APP LOAD FAILED*************************\n");
		  printf("************************PLS TRY TO DELETE*************************\n");
		sleep(2);
		ret= -1;
	}
	else
		ret=0;

	return ret;
}

/* 
 * modified by liugm 12/30/2010, let it retry when not fail
 */
int sys_shm_create(key_t key)
{
    int id;
    int cnt = 0;

retry:
	if ((id = shmget(key,APP_SHM_SIZE,777|IPC_CREAT))<0){
        if (cnt++ == 6) {
            //exit(-1);
			Uart_Printf(" %s fail.\n",__func__);
			return -1; //xl
		}
		goto retry;
	}
	return id;
}

void* sys_shm_attach(int id)
{
    void *addr;
	
	addr = (unsigned char *)shmat(id,NULL,0);
	if (addr == (void*)(-1)){
		Uart_Printf(" %s fail.\n",__func__);
		//exit(-1);
		return -1; //xl
	}
	return addr;
}

int sys_shm_detach(void *addr)
{
    if (shmdt(addr)<0){
		Uart_Printf(" %s fail.\n",__func__);
		perror("shmdt error!");
		return -1;
	}
	return 0;
}

int sys_msgque_create(key_t key)
{
    int id;
	if ((id=msgget(key,0777|IPC_CREAT))<0){
		Uart_Printf(" %s msgget1 fail.\n",__func__);
		return(-1);
	}
	/*firstly remove it for clear msgq*/
	if (msgctl(id,IPC_RMID,NULL)<0){
		Uart_Printf(" %s msgctl fail.\n",__func__);
		return(-1);
	}
	
	/*create it again */
	if ((id=msgget(key,0777|IPC_CREAT))<0){
		Uart_Printf(" %s msgget2 fail.\n",__func__);
		return(-1);
	}
	
	return id;
}

int sys_msgque_send(int msqid,const void *pmsg,int nbytes)
{
	int ret=0;

	if ( msgsnd(msqid,pmsg,nbytes,0)<0 ){
		/*OSMMI_ClrDisplay(0x30,255);
		OSMMI_DisplayASCII(0x30, 0, 0,(unsigned char*)"APP LOAD FAILED");
		OSMMI_DisplayASCII(0x30, 4, 0,(unsigned char*)"PLS TRY TO DELETE");*/
		printf("\n************************APP LOAD FAILED*************************\n");
		printf("************************PLS TRY TO DELETE*************************\n");
		sleep(2);
		ret= -1;
	}
	else {
		ret=0;
	}	
	return ret;
}

/*
 * Recv msg without wait and try many times if no msg.
 * Return bytes received or -1 when failed.
 * Modified by xiangliu 2013.1.25
 */
int sys_msgque_recv(int msqid,void *pmsg,int nbytes,long type)
{
	int ret = 0;
	sighandle();
rev:
	alarm(300);
	ret = msgrcv(msqid, pmsg,nbytes,type,0);
	int t =	alarm(0);
	if ( ret < 0 ){
		if (t == 0) {
			/* send msg to qlcd to show msgbox and rev msg*/
			send_msg_qlcd(100, 1);
			struct lcd_message msg;
			int r = 0;
			r = msgrcv(lcd_message_id, &msg, sizeof(msg), 49, 0);
			if (r < 0) {
				perror("msgrcv fail");
			}
			else if (r > 0) {
				printf("----Linuxpos Debug---- APP timeout! %d\n", msg.msg_data.gprs_d);
				if (msg.msg_data.gprs_d == 1) { // interrupt app
					((APPMSG *)pmsg)->m_data.pt_eve_out.download = 'D';
					((APPMSG *)pmsg)->m_data.pt_eve_out.treatment_status = OK;
					printf("----Linuxpos Debug---- APP timeout! Will be removed!\n");
					return 4;//<-FIXME remove the app ?
				} else {
					goto rev;
				}
			}
		}
		else if (t > 0) {
			perror("sys_msgque_recv  fail.");
		}
	}
	return ret;
}

/////////////////////  END ////////////////////////////////


