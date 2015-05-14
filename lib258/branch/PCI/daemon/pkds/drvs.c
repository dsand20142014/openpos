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

#include "drvs.h"


//#define KEY_DEVICE		"/dev/input/keyboard0"
//#define MAG_DEVICE		"/dev/s3c2410_mag"
//#define MFR_DEVICE		"/dev/rc531"
//#define ICC_DEVICE		"/dev/TDA8007"


#define		FIFO_NAME		"/tmp/drv_fifo"


#define	PCD_IOC_POLLING				_IOWR('P', 16, int)
#define	PCD_IOC_POLLEND				_IOW('P', 9, int)
#define MF_CMD_PROCESS				_IOWR('P', 0, DRV)

#define	ICC_IOC_POLLING_START		_IO('I', 16)
#define	ICC_IOC_POLLING_ABORT		_IO('I', 17)

#define IC_CMD_PROCESS				_IOWR('I', 0, DRV)
#define IC_CMD_GETSTATUS			_IOWR('I', 1, DRVOUT)
#define IC_CMD_GETDRVOUT			_IOWR('I', 2, DRVOUT)

extern unsigned int debug;

struct my_input_event {
	struct timeval time;
	unsigned short type;
	unsigned short key_code;
	unsigned int value;
};

static int icc_fd = -1;
static int mifare_fd = -1;

