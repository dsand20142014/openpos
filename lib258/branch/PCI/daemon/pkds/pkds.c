#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>

#include "osdrv.h"
#include "pkds.h"
#include "drvs.h"

const char * SERVER_NAME = "/tmp/pkds.sock";
const char * DEBUG_STR = "dbg=1";

unsigned int debug = 0;

#define DEBUG_OUT(dbg) do{ if(dbg) printf("****%s,%d****\n", __func__, __LINE__); }while(0)

#define MAX_CONNECTION_NUMBER 1

static int unix_socket_listen(const char *servername)
{
	int fd;
	int len, rval;
	int err;
	struct sockaddr_un un;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}

	unlink(servername);

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, servername);

	len = offsetof(struct sockaddr_un, sun_path) + strlen(servername);

	if (bind(fd, (struct sockaddr *)&un, len) < 0)
	{
		rval = -2;
	}
	else
	{
		if (listen(fd, MAX_CONNECTION_NUMBER) < 0)
		{
			rval =	-3;
		}
		else
		{
			return fd;
		}
	}

	err = errno;
	close(fd);
	errno = err;

	return rval;
}

const char * head_str = "/tmp/socket.";
static int unix_socket_accept(int listenfd, uid_t *uidptr)
{
	int clifd;
	int len, rval;
	int err;
	time_t staletime;
	struct sockaddr_un un;
	struct stat statbuf;
	char *str = NULL;

	memset(&un, 0, sizeof(un));

	len = sizeof(un);

	if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0)
	{
	   return(-1);
	}

	len -= offsetof(struct sockaddr_un, sun_path);
	un.sun_path[len] = 0;
	if (stat(un.sun_path, &statbuf) < 0)
	{
		rval = -2;
	}
	else
	{
		if (S_ISSOCK(statbuf.st_mode) )
		{
			if (uidptr != NULL)
			{
				str = strstr(un.sun_path, head_str);

				if (str != NULL)
				{
					str += strlen(head_str);
					*uidptr = atoi(str);
				}
				else
				{
					*uidptr = 0;
				}
			}

			if(debug != 0)
			{
				printf("connectting client = %s\n", un.sun_path);
			}

			unlink(un.sun_path);

			return clifd;
		}
		else
		{
			rval = -3;
		}
	}

	err = errno;
	close(clifd);
	errno = err;

	return(rval);
}

static void unix_socket_close(int fd)
{
   close(fd);
}

static void socket_error(int signo)
{
	printf("pid=%d - caught signal: %s\n", getpid(), strsignal(signo));
}

int verify_pid(int pid)
{

	return 0;
}

int verify_signature(char *filedata)
{

	return 0;
}

int get_cmd_len(int cmd)
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

