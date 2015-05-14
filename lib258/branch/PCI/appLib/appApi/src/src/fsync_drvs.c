#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <haldrv.h>
#include <signal.h>
#include <sys/mman.h>
#include <pthread.h>
#include <asm-generic/fcntl.h>
#include <sys/sem.h>


#include "fsync_drvs.h"

#include "pkds_client.h"


#define     FIFO_NAME       "/tmp/drv_fifo"



#define PCD_IOC_POLLING       _IOWR('P', 16, int)
#define PCD_IOC_POLLEND       _IOW('P',  9, int)

#define ICC_IOC_POLLING_START       _IO('I', 16)
#define ICC_IOC_POLLING_ABORT       _IO('I', 17)

extern int Get_App_Key(void);
extern void Drv_Set_Pid(int pid);

struct my_input_event {
        struct timeval time;
        unsigned short type;
        unsigned short key_code;
        unsigned int value;
};


extern int fd_icc;
extern int fd_mifare;
static NEW_DRV_TYPE Events;


/*
20130301 刘一峰
暂时这么用，pos软件部要求的
*/
void Os_Event_Release(void)
{
	NEW_DRV_TYPE new_drv;
	int uiRet;
	for(;;)
	{
		uiRet = Os_Wait_Event(KEY_DRV,&new_drv,1);
		if(!uiRet)
		{
			if(new_drv.drv_type == KEY_DRV)
				;
			else if(new_drv.drv_type == DRV_TIMEOUT)
				break;

		}
		else
		      break;
	}
}

