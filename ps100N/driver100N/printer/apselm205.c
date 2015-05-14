/** @defgroup printer
 *  @author  Zeng Shu (Email:zengshu@gmail.com)
 *  @version 1.2
 *  @date    2012-10-25
 @verbatim

 copyright (C) 2011上海杉德金卡信息科技有限公司

 @endverbatim
 *  @{
 */
#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/aio.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/kfifo.h>
#include <linux/wait.h>

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/kthread.h>

#include <linux/interrupt.h>
#include <linux/workqueue.h>

/** 平台相关代码开始 */
#include "board-mx25_3stack.h"
#include "iomux.h"
#include "mx25_pins.h"
#include "mach-mx25/machine.h"
#include "mx25printer.h"
#include "apselm205.h"
#include "mach-mx25/mxc_epit.h"
#include <linux/imx_adc.h>

static inline void begin_print(void);

static struct apselm205_dev_t *apselm205_dev_t_p;

#define MAX_ADC_VALUE 4095

#define DEFAULT_LINEFEED_TIME   400
#define DEFAULT_MIN_HEATTIME    535
#define DEFAULT_MAX_HEATTIME    850
#define DEFAULT_ADD_HEATTIME    800
#define DEFAULT_OVERHEAT_TEMPERATURE 1500
#define DEFAULT_RETNORMAL_TEMPERATURE 2000

static int linefeed_time = DEFAULT_LINEFEED_TIME;
static int min_heattime = DEFAULT_MIN_HEATTIME;
static int max_heattime = DEFAULT_MAX_HEATTIME;
static int add_heattime = DEFAULT_ADD_HEATTIME;
static int overheat_temprature = DEFAULT_OVERHEAT_TEMPERATURE;
static int retnormal_temprature = DEFAULT_RETNORMAL_TEMPERATURE;

static void inline open_printer(void)
{
    printer_set_latch(true);
    printer_set_sb(0x00);
    printer_set_power_on(true);
}

static void inline close_printer(void)
{
    gpio_set_value(apselm205_dev_t_p->printer_pin_dclk, 0);
    gpio_set_value(apselm205_dev_t_p->printer_pin_di, 0);
    printer_set_moto(0);
    printer_set_power_on(false);
}

//检测是否过热以及当前应该的加热时间
static int heat_thread(void *data)
{
    bool ret;
    static unsigned short result;
    while(1)
    {
        if(apselm205_dev_t_p->printing)
        {
            imx_adc_init();
            imx_adc_convert(GER_PURPOSE_ADC2,&result);
            imx_adc_deinit();

            if(result<=overheat_temprature)
            {
                ret=true;
            }
            else
            {
                ret=false;
            }
            apselm205_dev_t_p->overheat_flag=ret||printer_is_overheat();
            apselm205_dev_t_p->heattime=(result-overheat_temprature)*(max_heattime-min_heattime)/(MAX_ADC_VALUE-overheat_temprature)+min_heattime;

            ssleep(2);
        }
        else if(apselm205_dev_t_p->overheat_flag==true)
        {
            imx_adc_init();
            imx_adc_convert(GER_PURPOSE_ADC2,&result);
            imx_adc_deinit();
            if(result>=retnormal_temprature)
            {
                apselm205_dev_t_p->overheat_flag=false;
            }
            ssleep(2);
        }
        else
        {
            ssleep(2);
        }
    };
    return 0;
}

