#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <SDL_image.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <osdriver.h>
#include <sand_debug.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

int lcd_message_id = 18888;

key_t lcd_key = 1300;

#define nbytes 140

struct gif_loading_struct
{
	int x;
	int y;
	int w;
	int h;
	int stop;
	char name[128];
	int tx;
	int ty;
	int tw;
	int th;
	char text[512];
	char style[128];
	int tposition;
};

struct message_data {

    int api_id;

    int line;
    int colum;

    int gprs_d;
    int gprs_s;
    int modem_dial;
    int r1;
    int r2;
    int r3;
    int r4;

	int widget_type;
	int widget_key;
	int widget_info;
	
    unsigned char mtext[100];


};





struct lcd_messaage {


    long  mtype;
    struct message_data msg_data;

};




#define JN24 0

void lcd_init()
{
/*
    if ((lcd_message_id = msgget(lcd_key, 0777 | IPC_CREAT)) < 0){
        perror("message create");
        exit(-1);
    };
*/
again:
    /*get the lcd icon key*/
    if ((lcd_message_id = msgget(lcd_key ,0777))<0)  			/*creat a new message array!!*/
    {
        sleep(1);
//        printf("get lcd key failed!, %s %d\n",__func__, __LINE__);
        goto again;
    }

    //new add for JN24
    //zy 0311
#if 0//JN24
    Os__lcd_image(64,0, "null.bmp");
    Os__lcd_image(231,0, "logo.jpg");
#endif
}



unsigned char Os__GB2312_disp(unsigned char ucRow, unsigned char *pucPtr)
{

    struct lcd_messaage msg;

    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.msg_data.api_id = 0;
    msg.msg_data.line = ucRow;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    strncpy((char*)(msg.msg_data.mtext),(char*)(pucPtr), lt-1);
    msg.mtype = 50;

    if (pucPtr == NULL) {

        Uart_Printf("text is null, illegal\n");
        //exit(1);
        return 1;

    }

    if (*pucPtr == 0)
        return 1;

    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
            printf("%s %d\n", __func__, __LINE__);
            perror("message send");
            //exit(-1);
            return 1;
    }
    return 0;

}


void Os__GB2312_display(unsigned char line, unsigned char column, unsigned char *t)
{

    struct lcd_messaage msg;

    if (t == NULL) {

        Uart_Printf("text is null, illegal\n");
        //exit(1);
                return;

    }

    if (*t == 0)
                return;

    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.msg_data.api_id = 1;
    msg.msg_data.line = line;
    msg.msg_data.colum = column;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    strncpy((char*)msg.msg_data.mtext, (char*)t, lt-1);
    msg.mtype = 50;



    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
            printf("%s %d\n", __func__, __LINE__);
            perror("message send");
            //exit(-1);
                        return;
    }

        return;
}


void Os__display(unsigned line, unsigned colum, char *t)
{

    struct lcd_messaage msg;

    if (t == NULL) {

        Uart_Printf("text is null, illegal\n");
                return;

    }

    if (*t == 0)
                return;

    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.msg_data.api_id = 2;
    msg.msg_data.line = line;
    msg.msg_data.colum = colum;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    strncpy((char*)(msg.msg_data.mtext), (char*)t, lt-1);
    msg.mtype = 50;



    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
            printf("%s %d\n", __func__, __LINE__);
            perror("message send");
                        return;
    }
        return;
}


void Os__clr_display(unsigned char line)
{
    struct lcd_messaage msg;
    msg.msg_data.api_id = 3;
    msg.msg_data.line = line;
    msg.mtype = 50;
	msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;


    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
                printf("%s %d\n", __func__, __LINE__);
            perror("message send");
            //exit(-1);
            return;
    }
    return;
}


void Os__lcd_test()
{
    struct lcd_messaage msg;
    msg.msg_data.api_id = 12;
    msg.msg_data.line = 1;
	msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    msg.mtype = 50;



    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
                printf("%s %d\n", __func__, __LINE__);

            perror("message send");
            exit(-1);
    }

}

void Os__update_display()
{
    struct lcd_messaage msg;
    msg.msg_data.api_id = 6;
    msg.mtype = 50;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;

    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){

            printf("%s %d\n", __func__, __LINE__);
            perror("message send");
                        return;
    }
}


unsigned char Os__graph_display(unsigned line, unsigned char colum, unsigned char *drawing, unsigned char size)
{
    struct lcd_messaage msg;

    if (drawing == NULL)
    {

        Uart_Printf("[%s:%d] text is null, illegal\n",__func__,__LINE__);
        exit(1);

    }

    Uart_Printf("drawing is %s\n", drawing);


    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.msg_data.api_id = 7;
    msg.msg_data.line = line;
    msg.msg_data.colum = colum;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    msg.mtype = 50;
    strncpy((char*)msg.msg_data.mtext,(char*)drawing, lt-1);


    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
    {
        Uart_Printf("[%s:%d] message send fail \n",__func__,__LINE__);
            perror("message send");
            exit(-1);
    }
    return 0;
}

