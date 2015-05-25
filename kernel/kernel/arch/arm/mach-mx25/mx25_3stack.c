/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/smsc911x.h>
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULE)
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/mach/flash.h>
#endif

#include <mach/common.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/keypad.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include <mach/memory.h>
#include <mach/gpio.h>
#include <mach/mmc.h>

#include "board-mx25_3stack.h"
#include "crm_regs.h"
#include "iomux.h"

#include <linux/string.h>
#include <mach-mx25/machine.h>


#include <linux/reboot.h>
#include <linux/proc_fs.h>

#include <linux/delay.h>

static int machine=1;
static char machine_name[32]={'\0'};

static char uboot_ver[32]={'\0'};

int sand_machine(void)
{
    return machine;
}
EXPORT_SYMBOL(sand_machine);

static int __init sand_parse_string(char *string)
{
    if(strcmp(string,"DEMOBOARD")==0) {
        pr_alert("************** SAND DEMOBOARD ***************\n");
        machine=SAND_DEMOBOARD;
    } else if(strcmp(string,"PS400")==0) {
        pr_alert("************** SAND PS400 ***************\n");
        machine=SAND_PS400;
    } else if(strcmp(string,"PS4K")==0) {
        pr_alert("************** SAND PS4K ***************\n");
        machine=SAND_PS4K;
    } else if(strcmp(string,"IPS100N")==0) {
        pr_alert("************** SAND IPS100N ***************\n");
        machine=SAND_IPS100N;
    } else if(strcmp(string,"IPS420")==0) {
        pr_alert("************** SAND IPS420 ***************\n");
        machine=SAND_IPS420;
    }else if(strcmp(string,"PS4KV2")==0) {
        pr_alert("************** SAND PS4KV2 ***************\n");
        machine=SAND_PS4KV2;
    } else {
        pr_alert("************** WARNING: UNKNOW MACHINE ***************\n");
    }

    strncpy(machine_name,string,sizeof(machine_name));
    pr_info("***** %s******** machine = %d:%s \n", __func__,machine,string);

    return 1;
}
__setup("sand_machine=", sand_parse_string);

static int __init uboot_parse_string(char *string)
{
    strncpy(uboot_ver,string,sizeof(uboot_ver));
    return 1;
}

__setup("uboot_ver=", uboot_parse_string);

int sand_machine_name(char *buffer,char **stat,off_t off,int count,int *peof,void *data)
{
    int len=0;
    len +=snprintf(buffer,32,"%s\n",machine_name);
    return len;
}


int uboot_version(char *buffer,char **stat,off_t off,int count,int *peof,void *data)
{
    int len=0;
    len +=snprintf(buffer,32,"%s\n",uboot_ver);
    return len;
}

//led
#define Led_1 IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK)
#define Led_2 IOMUX_TO_GPIO(MX25_PIN_A18)
#define Led_3 IOMUX_TO_GPIO(MX25_PIN_A19)
#define Led_4 IOMUX_TO_GPIO(MX25_PIN_A24)
#define cpld_wifi IOMUX_TO_GPIO(MX25_PIN_A17)

static struct gpio_led gpio_leds[] ={
    {
        .name = "led1",
        //	.default_trigger = "heartbeat",
        .gpio = Led_1,
        .active_low = 0,
        .default_state = LEDS_GPIO_DEFSTATE_ON,
    },

    {
        .name = "led2",
        .gpio = Led_2,
        .active_low = 0,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
    },

    {
        .name = "led3",
        .gpio = Led_3,
        .active_low = 0,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
    },

    {
        .name = "led4",
        .gpio = Led_4,
        .active_low = 0,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
    },

    {
        .name = "cpldwifi",
        .gpio = cpld_wifi,
        .active_low = 0,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
    },

};

static struct gpio_led_platform_data gpio_led_info ={
    .leds 		= gpio_leds,
    .num_leds   = ARRAY_SIZE(gpio_leds),

};

static struct platform_device leds_gpio = {
    .name = "leds-gpio",
    .id = -1,
    .dev  = {
        .platform_data = &gpio_led_info,
    },
};

static void mxc_init_led(void)
{
    //printk("\r\n[%s,%d]\r\n",__func__,__LINE__);
    platform_device_register(&leds_gpio);
}


extern unsigned int spi_cpld_write(unsigned int offset, unsigned int reg_val);

/*!
 * @file mach-mx25/mx25_3stack.c
 *
 * @brief This file contains the board specific initialization routines.
 *
 * @ingroup MSL_MX25
 */

unsigned int mx25_3stack_board_io;

/* working point(wp): 0 - 399MHz; 1 - 266MHz; 2 - 133MHz; */
/* 24MHz input clock table */
static struct cpu_wp cpu_wp_mx25[] = {
    {
        .pll_rate = 399000000,
        .cpu_rate = 399000000,
        .cpu_podf = 0x0,
        .cpu_voltage = 1450000},
    {
        .pll_rate = 532000000,
        .cpu_rate = 266000000,
        .cpu_podf = 0x1,
        .cpu_voltage = 1340000},
    {
        .pll_rate = 532000000,
        .cpu_rate = 133000000,
        .cpu_podf = 0x3,
        .cpu_voltage = 1340000},
};
struct cpu_wp *get_cpu_wp(int *wp)
{
    *wp = 3;
    return cpu_wp_mx25;
}

static void mxc_nop_release(struct device *dev)
{
    /* Nothing */
}

#define GKEY_0  '0'
#define GKEY_1  '1'
#define GKEY_2  '2'
#define GKEY_3  '3'
#define GKEY_4  '4'
#define GKEY_5  '5'
#define GKEY_6  '6'
#define GKEY_7  '7'
#define GKEY_8  '8'
#define GKEY_9  '9'
#define GKEY_F1 0x40
#define GKEY_F2 0x41
#define GKEY_F3 0x42
#define GKEY_F4 0x43
#define GKEY_00 0x3C
#define GKEY_CANCEL 0x3B
#define GKEY_CLEAR  0x3D
#define GKEY_FEED   0x3E
#define GKEY_ENTER  0x3A
#define GKEY_CAMERA 0xCA