static irqreturn_t timer_int(int irq, void *devid)
{
    if(printer_is_havepaper()==true&&(apselm205_dev_t_p->overheat_flag)==false)
    {
        static int step=0;
        static int blackbit_count=0;
        static int i,j;
        static char bit_value;

        open_printer();

        //We need a prefeed if we just to print something.
        if(step<64)
        {
            printer_set_moto((apselm205_dev_t_p->printer_step)[step++%8]);
            set_timer_time_again(max_heattime);
            return IRQ_HANDLED;
        }

        if(step%4==0)
        {
            //Latch the data.
            printer_set_latch(false);
            printer_set_latch(true);

            printer_set_moto((apselm205_dev_t_p->printer_step)[step++%8]);

            if(blackbit_count==0)//No content.
            {
                printer_set_sb(0x00);
                set_timer_time_again(linefeed_time);
            }
            else if(blackbit_count>0 && blackbit_count<=(BYTES_PER_LINE*4))//Normal duty.
            {
                printer_set_sb(0x38);
                if(apselm205_dev_t_p->linefeed_flag)//Prev line is a emptyline, so we need to add heattime.
                {
                    set_timer_time_again(apselm205_dev_t_p->heattime+add_heattime);
                }
                else
                {
                    set_timer_time_again(apselm205_dev_t_p->heattime);
                }
            }
            else if(blackbit_count>(BYTES_PER_LINE*4) && blackbit_count<=(BYTES_PER_LINE*8))//Heavy duty.
            {
                printer_set_sb(0x30);
                if(apselm205_dev_t_p->linefeed_flag)//Prev line is a emptyline, so we need to add heattime.
                {
                    set_timer_time_again(max_heattime*2+add_heattime);
                }
                else
                {
                    set_timer_time_again(max_heattime*2);
                }
            }
            else//error
            {
                //Do nothing.
            }
        }
        else if(step%4==1)
        {
            printer_set_moto((apselm205_dev_t_p->printer_step)[step++%8]);

            if(blackbit_count==0)//No content.
            {
                printer_set_sb(0x00);
                set_timer_time_again(linefeed_time);
            }
            else if(blackbit_count>0 && blackbit_count<=(BYTES_PER_LINE*4))//Normal duty.
            {
                printer_set_sb(0x07);
                if(apselm205_dev_t_p->linefeed_flag)//Prev line is a emptyline, so we need to add heattime.
                {
                    set_timer_time_again(apselm205_dev_t_p->heattime+add_heattime);
                }
                else
                {
                    set_timer_time_again(apselm205_dev_t_p->heattime);
                }
            }
            else if(blackbit_count>(BYTES_PER_LINE*4) && blackbit_count<=(BYTES_PER_LINE*8))//Heavy duty.
            {
                printer_set_sb(0x0C);
                if(apselm205_dev_t_p->linefeed_flag)//Prev line is a emptyline, so we need to add heattime.
                {
                    set_timer_time_again(max_heattime*2+add_heattime+80);
                }
                else
                {
                    set_timer_time_again(max_heattime*2+80);
                }
            }
            else//error
            {
                //Do nothing.
            }
        }
        else if(step%4==2)
        {
            printer_set_moto((apselm205_dev_t_p->printer_step)[step++%8]);

            if(blackbit_count==0)//No content.
            {
                printer_set_sb(0x00);
                set_timer_time_again(linefeed_time);
            }
            else if(blackbit_count>0 && blackbit_count<=(BYTES_PER_LINE*4))//Normal duty.
            {
                printer_set_sb(0x38);
                if(apselm205_dev_t_p->linefeed_flag)//Prev line is a emptyline, so we need to add heattime.
                {
                    set_timer_time_again(max_heattime+add_heattime);
                }
                else
                {
                    set_timer_time_again(max_heattime);
                }
            }
            else if(blackbit_count>(BYTES_PER_LINE*4) && blackbit_count<=(BYTES_PER_LINE*8))//Heavy duty.
            {
                printer_set_sb(0x03);
                if(apselm205_dev_t_p->linefeed_flag)//Prev line is a emptyline, so we need to add heattime.
                {
                    set_timer_time_again(max_heattime*2+add_heattime);
                }
                else
                {
                    set_timer_time_again(max_heattime*2);
                }
            }
            else//error
            {
                //Do nothing.
            }
        }
        else if(step%4==3)
        {
            printer_set_moto((apselm205_dev_t_p->printer_step)[step++%8]);

            if(blackbit_count==0)
            {
                printer_set_sb(0x00);
            }
            else if(blackbit_count>0 && blackbit_count<=(BYTES_PER_LINE*4))
            {
                printer_set_sb(0x07);
            }
            else if(blackbit_count>(BYTES_PER_LINE*4) && blackbit_count<=(BYTES_PER_LINE*8))
            {
                printer_set_sb(0x00);
				if(apselm205_dev_t_p->linefeed_flag)//Prev line is a emptyline, so we need to add heattime.
                {
                    set_timer_time_again(max_heattime*2+add_heattime);
                }
                else
                {
                    set_timer_time_again(max_heattime*2);
                }
            }
            else
            {
                //Do nothing.
            }

            if(BYTES_PER_LINE==kfifo_get(apselm205_dev_t_p->kfifo,apselm205_dev_t_p->line_buf,BYTES_PER_LINE))
            {
                if(blackbit_count==0 || blackbit_count>(BYTES_PER_LINE*4))
                {
                    set_timer_time_again(linefeed_time);
                }
                else if(blackbit_count>0 && blackbit_count<=(BYTES_PER_LINE*4))
                {
                    if(apselm205_dev_t_p->linefeed_flag)
                    {
                        set_timer_time_again(apselm205_dev_t_p->heattime+add_heattime);
                    }
                    else
                    {
                        set_timer_time_again(apselm205_dev_t_p->heattime);
                    }
                }
                else
                {
                    //Do nothing.
                }

                if(blackbit_count)
                {
                    apselm205_dev_t_p->linefeed_flag=0;
                }

                blackbit_count=0;
                for(i=0; i<BYTES_PER_LINE; i++)
                {
                    for(j=7; j>=0; j--)
                    {
                        bit_value=(((apselm205_dev_t_p->line_buf)[i]>>j)&(0x01));
                        blackbit_count+=bit_value;
                        gpio_set_value(apselm205_dev_t_p->printer_pin_dclk, 0);
                        gpio_set_value(apselm205_dev_t_p->printer_pin_di, bit_value);
                        gpio_set_value(apselm205_dev_t_p->printer_pin_dclk, 1);
                    }
                }
                if(blackbit_count==0)
                {
                    apselm205_dev_t_p->linefeed_flag=1;
                }
            }
            else
            {
                step=0;
                kfifo_reset(apselm205_dev_t_p->kfifo);
                set_timer_off();
                close_printer();
                printer_set_paper_detect(false);
                apselm205_dev_t_p->printing=false;
            }
        }
    }
    else
    {
        close_printer();
        set_timer_time_again(1000000);
    }
    return IRQ_HANDLED;
}

