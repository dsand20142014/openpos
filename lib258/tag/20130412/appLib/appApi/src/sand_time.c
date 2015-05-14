#include <stdio.h>
#include <stdlib.h>
#include <asm/param.h> //for HZ define
#include <fcntl.h>
#include <time.h>
#include <linux/rtc.h>

#include "sand_incs.h"
#include "timer_driver.h"


#ifdef EN_DBG
#define dbg Uart_Printf
#else
#define dbg
#endif

#define DEV_RTC "/dev/misc/rtc"
static int fd_timer;	//定时器使用

#if 1
#undef RTC_RD_TIME
#undef RTC_SET_TIME
#define RTC_RD_TIME	_IOR('p', 0x09, struct rtc_time )/* Read RTC time   */
#define RTC_SET_TIME	_IOW('p', 0x0a, struct rtc_time) /* Set RTC time    */
#endif

static inline void os_sleep(__time_t sec,__suseconds_t us)
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = us;
    select(0, NULL, NULL, NULL, &tv);
}


/*
作用	：使进程临时产生中断
函数	： unsigned char Os__xdelay (unsigned short Delay) ;
等待	：是
说明	：使进程产生一个中断, 每一个单位为10ms (HZ=100(1s), HZ/1000=1ms)
返回值 	： OK = 成功  非 OK    = 错误
*/
unsigned char Os__xdelay (unsigned int Delay)
{
	
    unsigned int t=Delay*10000;
    os_sleep(t/1000000,t%1000000);
/*    
    unsigned int time=0;
    time=10000*Delay*(HZ/100);
    //printf("%d\n",time);
    usleep(time);
*/
    
    return 0;
}

/*
作用	：读EFT 终端日期和时间
函数	: Os__read_date_time Os__read_date_time (unsigned char *ptDateTime) ;
等待	：是
说明	  ：读系统的的日期和时间并存放在' ptDateTime '指向的BUFFER中
         'date and time' 返回为ASCII 并按下列格式存放：
         HourHourMinuteMinuteSecondSecondDayDayMonthMonthYearYear\0.
         HHMMSSDDMMYY
         如果时间或日期返回无意义, 函数返回 '00000000\0'。
返回值：无
*/
unsigned char Os__read_date_time (unsigned char *ptDateTime)
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
	p->tm_year -= 100;
	//处理未设置时间时读取错误
	if(p->tm_year < 0)
		p->tm_year = 70;

    sprintf(tm_hour, "%02d", p->tm_hour);
	sprintf(tm_min, "%02d", p->tm_min);
	sprintf(tm_sec, "%02d", p->tm_sec);
	sprintf(tm_mday, "%02d", p->tm_mday);
	sprintf(tm_mon, "%02d", p->tm_mon);	
	sprintf(tm_year, "%02d", p->tm_year);
	

	strcat(tm_hour, tm_min);
	//strcat(tm_hour, tm_sec);
	strcat(tm_hour, tm_mday);
	strcat(tm_hour, tm_mon);
	strcat(tm_hour, tm_year);
	
	#if 0 
	Uart_Printf("tm_hour is %s\n", tm_hour);
	Uart_Printf("tm_min is %s\n", tm_min);
	Uart_Printf("tm_sec is %s\n", tm_sec);
	Uart_Printf("tm_mday is %s\n", tm_mday);
	Uart_Printf("tm_mon is %s\n", tm_mon);
	Uart_Printf("tm_year is %s\n", tm_year);
	
	#endif
	
	strcpy(getDate, tm_hour);
	
    memcpy(ptDateTime, getDate, 11);

    return 0;
}



/*
作用	：读EFT终端时间。
函数	： void Os__read_time (unsigned char *ptTime) ;
等待	：是
说明	：Os__read_time 读得的系统时间将存放在 'ptTime'指向的BUFFER中,
时间将以ASCII 存放, 存放格式为： HHMM\0.
如果时间无意义, Os__read_time 返回 '0000\0' 。
返回值	：无
*/
void Os__read_time (unsigned char *ptTime)
{
    unsigned char tm_min[5],tm_hour[15],getDate[40], ret;
	unsigned char tbuff[10];

	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);
	
    memset(ptTime, 0, 4);
    memset(getDate, 0, 15);

	sprintf(tm_hour, "%02d", p->tm_hour);
	sprintf(tm_min, "%02d", p->tm_min);
	strcat(tm_hour, tm_min);
	strcpy(getDate, tm_hour);
	
	
    memcpy(ptTime, getDate, 5);
}