unsigned int wait_event_process(unsigned int driver, NEW_DRV_TYPE *new_drv, unsigned int timeout)
{
	char buf[512];

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

	unsigned int drv_id	= driver;

	if(drv_id > (DRV_TIMEOUT | KEY_DRV| MAG_DRV | MFR_DRV | ICC_DRV))
	{
		new_drv->drv_type = DRV_TIMEOUT;
		return DRV_ID_ERROR;
	}		
	
	if(debug != 0)
	{
		printf("---%s:%d drv_id=%08X---\n",__func__, __LINE__, drv_id);
	}

KEY_UP_CONTINUE:

	max_fd = 0;
	FD_ZERO(&rds);

	if(drv_id &	KEY_DRV)
	{
//		  if(key_fd	== 0)
		{
			key_fd = open(KEY_DEVICE, O_RDWR);
		}

		if(key_fd <	0)
		{
			printf("###	%s open	%s failed ###\n",  __func__, KEY_DEVICE);
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

//			  printf("---%s:%d---\n",__func__, __LINE__);

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
//		  if(mag_fd == 0)
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
		if(mifare_fd < 0)
		{
			mifare_fd = open(MFR_DEVICE, O_RDWR);
			if(mifare_fd < 0)
			{
				printf("###	%s mifare_fd < 0 ###\n", __func__);
				ret = MFR_DRV_ERROR;
				drv_id &= (~MFR_DRV);
				goto __open_mifare_end1;
			}
		}
		ioctl(mifare_fd,PCD_IOC_POLLING, NULL);
		FD_SET(mifare_fd, &rds);
		max_fd += mifare_fd;
	}

__open_mifare_end1:

	if(drv_id & ICC_DRV)
	{
		if(icc_fd < 0)
		{
			icc_fd = open(ICC_DEVICE, O_RDWR);

			if(icc_fd < 0)
			{
				printf("### %s icc_fd < 0 ###\n", __func__);
				ret	= ICC_DRV_ERROR;
				drv_id &= (~ICC_DRV);
				goto __open_icc_end1;
			}
		}

		ioctl(icc_fd,ICC_IOC_POLLING_START);

		FD_SET(icc_fd, &rds);
		max_fd += icc_fd;
	}

__open_icc_end1:

	ts.tv_sec = timeout/1000;
	ts.tv_nsec = (timeout%1000) * 1000000;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	if(debug != 0)
	{
		printf("%s:%d start pid=%d tid=%d timeout=%d\n", __func__, __LINE__, getpid(), pthread_self(), timeout);
	}

	while(1)
	{
		if((ts.tv_sec == 0)	&& (ts.tv_nsec == 0))
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
//						FD_CLR(key_fd, &rds);
//						continue;

						if(drv_id & MFR_DRV)
						{
							ioctl(mifare_fd,PCD_IOC_POLLEND, NULL);
						}

						if(drv_id & ICC_DRV)
						{
							ioctl(icc_fd,ICC_IOC_POLLING_ABORT);
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

//				printf("%s: read fifo_fd ret=%d :%x\n", __func__, ret, buf[0]);

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

//					printf("%s:%d read mag data:\n", __func__, __LINE__);
//					for(i=0; i<ret; i++)
//					{
//						printf("%02X ", buf[i]);
//					}
//					printf("\n");
					
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
			if(FD_ISSET(mifare_fd, &rds))
			{
				ret = read(mifare_fd, buf, 512);
				new_drv->drv_type = MFR_DRV;
				new_drv->drv_data.gen_status = DRV_ENDED;
				if(ret > 0)
				{
					memcpy((char *)&new_drv->drv_data, buf, 256);
					ret = 0;
				}
				else
				{
					printf("%s: read mifare	data fail: %d\n", __func__,	ret);
					ret = MAG_DRV_ERROR;
				}

				break;
			}
		}

		if(drv_id & ICC_DRV)
		{
			if(FD_ISSET(icc_fd, &rds))
			{
				ret = read(icc_fd, buf, 512);

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
		ioctl(mifare_fd,PCD_IOC_POLLEND, NULL);
	}

	if(drv_id & ICC_DRV)
	{
		ioctl(icc_fd,ICC_IOC_POLLING_ABORT);
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

	if(debug != 0)
	{
		printf("%s:%d end pid=%d tid=%d\n", __func__, __LINE__, getpid(), pthread_self());
	}

	return ret;
}

unsigned int wait_nokey_process(unsigned int driver, NEW_DRV_TYPE *new_drv, unsigned int timeout)
{
	char buf[512];

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

	unsigned int drv_id	= driver;

	if(drv_id > (DRV_TIMEOUT | KEY_DRV| MAG_DRV | MFR_DRV | ICC_DRV))
	{
		new_drv->drv_type = DRV_TIMEOUT;
		return DRV_ID_ERROR;
	}
	
	if(debug != 0)
	{
		printf("---%s:%d drv_id=%08X---\n",__func__, __LINE__, drv_id);
	}

KEY_UP_CONTINUE:

	max_fd = 0;
	FD_ZERO(&rds);

	if(drv_id & KEY_DRV)
	{
		if(access(FIFO_NAME, F_OK) == -1)
		{
			ret = mkfifo(FIFO_NAME, 0777);

			if(ret != 0)
			{
				printf("**********%s mkfifo error*********\n", __func__);
				drv_id &= (~KEY_DRV);
				goto __open_key_end1;
			}

//			  printf("---%s:%d---\n",__func__, __LINE__);

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
//		  if(mag_fd == 0)
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
		if(mifare_fd < 0)
		{
			mifare_fd = open(MFR_DEVICE, O_RDWR);
			if(mifare_fd < 0)
			{
				printf("###	%s mifare_fd < 0 ###\n", __func__);
				ret = MFR_DRV_ERROR;
				drv_id &= (~MFR_DRV);
				goto __open_mifare_end1;
			}
		}
		ioctl(mifare_fd,PCD_IOC_POLLING, NULL);
		FD_SET(mifare_fd, &rds);
		max_fd += mifare_fd;
	}

__open_mifare_end1:

	if(drv_id & ICC_DRV)
	{
		if(icc_fd < 0)
		{
			icc_fd = open(ICC_DEVICE, O_RDWR);

			if(icc_fd < 0)
			{
				printf("### %s icc_fd < 0 ###\n", __func__);
				ret	= ICC_DRV_ERROR;
				drv_id &= (~ICC_DRV);
				goto __open_icc_end1;
			}
		}

		ioctl(icc_fd,ICC_IOC_POLLING_START);

		FD_SET(icc_fd, &rds);
		max_fd += icc_fd;
	}

__open_icc_end1:

	ts.tv_sec = timeout/1000;
	ts.tv_nsec = (timeout%1000) * 1000000;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);

	if(debug != 0)
	{
		printf("%s:%d start pid=%d tid=%d timeout=%d\n", __func__, __LINE__, getpid(), pthread_self(), timeout);
	}

	while(1)
	{
		if((ts.tv_sec == 0)	&& (ts.tv_nsec == 0))
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

//				printf("%s: read fifo_fd ret=%d :%x\n", __func__, ret, buf[0]);

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

//					printf("%s:%d read mag data:\n", __func__, __LINE__);
//					for(i=0; i<ret; i++)
//					{
//						printf("%02X ", buf[i]);
//					}
//					printf("\n");

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
			if(FD_ISSET(mifare_fd, &rds))
			{
				ret = read(mifare_fd, buf, 512);
				new_drv->drv_type = MFR_DRV;
				new_drv->drv_data.gen_status = DRV_ENDED;
				if(ret > 0)
				{
					memcpy((char *)&new_drv->drv_data, buf, 256);
					ret = 0;
				}
				else
				{
					printf("%s: read mifare	data fail: %d\n", __func__,	ret);
					ret = MAG_DRV_ERROR;
				}

				break;
			}
		}

		if(drv_id & ICC_DRV)
		{
			if(FD_ISSET(icc_fd, &rds))
			{
				ret = read(icc_fd, buf, 512);

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
		ioctl(mifare_fd,PCD_IOC_POLLEND, NULL);
	}

	if(drv_id & ICC_DRV)
	{
		ioctl(icc_fd,ICC_IOC_POLLING_ABORT);
	}

	if(drv_id & MAG_DRV)
	{
		close(mag_fd);
	}

	if(drv_id & KEY_DRV)
	{
		close(fifo_fd);
	}

	if(debug != 0)
	{
		printf("%s:%d end pid=%d tid=%d\n", __func__, __LINE__, getpid(), pthread_self());
	}

	return ret;
}

unsigned int rfid_ioctl_process(char *data)
{
	if(mifare_fd < 0)
	{
		mifare_fd = open(MFR_DEVICE, O_RDWR);
		if(mifare_fd < 0)
		{
			printf("###	%s mifare_fd < 0 ###\n", __func__);
			return -1;
		}
	}
	
	ioctl(mifare_fd, MF_CMD_PROCESS, (DRV_MMAP *)data);
	
	return 0;
}

unsigned int rfid_read_process(char *data)
{
	if(mifare_fd < 0)
	{
		mifare_fd = open(MFR_DEVICE, O_RDWR);
		if(mifare_fd < 0)
		{
			printf("###	%s mifare_fd < 0 ###\n", __func__);
			return -1;
		}
	}
	
	read(mifare_fd, data, 512);
	
	return 0;
}

unsigned int icc_ioctl_process(char *data)
{
	if(icc_fd < 0)
	{
		icc_fd = open(ICC_DEVICE, O_RDWR);
		if(icc_fd < 0)
		{
			printf("###	%s icc_fd < 0 ###\n", __func__);
			return -1;
		}
	}
	
	ioctl(icc_fd, IC_CMD_PROCESS, (DRV_MMAP *)data);
	
	return 0;
}

unsigned int icc_read_process(char *data)
{
	if(icc_fd < 0)
	{
		icc_fd = open(ICC_DEVICE, O_RDWR);
		if(icc_fd < 0)
		{
			printf("###	%s icc_fd < 0 ###\n", __func__);
			return -1;
		}
	}
	
	read(icc_fd, data, 512);
	
	return 0;
}
