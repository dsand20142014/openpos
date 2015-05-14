#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <linux/rtc.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <error.h>
#include <sys/time.h>
#include <pthread.h>
#include "ostime.h"

static void change_system_date(unsigned char *d);
static void sync_rtc_time();
static unsigned char Os__write_time_wrap(unsigned char *ptTime);
static unsigned char Os__write_date_wrap(unsigned char *ptDate);
static int   timer_new_thread(void);
static void* timer_handle(void*);
static void* timer_function_handle(void*);
static int timer_start(unsigned char type,volatile unsigned int *ptimer);
static int timer_stop(volatile unsigned int *ptimer);
static inline void os_sleep(__time_t sec,__suseconds_t us);


int  g_timer_cnt;
int  g_inter_timer_cnt;

struct timer
{
    unsigned char type;
    int *ptimer;
};

struct inter_timer
{
    int *ptimer;
    void (*timer_proc)(void);
};


struct timer timer_tbl[MAXNUM_TIMER];
struct inter_timer inter_timer_tbl[MAXNUM_INTER_TIMER];

static inline void os_sleep(__time_t sec,__suseconds_t us)
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = us;
    select(0, NULL, NULL, NULL, &tv);
}

static void change_system_date(unsigned char *d)
{
	char cmd[128];

	memset(cmd, 0, 128);
	sprintf(cmd, "/bin/date -s %s",d);
	system(cmd);
}

static void sync_rtc_time()
{
    char cmd[128];

    memset(cmd, 0, 128);
    sprintf(cmd, "/sbin/hwclock -w");
    system(cmd);
}

void Os__read_date (unsigned char *ptDate)
{
    unsigned char Ptmp[20];

    memset(Ptmp,0x00,sizeof(Ptmp));

    Os__read_date_time(Ptmp);
    sprintf(ptDate,"%s",Ptmp+4);
}

void Os__read_date_format (unsigned char *ptDate)
{
    unsigned char Ptmp[10];
    unsigned int i=0;

    memset(Ptmp,0x00,sizeof(Ptmp));

    Os__read_date(Ptmp);
    while((*ptDate++ = Ptmp[i++])!='\0')
    {
        if(2==i || 4==i)
            *ptDate++ = '/';
    }
}

void Os__read_date_time(unsigned char *ptDateTime)
{
    unsigned char Ptmp[20];
    unsigned int i = 0;

    memset(Ptmp,0x00,sizeof(Ptmp));

    Os__read_date_time_all(Ptmp);
    while((*ptDateTime++ = Ptmp[i++]) != '\0')
    {
        if(4 == i)
            i += 2;
    }

}

void Os__read_date_time_all(unsigned char *ptDateTime)
{
    unsigned char tm_mday[5], tm_sec[5], tm_mon[5], tm_year[5], tm_min[5],tm_hour[50],getDate[40];
    char year;
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);

    memset(getDate, 0, 40);
    memset(tm_mday,0,5);
    memset(tm_mon,0,5);
    memset(tm_year,0,5);
    memset(tm_hour,0,50);
    memset(tm_min,0,5);
    memset(tm_sec,0,5);


    p->tm_mon++;
	//处理2000年以后p->tm_year大于100的问题
	if(p->tm_year >= 100)
    	p->tm_year -= 100;

    sprintf(tm_hour, "%02d", p->tm_hour);
    sprintf(tm_min, "%02d", p->tm_min);
    sprintf(tm_sec, "%02d", p->tm_sec);
    sprintf(tm_mday, "%02d", p->tm_mday);
    sprintf(tm_mon, "%02d", p->tm_mon);
    sprintf(tm_year, "%02d", p->tm_year);


    strcat(tm_hour, tm_min);
    strcat(tm_hour, tm_sec);
    strcat(tm_hour, tm_mday);
    strcat(tm_hour, tm_mon);
    strcat(tm_hour, tm_year);
    strcpy(getDate, tm_hour);
    memcpy(ptDateTime, getDate, 13);
}