/*
作用	：按时间格式读系统时间
函数	：void Os__read_time_format (unsigned char *ptTime) ;
等待	：是
说明	： 	Os__read_time_format 按时间格式读出系统时间, 读得得系统时间存放在'ptTime'指向得BUFFER中。
			时间是ASCII吗, 按下列格式存放：HourHour:MinuteMinute\0, 如果读出时间无意义, 函数返回'00:00\0' 。
返回值	：无
*/
void Os__read_time_format (unsigned char *ptTime)
{
    unsigned char tm_min[5],tm_hour[12],getDate[15];
    
    memset(getDate, 0, 15);
    memset(tm_hour,0,12);
    memset(tm_min,0,5);

   
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

 	sprintf(tm_hour, "%02d", p->tm_hour);
	sprintf(tm_min, "%02d", p->tm_min);
	strcat(tm_hour, ":");
	strcat(tm_hour, tm_min);
	strcpy(getDate, tm_hour);

    memcpy(ptTime, getDate, 6);
}

/*
作用	：读系统时间到秒。
函数	： void Os__read_time_sec (unsigned char *ptTime) ;
等待	：是
描述    : Os__read_time_sec读得的系统时间将存放在 'ptTime'指向的BUFFER中,
		时间将以ASCII 存放, 存放格式为：HourHourMinuteMinuteSecondSecond\0.
如果时间无意义, Os__read_time_sec返回'000000\0' 。
返回值  ：无
*/
void Os__read_time_sec (unsigned char *ptTime)
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
	p->tm_year -= 100;
	
    sprintf(tm_hour, "%02d", p->tm_hour);
	sprintf(tm_min, "%02d", p->tm_min);
	sprintf(tm_sec, "%02d", p->tm_sec);
	sprintf(tm_mday, "%02d", p->tm_mday);
	sprintf(tm_mon, "%02d", p->tm_mon);	
	sprintf(tm_year, "%02d", p->tm_year);
	

	strcat(tm_hour, tm_min);
	strcat(tm_hour, tm_sec);
	
	
	#if 0 
	Uart_Printf("tm_hour is %s\n", tm_hour);
	Uart_Printf("tm_min is %s\n", tm_min);
	Uart_Printf("tm_sec is %s\n", tm_sec);
	Uart_Printf("tm_mday is %s\n", tm_mday);
	Uart_Printf("tm_mon is %s\n", tm_mon);
	Uart_Printf("tm_year is %s\n", tm_year);
	
	#endif
	
	strcpy(getDate, tm_hour);
	
    memcpy(ptTime, getDate, 7);

   
}

/*
作用	：读EFT终端日期
函数	： void Os__read_date (unsigned char *ptDate) ;
等待	：是
说明	：Os__read_date读系统时间到buffer指针 'ptDate',
日期是ASCII 按格式DayDayMonthMonthYearYear\0.存放, 如果系统时间无意义, 系统返回'000000\0'。
		DDMMYY
返回值	：无
*/
void Os__read_date (unsigned char *ptDate)
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
	p->tm_year -= 100;
	//处理未设置时间时读取错误
	if(p->tm_year < 0)
		p->tm_year = 70;
	
    sprintf(tm_hour, "%02d", p->tm_hour);
	sprintf(tm_min, "%02d", p->tm_min);
	sprintf(tm_sec, "%02d", p->tm_sec);
	sprintf(tm_mday, "%02d", p->tm_mday);
	sprintf(tm_mon, "%02d", p->tm_mon);	
	sprintf(tm_year, "%02d", p->tm_year);
	

	strcat(tm_mday, tm_mon);
	strcat(tm_mday, tm_year);
	
	
	#if 0 
	Uart_Printf("tm_hour is %s\n", tm_hour);
	Uart_Printf("tm_min is %s\n", tm_min);
	Uart_Printf("tm_sec is %s\n", tm_sec);
	Uart_Printf("tm_mday is %s\n", tm_mday);
	Uart_Printf("tm_mon is %s\n", tm_mon);
	Uart_Printf("tm_year is %s\n", tm_year);
	
	#endif
	
	strcpy(getDate, tm_mday);
	
    memcpy(ptDate, getDate, 7);

    
   

}

