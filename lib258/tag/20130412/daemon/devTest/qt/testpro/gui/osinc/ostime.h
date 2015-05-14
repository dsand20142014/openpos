#ifndef OSTIME_H_INCLUDE
#define OSTIME_H_INCLUDE

/**
 * @defgroup 系统时间和计时器函数
 * @author 熊剑
 * @brief 对系统时间进行操作和计时器函数
 * @{
 */

/**
 * @def MAXNUM_TIMER
 * @brief 定时器最大个数
 **/
#define MAXNUM_TIMER              25

/**
 * @def MAXNUM_INTER_TIMER
 * @brief 带函数的定时器最大个数
 **/
#define MAXNUM_INTER_TIMER        10

/**
 * @def TIMER_10MS
 * @brief 定义10ms(=10000us)
 **/
#define TIMER_10MS                10000

/**
 * @def TIMER_SUCCESS
 * @brief 对计时器或系统时间操作成功
 **/
#define TIMER_SUCCESS              0

/**
 * @def TIMER_TIMERFULL
 * @brief 计时器已满
 **/
#define TIMER_TIMERFULL           (-1)

/**
 * @def TIMER_BADPARAM
 * @brief 参数不正确
 **/
#define TIMER_BADPARAM            (-2)

/**
 * @def TIMER_CREATETHREADFAIL
 * @brief 创建线程失败
 **/
#define TIMER_CREATETHREADFAIL    (-3)


/**
 * @def TIMERCOUNT_INGENICO
 * @brief 计时器计数最大值
 **/
#define TIMERCOUNT_INGENICO        10


/**
 * @fn unsigned char Os__xdelay (unsigned int Delay)
 * @brief 使进程产生一个中断,单位为10ms
 *
 * @param Delay 需要延迟的时间
 *
 * @return 返回0
 *
 * 例
 * @code
 * Os__xdelay(300);
 * //运行结果
 * 进程延迟3秒
 * @endcode
 */
unsigned char Os__xdelay (unsigned int Delay);

/**
 * @fn void Os__read_date (unsigned char *ptDate)
 * @brief 读取系统时间到指针'ptDate'.
 *
 * @param ptDate 日期目标字符串首地址
 *
 * @return  无返回值
 * @warning 日期是以ASCII按格式DayDayMonthMonthYearYear\0存放,
 *			如果系统时间无意义,系统返回'000000\0'.
 * 例
 * @code
 * unsigned char ptDate[10];
 * memset(ptDate,0x00,sizeof(ptDate));
 * Os__read_date(ptDate);
 * printf("%s\n",ptDate);
 *
 * //运行结果
 * ptDate = "291211"
 * @endcode
 */
void Os__read_date (unsigned char *ptDate);

/**
 * @fn void Os__read_date_format (unsigned char *ptDate)
 * @brief 读取系统时间到指针'ptDate'.
 *
 * @param ptDate 日期目标字符串首地址
 *
 * @return  无返回值
 * @warning 日期是以ASCII按格式DayDay/MonthMonth/YearYear\0存放,
 *			如果系统时间无意义,系统返回'00/00/00\0'.
 * 例
 * @code
 * unsigned char ptDate[10];
 * memset(ptDate,0x00,sizeof(ptDate));
 * Os__read_date_format(ptDate);
 * printf("%s\n",ptDate);
 *
 * //运行结果
 * ptDate = "29/12/11"
 * @endcode
 */
void Os__read_date_format (unsigned char *ptDate);

/**
 * @fn void Os__read_date_time(unsigned char *ptDateTime)
 * @brief 读取系统时间到指针'ptDateTime'.
 *
 * @param ptDateTime 日期时间目标字符串首地址
 *
 * @return  无返回值
 * @warning 日期时间是以ASCII按格式HHMMDDMMYY\0存放,
 *			如果系统时间无意义,系统返回'0000000000\0'.
 * 例
 * @code
 * unsigned char ptDateTime[20];
 * memset(ptDateTime,0x00,sizeof(ptDateTime));
 * Os__read_date_time(ptDateTime);
 * printf("%s\n",ptDateTime);
 *
 * //运行结果
 * ptDate = "1653291211"
 * @endcode
 */
void Os__read_date_time(unsigned char *ptDateTime);