#if defined(CONFIG_KEYBOARD_MXC) || defined(CONFIG_KEYBOARD_MXC_MODULE)

static u16 keymapping_ps400[20] = {
    GKEY_F1, GKEY_1, GKEY_4, GKEY_7, GKEY_FEED,
    GKEY_F2, GKEY_2, GKEY_5, GKEY_8, GKEY_0,
    GKEY_F3, GKEY_3, GKEY_6, GKEY_9, GKEY_00,
    GKEY_F4, GKEY_CANCEL, GKEY_CLEAR, GKEY_ENTER, GKEY_ENTER,
};

static u16 keymapping_demoboard[20] = {
    GKEY_FEED, GKEY_7, GKEY_4, GKEY_1, GKEY_F1,
    GKEY_0, GKEY_8, GKEY_5, GKEY_2, GKEY_F2,
    GKEY_00, GKEY_9,GKEY_6, GKEY_3, GKEY_F3,
    GKEY_ENTER, GKEY_ENTER, GKEY_CLEAR, GKEY_CANCEL, GKEY_F4,
};

static u16 keymapping_ips420[20] = {
    GKEY_F1, GKEY_1, GKEY_4, GKEY_7, GKEY_FEED,
    GKEY_F2, GKEY_2, GKEY_5, GKEY_8, GKEY_0,
    GKEY_F3, GKEY_3, GKEY_6, GKEY_9, GKEY_00,
    GKEY_F4, GKEY_CANCEL, GKEY_CLEAR, GKEY_ENTER, GKEY_CAMERA,
};

static struct resource mxc_kpp_resources[] = {
    [0] = {
        .start = MXC_INT_KPP,
        .end = MXC_INT_KPP,
        .flags = IORESOURCE_IRQ,
    }
};

static struct keypad_data keypad_plat_data_ps400 = {
    .rowmax = 4,
    .colmax = 5,
    .irq = MXC_INT_KPP,
    .learning = 0,
    .delay = 2,
    .matrix = keymapping_ps400,
};

static struct keypad_data keypad_plat_data_demoboard = {
    .rowmax = 4,
    .colmax = 5,
    .irq = MXC_INT_KPP,
    .learning = 0,
    .delay = 2,
    .matrix = keymapping_demoboard,
};

static struct keypad_data keypad_plat_data_ips420 = {
    .rowmax = 4,
    .colmax = 5,
    .irq = MXC_INT_KPP,
    .learning = 0,
    .delay = 2,
    .matrix = keymapping_ips420,
};

/* mxc keypad driver */
static struct platform_device mxc_keypad_device_ps400 = {
    .name = "mxc_keypad",
    .id = 0,
    .num_resources = ARRAY_SIZE(mxc_kpp_resources),
    .resource = mxc_kpp_resources,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &keypad_plat_data_ps400,
    },
};

static struct platform_device mxc_keypad_device_demoboard = {
    .name = "mxc_keypad",
    .id = 0,
    .num_resources = ARRAY_SIZE(mxc_kpp_resources),
    .resource = mxc_kpp_resources,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &keypad_plat_data_demoboard,
    },
};

static struct platform_device mxc_keypad_device_ips420 = {
    .name = "mxc_keypad",
    .id = 0,
    .num_resources = ARRAY_SIZE(mxc_kpp_resources),
    .resource = mxc_kpp_resources,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &keypad_plat_data_ips420,
    },
};

static void mxc_init_keypad(void)
{
    switch(sand_machine())
    {
        case SAND_DEMOBOARD:
            (void)platform_device_register(&mxc_keypad_device_demoboard);
            break;
        case SAND_IPS420:
        case SAND_IPS100N:
            (void)platform_device_register(&mxc_keypad_device_ips420);
            break;
        default:
            pr_alert("Warning: this machine will use the ps400 keyboard config.");
            (void)platform_device_register(&mxc_keypad_device_ps400);
            break;
    }
    return;
}
#else
static inline void mxc_init_keypad(void)
{
}
#endif

/* MTD NAND flash */

#if defined(CONFIG_MTD_NAND_MXC_V2) || defined(CONFIG_MTD_NAND_MXC_V2_MODULE)
#if 1
static struct mtd_partition mxc_nand_partitions[] = {
    {
        .name = "nand.uboot",
        .offset = 0,
        .size = 896 * 1024},
    {
        .name = "nand.cert",
        .offset = MTDPART_OFS_APPEND,
        .size = 128 * 1024},
    {
        .name = "nand.fpga",
        .offset = MTDPART_OFS_APPEND,
        .size = 1 * 1024 * 1024},
    {
        .name = "nand.kernel",
        .offset = MTDPART_OFS_APPEND,
        .size = 4 * 1024 * 1024},
    {
        .name = "nand.kernel_ro",
        .offset = MTDPART_OFS_APPEND,
        .size = 4 * 1024 * 1024},
    {
        .name = "nand.cramfs",
        .offset = MTDPART_OFS_APPEND,
        .size = 4 * 1024 * 1024},
    {
        .name = "nand.rootfs",
	 .offset = MTDPART_OFS_APPEND,
	 .size = 239 * 1024 * 1024},
    {
     .name = "nand.free",
     .offset = MTDPART_OFS_APPEND,
     .size = MTDPART_SIZ_FULL},
};
#endif