/*
作用	：读系统日期并格式化
函数	： void Os__read_date_format (unsigned char *ptDate) ;
等待	：是
说明	： Os__read_date_format 读系统日期, 并存放在'ptDate'.指向的BUFFER中
         以ASCII 的存放格式为：
         DayDay/MonthMonth/YearYear\0.
         如果日期无意义, 函数返回 '00/00/00\0'。
返回值	：无
*/
void Os__read_date_format (unsigned char *ptDate)
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
	p->tm_year -= 100;
	//处理未设置时间时读取错误
	if(p->tm_year < 0)
		p->tm_year = 70;
			
    sprintf(tm_hour, "%02d", p->tm_hour);
	sprintf(tm_min, "%02d", p->tm_min);
	sprintf(tm_sec, "%02d", p->tm_sec);
	sprintf(tm_mday, "%02d", p->tm_mday);
	sprintf(tm_mon, "%02d", p->tm_mon);	
	sprintf(tm_year, "%02d", p->tm_year);
	
	strcat(tm_mday, "/");
	strcat(tm_mday, tm_mon);
	strcat(tm_mday, "/");
	strcat(tm_mday, tm_year);
	
	
	#if 0 
	Uart_Printf("tm_hour is %s\n", tm_hour);
	Uart_Printf("tm_min is %s\n", tm_min);
	Uart_Printf("tm_sec is %s\n", tm_sec);
	Uart_Printf("tm_mday is %s\n", tm_mday);
	Uart_Printf("tm_mon is %s\n", tm_mon);
	Uart_Printf("tm_year is %s\n", tm_year);
	
	#endif
	
	strcpy(getDate, tm_mday);
	
    memcpy(ptDate, getDate, 9);


	
}


#if 0
static void change_system_date(unsigned char *d)
{

	int p_status;
	pid_t PID = fork();
	
	if (PID == 0) //child
	{
		
		if (execl("/bin/date", "date", "-s", d, NULL) < 0) {
			Uart_Printf("date set failed\n");
			exit(1);
		
		}

	}

	else if (PID < 0)
	{
 		Uart_Printf("process failed\n");
		exit(1);
	}

	else //parent
	{
		
		wait(&p_status);
		
		
	}



}


static void sync_rtc_time()
{

	int p_status;
	pid_t PID = fork();
	
	if (PID == 0) //child
	{
		if (execl("/sbin/hwclock", "hwclock", "-w") < 0) {
			Uart_Printf("rtc set failed\n");
			exit(1);
		
		}

	}

	else if (PID < 0)
	{
 		Uart_Printf("process failed\n");
		exit(1);
	}

	else //parent
	{
		
		wait(&p_status);
		
		
	}



}
#endif

/** 2011/6/9   **/
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

/** end 2011/6/9  **/



/*
作用	：设置新的系统日期
函数	： unsigned char Os__write_date (unsigned char *ptDate) ;
等待	：是
说明	： Os__write_date 设置新的系统日期,  'ptDate' 是存放时间BUFFER指针,
	BUFFER 内容为ASCII, 存放格式为 YearYear.MONTHMONTH.DAYDAY\0
返回值 ： OK  = 正确
          非OK = 错误
*/
unsigned char Os__write_date_wrap(unsigned char *ptDate)
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
   
 	return OK;

}

/*
作用	：修改EFT终端时间。
函数	：UCHAR Os__write_time (unsigned char *ptTime) ;
等待	：是
说明	：Os__write_time要修改的系统时间将存放在 'ptTime'指向的BUFFER中,
时间将以ASCII 存放, 存放格式为： HourHour:MinuteMinute:SSSS\0.
返回值 ：OK = 正确
　	   非OK = 错误
*/
unsigned char Os__write_time_wrap(unsigned char *ptTime)
{

	change_system_date(ptTime);
	sync_rtc_time();
    return OK;
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
	
	strcat(year_temp, ".");
	
	strcat(year_temp, month_temp);
	strcat(year_temp, ".");
	strcat(year_temp, day_temp);

	strcpy(rt, year_temp);
	
	result[0] = '2';
	result[1] = '0';

	Os__write_date_wrap(result);
	return OK;
	

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

	return OK;
}