/**
 * @fn void Os__read_date_time_all(unsigned char *ptDateTime)
 * @brief 读取系统时间到指针'ptDateTime'.
 *
 * @param ptDateTime 日期时间目标字符串首地址
 *
 * @return  无返回值
 * @warning 日期时间是以ASCII按格式HHMMSSDDMMYY\0存放,
 *			如果系统时间无意义,系统返回'000000000000\0'.
 * 例
 * @code
 * unsigned char ptDateTime[20];
 * memset(ptDateTime,0x00,sizeof(ptDateTime));
 * Os__read_date_time_all(ptDateTime);
 * printf("%s\n",ptDateTime);
 *
 * //运行结果
 * ptDate = "165322291211"
 * @endcode
 */
void Os__read_date_time_all(unsigned char *ptDateTime);

/**
 * @fn void Os__read_time (unsigned char *ptTime)
 * @brief 读取系统时间到指针'ptTime'.
 *
 * @param ptTime 时间目标字符串首地址
 *
 * @return  无返回值
 * @warning 时间是以ASCII按格式HHMM\0存放,
 *			如果系统时间无意义,系统返回'0000\0'.
 * 例
 * @code
 * unsigned char ptTime[10];
 * memset(ptTime,0x00,sizeof(ptTime));
 * Os__read_time(ptTime);
 * printf("%s\n",ptTime);
 *
 * //运行结果
 * ptDate = "1659"
 * @endcode
 */
void Os__read_time (unsigned char *ptTime);

/**
 * @fn void Os__read_time_format (unsigned char *ptTime)
 * @brief 读取系统时间到指针'ptTime'.
 *
 * @param ptTime 时间目标字符串首地址
 *
 * @return  无返回值
 * @warning 时间是以ASCII按格式HHMM\0存放,
 *			如果系统时间无意义,系统返回'0000\0'.
 * 例
 * @code
 * unsigned char ptTime[10];
 * memset(ptTime,0x00,sizeof(ptTime));
 * Os__read_time_format(ptTime);
 * printf("%s\n",ptTime);
 *
 * //运行结果
 * ptDate = "16:59"
 * @endcode
 */
void Os__read_time_format (unsigned char *ptTime);

/**
 * @fn void Os__read_time_sec (unsigned char *ptTime)
 * @brief 读取系统时间到指针'ptTime'.
 *
 * @param ptTime 时间目标字符串首地址
 *
 * @return  无返回值
 * @warning 时间是以ASCII按格式HHMMSS\0存放,
 *			如果系统时间无意义,系统返回'000000\0'.
 * 例
 * @code
 * unsigned char ptTime[10];
 * memset(ptTime,0x00,sizeof(ptTime));
 * Os__read_time_sec(ptTime);
 * printf("%s\n",ptTime);
 *
 * //运行结果
 * ptDate = "165912"
 * @endcode
 */
void Os__read_time_sec (unsigned char *ptTime);

/**
 * @fn unsigned char Os__write_date (unsigned char *ptDate)
 * @brief 指针'ptDate'所指的日期设置到系统中.
 *
 * @param ptDate 日期字符串首地址
 *
 * @return  设置成功返回0,失败返回非0
 * @warning 日期格式为DDMMYY.
 *
 * 例
 * @code
 * unsigned char ptDate[10];
 * memset(ptDate,0x00,sizeof(ptDate));
 * sprintf(ptDate,"%s","291211");
 * printf("%u\n",Os__write_date(ptDate));
 *
 * //运行结果
 * return 0;
 * @endcode
 */
unsigned char Os__write_date (unsigned char *ptDate);

/**
 * @fn unsigned char Os__write_time (unsigned char *ptTime)
 * @brief 指针'ptTime'所指的时间设置到系统中.
 *
 * @param ptTime 时间字符串首地址
 *
 * @return  设置成功返回0,失败返回非0
 * @warning 时间格式为HHMM,系统默认将秒针设置为30.
 *
 * 例
 * @code
 * unsigned char ptTime[10];
 * memset(ptTime,0x00,sizeof(ptTime));
 * sprintf(ptTime,"%s","1340");
 * printf("%u\n",Os__write_time(ptTime));
 *
 * //运行结果
 * return 0;
 * @endcode
 */
unsigned char Os__write_time (unsigned char *ptTime);