#if 0
static struct mtd_partition mxc_nand_partitions[] = {
    {
        .name = "nand.uboot",
        .offset = 0,
        .size = 1 * 1024 * 1024},
    {
        .name = "nand.fpga",
        .offset = MTDPART_OFS_APPEND,
        .size = 1 * 1024 * 1024},
    {
        .name = "nand.kernel",
        .offset = MTDPART_OFS_APPEND,
        .size = 4 * 1024 * 1024},
    {
        .name = "nand.cramfs",
        .offset = MTDPART_OFS_APPEND,
        .size = 4 * 1024 * 1024},
    {
        .name = "nand.rootfs",
        .offset = MTDPART_OFS_APPEND,
        .size = MTDPART_SIZ_FULL},
};
#endif

static struct flash_platform_data mxc_nand_data = {
    .parts = mxc_nand_partitions,
    .nr_parts = ARRAY_SIZE(mxc_nand_partitions),
    .width = 1,
};

static struct platform_device mxc_nand_mtd_device = {
    .name = "mxc_nandv2_flash",
    .id = 0,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &mxc_nand_data,
    },
};

static void mxc_init_nand_mtd(void)
{
    if (__raw_readl(MXC_CCM_RCSR) & MXC_CCM_RCSR_NF16B)
        mxc_nand_data.width = 2;

    platform_device_register(&mxc_nand_mtd_device);
}
#else
static inline void mxc_init_nand_mtd(void)
{
}
#endif

#if defined(CONFIG_FB_MXC_SYNC_PANEL) || \
    defined(CONFIG_FB_MXC_SYNC_PANEL_MODULE)
static const char fb_tm_mode[] = "TM-QVGA";
static const char fb_jh_mode[] = "JH-QVGA";

/* mxc lcd driver */
static struct platform_device mxc_tm_fb_device = {
    .name = "mxc_sdc_fb",
    .id = 0,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &fb_tm_mode,
        .coherent_dma_mask = 0xFFFFFFFF,
    },
};

static struct platform_device mxc_jh_fb_device = {
    .name = "mxc_sdc_fb",
    .id = 0,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &fb_jh_mode,
        .coherent_dma_mask = 0xFFFFFFFF,
    },
};

/*
 * Power on/off CPT VGA panel.
 */
void board_power_lcd(int on)
{
    if (on)
        mx2fb_set_brightness(MXC_DEFAULT_INTENSITY);
    else
        mx2fb_set_brightness(MXC_INTENSITY_OFF);
}
EXPORT_SYMBOL_GPL(board_power_lcd);

static void mxc_init_fb(void)
{
    switch(sand_machine())
    {
        case SAND_IPS100N:
            pr_alert("zengshu:init JH_QVGA\n");
            (void)platform_device_register(&mxc_jh_fb_device);
            break;
        default:
            pr_alert("zengshu:init JH_QVGA\n");
            (void)platform_device_register(&mxc_tm_fb_device);
            break;
    }
}
#else
static inline void mxc_init_fb(void)
{
}
#endif

#if defined(CONFIG_BACKLIGHT_MXC)
static struct platform_device mxcbl_devices[] = {
#if defined(CONFIG_BACKLIGHT_MXC_LCDC) || \
    defined(CONFIG_BACKLIGHT_MXC_LCDC_MODULE)
    {
        .name = "mxc_lcdc_bl",
        .id = 0,
    },
#elif defined(CONFIG_BACKLIGHT_MXC_CPLD) || \
    defined(CONFIG_BACKLIGHT_MXC_CPLD_MODULE)
    {
        .name = "mxc_cpld_bl",
        .id = 0,
    },
#endif
};

static inline void mxc_init_bl(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(mxcbl_devices); i++)
        platform_device_register(&mxcbl_devices[i]);
}
#else
static inline void mxc_init_bl(void)
{
}
#endif

#if 0

/*!
 * Power Key interrupt handler.
 */
static irqreturn_t power_key_int(int irq, void *dev_id)
{
    pr_info("on-off key pressed\n");
    return 0;
}

#endif

extern void (*pm_power_off)(void);

#define CPLD_HARDWARE_POWEROFF_REG  0xA0300000

static unsigned char *hardware_poweroff_addr = NULL;

void __imx258_hardware_off(void)
{
    *hardware_poweroff_addr |= (3<<2);
}

int __imx258__poweroff_dev=0;
EXPORT_SYMBOL(__imx258__poweroff_dev);
irqreturn_t  __imx258_poweroff_interrupt(int irq, void *dev_id);

/*!
 * Power Key initialization.
 */
static int __init mxc_init_power_key(void)
{
//	/*Set power key as wakeup resource */
//	int irq, ret;

//	mxc_request_iomux(MX25_PIN_A25, MUX_CONFIG_ALT5);
//	mxc_iomux_set_pad(MX25_PIN_A25, PAD_CTL_DRV_NORMAL);
//	gpio_request(IOMUX_TO_GPIO(MX25_PIN_A25), NULL);
//	gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_A25));
//
//	irq = IOMUX_TO_IRQ(MX25_PIN_A25);
//	set_irq_type(irq, IRQF_TRIGGER_RISING);
//	ret = request_irq(irq, power_key_int, 0, "power_key", 0);
//	if (ret)
//		pr_info("register on-off key interrupt failed\n");
//	else
//		enable_irq_wake(irq);


    hardware_poweroff_addr = (unsigned char *)ioremap_nocache(CPLD_HARDWARE_POWEROFF_REG, 1);
    if(!hardware_poweroff_addr)
    {
        pr_info("hardware_poweroff_addr ioremap_nocache failed\n");
    }
    else
    {
        pm_power_off = __imx258_hardware_off;

        pr_info("hardware_poweroff_reg = %02X\n", *hardware_poweroff_addr);
        
        mxc_free_iomux(MX25_PIN_GPIO_E, MUX_CONFIG_GPIO);
        mxc_request_iomux(MX25_PIN_GPIO_E, MUX_CONFIG_FUNC);
        mxc_iomux_set_pad(MX25_PIN_GPIO_E, PAD_CTL_PKE_NONE);
        
        gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_E));
        set_irq_type(IOMUX_TO_IRQ(MX25_PIN_GPIO_E), IRQF_TRIGGER_FALLING);
        
        if(request_irq(IOMUX_TO_IRQ(MX25_PIN_GPIO_E), __imx258_poweroff_interrupt, IRQF_DISABLED, "poweroff_irq", &__imx258__poweroff_dev))
        {
            pr_info("request_irq poweroff_irq failed\n");
        }
        else
        {
            pr_info("request_irq poweroff_irq success\n");
        }        
    }

    return 0;