int driver_process(int cmd, char *data)
{
	int drvid;
	int timeout;
	int ret = -1;
	NEW_DRV_TYPE new_drv;

	switch(cmd)
	{
		case WAIT_NOKEY:
		{
			memcpy(&drvid, data, sizeof(int));
			memcpy(&timeout, data+sizeof(int), sizeof(int));

			if(debug != 0)
			{
				printf("****%s,%d drvid=%08x****\n", __func__, __LINE__, drvid);
				printf("****%s,%d timeout=%d****\n", __func__, __LINE__, timeout);
			}

			ret = wait_nokey_process(drvid, &new_drv, timeout);

			if(ret == 0)
			{
				memcpy(data, &new_drv, sizeof(new_drv));
			}
			break;
		}

		case WAIT_EVENT:
		{
			memcpy(&drvid, data, sizeof(int));
			memcpy(&timeout, data+sizeof(int), sizeof(int));

			if(debug != 0)
			{
				printf("****%s,%d drvid=%08x****\n", __func__, __LINE__, drvid);
				printf("****%s,%d timeout=%d****\n", __func__, __LINE__, timeout);
			}

			ret = wait_event_process(drvid, &new_drv, timeout);

			if(ret == 0)
			{
				memcpy(data, &new_drv, sizeof(new_drv));
			}
			break;
		}

		case RFID_IOCTL:
		{
			ret = rfid_ioctl_process(data);
			break;
		}

		case RFID_READ:
		{
			ret = rfid_read_process(data);
			break;
		}

		case ICC_IOCTL:
		{
			ret = icc_ioctl_process(data);
			break;
		}

		case ICC_READ:
		{
			ret = icc_read_process(data);
			break;
		}

		default:
		{
			break;
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	int listenfd;
	int connfd;
	int cmd, pid;
	int to_send,sent;
	int size, temp, len;
	int ret, i;
	char buf[2048];
	char head[16];

	if(argc > 1)
	{
		if(strncmp(argv[1], DEBUG_STR, strlen(DEBUG_STR)) == 0)
		{
			debug = 1;
		}
	}

	signal(SIGPIPE, socket_error);

	listenfd = unix_socket_listen(SERVER_NAME);
	if(listenfd<0)
	{
		printf("pkds error[%d] when listening, please restart.\n",errno);
		return 0;
	}

	while(1)
	{
		connfd = unix_socket_accept(listenfd, &pid);

		if(debug != 0)
		{
			printf("connectting client pid = %d\n", pid);
		}

		if(pid <= 0)
		{
			printf("connectting client pid invalid\n");
			unix_socket_close(connfd);
			continue;
		}

		if(connfd < 0)
		{
			printf("Error[%d] when accepting...\n",errno);
			continue;
		}

		DEBUG_OUT(debug);

		//receive cmd+pid
		size = recv(connfd, buf, sizeof(int)*2, 0);

		if(size == -1)
		{
			printf("pkds error[%d] when receive cmd+pid:%s\n",errno,strerror(errno));
			unix_socket_close(connfd);
			continue;
		}

		memcpy(&cmd, buf, sizeof(int));
		memcpy(&pid, buf+sizeof(int), sizeof(int));

		if(debug != 0)
		{
			printf("****%s,%d cmd=%x****\n", __func__, __LINE__, cmd);
			printf("****%s,%d pid=%d****\n", __func__, __LINE__, pid);
		}

		if(verify_pid(pid) == 0)
		{
			temp = PID_VERIFIED;
		}
		else
		{
			temp = 0xFFFF;
		}
		memcpy(buf, &temp, sizeof(int));
		//send pid verification result
		size = send(connfd, buf, sizeof(int), 0);
		if(size == -1)
		{
			printf("pkds error[%d] when send pid verification result:%s.\n",errno, strerror(errno));
			unix_socket_close(connfd);
			continue;
		}

		DEBUG_OUT(debug);

		if(temp != PID_VERIFIED)
		{
			//receive file + signature
			size = recv(connfd, buf, sizeof(int)*2, 0);

			if(size == -1)
			{
				printf("pkds error[%d] when receive file signature data:%s.\n",errno,strerror(errno));
				unix_socket_close(connfd);
				continue;
			}

			DEBUG_OUT(debug);

			if(verify_signature(buf) != 0)
			{
				memcpy(buf, &temp, sizeof(int));
				//send file verification result
				size = send(connfd, buf, sizeof(int), 0);
				if(size == -1)
				{
					printf("pkds error[%d] when send file verification :%s.\n",errno, strerror(errno));
					unix_socket_close(connfd);
					continue;
				}

				if(debug != 0)
				{
					printf("****%s,%d****\n", __func__, __LINE__);
				}
			}
		}

		len = get_cmd_len(cmd);
		if(len == 0)
		{
			printf("****%s,%d cmd error****\n", __func__, __LINE__);
			unix_socket_close(connfd);
			continue;
		}

		//receive data
		size = recv(connfd, buf, len, 0);

		if(size == -1)
		{
			printf("pkds error[%d] when receive data:%s.\n",errno,strerror(errno));
			unix_socket_close(connfd);
			continue;
		}

		if(driver_process(cmd, buf) == 0)
		{
			temp = FUNC_SUCCESS;
		}
		else
		{
			temp = 0xFFFF;
		}

		memcpy(head, &temp, sizeof(int));
		//send function process result
		size = send(connfd, head, sizeof(int), 0);
		if(size == -1)
		{
			printf("pkds error[%d] when %s send function process result:%s.\n",errno,__func__, strerror(errno));
			unix_socket_close(connfd);
			continue;
		}

		DEBUG_OUT(debug);

		if(temp != FUNC_SUCCESS)
		{
			//do not send data
			unix_socket_close(connfd);
			continue;
		}

		//send data
		to_send = len;
		sent = 0;
		size = 0;
		do
		{
			size = send(connfd, buf+sent, to_send-sent, 0);
			if(size == -1)
			{
				printf("pkds error[%d] when send data:%s.\n",errno, strerror(errno));
				break;
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

		DEBUG_OUT(debug);

		if(debug != 0)
		{
			printf("%s:%d pkds send data:\n", __func__, __LINE__);
			for(i=0; i<32; i++)
			{
			    printf("%02X ", buf[i]);
			}
			printf("\n");
		}

		unix_socket_close(connfd);

	}

	return 0;
}