/**
 * @fn int Os__timer_start (int *ptTimer)
 * @brief	设置一个定时器,'ptTimer'是定时器的时间地址,时间变量10ms递减一次,
 *			当时间递减到零时,定时接束,所以定时器定时一个单位为10ms.
 *
 * @param ptTimer 定时器的时间地址
 *
 * @return  成功启动定时器返回0,定时器满返回-1(系统使用了太多的定时器),参数错误返回-2
 *
 * 例
 * @code
 * unsigned int ptTimer = 20;
 * timer_init();
 * if(Os__timer_start(&ptTimer) == 0)
 * {
 * 	   while(ptTimer != 0);
 *     Os__timer_stop(&ptTimer);
 * }
 * else
 *     printf("Timer start fail!\n");
 * @endcode
 */
int Os__timer_start (int *ptTimer);

/**
 * @fn void timer_init(void)
 * @brief	初始化定时器
 *
 * @param 无参数
 *
 * @return  无返回值
 *
 * 例
 * @code
 * unsigned int ptTimer = 20;
 * timer_init();
 * if(Os__timer_start(&ptTimer) == 0)
 * {
 * 	   while(ptTimer != 0);
 *     Os__timer_stop(&ptTimer);
 * }
 * else
 *     printf("Timer start fail!\n");
 * @endcode
 */
void timer_init(void);

/**
 * @fn int Os__timer_stop (int * ptTimer)
 * @brief	此函数必须停止以'Os__timer_start'函数打开的定时器,
 * 			否则MONOS将继续递减定时变量,直到变量不存在(假设变量非全局变量).
 *
 * @param ptTimer 定时器的时间地址
 *
 * @return  成功返回0,失败返回-2
 * @warning 函数仅仅停止自动递减定时器变量,但不影响变量值,
 *			所以应用在停止定时器前可以检查变量的正确性。
 *
 * 例
 * @code
 * unsigned int ptTimer = 20;
 * timer_init();
 * if(Os__timer_start(&ptTimer) == 0)
 * {
 * 	   while(ptTimer != 0);
 *     Os__timer_stop(&ptTimer);
 * }
 * else
 *     printf("Timer start fail!\n");
 * @endcode
 */
int Os__timer_stop (int * ptTimer);

/**
 * @fn int timer_Internal_start(int *ptimer,void (*pfnProc)(void))
 * @brief	设置一个定时器,'ptimer'是定时器的时间地址,时间变量10ms递减一次,
 *			当时间递减到零时,执行pfnProc所指向的函数地址.
 *
 * @param ptimer 定时器的时间地址
 * @param pfnProc 当定时器时间为0时,所要执行的函数的函数地址
 ×
 * @return  成功启动定时器返回0,定时器满返回-1(系统使用了太多的定时器),参数错误返回-2
 *
 * 例
 * @code
 * void pfnProc(void)
 * {
 *     printf("timer is time out!\n");
 * }
 *
 * volatile unsigned int ptTimer = 20;
 * timer_init();
 * if(timer_Internal_start(&ptTimer,pfnProc) == 0)
 * {
 * 	   while(ptTimer != 0);
 *     timer_Internal_stop(&ptTimer);
 * }
 * else
 *     printf("Timer start fail!\n");
 * //运行结果
 * timer is time out!
 * @endcode
 */
int timer_Internal_start(int *ptimer,void (*pfnProc)(void));

/**
 * @fn int timer_Internal_stop(int *ptimer)
 * @brief	此函数必须停止以'timer_Internal_start'函数打开的定时器,
 * 			否则MONOS将继续递减定时变量,直到变量不存在(假设变量非全局变量).
 *
 * @param ptimer 定时器的时间地址
 *
 * @return  成功返回0,失败返回-2
 * @warning 函数仅仅停止自动递减定时器变量,但不影响变量值,
 *			所以应用在停止定时器前可以检查变量的正确性。
 *
 * 例
 * @code
 * void pfnProc(void)
 * {
 *     printf("timer is time out!\n");
 * }
 *
 * volatile unsigned int ptTimer = 20;
 * timer_init();
 * if(timer_Internal_start(&ptTimer,pfnProc) == 0)
 * {
 * 	   while(ptTimer != 0);
 *     timer_Internal_stop(&ptTimer);
 * }
 * else
 *     printf("Timer start fail!\n");
 * //运行结果
 * timer is time out!
 * @endcode
 */
int timer_Internal_stop(int *ptimer);

#endif //OSTIME_H_INCLUDE
/** @} end of etc_fns */