//	return ret;
}

late_initcall(mxc_init_power_key);

static struct spi_board_info mxc_spi_board_info[] __initdata = {
#if 0
	{
	 .modalias = "RF_SPI",
	 .max_speed_hz = 4000000,
	 .bus_num = 1,
	 .chip_select = 1,
	 .mode = SPI_MODE_0,
	 },
#endif
#if 1
	 {
	 .modalias = "LCD_SPI",
	 .max_speed_hz = 250000,	/* max spi SCK clock speed in HZ */
	 .bus_num = 1,
	 .chip_select = 0,
	 .mode = SPI_MODE_0,
	 },
#endif
};

static struct mxc_camera_platform_data camera_data = {
    .core_regulator = NULL,
    .io_regulator = NULL,
    .analog_regulator = NULL,
    .gpo_regulator = NULL,
    .mclk = 24000000,
};

static struct i2c_board_info mxc_i2c_board_info[] __initdata = {
#if 0
    {
        .type = "sgtl5000-i2c",
        .addr = 0x0a,
    },
    {
        .type = "ak5702-i2c",
        .addr = 0x13,
    },
#endif
#if 0
    {
        .type = "ov2640",
        .addr = 0x30,
        .platform_data = (void *)&camera_data,
    },
#endif
    {
        .type = "ov2659",
        .addr = 0x30,
        .platform_data = (void *)&camera_data,
    },
    {
        .type = "hm2057",
        .addr = 0x24,
        .platform_data = (void *)&camera_data,
    },
    {
        .type = "gps",
        .addr = 0x42,
    },
};

#if defined(CONFIG_SND_SOC_IMX_3STACK_SGTL5000) \
    || defined(CONFIG_SND_SOC_IMX_3STACK_SGTL5000_MODULE)
static struct mxc_audio_platform_data sgtl5000_data = {
    .ssi_num = 2,
    .src_port = 1,
    .ext_port = 4,
    .hp_irq = IOMUX_TO_IRQ(MX25_PIN_A10),
    .hp_status = headphone_det_status,
    .sysclk = 8300000,
};

static struct platform_device mxc_sgtl5000_device = {
    .name = "imx-3stack-sgtl5000",
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &sgtl5000_data,
    },
};

static void mxc_init_sgtl5000(void)
{
    struct clk *cko1, *parent;
    unsigned long rate;

    /* cko1 clock */
    mxc_request_iomux(MX25_PIN_CLKO, MUX_CONFIG_FUNC);

    cko1 = clk_get(NULL, "clko_clk");
    if (IS_ERR(cko1))
        return;
    parent = clk_get(NULL, "ipg_clk");
    if (IS_ERR(parent))
        return;
    clk_set_parent(cko1, parent);
    rate = clk_round_rate(cko1, 13000000);
    if (rate < 8000000 || rate > 27000000) {
        pr_err("Error: SGTL5000 mclk freq %ld out of range!\n", rate);
        clk_put(parent);
        clk_put(cko1);
        return;
    }
    clk_set_rate(cko1, rate);
    clk_enable(cko1);
    sgtl5000_data.sysclk = rate;
    sgtl5000_enable_amp();
    platform_device_register(&mxc_sgtl5000_device);
}
#else
static inline void mxc_init_sgtl5000(void)
{
}
#endif



static void mxc_init_clko(void)
{
    struct clk *cko1, *parent;
    unsigned long rate;

    /* cko1 clock */
    mxc_request_iomux(MX25_PIN_CLKO, MUX_CONFIG_FUNC);

    __raw_writel(__raw_readl(MXC_CCM_MCR) | (1<<MXC_CCM_MCR_CLKO_EN_OFFSET), MXC_CCM_MCR);

    //per[0] = upll
    __raw_writel(__raw_readl(MXC_CCM_MCR) | (1<<0), MXC_CCM_MCR);

    cko1 = clk_get(NULL, "clko_clk");
    if (IS_ERR(cko1))
        return;
    parent = clk_get(NULL, "cpu_clk");
    if (IS_ERR(parent))
        return;
    clk_set_parent(cko1, parent);

    rate = clk_round_rate(cko1, 50000000);

    clk_set_rate(cko1, rate);
    clk_enable(cko1);

    pr_info("clko freq: %ld\n", rate);
}

static void backlight_release(struct device * dev)
{
    return ;
}

static struct platform_device backlight_device ={
    .name = "backlight",
    .id   = 3,
    .dev  = {
        .release  =  backlight_release,
    },
};

static void buzzer_release(struct device * dev)
{
    return ;
}

static struct platform_device buzzer_device ={
    .name = "buzzer",
    .id   = 0,
    .dev  = {
        .release  =  buzzer_release,
    },
};



#define PWM1_ADDRESS    0x53FE0000
#define PWM4_ADDRESS    0x53FC8000

#define BUZZER_FEQ      2750
#define BACKLIGHT_FEQ   20000

#define PWMCR           0
#define PWMSR           1
#define PWMIR           2
#define PWMSAR          3
#define PWMPR           4
#define PWMCNR          5

struct pwm_struct {
    unsigned long freq;
    unsigned long rate;
    unsigned int  contrast;
    struct clk *pwm_clk;
    struct clk *per_pwm_clk;
    unsigned long *pwm_addr;
};

