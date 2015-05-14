#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
/*
 * Copyright 2011 SAND, Inc. All Rights Reserved.
 */
/**
 * @file	src/sand_key.c
 * @brief	SAND_KEY按键API
 * @ingroup	SAND_KEY
 */

//#include <signal.h>
//#include <sys/mman.h>
#include <input.h>
//#include <pthread.h>

#include "osdriver.h"

#include <sys/shm.h>
#include "fsync_drvs.h"

#define KEYDEV_PATH "/dev/input/keyboard0"

#define     API_KEY_UNLOCK      0
#define     API_KEY_LOCKED      1


extern int Get_Drv_Pid(void);




static int lock = 0;
static pthread_t tid;
static DRV_OUT drvout;

void getKey_thr(void)
{
        int key_fd,ret;
        struct input_event key;

        lock=1;
        memset(&drvout,0,sizeof(drvout));
        key_fd = open(KEYDEV_PATH, O_RDONLY);
        if(key_fd<0)
        {
                perror(KEYDEV_PATH);
        }
        while(1)
        {
                ret=read(key_fd,&key,sizeof(key));
                if(ret>0)
                {
                        if(key.value==1)
                        {
                                drvout.gen_status=DRV_ENDED;
                                drvout.xxdata[1]=key.code;
                        }
                }
        }
        close(key_fd);
        lock=0;
}


#if     0

unsigned char Os__xget_key (void)
{
        if(lock==0)
        {
                pthread_create(&tid, NULL, (void*)getKey_thr, NULL);
        }
        drvout.gen_status=DRV_RUNNING;
        while(drvout.gen_status==DRV_RUNNING)
        {
            usleep(500);
        }
        return drvout.xxdata[1];
}

DRV_OUT * Os__get_key (void)
{
        if(lock==0)
        {
                pthread_create(&tid, NULL, (void*)getKey_thr, NULL);
        }
        drvout.gen_status=DRV_RUNNING;
        return &drvout;
}


#endif

#define     APP_KEY_SHM_KEY     487226

#define     APP_KEY_SHM_FLAG    0x5A5AA5A5

static int  App_Key_Shmid = -1;
static void *Shm_Mem = 0;

typedef struct 
{
    unsigned char lock;
    unsigned char key;
    int           pid;
    unsigned long flag;
}APP_KEY;


void Os__set_key(unsigned char key)
{
    int pid;    
    int timeout=0;
    APP_KEY *app_key;

//    printf("*****%s in*****key=%c***\n",  __func__, key);

    if(App_Key_Shmid < 0)
    {
        App_Key_Shmid = shmget((key_t)APP_KEY_SHM_KEY, 4096, IPC_CREAT);
        Shm_Mem = shmat(App_Key_Shmid, (void *)0, 0);
        
        app_key = (struct APP_KEY *)Shm_Mem;
        
        if(app_key->flag != APP_KEY_SHM_FLAG)
        {
        	memset(Shm_Mem, 0, 4096);
        	app_key->flag = APP_KEY_SHM_FLAG;
        }        
//        printf("    --------------------%s   [%d]\n", __func__, __LINE__);
    }
    else
    {
    	app_key = (struct APP_KEY *)Shm_Mem;
    }
   

    if(app_key->lock != API_KEY_UNLOCK)
    {
//        printf("*****%s locked********\n",  __func__);

        return;
    }

//    printf("*****%s ****App_Key_Shmid=%d****\n",  __func__, App_Key_Shmid);


    app_key->lock = API_KEY_LOCKED;
    app_key->key = key;

    pid = app_key->pid;
    
//    printf("*****%s pid,%d********\n",  __func__, pid);    

    while(pid <= 0)
    {    
        usleep(50);
        
        timeout++;
        if(timeout > 1000)
        {
            return;
        }
    }

    kill(pid, APP_KEY_SIG);

//    printf("*****%s out pid=%d*******\n",  __func__, pid);
}