char dat[BYTES_PER_LINE];
static void apselm_test(void)
{
    if(apselm205_dev_t_p->printing==false)
    {
        int i;
        memset(dat,0xf0,BYTES_PER_LINE);
        for(i=0; i<400; i++)
        {
            if(apselm205_dev_t_p->kfifo_maxlen-kfifo_len(apselm205_dev_t_p->kfifo)>BYTES_PER_LINE)
                kfifo_put(apselm205_dev_t_p->kfifo,dat,BYTES_PER_LINE);
        }
        begin_print();
    }
}

static int apselm205_feedpaper(int line)
{
    int i;
    int count=0;
    memset(dat,0x00,BYTES_PER_LINE);
    for(i=0; i<line; i++)
    {
        if(apselm205_dev_t_p->kfifo_maxlen-kfifo_len(apselm205_dev_t_p->kfifo)>BYTES_PER_LINE)
        {
            kfifo_put(apselm205_dev_t_p->kfifo,dat,BYTES_PER_LINE);
            count++;
        }
    }
    if(apselm205_dev_t_p->printing==false)
    {
        begin_print();
    }
    return count;
}

static inline void begin_print(void)
{
    int i,j;

    open_printer();
    printer_set_paper_detect(true);
    apselm205_dev_t_p->printing=true;

    if(BYTES_PER_LINE==kfifo_get(apselm205_dev_t_p->kfifo,apselm205_dev_t_p->line_buf,BYTES_PER_LINE))
    {
        for(i=0; i<BYTES_PER_LINE; i++)
        {
            for(j=7; j>=0; j--)
            {
                gpio_set_value(apselm205_dev_t_p->printer_pin_dclk, 0);
                gpio_set_value(apselm205_dev_t_p->printer_pin_di, (((apselm205_dev_t_p->line_buf)[i]>>j)&(0x01)));
                gpio_set_value(apselm205_dev_t_p->printer_pin_dclk, 1);
            }
        }
    }
    set_timer_on(100000);
}