static struct pwm_struct *buzzer;
static struct pwm_struct *backlight;


#define BUZZER_GATE_ON      1
#define BUZZER_GATE_OFF     0

static unsigned int Buzzer_State;

static __init void mxc_init_pwm1(void)
{
    buzzer = (struct pwm_struct *)kmalloc(sizeof(struct pwm_struct), GFP_KERNEL);

    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_PWM, MUX_CONFIG_ALT7);

    /* pwm1 output */
    mxc_request_iomux(MX25_PIN_PWM, MUX_CONFIG_FUNC);
    mxc_iomux_set_pad(MX25_PIN_PWM, PAD_CTL_PKE_NONE);

    buzzer->pwm_clk = clk_get(&buzzer_device.dev, "pwm_clk");
    buzzer->rate = clk_get_rate(buzzer->pwm_clk);

    clk_enable(buzzer->pwm_clk);

    Buzzer_State = BUZZER_GATE_ON;

    pr_info("pwm_clk get freq: %ld\n", buzzer->rate);

    buzzer->freq = BUZZER_FEQ;
    buzzer->contrast = 50;

    buzzer->pwm_addr = (unsigned long *)ioremap_nocache(PWM1_ADDRESS, 4);

    *(buzzer->pwm_addr + PWMCR) = 0;

    *(buzzer->pwm_addr + PWMPR) = ((buzzer->rate / buzzer->freq) - 2);

    *(buzzer->pwm_addr + PWMSAR) = 0;

    *(buzzer->pwm_addr + PWMCR) |= (2<<16) | (0<<18);
    *(buzzer->pwm_addr + PWMIR) |= (1<<1);

    *(buzzer->pwm_addr + PWMCR) |= (1<<0);

    pr_info("pwm1 freq: %ld\n", buzzer->rate/(*(buzzer->pwm_addr + PWMPR) + 2));
}

void Buzzer_Contrast_Set(unsigned int contrast)
{
    buzzer->contrast = contrast;

    *(buzzer->pwm_addr + PWMCR) &= (~(1<<0));
    *(buzzer->pwm_addr + PWMPR) = ((buzzer->rate / buzzer->freq) - 2);
    *(buzzer->pwm_addr + PWMSAR) = ((*(buzzer->pwm_addr + PWMPR)) * buzzer->contrast) / 100;
    *(buzzer->pwm_addr + PWMCR) |= (1<<0);
}
EXPORT_SYMBOL(Buzzer_Contrast_Set);

void Buzzer_Freq_Set(unsigned long freq)
{
    if(freq == 0)
    {
        return;
    }

    *(buzzer->pwm_addr + PWMCR) &= (~(1<<0));

    buzzer->freq = freq;

    *(buzzer->pwm_addr + PWMPR) = ((buzzer->rate / buzzer->freq) - 2);
    *(buzzer->pwm_addr + PWMSAR) = ((*(buzzer->pwm_addr + PWMPR)) * buzzer->contrast) / 100;
    *(buzzer->pwm_addr + PWMCR) |= (1<<0);
}
EXPORT_SYMBOL(Buzzer_Freq_Set);

void Buzzer_On(unsigned long freq)
{
    if(Buzzer_State == BUZZER_GATE_OFF)
    {
        return;
    }

    if(freq != 0)
    {
        buzzer->freq = freq;
    }

    *(buzzer->pwm_addr + PWMCR) &= (~(1<<0));

    *(buzzer->pwm_addr + PWMPR) = ((buzzer->rate / buzzer->freq) - 2);
    *(buzzer->pwm_addr + PWMSAR) = ((*(buzzer->pwm_addr + PWMPR)) * buzzer->contrast) / 100;
    *(buzzer->pwm_addr + PWMCR) |= (1<<0);
}
EXPORT_SYMBOL(Buzzer_On);

void Buzzer_Off(void)
{
    if(Buzzer_State == BUZZER_GATE_OFF)
    {
        return;
    }

    *(buzzer->pwm_addr + PWMCR) &= (~(1<<0));

    *(buzzer->pwm_addr + PWMSAR) = 0;

    *(buzzer->pwm_addr + PWMCR) |= (1<<0);
}
EXPORT_SYMBOL(Buzzer_Off);


void Buzzer_Gate_Set(bool state)
{
    if(state == true)
    {
        Buzzer_State = BUZZER_GATE_ON;
    }
    else
    {
        Buzzer_State = BUZZER_GATE_OFF;
    }
}
EXPORT_SYMBOL(Buzzer_Gate_Set);

static struct timer_list t_list;

static void Poweron_Beep(unsigned long arg)
{
    Buzzer_Off();
    del_timer(&t_list);
}

#define LCD_CTRL_REG 0xA0B00000
#define LCD_LIGHT_REG 0xA0B00001
unsigned char *lcd_ctrl_reg_p=NULL;
unsigned char *lcd_light_reg_p=NULL;

