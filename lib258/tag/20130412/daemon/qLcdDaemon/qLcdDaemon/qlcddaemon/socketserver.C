#include "socketserver.h"
#include "osdriver.h"
#include "fsync_drvs.h"
#include "osevent.h"
#include "sand_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define SERVPORT 3333 /*服务器监听端口号 */
#define BACKLOG 10 /* 最大同时连接请求数 */


#define LOADING "loading"
#define GIFSTOP "gifstop"

void SocketServerThread::run()
{
    int sockfd,client_fd; /* sock_fd：监听socket；client_fd：数据传输socket */
    struct sockaddr_in my_addr; /* 本机地址信息 */
    struct sockaddr_in remote_addr; /* 客户端地址信息 */
    int sin_size, recvbytes;


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket创建出错！\n"); exit(1);
    }

    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(SERVPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind出错！\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen出错！\n");
        exit(1);
    }
    system("ifconfig lo up");
    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, (socklen_t *)&sin_size)) == -1)
        {
            perror("accept出错\n");
            continue;
        }
//        printf("received a connection from %d \n", inet_ntoa(remote_addr.sin_addr));


        /****************发送数据******************
        if (!fork())
        {
            // 子进程代码段
            if (send(client_fd, "Hello, you are connected from server! ", 100, 0) == -1)
            {
                perror("send出错！\n");
                close(client_fd);
                exit(0);
            }
        }
        ****************发送接收数据end******************/

        /****************接收数据******************/
        if ((recvbytes=recv(client_fd, g_gif_content, sizeof(struct gif_loading_struct), 0)) ==-1)
        {
            perror("recv出错！\n");
            exit(1);
        }
        g_gif_content[recvbytes] = '\0';
        emit gifLoadingSign((struct gif_loading_struct *)g_gif_content);
        /****************接收数据end******************/

        close(client_fd);
    }
}