unsigned int Os_Wait_Event(unsigned int driver, NEW_DRV_TYPE *new_drv, unsigned int timeout)
{
	char buf[512];
	unsigned int temp;

    int i;
	int ret;


	int key_fd = 0;
	int mag_fd = 0;
	int fifo_fd = -1;
	int max_fd = 0;

	struct my_input_event key;

	struct timespec ts;
	fd_set rds;
	sigset_t sigmask;

	unsigned int drv_id = driver;

    if(drv_id > (DRV_TIMEOUT | KEY_DRV| MAG_DRV | MFR_DRV | ICC_DRV))
    {
        new_drv->drv_type = DRV_TIMEOUT;
        return DRV_ID_ERROR;
    }

    memcpy(buf, &drv_id, sizeof(int));
    temp = timeout;
    memcpy(buf+sizeof(int), &temp, sizeof(int));

    if(pkds_exchange(WAIT_EVENT, buf, new_drv) != 0)
    {
        new_drv->drv_type = DRV_TIMEOUT;
        return DRV_ID_ERROR;
    }
    return 0;

    printf("---%s:%d drv_id=%08X---\n",__func__, __LINE__, drv_id);

KEY_UP_CONTINUE:

    max_fd = 0;
    FD_ZERO(&rds);

    if(drv_id & KEY_DRV)
    {
//        if(key_fd == 0)
        {
            key_fd = open(KEY_DEVICE, O_RDWR);
        }

	    if(key_fd < 0)
	    {
	    	printf("### %s open %s failed ###\n",  __func__, KEY_DEVICE);
	    	ret = KEY_DRV_ERROR;
	    	drv_id &= (~KEY_DRV);
	    	goto __open_key_end1;
	    }

	    FD_SET(key_fd, &rds);
	    max_fd += key_fd;

        if(access(FIFO_NAME, F_OK) == -1)
        {
            ret = mkfifo(FIFO_NAME, 0777);

            if(ret != 0)
            {
                printf("**********%s mkfifo error*********\n", __func__);
                drv_id &= (~KEY_DRV);
                goto __open_key_end1;
            }

//            printf("---%s:%d---\n",__func__, __LINE__);

            fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
            if(fifo_fd < 0)
            {
                printf("**********%s %d open %s error*********\n", __func__, __LINE__, FIFO_NAME);
                drv_id &= (~KEY_DRV);
                goto __open_key_end1;
            }
        }
        else
        {
            fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
            if(fifo_fd < 0)
            {
                printf("**********%s %d open %s error*********\n", __func__, __LINE__, FIFO_NAME);
                drv_id &= (~KEY_DRV);
                goto __open_key_end1;
            }
        }

        FD_SET(fifo_fd, &rds);
        max_fd += fifo_fd;
    }

__open_key_end1:

    if(drv_id & MAG_DRV)
    {
//        if(mag_fd == 0)
        {
            mag_fd = open(MAG_DEVICE, O_RDWR);
        }

	    if(mag_fd < 0)
	    {
	    	printf("open %s failed\n", MAG_DEVICE);
	    	ret = MAG_DRV_ERROR;
	    	drv_id &= (~MAG_DRV);
	    	goto __open_mag_end1;
	    }

	    FD_SET(mag_fd, &rds);
	    max_fd += mag_fd;
    }

__open_mag_end1:

    if(drv_id & MFR_DRV)
    {
        if(fd_mifare < 0)
        {
            ret = OSMIFARE_Open();
            if(ret < 0)
            {
                printf("### %s  fd_mifare < 0 ###\n", __func__);
                ret = MFR_DRV_ERROR;
                drv_id &= (~MFR_DRV);
                goto __open_mifare_end1;
            }
        }
        ioctl(fd_mifare,PCD_IOC_POLLING, NULL);
	    FD_SET(fd_mifare, &rds);
	    max_fd += fd_mifare;
    }

__open_mifare_end1:

    if(drv_id & ICC_DRV)
    {
        if(fd_icc < 0)
        {
            OSICC_Init();

            if(fd_icc < 0)
            {
                printf("### %s  fd_icc < 0 ###\n", __func__);
                ret = ICC_DRV_ERROR;
                drv_id &= (~ICC_DRV);
                goto __open_icc_end1;
            }
        }

        ioctl(fd_icc,ICC_IOC_POLLING_START);

	    FD_SET(fd_icc, &rds);
	    max_fd += fd_icc;
    }

__open_icc_end1:

    ts.tv_sec = timeout/1000;
    ts.tv_nsec = (timeout%1000) * 1000000;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGTERM);

	printf("%s:%d start pid=%d tid=%d timeout=%d\n", __func__, __LINE__, getpid(), pthread_self(), timeout);

	while(1)
	{
	    if((ts.tv_sec == 0) && (ts.tv_nsec == 0))
	    {
	        ret = pselect(max_fd + 1, &rds, NULL, NULL, NULL, &sigmask);
	    }
	    else
	    {
	        ret = pselect(max_fd + 1, &rds, NULL, NULL, &ts, &sigmask);
	    }

		if(ret < 0)
		{
			printf("%s was interrupte by a signal\n",  __func__);
			ret = INT_ERROR;
			new_drv->drv_type = DRV_TIMEOUT;
			break;
		}
		else if(ret == 0)
		{
			ret = 0;
			new_drv->drv_type = DRV_TIMEOUT;
			break;
		}

		if(drv_id & KEY_DRV)
		{
		    if (FD_ISSET(key_fd, &rds))
		    {
		    	ret = read(key_fd, &key, sizeof(key));
	            if(ret > 0)
	            {
                    if(key.value==1)
                    {
                        new_drv->drv_type = KEY_DRV;
                        new_drv->drv_data.gen_status = DRV_ENDED;
	                    new_drv->drv_data.xxdata[1] = key.key_code;
	                    new_drv->drv_data.xxdata[2] = key.value;
	                    ret = 0;
	                    break;
	                }
	                else
	                {
//	                    FD_CLR(key_fd, &rds);
//	                    continue;

	                    if(drv_id & MFR_DRV)
	                    {
	                        ioctl(fd_mifare,PCD_IOC_POLLEND, NULL);
	                    }

	                    if(drv_id & ICC_DRV)
	                    {
	                        ioctl(fd_icc,ICC_IOC_POLLING_ABORT);
	                    }

	                    if(drv_id & KEY_DRV)
	                    {
	                        close(fifo_fd);
	                        close(key_fd);
	                    }

	                    goto KEY_UP_CONTINUE;
	                }
	            }
	            else
	            {
	                printf("%s: read key data fail: %d\n", __func__, ret);
	                ret = KEY_DRV_ERROR;
	                break;
	            }
		    }

		    if (FD_ISSET(fifo_fd, &rds))
		    {
		        ret = read(fifo_fd, buf, 100);
		        close(fifo_fd);
		        drv_id &= (~KEY_DRV);

//		        printf("%s: read fifo_fd ret=%d :%x\n", __func__, ret, buf[0]);

		        new_drv->drv_type = KEY_DRV;
		        new_drv->drv_data.gen_status = DRV_ENDED;
		        new_drv->drv_data.xxdata[1] = buf[0];
		        new_drv->drv_data.xxdata[2] = 1;
		        ret = 0;
		        break;
		    }
		}

		if(drv_id & MAG_DRV)
        {
		    if(FD_ISSET(mag_fd, &rds))
		    {
		    	ret = read(mag_fd, buf, 512);

	            new_drv->drv_type = MAG_DRV;
	            new_drv->drv_data.gen_status = DRV_ENDED;

	            if(ret > 0)
	            {
	                memcpy((char *)&new_drv->drv_data, buf, ret);

	        		printf("%s:%d read mag data:\n", __func__, __LINE__);
	        		for(i=0; i<ret; i++)
	        		{
	        		    printf("%02X ", buf[i]);
	        		}
	        		printf("\n");

	                ret = 0;
	            }
	            else
	            {
	                printf("%s: read mag data fail: %d\n", __func__, ret);
	                ret = MAG_DRV_ERROR;
	            }

	        	break;
		    }
		}

        if(drv_id & MFR_DRV)
        {
		    if(FD_ISSET(fd_mifare, &rds))
		    {
		    	ret = read(fd_mifare, buf, 512);
	            new_drv->drv_type = MFR_DRV;
	            new_drv->drv_data.gen_status = DRV_ENDED;
	            if(ret > 0)
	            {
	                memcpy((char *)&new_drv->drv_data, buf, 256);
	                ret = 0;
	            }
	            else
	            {
	                printf("%s: read mifare data fail: %d\n", __func__, ret);
	                ret = MAG_DRV_ERROR;
	            }

	            break;
		    }
		}

		if(drv_id & ICC_DRV)
		{
		    if(FD_ISSET(fd_icc, &rds))
		    {
		    	ret = read(fd_icc, buf, 512);

	            new_drv->drv_type = ICC_DRV;
	            new_drv->drv_data.gen_status = DRV_ENDED;

	            if(ret > 0)
	            {
	                memcpy((char *)&new_drv->drv_data, buf, 256);
	                ret = 0;
	            }
	            else
	            {
	                printf("%s: read icc data fail: %d\n", __func__, ret);
	                ret = ICC_DRV_ERROR;
	            }

	            break;
		    }
        }
	}
	if(drv_id & MFR_DRV)
	{
	    ioctl(fd_mifare,PCD_IOC_POLLEND, NULL);
	}

	if(drv_id & ICC_DRV)
	{
	    ioctl(fd_icc,ICC_IOC_POLLING_ABORT);
	}

	if(drv_id & MAG_DRV)
	{
	    close(mag_fd);
	}

	if(drv_id & KEY_DRV)
	{
	    close(fifo_fd);

	    close(key_fd);
	}

    printf("%s:%d end pid=%d tid=%d\n", __func__, __LINE__, getpid(), pthread_self());

    return ret;
}