unsigned char Os__display_qt(unsigned char ucFont, unsigned char line, unsigned char colum, unsigned char * t, int widget_type, int widget_key, int widget_info)
{
	struct lcd_messaage msg;

	
    if (t == NULL) {
        Uart_Printf("[%s:%d] text is null, illegal\n",__func__,__LINE__);
        return 0;
    }

    if (*t == 0)
        return -1;


    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.msg_data.api_id = 8;
    msg.msg_data.line = line;
    msg.msg_data.colum = colum;
    msg.msg_data.widget_type = widget_type;
    msg.msg_data.widget_key = widget_key;
    msg.msg_data.widget_info = widget_info;
    strncpy((char*)msg.msg_data.mtext, (char*)t, lt-1);
    msg.mtype = 50;



    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
            printf("%s %d\n", __func__, __LINE__);

            perror("message send");
            exit(-1);
    }
    return 0;
}

unsigned char OSMMI_DisplayASCII (unsigned char ucFont, unsigned char line, unsigned char colum, unsigned char * t)
{
    struct lcd_messaage msg;

    if (t == NULL) {
        Uart_Printf("[%s:%d] text is null, illegal\n",__func__,__LINE__);
        exit(1);
    }

    if (*t == 0)
        return -1;



    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.msg_data.api_id = 8;
    msg.msg_data.line = line;
    msg.msg_data.colum = colum;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    strncpy((char*)msg.msg_data.mtext, (char*)t, lt-1);
    msg.mtype = 50;



    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
            printf("%s %d\n", __func__, __LINE__);

            perror("message send");
            exit(-1);
    }
    return 0;

}


void OSMMI_ClrDisplay(unsigned char ucFont, unsigned char line)
{

    struct lcd_messaage msg;
    msg.msg_data.api_id = 9;
    msg.msg_data.line = line;
    msg.mtype = 50;
	msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;


    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
            printf("%s %d\n", __func__, __LINE__);

            perror("message send");
            exit(-1);
    }

}

/*
*  flag = 1 modem icon is on
*  flag = 0 modem icon is off
*/
void Os__lcd_modem_on_off(char flag)
{
    struct lcd_messaage msg;

    //Uart_Printf("%s:%d\n",__FUNCTION__,__LINE__);
    msg.mtype = 50;
    msg.msg_data.api_id = 11;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    msg.msg_data.modem_dial = flag;
    if (msgsnd(lcd_message_id, &msg, 140, 0) < 0 ){
        perror("XXXXX message send");
        exit(-1);
    }
}

/*
*
*
*/
void Os__lcd_image(int x,int y,char *file)
{

    struct lcd_messaage msg;

    //Uart_Printf("%s:%d\n",__FUNCTION__,__LINE__);

    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.mtype = 50;
    msg.msg_data.api_id = 13;
    msg.msg_data.colum = y;
    msg.msg_data.line = x;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    strncpy((char*)msg.msg_data.mtext,(char*)file,lt-1);

    if (msgsnd(lcd_message_id, &msg, 140, 0) < 0 ){
        perror("XXXXX message send");
        exit(-1);
    }
}

void Os__lcd_image_random(int x,int y,char *file)
{
    struct lcd_messaage msg;

    memset(&msg , 0, sizeof(msg));
    int lt = sizeof(msg.msg_data.mtext);

    msg.mtype = 50;
    msg.msg_data.api_id = 14;
    msg.msg_data.colum = y;
    msg.msg_data.line = x;
    msg.msg_data.widget_type = 0;
    msg.msg_data.widget_key = NULL;
    msg.msg_data.widget_info = 0;
    strncpy((char*)msg.msg_data.mtext,(char*)file, lt-1);

    if (msgsnd(lcd_message_id, &msg, 140, 0) < 0 )
    {
        perror("XXXXX message send");
        exit(-1);
    }
}

/**************************************************************
*   Ignor Pinpad
*   Liuguoming 2010-03-24
***************************************************************
*/
void Os__display_pp(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
}

void Os__clr_display_pp(unsigned char ucRow)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
}

unsigned char Os__light_on_pp(void)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return 0;
}

unsigned char Os__light_off_pp(void)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return 0;
}

void Os__GB2312_display_pp(unsigned char ucRow)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
}

void Os__beep_pp(void)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
}

void Os__crypt_pp(unsigned char *pucCryptKey)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
}

/*
 *    Pinpad Keyboard
 */
DRV_OUT* Os__get_key_pp(void)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return ((DRV_OUT*) 0);
}

DRV_OUT* Os__get_varkey_pp(unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr,unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return ((DRV_OUT*) 0);
}

DRV_OUT* Os__get_fixkey_pp(unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return ((DRV_OUT*) 0);
}

DRV_OUT* Os__get_pin_pp(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return ((DRV_OUT*) 0);
}

unsigned char Os__xget_key_pp(void)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return  0;
}

unsigned char* Os__xget_varkey_pp(unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr,unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return NULL;
}

unsigned char* Os__xget_fixkey_pp(unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return NULL;
}

unsigned char* Os__xget_pin_pp(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return NULL;
}