static ssize_t apselm205_write (struct file *file ,const char *buf, size_t count, loff_t *f_ops)
{
    int ret=0;

    if(count%48 != 0)
    {
        //pr_alert("Printer only accept data size at integer multiples of 48.\n");
        ret=0;
    }
    if(!printer_is_havepaper() || apselm205_dev_t_p->overheat_flag)
    {
        //pr_alert("Printer is overheat or no paper,refuse to accept write.please keep check the state.\n");
        ret = -((apselm205_dev_t_p->overheat_flag)<<1|(!printer_is_havepaper()));
    }
    if (apselm205_dev_t_p->kfifo_maxlen-kfifo_len(apselm205_dev_t_p->kfifo)<count)
    {
        //pr_alert("Printer kfifo is full,refuse to accept write.suggest to write again after 5 seconds.\n");
        ret=-ENOBUFS;
    }
    else
    {
        /*
        pr_alert("Printer accept %d data:",count);
        for(i=0;i<count/8;i++)
        {
            if(i%6==0)
                pr_alert("\n");
            pr_alert("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
                     buf[i*8],buf[i*8+1],buf[i*8+2],buf[i*8+3],buf[i*8+4],buf[i*8+5],buf[i*8+6],buf[i*8+7]);
        }
        */
        ret = kfifo_put(apselm205_dev_t_p->kfifo,(unsigned char *)buf, count);
        if(kfifo_len(apselm205_dev_t_p->kfifo)==count)
        {
            begin_print();
            apselm205_dev_t_p->linefeed_flag=1;
        }
    }
    return ret;
}

