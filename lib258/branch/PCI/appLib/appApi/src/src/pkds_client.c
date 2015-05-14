#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "osdrv.h"
#include "fsync_drvs.h"
#include "pkds_client.h"

#define		CLIENT_TEST		0

const char * SERVER_NAME = "/tmp/pkds.sock";

static int unix_socket_conn(const char *servername)
{
	int fd;
	int len, rval;
	int err;
	int pid;
	struct sockaddr_un un;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd < 0)
	{
		printf("client socket failed\n");

		return(-1);
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	pid = getpid();
	sprintf(un.sun_path, "/tmp/socket.%05d", pid);

#if	CLIENT_TEST
	printf("client pid=%05d len=%d\n", pid, strlen(un.sun_path));
#endif

	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

	unlink(un.sun_path);

	if (bind(fd, (struct sockaddr *)&un, len) != 0)
	{
#if	CLIENT_TEST
		printf("client bind failed\n");
#endif

		printf("Error[%d] when bind :%s.\n",errno,strerror(errno));

		rval= -2;
	}
	else
	{
		memset(&un, 0, sizeof(un));
		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, servername);
		len = offsetof(struct sockaddr_un, sun_path) + strlen(servername);
		if (connect(fd, (struct sockaddr *)&un, len) < 0)
		{
			printf("client connect failed\n");

			rval= -4;
		}
		else
		{
			return (fd);
		}
	}

	err = errno;
	close(fd);
	errno = err;

	return rval;
}

static void unix_socket_close(int fd)
{
	close(fd);
}

const char *EXE_FILE = "exe";
const char *CWD_FILE = "cwd";
int get_caller_name(char *name)
{
     char path[512]={0};
     char filepath[512]={0};
     char oldpath[512]={0};

     int fd;
     int i,filesize;

     memset(oldpath, 0 ,sizeof(oldpath));
     getcwd(oldpath, sizeof(oldpath));

     sprintf(filepath, "/proc/%d", getpid());

     if( -1 != chdir(filepath))
     {
     	if( 0 < readlink(EXE_FILE, path ,511) )
     	{
     		strcpy(name, path);

#if	CLIENT_TEST
     		printf("app caller is:%s\n", path);
#endif
     		chdir(oldpath);
     		memset(path, 0 ,sizeof(path));
     		getcwd(path, sizeof(path));

#if	CLIENT_TEST
     		printf("return path,now:%s\n", path);
#endif
     	}
     	else
     	{
     		return -1;
     	}
     }
     else
     {
     	return -1;
     }

     return 0;
}

static void socket_error(int signo)
{
	printf("pid=%d - caught signal: %s\n", getpid(), strsignal(signo));
}

static int get_cmd_len(int cmd)
{
	int len = 0;

	switch(cmd)
	{
		case RFID_IOCTL:
		case ICC_IOCTL:
		{
			len = sizeof(DRV_MMAP);
			break;
		}

		case RFID_READ:
		case ICC_READ:
		{
			len = sizeof(DRVOUT);
			break;
		}

		case WAIT_EVENT:
		case WAIT_NOKEY:
		{
			len = sizeof(NEW_DRV_TYPE);
			break;
		}

		default:
		{
			break;
		}
	}

	return len;
}