void Os__crypt(unsigned char *pucCryptKey)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
}

DRV_OUT* Os__get_fixkey(unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return ((DRV_OUT*) 0);
}

DRV_OUT* Os__get_pin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return ((DRV_OUT*) 0);
}

unsigned char   Os__read_eeprom(unsigned short start,unsigned char length,unsigned char *pt)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
    return 0;
}

void OSAPP_ChangeName(unsigned char *pucName)
{
    Uart_Printf("%s:no pinpad ! \n",__func__);
}


#define SERVPORT 3333
#define MAXDATASIZE 100 /*每次最大数据传输量 */


#define LOADING "loading"
#define GIFSTOP "gifstop"

struct gif_loading_struct g_gif_content;

void Os__gif_start(char *name, int x, int y, int w, int h)
{
	char path_buf[128];
	memset(path_buf, 0, 128);
	getcwd(path_buf, 128);
	
	memset(g_gif_content.name, 0, 128);
	strcat(g_gif_content.name, path_buf);
	strcat(g_gif_content.name, "/image/");
	strcat(g_gif_content.name, name);

	g_gif_content.x = x;
	g_gif_content.y = y;
	g_gif_content.w = w;
	g_gif_content.h = h;
	g_gif_content.stop = 0;
	g_gif_content.tposition = 0;
	g_gif_content.text[0] = '\0';
	Os__socket_client((char *)&g_gif_content);
}

void Os__gif_text_start(char *name, int x, int y, int w, int h, char *text, int tx, int ty, int tw, int th, char *style)
{
	char path_buf[128];
	memset(path_buf, 0, 128);
	getcwd(path_buf, 128);
	
	memset(g_gif_content.name, 0, 128);
	strcat(g_gif_content.name, path_buf);
	strcat(g_gif_content.name, "/image/");
	strcat(g_gif_content.name, name);

	g_gif_content.x = x;
	g_gif_content.y = y;
	g_gif_content.w = w;
	g_gif_content.h = h;
	g_gif_content.stop = 0;
	g_gif_content.tx = tx;
	g_gif_content.ty = ty;
	g_gif_content.tw = tw;
	g_gif_content.th = th;
	g_gif_content.tposition = 0;
	if (text) {
		if (strlen(text) > 512)
			text[512] = '\0';
		strcpy(g_gif_content.text, text);		
	}
	if (style) {
		if (strlen(style) > 128)
			text[128] = '\0';
		strcpy(g_gif_content.style, style);		
	}
	Os__socket_client(&g_gif_content);
	//printf("xxxxxxxxxxx %s\n", g_gif_content.text);
}

void Os__gif_text_auto_start(char *name, char *text, int textPosition, char *style)
{
	char path_buf[128];
	memset(path_buf, 0, 128);
	getcwd(path_buf, 128);
	
	memset(g_gif_content.name, 0, 128);
	strcat(g_gif_content.name, path_buf);
	strcat(g_gif_content.name, "/image/");
	strcat(g_gif_content.name, name);
	
	g_gif_content.stop = 0;
	g_gif_content.tposition = textPosition;
	if (text) {
		if (strlen(text) > 512)
			text[512] = '\0';
		strcpy(g_gif_content.text, text);		
	}
	if (style) {
		if (strlen(style) > 128)
			text[128] = '\0';
		strcpy(g_gif_content.style, style);		
	}
	Os__socket_client(&g_gif_content);
}

void Os__gif_stop(void)
{
	memset(g_gif_content.name, 0, 128);
	g_gif_content.x = 0;
	g_gif_content.y = 0;
	g_gif_content.w = 0;
	g_gif_content.h = 0;
	g_gif_content.stop = 1;
	Os__socket_client(&g_gif_content);
}

void Os__socket_client(char *content)
{
	int sockfd, recvbytes;
	char buf[MAXDATASIZE];
	struct sockaddr_in serv_addr;
	struct in_addr serv_ip;

	if (0 == inet_aton("127.0.0.1", &serv_ip))
	{
		perror("inet_aton error！\n");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket创建出错！\n");
		close(sockfd);
		exit(1);
	}

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERVPORT);
	serv_addr.sin_addr = serv_ip;
	bzero(&(serv_addr.sin_zero),8);
	
	if (connect(sockfd, (struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1) 
	{
		perror("connect出错！\n");
		close(sockfd);
		exit(1);
	}
	
	/****************发送数据******************/
	if (send(sockfd, content, sizeof(struct gif_loading_struct), 0) == -1)
	{
		perror("send出错！\n");
		close(sockfd);
		exit(0);
	}
	
	/****************发送接收数据end******************/
	close(sockfd);	
		
	/****************接收数据******************
	if ((recvbytes=recv(sockfd, buf, MAXDATASIZE, 0)) ==-1) 
	{
		perror("recv出错！\n");
		exit(1);
	}
	buf[recvbytes] = '\0';
	printf("Received: %s\n",buf);
	close(sockfd);
	****************接收数据end******************/
}
/////////////////////// END //////////////////////
