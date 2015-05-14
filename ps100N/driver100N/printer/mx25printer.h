/** @defgroup printer
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 1.0
 *  @date    2012-10-25
 @verbatim
 copyright (C) 2010上海杉德金卡信息科技有限公司
@endverbatim
 *  @{
 */
#ifndef MX25PRINTER_H_INCLUDED
#define MX25PRINTER_H_INCLUDED

extern int dbg;

/** 定义IOCTL命令 */
#define PRINTER_IOC_MAGIC               'S'
#define PRINTER_FEED_PAPER              _IOW(PRINTER_IOC_MAGIC,  1, int)//走纸；
#define PRINTER_PRINT_VCC               _IOW(PRINTER_IOC_MAGIC,  2, int)//未实现；
#define PRINTER_PRINT_FONT              _IOW(PRINTER_IOC_MAGIC,  3, int)//未实现；
#define PRINTER_PRINT_TEST              _IOW(PRINTER_IOC_MAGIC,  4, int)//测试；
#define PRINTER_PRINT_CHECKPAPER        _IOWR(PRINTER_IOC_MAGIC, 5, int)//检纸；
#define PRINTER_PRINT_CFGPARAM          _IOWR(PRINTER_IOC_MAGIC, 6, int)//未实现；
#define PRINTER_PRINT_CLEANBUF          _IOWR(PRINTER_IOC_MAGIC, 7, int)//清缓存；
#define PRINTER_PRINT_CHKSTATUS         _IOWR(PRINTER_IOC_MAGIC, 8, int)//异常状态；
#define PRINTER_PRINT_SETVF             _IOWR(PRINTER_IOC_MAGIC, 9, int)//未实现；
#define PRINTER_PRINT_END               _IOWR(PRINTER_IOC_MAGIC,10, int)//是否打印结束；
#define PRINTER_PRINT_SETMINHEATTIME    _IOWR(PRINTER_IOC_MAGIC,11, int)//最小加热时间，不能低于400，打印机走纸不动，默认535；
#define PRINTER_PRINT_SETMAXHEATTIME    _IOWR(PRINTER_IOC_MAGIC,12, int)//最大加热时间，不高于1200，默认850，过高极易过热；
#define PRINTER_PRINT_SETLINEFEEDTIME   _IOWR(PRINTER_IOC_MAGIC,13, int)//走纸时间，默认400。很少需要改动，影响走纸速度；
#define PRINTER_PRINT_SETADDHEATTIME    _IOWR(PRINTER_IOC_MAGIC,14, int)//加热补偿时间，默认800，在空白行之后自动加在当前加热时间上，一般不用改动；
#define PRINTER_PRINT_SETOVERHEATTEMP   _IOWR(PRINTER_IOC_MAGIC,15, int)//过热温度，默认1500，越大过热温度越低，其值与温度近似线性关系；
#define PRINTER_PRINT_SETRETNORMALTEMP  _IOWR(PRINTER_IOC_MAGIC,16, int)//回复非过热状态温度，默认2000，必须大于过热温度；
#define PRINTER_IOC_MAXNR               16

/** cpld寄存器的物理地址 */
#define DEMO_REG_PRINT_CS1 0xb2700000
#define DEMO_REG_PRINT_CS2 0xb2700001
#define DEMO_REG_PRINT_CS3 0xb2700002
#define DEMO_REG_PRINT_CS4 0xb2700003
#define DEMO_REG_CPLD_DBG  0xb2e00000

#define PS400_REG_PRINT_CS1 0xb2500000
#define PS400_REG_PRINT_CS2 0xb2500001
#define PS400_REG_PRINT_CS3 0xb2500002
#define PS400_REG_PRINT_CS4 0xb2500003
#define PS400_REG_CPLD_DBG  0xb2e00000

#define IPS420_REG_PRINT_CS1 0xa0500000
#define IPS420_REG_PRINT_CS2 0xa0500001
#define IPS420_REG_PRINT_CS3 0xa0500002
#define IPS420_REG_PRINT_CS4 0xa0500003
#define IPS420_REG_CPLD_DBG  0xa0e00000

void set_timer_on(unsigned int us);
void set_timer_time_again(unsigned int us);
void set_timer_off(void);

/** 定义打印机的型号 */
#define PRINTER_TYPE_APSELM205  0
#define PRINTER_TYPE_MU111SII   7

/**
* @brief 用于存储无论对于热敏还是针打都是相对
* 固定的信息。
*/
struct printer_t
{
    /** cpld内部四个寄存器 */
    volatile uint8_t* reg_print_cs1;
    volatile uint8_t* reg_print_cs2;
    volatile uint8_t* reg_print_cs3;
    volatile uint8_t* reg_print_cs4;

    /** 用于检测cpld是否工作正常的debug寄存器 */
    volatile uint8_t* reg_cpld_dbg;

    /** 存储当前打印机的类型 */
    uint8_t machine_type;
};

/**
* @brief 初始化cpld打印机控制的结构
* @return 申请并初始化好的printer_t指针
*/
struct printer_t* printer_t_init(void);

/**
* @brief 释放cpld打印机控制的结构
*/
void printer_t_exit(void);

/**
* @brief PRINT_POWER ON为内部信号，“1”打开打印机电源控制，
* “0”为关闭打印机电源控制，对于PRINT_POWER输出管脚定义为漏
* 极开路，其逻辑关系为PRINT_POWER ON和打印机过热信号 逻辑
* 与产生。（热敏和套打都有此信号）
* @param val true:电源接通 false:断开
*/
void printer_set_power_on(bool val);

/**
* @brief 热敏打印机 锁存信号 latch 信号
*/
void printer_set_latch(bool val);

/**
* @brief PHOTO_ON：热敏打印机检测纸使能
*/
void printer_set_paper_detect(bool val);

bool is_printer_paper_detect_enable(void);
/**
* @brief 加热片选通
*/
void printer_set_sb(uint8_t val);
uint8_t printer_read_sb(void);
/**
* @brief 马达控制信号：套打印为全部信号，热敏只有低BIT[3:0]
*/
void printer_set_moto(uint8_t val);
uint8_t printer_read_moto(void);

/**
* @brief 检查打印机过热
*/
bool printer_is_overheat(void);

/**
* @brief 检测打印机缺纸
*/
bool printer_is_havepaper(void);

void print_all_reg(void);

#endif //MX25PRINTER_H_INCLUDED
/** @} */ //注释文件结束