void Os__read_time (unsigned char *ptTime)
{
    Os__read_time_sec(ptTime);
    ptTime[4] = '\0';
}

void Os__read_time_format (unsigned char *ptTime)
{
    unsigned char Ptmp[10];
    unsigned int i=0;

    memset(Ptmp,0x00,sizeof(Ptmp));

    Os__read_time(Ptmp);
    while((*ptTime++ = Ptmp[i++]) != '\0')
    {
        if(2 == i)
            *ptTime++ = ':';
    }
}

void Os__read_time_sec (unsigned char *ptTime)
{
    unsigned char Ptmp[20];

    memset(Ptmp,0x00,sizeof(Ptmp));

    Os__read_date_time_all(Ptmp);
    memcpy(ptTime,Ptmp,6);
    ptTime[6] = '\0';

}

static unsigned char Os__write_date_wrap(unsigned char *ptDate)
{

    unsigned char tm_mday[5], tm_sec[5], tm_mon[5], tm_year[5], tm_min[5],tm_hour[50],getDate[40];
    char year;
    time_t timep;
    struct tm *p;
    char temp[50];

    strcpy(temp, ptDate);

    time(&timep);
    p = localtime(&timep);

    memset(getDate, 0, 40);
    memset(tm_mday,0,5);
    memset(tm_mon,0,5);
    memset(tm_year,0,5);
    memset(tm_hour,0,50);
    memset(tm_min,0,5);
    memset(tm_sec,0,5);

    p->tm_mon++;
    p->tm_year -= 100;

    sprintf(tm_hour, "%02d", p->tm_hour);
    sprintf(tm_min, "%02d", p->tm_min);
    sprintf(tm_sec, "%02d", p->tm_sec);
    sprintf(tm_mday, "%02d", p->tm_mday);
    sprintf(tm_mon, "%02d", p->tm_mon);
    sprintf(tm_year, "%02d", p->tm_year);

    strcat(temp, "-");
    strcat(temp, tm_hour);
    strcat(temp, ":");
    strcat(temp, tm_min);
    strcat(temp, ":");
    strcat(temp, tm_sec);

    change_system_date(temp);
    sync_rtc_time();

    return TIMER_SUCCESS;
}

static unsigned char Os__write_time_wrap(unsigned char *ptTime)
{
    change_system_date(ptTime);
    sync_rtc_time();
    return TIMER_SUCCESS;
}

unsigned char Os__write_date(unsigned char *ptDate)
{
    char day_temp[3];
    char month_temp[3];
    char year_temp[9];
    char result[11];
    unsigned int d_temp;
    char *rt;
    rt = result;
    rt += 2;

/*Check and validate date month year*/

    day_temp[0] = ptDate[0];
    day_temp[1] = ptDate[1];
    day_temp[0] = day_temp[0] - 48;
    day_temp[1] = day_temp[1] - 48;
    d_temp = day_temp[0] * 10 + day_temp[1];

    if (d_temp >= 32)
        return 87;

    month_temp[0] = ptDate[2];
    month_temp[1] = ptDate[3];
    month_temp[0] = month_temp[0] - 48;
    month_temp[1] = month_temp[1] - 48;
    d_temp = month_temp[0] * 10 + month_temp[1];

    if (d_temp >= 13)
        return 87;

    day_temp[0] = ptDate[0];
    day_temp[1] = ptDate[1];
    day_temp[2] = '\0';

    month_temp[0] = ptDate[2];
    month_temp[1] = ptDate[3];
    month_temp[2] = '\0';

    year_temp[0] = ptDate[4];
    year_temp[1] = ptDate[5];
    year_temp[2] = '\0';
    
    //因为IPS420为32位系统,时间如果超过2038年1月19日星期二凌晨03:14:07 
    //将造成时间溢出,设置时间不成功问题
    d_temp = (year_temp[0] - 48) * 10 + (year_temp[1] - 48);
    if (d_temp >= 38 && d_temp <= 69)
        return 87;
	
    strcat(year_temp, ".");

    strcat(year_temp, month_temp);
    strcat(year_temp, ".");
    strcat(year_temp, day_temp);

    strcpy(rt, year_temp);

	//满足设置1970到2037年所有日期
	if(d_temp >= 70)
	{
		result[0] = '1';
    	result[1] = '9';
    }
    else
    {
    	result[0] = '2';
    	result[1] = '0';
    }

    Os__write_date_wrap(result);
    return TIMER_SUCCESS;
}

