/** @defgroup printer
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 1.2
 *  @date    2012-10-25
 @verbatim

 copyright (C) 2011上海杉德金卡信息科技有限公司

 @endverbatim
 *  @{
 */
#ifndef APSELM205_H_INCLUDED
#define APSELM205_H_INCLUDED

#include "mx25printer.h"
#include <linux/workqueue.h>

#define BYTES_PER_LINE 48
#define HARDWARE_OVERHEAT 1

#define DEMO_PRINTER_PIN_DI      MX25_PIN_EB0
#define DEMO_PRINTER_PIN_DICLK   MX25_PIN_POWER_FAIL

#define PS400_PRINTER_PIN_DI      MX25_PIN_UPLL_BYPCLK
#define PS400_PRINTER_PIN_DICLK   MX25_PIN_EXT_ARMCLK

#define IPS420_PRINTER_PIN_DI	MX25_PIN_UPLL_BYPCLK
#define IPS420_PRINTER_PIN_DICLK	MX25_PIN_EXT_ARMCLK

static const uint8_t demo_apselm205_step[4]={0x0f,0x0e,0x0a,0x0b};///声明了步进电机的4个相位状态，步进电机始终是在这四个状态里循环。

static const uint8_t ps400_apselm205_step[4]={0x05,0x06,0x0a,0x09};

static const uint8_t ips420_apselm205_step[8]={0x06,0x07,0x05,0x0d,0x09,0x0b,0x0a,0x0e};

/** 打印机的数据结构 */
struct apselm205_dev_t
{
    struct cdev cdev;                                   ///字符设备数据结构。
    struct class *class;                                ///设备类结构。
    dev_t devno;                                        ///设备号。

    uint8_t line_buf[BYTES_PER_LINE];                   ///行缓冲

    spinlock_t      kfifo_lock;                         ///
    unsigned int    kfifo_order;                        ///
    struct kfifo *  kfifo;                              ///
    unsigned char * kfifo_buffer;                       ///
    unsigned int    kfifo_maxlen;                       ///PAGESIZE<<order

	uint32_t printer_pin_di;
	uint32_t printer_pin_dclk;
	const uint8_t* printer_step;

	bool printing;
	bool overheat_flag;
	bool linefeed_flag;
    unsigned int heattime;

    struct task_struct *heat_tsk;
};

int apselm205_init(struct printer_t * printer_t_p);
void apselm205_exit(void);

#endif // APSELM205_H_INCLUDED
/** @} */