static __init void mxc_init_pwm4(void)
{
    if(sand_machine()==SAND_IPS100N)
    {
        lcd_ctrl_reg_p = (unsigned char *)ioremap_nocache(LCD_CTRL_REG, 1);
        lcd_light_reg_p = (unsigned char *)ioremap_nocache(LCD_LIGHT_REG, 1);
        *lcd_ctrl_reg_p = 0xC0;
        *lcd_light_reg_p = 0xAA;
    }
    else
    {
        backlight = (struct pwm_struct *)kmalloc(sizeof(struct pwm_struct), GFP_KERNEL);

        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_FUNC);
        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT1);
        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT2);
        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT3);
        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT4);
        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT5);
        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT6);
        mxc_free_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT7);

        /* pwm4 output */
        mxc_request_iomux(MX25_PIN_CONTRAST, MUX_CONFIG_ALT4);
        mxc_iomux_set_pad(MX25_PIN_CONTRAST, PAD_CTL_PKE_NONE);

        backlight->pwm_clk = clk_get(&backlight_device.dev, "pwm_clk");
        backlight->rate = clk_get_rate(backlight->pwm_clk);

        clk_enable(backlight->pwm_clk);

        pr_info("pwm4_clk get freq: %ld\n", backlight->rate);

        backlight->freq = BACKLIGHT_FEQ;
        backlight->contrast = 99;

        backlight->pwm_addr = (unsigned long *)ioremap_nocache(PWM4_ADDRESS, 4);

        *(backlight->pwm_addr + PWMCR) = 0;

        *(backlight->pwm_addr + PWMPR) = ((backlight->rate / backlight->freq) - 2);
        *(backlight->pwm_addr + PWMSAR) = ((*(backlight->pwm_addr + PWMPR)) * backlight->contrast) / 100;

        *(backlight->pwm_addr + PWMCR) |= (2<<16) | (0<<18);
        *(backlight->pwm_addr + PWMIR) |= (1<<1);

        *(backlight->pwm_addr + PWMCR) |= (1<<0);

        pr_info("pwm4 freq: %ld\n", backlight->rate/(*(backlight->pwm_addr + PWMPR) + 2));
        pr_info("lcd backlight open\n");
    }
}

void Backlight_Contrast_Set(unsigned int contrast)
{
    if(sand_machine()==SAND_IPS100N)
    {
        return;
    }
    else
    {
        *(backlight->pwm_addr + PWMCR) &= (~(1<<0));

        backlight->contrast = contrast;

        *(backlight->pwm_addr + PWMPR) = ((backlight->rate / backlight->freq) - 2);
        *(backlight->pwm_addr + PWMSAR) = ((*(backlight->pwm_addr + PWMPR)) * backlight->contrast) / 100;
        *(backlight->pwm_addr + PWMCR) |= (1<<0);
    }
}
EXPORT_SYMBOL(Backlight_Contrast_Set);

void Backlight_Freq_Set(unsigned long freq)
{
    if(sand_machine()==SAND_IPS100N)
    {
        return;
    }
    else
    {
        if(freq == 0)
        {
            return;
        }

        *(backlight->pwm_addr + PWMCR) &= (~(1<<0));

        backlight->freq = freq;

        *(backlight->pwm_addr + PWMPR) = ((backlight->rate / backlight->freq) - 2);
        *(backlight->pwm_addr + PWMSAR) = ((*(backlight->pwm_addr + PWMPR)) * backlight->contrast) / 100;
        *(backlight->pwm_addr + PWMCR) |= (1<<0);
    }
}
EXPORT_SYMBOL(Backlight_Freq_Set);

void Backlight_On(unsigned long freq)
{
    if(sand_machine()==SAND_IPS100N)
    {
        if(lcd_ctrl_reg_p == NULL)
            return;
        if(lcd_light_reg_p == NULL)
            return;
        *lcd_ctrl_reg_p = 0xC0;
        *lcd_light_reg_p = 0xAA;
    }
    else
    {
        if(freq != 0)
        {
            backlight->freq = freq;
        }

        *(backlight->pwm_addr + PWMCR) &= (~(1<<0));

        *(backlight->pwm_addr + PWMPR) = ((backlight->rate / backlight->freq) - 2);
        *(backlight->pwm_addr + PWMSAR) = ((*(backlight->pwm_addr + PWMPR)) * backlight->contrast) / 100;
        *(backlight->pwm_addr + PWMCR) |= (1<<0);
    }
}
EXPORT_SYMBOL(Backlight_On);

void Backlight_Off(void)
{
    if(sand_machine()==SAND_IPS100N)
    {
        if(lcd_ctrl_reg_p == NULL)
            return;
        if(lcd_light_reg_p == NULL)
            return;
        *lcd_ctrl_reg_p = 0x00;
        *lcd_light_reg_p = 0x00;
    }
    else
    {
        *(backlight->pwm_addr + PWMCR) &= (~(1<<0));

        *(backlight->pwm_addr + PWMSAR) = 0;

        *(backlight->pwm_addr + PWMCR) |= (1<<0);
    }
}
EXPORT_SYMBOL(Backlight_Off);

#if defined(CONFIG_SND_SOC_IMX_3STACK_AK5702) \
    || defined(CONFIG_SND_SOC_IMX_3STACK_AK5702_MODULE)
static struct platform_device mxc_ak5702_device = {
    .name = "imx-3stack-ak5702",
    .dev = {
        .release = mxc_nop_release,
    },
};

static void mxc_init_ak5702(void)
{
    platform_device_register(&mxc_ak5702_device);
}
#else
static inline void mxc_init_ak5702(void)
{
}
#endif

#if  defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)
static struct resource smsc911x_resources[] = {
    {
        .start = LAN9217_BASE_ADDR,
        .end = LAN9217_BASE_ADDR + 255,
        .flags = IORESOURCE_MEM,
    },
    {
        .start = MXC_BOARD_IRQ_START,
        .flags = IORESOURCE_IRQ,
    }
};

struct smsc911x_platform_config smsc911x_config = {
    .irq_polarity = SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
    .flags = 0x8000 | SMSC911X_USE_16BIT | SMSC911X_FORCE_INTERNAL_PHY,
};

static struct platform_device smsc_lan9217_device = {
    .name = "smsc911x",
    .id = 0,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &smsc911x_config,
    },
    .num_resources = ARRAY_SIZE(smsc911x_resources),
    .resource = smsc911x_resources,
};

static int __init mxc_init_enet(void)
{
    (void)platform_device_register(&smsc_lan9217_device);
    return 0;
}
#else
static int __init mxc_init_enet(void)
{
    return 0;
}
#endif

late_initcall(mxc_init_enet);