static int apselm205_ioctl (struct inode *inode ,struct file *file, unsigned int cmd, unsigned long data)
{
    int ret = 0;

    if (_IOC_TYPE(cmd) != PRINTER_IOC_MAGIC)
    {
        if(dbg)
            pr_err("IOC_MAGIC not match.\n");
        return -ENOTTY;
    }
    if (_IOC_NR(cmd) > PRINTER_IOC_MAXNR)
    {
        if(dbg)
            pr_err("cmd is large than IOC_MAXNR.\n");
        return -ENOTTY;
    }

    if (_IOC_DIR(cmd) & _IOC_READ)
        ret = !access_ok(VERIFY_WRITE, (void __user *)data, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        ret =  !access_ok(VERIFY_READ, (void __user *)data, _IOC_SIZE(cmd));
    if (ret)
    {
        if(dbg)
            pr_err("READ WRITE check fault.\n");
        return -EFAULT;
    }
    switch(cmd)
    {
    case PRINTER_PRINT_TEST:
    {
        apselm_test();
        ret=0;
    }
    break;
    case PRINTER_PRINT_CHKSTATUS:
    {
        unsigned int status = ((apselm205_dev_t_p->overheat_flag)<<1|(!printer_is_havepaper()));
        ret = copy_to_user((unsigned int *)data,
                           (unsigned int *)&status, sizeof(unsigned int));

    }
    break;
    case PRINTER_PRINT_SETVF:     ///FIXME:无作用
        ret = 0;
        break;
    case PRINTER_PRINT_END:       //return 1, printing is end,return 0, the printer is working.
    {
        unsigned int status = !apselm205_dev_t_p->printing;
        ret = copy_to_user((unsigned int *)data,
                           (unsigned int *)&status, sizeof(unsigned int));
    }
    break;
    case PRINTER_PRINT_CHECKPAPER:
    {
        ret = !printer_is_havepaper();
    }
    break;
    case PRINTER_FEED_PAPER:
    {
        unsigned int line;
        ret = copy_from_user(&line,(unsigned int *)data,sizeof(unsigned int));
        if(!ret)
        {
            ret=apselm205_feedpaper(line);
        }
    }
    break;
    case PRINTER_PRINT_CLEANBUF:
    {
        /*Note:usage of kfifo_reset is dangerous. It should be only called
        when the fifo is exclusived locked or when it is secured that no other
        thread is accessing the fifo.*/
        kfifo_reset(apselm205_dev_t_p->kfifo);
    }
    break;
    case PRINTER_PRINT_SETMINHEATTIME:
    {
        ret = copy_from_user(&min_heattime,(int *)data,sizeof(int));
    }
    break;
    case PRINTER_PRINT_SETMAXHEATTIME:
    {
        ret = copy_from_user(&max_heattime,(int *)data,sizeof(int));
    }
    break;
    case PRINTER_PRINT_SETLINEFEEDTIME:
    {
        ret = copy_from_user(&linefeed_time,(int *)data,sizeof(int));
    }
    break;
    case PRINTER_PRINT_SETADDHEATTIME:
    {
        ret = copy_from_user(&add_heattime,(int *)data,sizeof(int));
    }
    break;
    case PRINTER_PRINT_SETOVERHEATTEMP:
    {
        ret = copy_from_user(&overheat_temprature,(int *)data,sizeof(int));
    }
    break;
    case PRINTER_PRINT_SETRETNORMALTEMP:
    {
        ret = copy_from_user(&retnormal_temprature,(int *)data,sizeof(int));
    }
    break;
    default:
        ret=-ENOTTY;
        break;
    }
    return ret;
}

ssize_t apselm205_open (struct inode *inode ,struct file *file)
{
    return 0;
}

ssize_t apselm205_release (struct inode  *inode ,struct file *file)
{
    return 0;
}

struct file_operations apselm205_ops =
{
    .write = apselm205_write,
    .ioctl = apselm205_ioctl,
    .open = apselm205_open,
    .release = apselm205_release,
};

static int __init apselm205_setup_cdev(struct cdev *dev,int index)
{
    int ret=0;

    cdev_init(dev,&apselm205_ops);
    dev->owner=THIS_MODULE;
    dev->ops=&apselm205_ops;

    ret=cdev_add(dev,apselm205_dev_t_p->devno,index);
    if (ret)
    {
        if(dbg)
            pr_err("Error %d adding GZ%d\n",ret,index);
        goto seterr_a;
    }
    apselm205_dev_t_p->class = class_create(THIS_MODULE, "apselm205_class");
    if (IS_ERR(apselm205_dev_t_p->class))
    {
        ret = -EINVAL;
        if(dbg)
            pr_err("apselm205: failed in creating class.\n");
        goto seterr_b;
    }

    if(NULL==device_create(apselm205_dev_t_p->class, NULL, apselm205_dev_t_p->devno, NULL, "printer"))
    {
        ret = -EINVAL;
        if(dbg)
            pr_err("printer: failed in creating class device.\n");
        goto seterr_c;
    }
    return ret;
seterr_c:
    class_destroy(apselm205_dev_t_p->class);
seterr_b:
    cdev_del(dev);
seterr_a:
    return ret;
}


static int __init apselm205_init_register(void)
{
    int ret = -ENODEV;
    /** 为apselm205设备结构分配空间 */
    apselm205_dev_t_p=kzalloc(sizeof(struct apselm205_dev_t),GFP_KERNEL);
    if(apselm205_dev_t_p == NULL)
    {
        ret = -ENOMEM;
        goto regerr_a;
    }

    if(SAND_DEMOBOARD==sand_machine())
    {
        apselm205_dev_t_p->printer_pin_di=IOMUX_TO_GPIO(DEMO_PRINTER_PIN_DI);
        apselm205_dev_t_p->printer_pin_dclk=IOMUX_TO_GPIO(DEMO_PRINTER_PIN_DICLK);
        apselm205_dev_t_p->printer_step=demo_apselm205_step;
    }
    else if(SAND_IPS420==sand_machine())
    {
        apselm205_dev_t_p->printer_pin_di=IOMUX_TO_GPIO(IPS420_PRINTER_PIN_DI);
        apselm205_dev_t_p->printer_pin_dclk=IOMUX_TO_GPIO(IPS420_PRINTER_PIN_DICLK);
        apselm205_dev_t_p->printer_step=ips420_apselm205_step;
    }
    else
    {
        apselm205_dev_t_p->printer_pin_di=IOMUX_TO_GPIO(PS400_PRINTER_PIN_DI);
        apselm205_dev_t_p->printer_pin_dclk=IOMUX_TO_GPIO(PS400_PRINTER_PIN_DICLK);
        apselm205_dev_t_p->printer_step=ps400_apselm205_step;
    }

    /** 初始化写入缓冲 */
    spin_lock_init(&apselm205_dev_t_p->kfifo_lock);
    apselm205_dev_t_p->kfifo_order = 6;
    apselm205_dev_t_p->kfifo_buffer = (unsigned char *) __get_free_pages(GFP_KERNEL, apselm205_dev_t_p->kfifo_order);
    if (!apselm205_dev_t_p->kfifo_buffer)
    {
        if(dbg)
            pr_err("kfifo_buffer malloc error \n");
        ret =  -ENOMEM;
        goto  regerr_b;
    }
    apselm205_dev_t_p->kfifo_maxlen = PAGE_SIZE<<apselm205_dev_t_p->kfifo_order;
    memset(apselm205_dev_t_p->kfifo_buffer,0,apselm205_dev_t_p->kfifo_maxlen);

    apselm205_dev_t_p->kfifo = kfifo_init(apselm205_dev_t_p->kfifo_buffer,apselm205_dev_t_p->kfifo_maxlen,GFP_KERNEL,&(apselm205_dev_t_p->kfifo_lock));

    /** 申请gpio */
    if(SAND_DEMOBOARD==sand_machine())
    {
        ret=mxc_request_iomux(DEMO_PRINTER_PIN_DI, MUX_CONFIG_ALT5);
        mxc_request_iomux(DEMO_PRINTER_PIN_DICLK, MUX_CONFIG_ALT5);

    }
    else if(SAND_IPS420==sand_machine())
    {
        ret=mxc_request_iomux(IPS420_PRINTER_PIN_DI, MUX_CONFIG_ALT5);
        mxc_request_iomux(IPS420_PRINTER_PIN_DICLK, MUX_CONFIG_ALT5);
    }
    else
    {
        ret=mxc_request_iomux(PS400_PRINTER_PIN_DI, MUX_CONFIG_ALT5);
        mxc_request_iomux(PS400_PRINTER_PIN_DICLK, MUX_CONFIG_ALT5);
    }
    ret=gpio_request(apselm205_dev_t_p->printer_pin_di, "printer_di");
    ret=gpio_direction_output(apselm205_dev_t_p->printer_pin_di, 0);

    gpio_request(apselm205_dev_t_p->printer_pin_dclk, "printer_diclk");
    gpio_direction_output(apselm205_dev_t_p->printer_pin_dclk, 0);

    ret=request_irq(EPIT1_INT, timer_int, IRQF_TIMER, 0, 0);
    apselm205_dev_t_p->printing=false;



    /** 初始化内核线程 */
    /*
    apselm205_dev_t_p->print_task=kthread_create(print_thread, NULL, "sand_printd");
    wake_up_process(apselm205_dev_t_p->print_task);
    */
    apselm205_dev_t_p->heattime=max_heattime;
    apselm205_dev_t_p->overheat_flag=false;
    apselm205_dev_t_p->heat_tsk = kthread_run(heat_thread, NULL, "printer_heat");

    /** 由系统分配设备号 */
    ret=alloc_chrdev_region(&(apselm205_dev_t_p->devno),0,1,"printer");
    if (ret<0)
    {
        if(dbg)
            pr_err("printer init_module failed with %d.\n",ret);
        goto regerr_d;
    }
    else
    {
        if(dbg)
            pr_err("printer init_module success, major=%d, minor=%d.\n",
                   MAJOR(apselm205_dev_t_p->devno), MINOR(apselm205_dev_t_p->devno));
    }

    /** 注册设备并建立设备节点 */
    ret=apselm205_setup_cdev(&apselm205_dev_t_p->cdev,1);
    if(ret)
    {
        ret = -ENODEV;
        goto regerr_d;
    };



    return 0;
regerr_d:
    unregister_chrdev_region(apselm205_dev_t_p->devno, 1);
    gpio_request(apselm205_dev_t_p->printer_pin_di, NULL);
    gpio_request(apselm205_dev_t_p->printer_pin_dclk, NULL);
    if(SAND_DEMOBOARD==sand_machine())
    {
        mxc_free_iomux(DEMO_PRINTER_PIN_DI, MUX_CONFIG_GPIO);
        mxc_free_iomux(DEMO_PRINTER_PIN_DICLK, MUX_CONFIG_GPIO);
    }
    else if(SAND_IPS420==sand_machine())
    {
        mxc_free_iomux(IPS420_PRINTER_PIN_DI, MUX_CONFIG_GPIO);
        mxc_free_iomux(IPS420_PRINTER_PIN_DICLK, MUX_CONFIG_GPIO);
    }
    else
    {
        mxc_free_iomux(PS400_PRINTER_PIN_DI, MUX_CONFIG_GPIO);
        mxc_free_iomux(PS400_PRINTER_PIN_DICLK, MUX_CONFIG_GPIO);
    }


    free_pages((unsigned long)apselm205_dev_t_p->kfifo_buffer,apselm205_dev_t_p->kfifo_order);
regerr_b:
    kfree(apselm205_dev_t_p);
regerr_a:
    return ret;
}

int __init apselm205_init(struct printer_t *printer_t_p)
{
    int  ret;
    ret = apselm205_init_register();
    return ret;
}

void apselm205_exit(void)
{
    device_destroy(apselm205_dev_t_p->class,apselm205_dev_t_p->devno);
    class_destroy(apselm205_dev_t_p->class);
    cdev_del(&apselm205_dev_t_p->cdev);
    unregister_chrdev_region(apselm205_dev_t_p->devno, 1);
    gpio_request(apselm205_dev_t_p->printer_pin_di, NULL);
    gpio_request(apselm205_dev_t_p->printer_pin_dclk, NULL);
    if(SAND_DEMOBOARD==sand_machine())
    {
        mxc_free_iomux(DEMO_PRINTER_PIN_DI, MUX_CONFIG_GPIO);
        mxc_free_iomux(DEMO_PRINTER_PIN_DICLK, MUX_CONFIG_GPIO);
    }
    else if(SAND_IPS420==sand_machine())
    {
        mxc_free_iomux(IPS420_PRINTER_PIN_DI, MUX_CONFIG_GPIO);
        mxc_free_iomux(IPS420_PRINTER_PIN_DICLK, MUX_CONFIG_GPIO);
    }
    else
    {
        mxc_free_iomux(PS400_PRINTER_PIN_DI, MUX_CONFIG_GPIO);
        mxc_free_iomux(PS400_PRINTER_PIN_DICLK, MUX_CONFIG_GPIO);
    }

    free_pages((unsigned long)apselm205_dev_t_p->kfifo_buffer,apselm205_dev_t_p->kfifo_order);
    kfree(apselm205_dev_t_p);
}
/** @} */

