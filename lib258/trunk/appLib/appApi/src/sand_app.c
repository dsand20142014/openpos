/* APIs for OSAPP_ */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include <sand_main.h>
#include <osdrv.h>
#include <osevent.h>


static int OSAPP_shm_write(key_t key, unsigned char *data, int length)
{
	int shm_id;
	int app_id;
	unsigned char *shm_addr;
	int offset = 0;

	app_id = main_get_app_id();

	if (( shm_id=shmget(key,APP_SHM_SIZE,777|IPC_CREAT))<0 ){
		perror("### shmget failed.");
		return (-1);
	}

	shm_addr = shmat(shm_id,NULL,0);

	/* source msgque key */
	extern int app_que_key;
	memcpy(shm_addr + offset, &app_que_key, sizeof(int));
	offset += sizeof(int);

	/* source app id */
	memcpy(shm_addr + offset, &app_id, sizeof(int));
	offset += sizeof(int);

	/* dest app id ,default is 0 . means to ALL */
	int dst_app_id = 0;
	memcpy(shm_addr + offset, &dst_app_id, sizeof(int));
	offset += sizeof(int);

	/* data length */
	memcpy(shm_addr + offset, &length, sizeof(int));
	offset += sizeof(int);

	/* data */
	memcpy(shm_addr + offset, data , length);
	offset += length;

	shmdt(shm_addr);

	return 0;

}

static int OSAPP_put_msg(int qid)
{
    APPMSG msg;
	int retval=0;
	
 	msg.m_type = APP_MSG_TYPE;
	strcpy(msg.m_data, "OSAPP_REQ");
	retval = msgsnd(qid, &msg,APP_MSG_LEN,0); // send msg
	if (retval<0){
		perror("### msgsnd fail.\n");
		return (-1);
	}
	return retval;


}

static int OSAPP_get_msg(int qid)
{
    int nbytes=0;
	APPMSG msg;
	
	while(1){
		nbytes = msgrcv(qid ,&msg,APP_MSG_LEN,APP_MSG_TYPE,0);
		if (nbytes < 0){
			perror("### msgrcv fail.\n");
			return (-1);
		}
        if ( strncmp(msg.m_data,"OSAPP_ACK",9) == 0 )//match
			break;
		
	}
	return nbytes;

}	

/*read from SHM  */
static int OSAPP_shm_read(key_t key, unsigned char *data, int *length)
{
	int shm_id;
	int app_id;
	unsigned char *shm_addr;
	int offset = 0;
	int src_app_id = 0;
	int dst_app_id = 0;

    app_id = main_get_app_id();
	
	if (( shm_id=shmget(key, APP_SHM_SIZE, 777|IPC_CREAT))<0 ){
		perror("### shmget failed.");
		return (-1);
	}

	shm_addr = shmat(shm_id, NULL, 0);
    /* src msgque key */
	/* skip*/
	offset += 4;

	/* src app id */
	memcpy(&src_app_id, shm_addr + offset, sizeof(int));
	offset += sizeof(int);

	if ( src_app_id != app_id ){
		printf("[%s:%d]--app id not match!!!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	/* answered app id */
	memcpy(&dst_app_id, shm_addr + offset, sizeof(int));
	offset += sizeof(int);

	/* data length */
	memcpy(length, shm_addr + offset, sizeof(int));
	offset += sizeof(int);

	/* data */
	memcpy(data, shm_addr + offset, (int)(*length));
	offset += *length;

	shmdt(shm_addr);

	return 0;

}

/* apis for Hubei */
int  OSAPP_event_run_request(unsigned char *ibuf, int ilen, unsigned char *obuf,int *olen)
{
	extern int shm_key, mas_que_id,app_que_id;

	if (OSAPP_shm_write(shm_key, ibuf, ilen) < 0 )
		return -1;

	if (OSAPP_put_msg(mas_que_id) < 0)
		return -1;
	
	/* blocked until linuxpos response */
	if (OSAPP_get_msg(app_que_id) < 0)
		return -1;

	if (OSAPP_shm_read(shm_key, obuf, olen) < 0)
		return -1;

	return 0;
}




//////////////////////////////////////////  end ////////////////////////////////////