unsigned char Os__write_time(unsigned char *ptTime)
{
    char hour_temp[9];
    char min_temp[3];
    unsigned int d_temp;

    /*Check and validate time*/

    hour_temp[0] = ptTime[0];
    hour_temp[1] = ptTime[1];
    hour_temp[0] = hour_temp[0] - 48;
    hour_temp[1] = hour_temp[1] - 48;
    d_temp = hour_temp[0] * 10 + hour_temp[1];

    if (d_temp >= 25)
        return 87;

    min_temp[0] = ptTime[2];
    min_temp[1] = ptTime[3];
    min_temp[0] = min_temp[0] - 48;
    min_temp[1] = min_temp[1] - 48;
    d_temp = min_temp[0] * 10 + min_temp[1];

    if (d_temp >= 60)
        return 87;

    hour_temp[0] = ptTime[0];
    hour_temp[1] = ptTime[1];
    hour_temp[2] = '\0';

    min_temp[0] = ptTime[2];
    min_temp[1] = ptTime[3];
    min_temp[2] = '\0';

    strcat(hour_temp, ":");

    strcat(hour_temp, min_temp);
    strcat(hour_temp, ":");
    strcat(hour_temp, "30");

    Os__write_time_wrap(hour_temp);

    return TIMER_SUCCESS;
}

unsigned char Os__xdelay (unsigned int Delay)
{
    unsigned int t=Delay*10000;
    os_sleep(t/1000000,t%1000000);
    return TIMER_SUCCESS;
}

static int timer_start(unsigned char type,volatile unsigned int *ptimer)
{
    unsigned char i;
    if( !ptimer ){
        return(TIMER_BADPARAM);
    }
    //如果需要开启的定时器在定时器表中,不再重复开启
    for(i=0;i<MAXNUM_TIMER;i++){
        if( timer_tbl[i].ptimer == ptimer ){
            return TIMER_SUCCESS;
        }
    }
    for(i=0;i<MAXNUM_TIMER;i++){
        if( !timer_tbl[i].ptimer ){
            timer_tbl[i].type = type;
            timer_tbl[i].ptimer = ptimer;
            return TIMER_SUCCESS;
        }
    }
    return TIMER_TIMERFULL;
}

int Os__timer_start (int *ptTimer)
{
    return timer_start(0,ptTimer);
}

void timer_init(void)
{
    int i;

    g_timer_cnt = 0;
    for(i=0;i<MAXNUM_TIMER;i++){
        timer_tbl[i].ptimer = 0;
    }

    g_inter_timer_cnt = 0;
    for(i=0; i<MAXNUM_INTER_TIMER; i++){
        inter_timer_tbl[i].ptimer = 0;
        inter_timer_tbl[i].timer_proc = 0;
    }
    timer_new_thread();
}

static int timer_new_thread(void)
{
    pthread_t ptid_timer;
    pthread_t ptid_timer_function;

    if (pthread_create(&ptid_timer,NULL,timer_handle,NULL)< 0)
    {
        perror("timer_handle thread fail!");
        exit(TIMER_CREATETHREADFAIL);
    }
    if (pthread_create(&ptid_timer_function,NULL,timer_function_handle,NULL)<0)
    {
        perror("timer_function_handle thread fail!");
        exit(TIMER_CREATETHREADFAIL);
    }
    return TIMER_SUCCESS;
}