void Drv_Set_Pid(int pid)
{
    APP_KEY *app_key;

    if(App_Key_Shmid < 0)
    {
        App_Key_Shmid = shmget((key_t)APP_KEY_SHM_KEY, 4096, IPC_CREAT);
        Shm_Mem = shmat(App_Key_Shmid, (void *)0, 0);
        
        app_key = (struct APP_KEY *)Shm_Mem;
        
        if(app_key->flag != APP_KEY_SHM_FLAG)
        {
        	memset(Shm_Mem, 0, 4096);
        	app_key->flag = APP_KEY_SHM_FLAG;
        }        
//        printf("    --------------------%s   [%d]\n", __func__, __LINE__);
    }
    else
    {
    	app_key = (struct APP_KEY *)Shm_Mem;
    }

    app_key->pid = pid;

//    printf("*****%s ****pid=%d****\n",  __func__, pid);    
}

int Get_App_Key(void)
{
    int temp = -1;
    APP_KEY *app_key;

    if(App_Key_Shmid < 0)
    {
        App_Key_Shmid = shmget((key_t)APP_KEY_SHM_KEY, 4096, IPC_CREAT);
        Shm_Mem = shmat(App_Key_Shmid, (void *)0, 0);
        
        app_key = (struct APP_KEY *)Shm_Mem;
        
        if(app_key->flag != APP_KEY_SHM_FLAG)
        {
        	memset(Shm_Mem, 0, 4096);
        	app_key->flag = APP_KEY_SHM_FLAG;
        }        
//        printf("    --------------------%s   [%d]\n", __func__, __LINE__);
    }
    else
    {
    	app_key = (struct APP_KEY *)Shm_Mem;
    }

//    printf("*****%s ****App_Key_Shmid=%d****\n",  __func__, App_Key_Shmid);        
    
    if(app_key->lock != API_KEY_UNLOCK)
    {
        temp = app_key->key;
        app_key->lock = API_KEY_UNLOCK;
    }
    
    app_key->pid = 0;
    
//    printf("*****%s out****temp=%d****\n",  __func__, temp);

    return temp;
}




#if     1

#if 0


//select

unsigned char Os__xget_key (void)
{
	struct timeval tm;
	fd_set rds;
	int max_fd;

	int key_fd;

	int ret;

	struct input_event key;

    key_fd = open(KEYDEV_PATH, O_RDWR);

	if(key_fd < 0)
	{
		printf("open %s failed\n", KEYDEV_PATH);
		return 0;
	}

	while(1)
	{
		tm.tv_sec = 3;
		tm.tv_usec = 0;

		FD_ZERO(&rds);
		FD_SET(key_fd, &rds);

		max_fd = key_fd;

		ret = select(max_fd + 1, &rds, NULL, NULL, &tm);

		if(ret < 0)
		{
			perror("select key_fd fail\n");
			return 0;
		}
		else if(ret == 0)
		{
//			printf("select %ds timeout\n", 3);
			continue;
		}

		if (FD_ISSET(key_fd, &rds))
		{
			read(key_fd, &key, sizeof(key));
   			printf("select key_fd :\n");
            printf("read key value=%d code=%02X\n", key.value, key.code);

            if(key.value==1)
            {
                drvout.gen_status=DRV_ENDED;
                drvout.xxdata[1]=key.code;
                return drvout.xxdata[1];
            }
		}

	}
}


#endif


unsigned char Os__xget_key (void)
{
    int ret;
    NEW_DRV_TYPE new_drv;

//    printf("*****%s in********\n",  __func__);

    while(1)
    {
        ret = Os_Wait_Event(KEY_DRV, &new_drv, 5000);

        if(ret != 0)
        {
//            printf("*****Os_Wait_Event ret= 0 %d********\n", __LINE__);
            continue;
        }
        else
        {
            if(new_drv.drv_type == KEY_DRV)
            {
                drvout.xxdata[1] = new_drv.drv_data.xxdata[1];
                ret = drvout.xxdata[1];
                break;
            }
            else if(new_drv.drv_type == DRV_TIMEOUT)
            {
//                printf("*****%s timeout %d********\n",  __func__,__LINE__);
                continue;
            }
            else
            {
//               printf("*****%s %d, %d********\n", __func__,__LINE__, new_drv.drv_type);
            }
        }
    }

//    printf("*****%s out********\n",  __func__);

    return ret;
}

DRV_OUT * Os__get_key (void)
{
        printf("%s is null, please use new api: Os_Wait_Event********\n",  __func__);

//        drvout.xxdata[1] = Os__xget_key();

//        drvout.gen_status=DRV_RUNNING;

        drvout.gen_status=DRV_ENDED;

//        printf("*****%s out********\n",  __func__);

        return &drvout;
}

#endif