#if defined(CONFIG_FEC) || defined(CONFIG_FEC_MODULE)
static struct resource mxc_fec_resources[] = {
    {
        .start	= FEC_BASE_ADDR,
        .end	= FEC_BASE_ADDR + 0xfff,
        .flags	= IORESOURCE_MEM
    }, {
        .start	= MXC_INT_FEC,
        .end	= MXC_INT_FEC,
        .flags	= IORESOURCE_IRQ
    },
};

struct platform_device mxc_fec_device = {
    .name = "fec",
    .id = 0,
    .num_resources = ARRAY_SIZE(mxc_fec_resources),
    .resource = mxc_fec_resources,
};

static __init int mxc_init_fec(void)
{
    return platform_device_register(&mxc_fec_device);
}
#else
static inline int mxc_init_fec(void)
{
    return 0;
}
#endif

#if defined(CONFIG_IMX_SIM) || defined(CONFIG_IMX_SIM_MODULE)
/* Used to configure the SIM bus */
static struct mxc_sim_platform_data sim1_data = {
    .clk_rate = 5000000,
    .clock_sim = "sim1_clk",
    .power_sim = NULL,
    .init = NULL,
    .exit = NULL,
    .detect = 1,
};

/*!
 * Resource definition for the SIM
 */
static struct resource mxc_sim1_resources[] = {
    [0] = {
        .start = SIM1_BASE_ADDR,
        .end = SIM1_BASE_ADDR + SZ_4K - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = MXC_INT_SIM1,
        .end = MXC_INT_SIM1,
        .flags = IORESOURCE_IRQ,
    },
    [2] = {
        .start = 0,
        .end = 0,
        .flags = IORESOURCE_IRQ,
    },
};

/*! Device Definition for IMX SIM */
static struct platform_device mxc_sim1_device = {
    .name = "mxc_sim",
    .id = 0,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &sim1_data,
    },
    .num_resources = ARRAY_SIZE(mxc_sim1_resources),
    .resource = mxc_sim1_resources,
};

static inline void mxc_init_sim(void)
{
    (void)platform_device_register(&mxc_sim1_device);
}
#else
static inline void mxc_init_sim(void)
{
}
#endif

#if defined(CONFIG_MMC_IMX_ESDHCI) || defined(CONFIG_MMC_IMX_ESDHCI_MODULE)
static struct mxc_mmc_platform_data mmc1_data = {
    .ocr_mask = MMC_VDD_29_30 | MMC_VDD_32_33,
    .caps = MMC_CAP_4_BIT_DATA,
    .min_clk = 400000,
    .max_clk = 52000000,
    //	.card_inserted_state = 1,
    //xu.xb
    .card_inserted_state = 0,
    .status = sdhc_get_card_det_status,
    .wp_status = sdhc_write_protect,
    .clock_mmc = "esdhc_clk",
};

/*!
 * Resource definition for the SDHC1
 */
static struct resource mxcsdhc1_resources[] = {
    [0] = {
        .start = MMC_SDHC1_BASE_ADDR,
        .end = MMC_SDHC1_BASE_ADDR + SZ_4K - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = MXC_INT_SDHC1,
        .end = MXC_INT_SDHC1,
        .flags = IORESOURCE_IRQ,
    },
    [2] = {
        //	       .start = IOMUX_TO_IRQ(MX25_PIN_A15),
        //	       .end = IOMUX_TO_IRQ(MX25_PIN_A15),
        //xu.xb
        .start = IOMUX_TO_IRQ(MX25_PIN_D9),
        .end = IOMUX_TO_IRQ(MX25_PIN_D9),
        .flags = IORESOURCE_IRQ,
    },
};

/*! Device Definition for MXC SDHC1 */
static struct platform_device mxcsdhc1_device = {
    .name = "mxsdhci",
    .id = 0,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &mmc1_data,
    },
    .num_resources = ARRAY_SIZE(mxcsdhc1_resources),
    .resource = mxcsdhc1_resources,
};

#ifdef CONFIG_MMC_IMX_ESDHCI_SELECT2
static struct mxc_mmc_platform_data mmc2_data = {
    .ocr_mask = MMC_VDD_29_30 | MMC_VDD_32_33,
    .caps = MMC_CAP_4_BIT_DATA,
    .min_clk = 400000,
    .max_clk = 52000000,
    .card_fixed = 1,
    .card_inserted_state = 1,
    .status = sdhc_get_card_det_status,
    .clock_mmc = "esdhc2_clk",
};

/*!
 * Resource definition for the SDHC2
 */
static struct resource mxcsdhc2_resources[] = {
    [0] = {
        .start = MMC_SDHC2_BASE_ADDR,
        .end = MMC_SDHC2_BASE_ADDR + SZ_4K - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = MXC_INT_SDHC2,
        .end = MXC_INT_SDHC2,
        .flags = IORESOURCE_IRQ,
    },
};

/*! Device Definition for MXC SDHC2 */
static struct platform_device mxcsdhc2_device = {
    .name = "mxsdhci",
    .id = 1,
    .dev = {
        .release = mxc_nop_release,
        .platform_data = &mmc2_data,
    },
    .num_resources = ARRAY_SIZE(mxcsdhc2_resources),
    .resource = mxcsdhc2_resources,
};
#endif

static inline void mxc_init_mmc(void)
{
    (void)platform_device_register(&mxcsdhc1_device);
#ifdef CONFIG_MMC_IMX_ESDHCI_SELECT2
    (void)platform_device_register(&mxcsdhc2_device);
#endif
}
#else
static inline void mxc_init_mmc(void)
{
}
#endif

static void __init mx25_3stack_timer_init(void)
{
    mx25_clocks_init(24000000);
}

static struct sys_timer mxc_timer = {
    .init = mx25_3stack_timer_init,
};