/*
什么功能?
类似于MSG_ReadMach_Date
*/
void OSDATE_ReadTimeSec(unsigned char *pucTime)
{
    Os__read_time_sec(pucTime);
}



#if 0
/*
作用	：启动定时器
函数	： unsigned char Os__timer_start (unsigned short *ptTimer) ;
等待	：否
说明	：Os__timer_start 设置一个定时器, 'ptTimer' 是定时的时间地址, 时间变量10ms递减一次,
		当时间递减到零时, 定时接束, 所以定时器定时一个单位为10ms(HZ/1000=0.01s=10ms).
返回值：OK   = 成功
		非OK   = 错误 (系统使用了太多的定时器).
*/
unsigned char Os__timer_start (unsigned int *ptTimer)
{
    unsigned int Timeout;
    Timeout=(unsigned int )*ptTimer;

    fd_timer= open(TIMER_DEV_NAME, O_RDWR);
    if ( fd_timer== -1 )
    {
        Uart_Printf("Cann't open file ");
        return 1;
    }
    
//	Uart_Printf("start timer %d \n", Timeout);
    ioctl(fd_timer, CMD_PIT_ADD, &Timeout);

    return 0;
}
/*
作用	：停止定时
函数	：void Os__timer_stop (unsigned short * ptTimer) ;
等待	：是
说明	：停止'Os__timer_start'开始的定时器, 应用必须停止'Os__timer_start'函数打开的定时器,
		否则MONOS将继续递减定时变量, 直到变量不存在（假设变量非全局变量）。
注意	：函数仅仅停止自动递减定时器变量, 但不影响变量值, 所以应用在停止定时器前可以检查变量的正确性。
返回值	：无
*/
void Os__timer_stop (unsigned int * ptTimer)
{
    if (!fd_timer)
    {
        Uart_Printf("timer has been closed!\n");
        return;
    }
    ioctl(fd_timer, CMD_PIT_STOP);
    close(fd_timer);
    fd_timer = -1;
    return;
}

#endif


unsigned int Os__timer_remain ()
{
    unsigned int remain;
    if (!fd_timer)
    {
        Uart_Printf("timer has been closed!\n");
        return -1;
    }
    ioctl(fd_timer, CMD_PIT_GET, &remain);

    return remain;
}

//延时测试
unsigned char OS__timer_test()
{
    volatile unsigned int Timeout, T2, i;

    Timeout=1000;
    T2=50;

    Os__timer_start((unsigned int*)&Timeout);
    Uart_Printf("start to dec %ld!\n", Timeout);

    do
    {
        T2 = Os__timer_remain();
        if (T2 < 888 ) break;
    }
    while (T2!=0);
    Uart_Printf("%s Timeout=%d\n", __func__, T2);


    Os__timer_stop((unsigned int*)&Timeout);
    T2 = Os__timer_remain();
    Uart_Printf("end T2=%d\n", T2);

    return 0;

    Timeout =10000;
    Os__timer_start((unsigned int*)&Timeout);
    Uart_Printf("start to dec %ld!\n", Timeout);

    do
    {
        //Uart_Printf(".");
        T2 = Os__timer_remain();
        if (T2==678)
        {
            Os__timer_stop((unsigned int *)&Timeout);
            break;
        }
    }
    while (T2!=0);

    T2 = Os__timer_remain();
    Uart_Printf("T2＝ %ld\n", T2);


    return 0;
}


/*************************************
 * 时间设置测试程序
 *************************************/