const char * LINUXPOS_NAME = "/daemon/linuxpos.dm";

unsigned int Os_Wait_Event_NoKey(unsigned int driver, NEW_DRV_TYPE *new_drv, unsigned int timeout)
{
	char buf[512];
	unsigned int temp;

    int i;
	int ret;

	static int key_fd = 0;
//	static int mag_fd = 0;
	int mag_fd = 0;
	int fifo_fd = -1;
	int appmsg_fd = -1;
	int max_fd = 0;

	struct my_input_event key;

	struct timespec ts;
	fd_set rds;
	sigset_t sigmask;

	unsigned int drv_id = driver;

    if(drv_id > (DRV_TIMEOUT | KEY_DRV| MAG_DRV | MFR_DRV | ICC_DRV))
    {
        new_drv->drv_type = DRV_TIMEOUT;
        return DRV_ID_ERROR;
    }

    if(get_caller_name(buf) != 0)
    {
        new_drv->drv_type = DRV_TIMEOUT;
        return DRV_ID_ERROR;
    }
    else
    {
    	if(strncmp(buf, LINUXPOS_NAME, strlen(LINUXPOS_NAME)) != 0)
    	{
        	new_drv->drv_type = DRV_TIMEOUT;
        	return DRV_ID_ERROR;
    	}
   	}

    memcpy(buf, &drv_id, sizeof(int));
    temp = timeout;
    memcpy(buf+sizeof(int), &temp, sizeof(int));

    if(pkds_exchange(WAIT_NOKEY, buf, new_drv) != 0)
    {
        new_drv->drv_type = DRV_TIMEOUT;
        return DRV_ID_ERROR;
    }
    
    //printf("---%s:%d new_drv->drv_type=%08X---\n",__func__, __LINE__, new_drv->drv_type);
    
    //printf("---%s:%d drv_data->drv_status=%08X---\n",__func__, __LINE__, new_drv->drv_data.drv_status);
    
    return 0;

    printf("---%s:%d drv_id=%08X---\n",__func__, __LINE__, drv_id);

    max_fd = 0;
    FD_ZERO(&rds);

    if(drv_id & KEY_DRV)
    {
//        printf("---%s:%d---\n",__func__, __LINE__);

        if(access(FIFO_NAME, F_OK) == -1)
        {
            ret = mkfifo(FIFO_NAME, 0777);

            if(ret != 0)
            {
                printf("**********%s mkfifo error*********\n", __func__);
                drv_id &= (~KEY_DRV);
                goto __open_key_end2;
            }

//            printf("---%s:%d---\n",__func__, __LINE__);

            fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
            if(fifo_fd < 0)
            {
                printf("**********%s open %s error*********\n", __func__, FIFO_NAME);
                drv_id &= (~KEY_DRV);
                goto __open_key_end2;
            }

//            printf("---%s:%d---\n",__func__, __LINE__);
        }
        else
        {
//            printf("---%s:%d---\n",__func__, __LINE__);

            fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
            if(fifo_fd < 0)
            {
//                printf("**********%s open %s error*********\n", __func__, FIFO_NAME);
                drv_id &= (~KEY_DRV);
                goto __open_key_end2;
            }

//            printf("---%s:%d---\n",__func__, __LINE__);
        }

        FD_SET(fifo_fd, &rds);
        max_fd += fifo_fd;
    }

__open_key_end2:

    if(drv_id & MAG_DRV)
    {
        if(mag_fd == 0)
        {
            mag_fd = open(MAG_DEVICE, O_RDWR);
        }

	    if(mag_fd < 0)
	    {
	    	printf("open %s failed\n", MAG_DEVICE);
	    	ret = MAG_DRV_ERROR;
	    	drv_id &= (~MAG_DRV);
	    	goto __open_mag_end2;
	    }

	    FD_SET(mag_fd, &rds);
	    max_fd += mag_fd;
    }

__open_mag_end2:

    if(drv_id & MFR_DRV)
    {
        if(fd_mifare < 0)
        {
            ret = OSMIFARE_Open();
            if(ret < 0)
            {
                printf("### %s  fd_mifare < 0 ###\n", __func__);
                ret = MFR_DRV_ERROR;
                drv_id &= (~MFR_DRV);
                goto __open_mifare_end2;
            }
        }
        ioctl(fd_mifare,PCD_IOC_POLLING, NULL);
	    FD_SET(fd_mifare, &rds);
	    max_fd += fd_mifare;
    }

__open_mifare_end2:

    if(drv_id & ICC_DRV)
    {
        if(fd_icc < 0)
        {
            OSICC_Init();

            if(fd_icc < 0)
            {
                printf("### %s  fd_icc < 0 ###\n", __func__);
                ret = ICC_DRV_ERROR;
                drv_id &= (~ICC_DRV);
                goto __open_icc_end2;
            }
        }

        ioctl(fd_icc,ICC_IOC_POLLING_START);

	    FD_SET(fd_icc, &rds);
	    max_fd += fd_icc;
    }

__open_icc_end2:

    ts.tv_sec = timeout/1000;
    ts.tv_nsec = (timeout%1000) * 1000000;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGTERM);

    printf("%s:%d start pid=%d tid=%d\n", __func__, __LINE__, getpid(), pthread_self());

	while(1)
	{
	    if((ts.tv_sec == 0) && (ts.tv_nsec == 0))
	    {
	        ret = pselect(max_fd + 1, &rds, NULL, NULL, NULL, &sigmask);
	    }
	    else
	    {
	        ret = pselect(max_fd + 1, &rds, NULL, NULL, &ts, &sigmask);
	    }

		if(ret < 0)
		{
			printf("%s was interrupte by a signal\n",  __func__);
			ret = INT_ERROR;
			new_drv->drv_type = DRV_TIMEOUT;
			break;
		}
		else if(ret == 0)
		{
			ret = 0;
			new_drv->drv_type = DRV_TIMEOUT;
			break;
		}

		if(drv_id & KEY_DRV)
		{
		    if (FD_ISSET(fifo_fd, &rds))
		    {
		        ret = read(fifo_fd, buf, 100);
		        close(fifo_fd);
		        drv_id &= (~KEY_DRV);

//		        printf("%s: read fifo_fd ret=%d :%x\n", __func__, ret, buf[0]);

		        new_drv->drv_type = KEY_DRV;
		        new_drv->drv_data.gen_status = DRV_ENDED;
		        new_drv->drv_data.xxdata[1] = buf[0];
		        new_drv->drv_data.xxdata[2] = 1;
		        ret = 0;
		        break;
		    }
		}

		if(drv_id & MAG_DRV)
        {
		    if(FD_ISSET(mag_fd, &rds))
		    {
		    	ret = read(mag_fd, buf, 512);

	            new_drv->drv_type = MAG_DRV;
	            new_drv->drv_data.gen_status = DRV_ENDED;

	            if(ret > 0)
	            {
	                memcpy((char *)&new_drv->drv_data, buf, ret);

	        		printf("%s:%d read mag data:\n", __func__, __LINE__);
	        		for(i=0; i<ret; i++)
	        		{
	        		    printf("%02X ", buf[i]);
	        		}
	        		printf("\n");

	                ret = 0;
	            }
	            else
	            {
	                printf("%s: read mag data fail: %d\n", __func__, ret);
	                ret = MAG_DRV_ERROR;
	            }

	        	break;
		    }
		}

        if(drv_id & MFR_DRV)
        {
		    if(FD_ISSET(fd_mifare, &rds))
		    {
		    	ret = read(fd_mifare, buf, 512);

	            new_drv->drv_type = MFR_DRV;
	            new_drv->drv_data.gen_status = DRV_ENDED;

	            if(ret > 0)
	            {
	                memcpy((char *)&new_drv->drv_data, buf, 256);
	                ret = 0;
	            }
	            else
	            {
	                printf("%s: read mifare data fail: %d\n", __func__, ret);
	                ret = MAG_DRV_ERROR;
	            }

	            break;
		    }
		}

		if(drv_id & ICC_DRV)
		{
		    if(FD_ISSET(fd_icc, &rds))
		    {
		    	ret = read(fd_icc, buf, 512);

	            new_drv->drv_type = ICC_DRV;
	            new_drv->drv_data.gen_status = DRV_ENDED;

	            if(ret > 0)
	            {
	                memcpy((char *)&new_drv->drv_data, buf, 256);
	                ret = 0;
	            }
	            else
	            {
	                printf("%s: read icc data fail: %d\n", __func__, ret);
	                ret = ICC_DRV_ERROR;
	            }

	            break;
		    }
        }
	}

    if(drv_id & MFR_DRV)
    {
	    ioctl(fd_mifare,PCD_IOC_POLLEND, NULL);
	}

    if(drv_id & ICC_DRV)
    {
	    ioctl(fd_icc,ICC_IOC_POLLING_ABORT);
	}

	if(drv_id & KEY_DRV)
	{
	    close(fifo_fd);
	}

	if(drv_id & MAG_DRV)
	{
	    close(mag_fd);
	}

	printf("%s:%d end pid=%d tid=%d\n", __func__, __LINE__, getpid(), pthread_self());

    return ret;
}