#if defined(CONFIG_CAN_FLEXCAN) || defined(CONFIG_CAN_FLEXCAN_MODULE)
static void flexcan_xcvr_enable(int id, int en)
{
    static int pwdn;

    if (id != 1)		/* MX25 3-stack uses only CAN2 */
        return;

    if (en) {
        if (!pwdn++)
            gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D14), 0);
    } else {
        if (!--pwdn)
            gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D14), 1);
    }
}

struct flexcan_platform_data flexcan_data[] = {
    {
        .core_reg = NULL,
        .io_reg = NULL,
        .xcvr_enable = flexcan_xcvr_enable,
        .active = gpio_can_active,
        .inactive = gpio_can_inactive,},
    {
        .core_reg = NULL,
        .io_reg = NULL,
        .xcvr_enable = flexcan_xcvr_enable,
        .active = gpio_can_active,
        .inactive = gpio_can_inactive,},
};
#endif

/*!
 * Board specific fixup function. It is called by \b setup_arch() in
 * setup.c file very early on during kernel starts. It allows the user to
 * statically fill in the proper values for the passed-in parameters. None of
 * the parameters is used currently.
 *
 * @param  desc         pointer to \b struct \b machine_desc
 * @param  tags         pointer to \b struct \b tag
 * @param  cmdline      pointer to the command line
 * @param  mi           pointer to \b struct \b meminfo
 */
static void __init fixup_mxc_board(struct machine_desc *desc, struct tag *tags,
        char **cmdline, struct meminfo *mi)
{
    mxc_cpu_init();

#ifdef CONFIG_DISCONTIGMEM
    do {
        int nid;
        mi->nr_banks = MXC_NUMNODES;
        for (nid = 0; nid < mi->nr_banks; nid++)
            SET_NODE(mi, nid);
    } while (0);
#endif
}


spinlock_t poweroff_lock=SPIN_LOCK_UNLOCKED;
irqreturn_t  __imx258_poweroff_interrupt(int irq, void *dev_id)
{
    //    pr_info("imx258_poweroff_interrupt isr: ");

    spin_lock(&poweroff_lock);

    if(*hardware_poweroff_addr & (1<<4))
    {
        Buzzer_On(BUZZER_FEQ);
        mdelay(500);
        Buzzer_Off();

        orderly_poweroff(1);
    }
    else
    {
        pr_info("poweroff_interrupt is not poweroff interrupt\n");
    }

    spin_unlock(&poweroff_lock);

    return IRQ_HANDLED;
}

EXPORT_SYMBOL(__imx258_poweroff_interrupt);

/*!
 * Board specific initialization.
 */
static void __init mxc_board_init(void)
{
    pr_info("AIPS1 VA base: 0x%p\n", IO_ADDRESS(AIPS1_BASE_ADDR));
    mxc_cpu_common_init();
    mxc_register_gpios();
    mx25_3stack_gpio_init();
    early_console_setup(saved_command_line);
    mxc_init_keypad();
#ifdef CONFIG_I2C
    i2c_register_board_info(0, mxc_i2c_board_info,
            ARRAY_SIZE(mxc_i2c_board_info));
#endif
    spi_register_board_info(mxc_spi_board_info,
            ARRAY_SIZE(mxc_spi_board_info));
    //mx25_3stack_init_mc34704(); //we don't have 34704 <zengshu>
    mxc_init_clko();  //clko for cpld

    //for backlight
    mxc_init_pwm4();
    mxc_init_fb();

    mxc_init_bl();			//ps400 we have our own backlight ctrl using CPLD <zengshu>

    mxc_init_nand_mtd();
    //	mxc_init_sgtl5000();
    //	mxc_init_ak5702();
    mxc_init_mmc();
    mxc_init_sim();

    //pull up power 5v
    if((SAND_PS400==sand_machine())||(SAND_IPS420==sand_machine()))
    {
        mxc_request_iomux(MX25_PIN_VSTBY_REQ, MUX_CONFIG_ALT5);
        gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ),"VSTBY");
        gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ), 1);
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ), 1);
    }else if(SAND_IPS100N==sand_machine()) {
        mxc_request_iomux(MX25_PIN_VSTBY_REQ, MUX_CONFIG_ALT5);
        gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ),"VSTBY");
        gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ), 1);
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ), 1);
        mxc_init_fec();
    } else {
        mxc_init_fec();
    }

    //for buzzer
    mxc_init_pwm1();

    t_list.expires=jiffies;
    t_list.function=Poweron_Beep;
    init_timer(&t_list);
    t_list.expires=jiffies + (HZ/5);
    add_timer(&t_list);
    Buzzer_On(BUZZER_FEQ);

    mxc_init_led();

    //CPLD RESET
    if(sand_machine()!=SAND_IPS100N)
    {
        mxc_free_iomux(MX25_PIN_BCLK, MUX_CONFIG_FUNC);
        mxc_request_iomux(MX25_PIN_BCLK, MUX_CONFIG_GPIO);
        gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_BCLK), 1);
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_BCLK), 1);
    }

    pr_info("***** %s******** machine = %d \n", __func__,machine);

    //create a /proc/sand_machine node
    create_proc_read_entry("sand_machine",0,0,sand_machine_name,NULL);

    //create a /proc/uboot_ver node
    create_proc_read_entry("uboot_ver",0,0,uboot_version,NULL);
}

/*
 * The following uses standard kernel macros define in arch.h in order to
 * initialize __mach_desc_MX25_3DS data structure.
 */
/* *INDENT-OFF* */
MACHINE_START(MX25_3DS, "SAND MX25 Board based on the Freescale MX25 3-Stack Board")
	/* Maintainer: Freescale Semiconductor, Inc. */
	.phys_io = AIPS1_BASE_ADDR,
	.io_pg_offst = ((AIPS1_BASE_ADDR_VIRT) >> 18) & 0xfffc,
	.boot_params = PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mx25_map_io,
	.init_irq = mxc_init_irq,
	.init_machine = mxc_board_init,
	.timer = &mxc_timer,
MACHINE_END