void Os_time_test(char *pDate)
{
    unsigned char test1[20], *test;

    test=test1;
    memset(test, 0, 20);

    memset(test, 0, 20);

#if 0
    if (pDate==NULL)
        strcpy(test, "010316465508");
    else
        memcpy(test, pDate, strlen(pDate) );

    OS__write_date_time(test);
    Uart_Printf("Os_write_date_time, HHMMSSDDMMYY: %s\n", test);
#endif


    memset(test, 0, 20);
    Os__read_date_time(test);
    Uart_Printf("Os__read_date_time, HHMMSSDDMMYY: %s\n", test);

#if 0
    memset(test, 0, 20);
    Os__read_date(test);
    Uart_Printf("\nOs__read_date, DDMMYY: %s\n", test);

    memset(test, 0, 20);
    Os__read_date_format(test);
    Uart_Printf("Os__read_date_format, DD/MM/YY: %s\n", test);


    memset(test, 0, 20);
    Os__read_time(test);
    Uart_Printf("Os__read_time, HHMM: %s\n", test);

    memset(test, 0, 20);
    Os__read_time_format(test);
    Uart_Printf("Os__read_time_format, HH:MM: %s\n", test);

    memset(test, 0, 20);
    Os__read_time_sec(test);
    Uart_Printf("Os__read_time_sec, HHMMSS: %s\n", test);
#endif


#if 0
    Uart_Printf("\n\nI will set time & date\n");
    memset(test,0, 20);
    strcpy(test, "251208");
    Os__write_date(test);

    memset(test, 0, 20);
    Os__read_date_time(test);
    Uart_Printf("Os__read_date_time, HHMMDDMMYY: %s\n", test);

    memset(test,0, 20);
    memcpy(test, "192030", 6);
    Os__write_time(test);

    memset(test, 0, 20);
    Os__read_date_time(test);
    Uart_Printf("Os__read_date_time, HHMMSSDDMMYY: %s\n", test);
#endif

}

/*************************************
 * 定时器测试程序
 *************************************/
int OS_timer_test()
{
    int fd;
    int result;
    int i;
    char buf[]="helloworld";
    char inbuf[20];
    unsigned long ioctltest;
    unsigned long delaytime=1000, remain=1;

    memset(inbuf, 0, 20);

    fd = open(TIMER_DEV_NAME, O_RDWR);
    if ( fd == -1 )
    {
        Uart_Printf("Cann't open file\n");
        exit(0);
    }

    ioctl(fd, CMD_PIT_ADD, &delaytime);

#if 1
    do
    {
        ioctl(fd, CMD_PIT_GET, &remain);
        Uart_Printf("remain=%d\n", remain);
    }
    while (remain>500);
#endif
    Uart_Printf("stop!\n");

exit:
    close(fd);
    return 0;
}


#if 0
int main(char argc, char *argv[])
{
    char *pDate=NULL;
    char cmd;
    unsigned char test[20];

    OS__timer_test();
    //OS_timer_test();
    return 0;

    memset(test, 0, 20);
    if (argc<3)
    {
        cmd='r';
        memset(test, 0, 20);
        Os__read_date_time(test);
        Uart_Printf("Os__read_date_time, HHMMSSDDMMYY: %s\n", test);

        memset(test, 0, 20);
        Os__read_date(test);
        Uart_Printf("\nOs__read_date, DDMMYY: %s\n", test);

        memset(test, 0, 20);
        Os__read_date_format(test);
        Uart_Printf("Os__read_date_format, DD/MM/YY: %s\n", test);


        memset(test, 0, 20);
        Os__read_time(test);
        Uart_Printf("Os__read_time, HHMM: %s\n", test);

        memset(test, 0, 20);
        Os__read_time_format(test);
        Uart_Printf("Os__read_time_format, HH:MM: %s\n", test);

        memset(test, 0, 20);
        Os__read_time_sec(test);
        Uart_Printf("Os__read_time_sec, HHMMSS: %s\n", test);
    }
    else if (argc==3)
    {
        cmd='w';
        pDate=argv[2];
        OS__write_date_time(pDate);
        Uart_Printf("Os_write_date_time, MMDDHHMMSSYY: %s\n", pDate);

        memset(test, 0, 20);
        Os__read_date_time(test);
        Uart_Printf("Os__read_date_time, HHMMSSDDMMYY: %s\n", test);

        Uart_Printf("set date(DDMMYY):\n");
        gets(test);
        Os__write_date(test);
        Uart_Printf("OS__write_date, DDMMYY: %s\n", test);

        memset(test, 0, 20);
        Os__read_date_time(test);
        Uart_Printf("Os__read_date_time, HHMMSSDDMMYY: %s\n", test);

        Uart_Printf("set time(HHMMSS):\n");
        gets(test);
        Os__write_time(test);
        Uart_Printf("OS__write_time, HHMMSS: %s\n", test);

        memset(test, 0, 20);
        Os__read_date_time(test);
        Uart_Printf("Os__read_date_time, HHMMSSDDMMYY: %s\n", test);
    }

}
#endif