static int time_count=0;
static void* timer_handle(void *p)//定时器线程，要求尽快返回，仅作计数功能
{
    time_count=0;
    while(1)
    {
        time_count++;
        g_timer_cnt++;
        if( g_timer_cnt >= TIMERCOUNT_INGENICO)
        {
            g_timer_cnt = 0;
        }
        os_sleep(0,10000);
    }
}

static void* timer_function_handle(void *p)//另开一线程处理回调函数
{
    unsigned char i;
    unsigned int timer_value;
    time_count=0;
    while(1)
    {
        for(i=0;i<g_inter_timer_cnt;i++)//将每个内部定时器内部计时自减，当减到小于等于0时执行对应函数
        {
            if( inter_timer_tbl[i].ptimer )
            {
                timer_value = *inter_timer_tbl[i].ptimer;
                if( timer_value )
                {
                    timer_value-=time_count;
                    *inter_timer_tbl[i].ptimer = timer_value;
                    if( timer_value<=0 )
                    {
                        if( inter_timer_tbl[i].timer_proc )
                        {
                            (*(inter_timer_tbl[i].timer_proc))();
                        }
                    }
                }
            }
        }
        for(i=0; i<MAXNUM_TIMER; i++)//更新外部计时器的计时值
        {
            if( timer_tbl[i].ptimer )
            {
                timer_value = *(timer_tbl[i].ptimer);
                if( timer_value )
                    timer_value-=time_count;
                *(timer_tbl[i].ptimer) = timer_value;
                if( timer_value<=0 )
                {
                    timer_tbl[i].ptimer = 0;
                }
            }
        }
        time_count=0;
        os_sleep(0,10000);
    }
}

static int timer_stop(volatile unsigned int *ptimer)
{
    unsigned char i;

    if( !ptimer )
    {
        return(TIMER_BADPARAM);
    }
    for(i=0;i<MAXNUM_TIMER;i++)
    {
        if(timer_tbl[i].ptimer == ptimer)
        {
            timer_tbl[i].ptimer = 0;
            break;
        }
    }
    return TIMER_SUCCESS;
}

int Os__timer_stop (int * ptTimer)
{
    return timer_stop(ptTimer);
}

int timer_Internal_start(int *ptimer,void (*pfnProc)(void))
{
    unsigned char i;

    if( !ptimer ){
        return(TIMER_BADPARAM);
    }

    for(i=0;i<MAXNUM_INTER_TIMER;i++)
    {
        if( inter_timer_tbl[i].ptimer == ptimer )
        {
            inter_timer_tbl[i].timer_proc = pfnProc;
            goto start;
        }
        if( !inter_timer_tbl[i].ptimer )
        {
            inter_timer_tbl[i].ptimer = ptimer;
            inter_timer_tbl[i].timer_proc = pfnProc;
            g_inter_timer_cnt ++;
            goto start;
        }
    }
    return(TIMER_TIMERFULL);
start:
    return(TIMER_SUCCESS);
}

int timer_Internal_stop(int *ptimer)
{
    unsigned char i;

    if( !ptimer ){
        return(TIMER_BADPARAM);
    }
    for(i=0;i<g_inter_timer_cnt;i++)
    {
        if(inter_timer_tbl[i].ptimer == ptimer )
        {
            if( i != g_inter_timer_cnt-1 )
            {
                inter_timer_tbl[i].ptimer = inter_timer_tbl[g_inter_timer_cnt-1].ptimer;
                inter_timer_tbl[i].timer_proc = inter_timer_tbl[g_inter_timer_cnt-1].timer_proc;
                inter_timer_tbl[g_inter_timer_cnt-1].ptimer = 0;
                inter_timer_tbl[g_inter_timer_cnt-1].timer_proc = 0;
            }else
            {
                inter_timer_tbl[i].ptimer = 0;
                inter_timer_tbl[i].timer_proc = 0;
            }
            g_inter_timer_cnt --;
            break;
        }
    }
    return(TIMER_SUCCESS);
}