int pkds_exchange(int cmd, char *input, char *output)
{
	static int init_status = 0;
	int connfd;
	int len, size, i;
	int to_send, sent;
	int to_recv, recved;
	int temp, retry=2;
	char *buf = NULL;
	char head[16];

	buf = (char *)malloc(2048);

	if(buf == NULL)
	{
		return -1;
	}

	if(init_status == 0)
	{
		init_status = 0xFFFF;
		signal(SIGPIPE, socket_error);
	}

	connfd = unix_socket_conn(SERVER_NAME);
	if(connfd<0)
	{
		printf("%s error[%d] when connecting...\n",__func__, errno);

		sleep(5);

		goto __error;
	}

	memcpy(head, &cmd, sizeof(int));
	temp = getpid();
	memcpy(head+sizeof(int), &temp, sizeof(int));

#if	CLIENT_TEST
	printf("****%s,%d****\n", __func__, __LINE__);
#endif

	//send cmd+pid
	size = send(connfd, head, sizeof(int)*2, 0);
	if(size == -1)
	{
		printf("%s error[%d] when send cmd+pid:%s.\n", __func__, errno, strerror(errno));
		goto __error;
	}

#if	CLIENT_TEST
	printf("****%s,%d****\n", __func__, __LINE__);
#endif

	do
	{
		//receive pid verification result
		size = recv(connfd, &temp, sizeof(int), 0);
		if(size == -1)
		{
			printf("%s error[%d] when recv pid verify result:%s.\n", __func__, errno, strerror(errno));
			goto __error;
		}

#if	CLIENT_TEST
		printf("****%s,%d ret=%x****\n", __func__, __LINE__, temp);
#endif

		if(temp != PID_VERIFIED)
		{
			if(retry == 0)
			{
				goto __error;
			}

			//send file + signature
			size = send(connfd, head, sizeof(int)*2, 0);
			if(size == -1)
			{
				printf("%s error[%d] when send file+signature:%s.\n", __func__, errno, strerror(errno));
				goto __error;
			}
		}
		else
		{
			break;
		}
	}while(retry--);

#if	CLIENT_TEST
	printf("****%s,%d****\n", __func__, __LINE__);
#endif

	len = get_cmd_len(cmd);
	if(len == 0)
	{
		printf("****%s,%d cmd error****\n", __func__, __LINE__);
		unix_socket_close(connfd);
		goto __error;
	}
	
	memcpy(buf, input, len);

	//send data
	to_send = len;
	sent = 0;
	size = 0;
	do
	{
		size = send(connfd, buf+sent, to_send-sent, 0);
		if(size == -1)
		{
			printf("%s error[%d] when send data:%s.\n",__func__, errno, strerror(errno));
			goto __error;
		}

		sent += size;
		if(sent == to_send)
		{
			break;
		}
		else
		{
			usleep(10000);
		}
	}while (1);

#if	CLIENT_TEST
	printf("****%s,%d****\n", __func__, __LINE__);
#endif

	//receive function process result
	size = recv(connfd, &temp, sizeof(int), 0);
	if(size == -1)
	{
		printf("Error[%d] when %s recv function process:%s.\n",errno,__func__, strerror(errno));
		goto __error;
	}

#if	CLIENT_TEST
	printf("****%s,%d****\n", __func__, __LINE__);
#endif

	if(temp != FUNC_SUCCESS)
	{
		goto __error;
	}

	//receiv data
	to_recv = len;
	recved = 0;
	size = 0;
	do
	{
		size = recv(connfd, buf+recved, to_recv-recved, 0);
		if(size == -1)
		{
			printf("%s error[%d] when recv function process:%s.\n",__func__, errno, strerror(errno));
			goto __error;
		}
		recved += size;
		if(recved == to_recv)
		{
			break;
		}
		else
		{
			usleep(10000);
		}
	}while (1);

#if	CLIENT_TEST

	printf("****%s,%d****\n", __func__, __LINE__);

	printf("%s:%d receive pkds data:\n", __func__, __LINE__);
	for(i=0; i<32; i++)
	{
	    printf("%02X ", buf[i]);
	}
	printf("\n");

#endif

	unix_socket_close(connfd);

	memcpy(output, buf, len);

	free(buf);

#if	CLIENT_TEST
	printf("****%s,%d****\n", __func__, __LINE__);
#endif

	return 0;

__error:

#if	CLIENT_TEST
	printf("****%s,%d****\n", __func__, __LINE__);
#endif

	unix_socket_close(connfd);

	free(buf);

	return -1;
}

