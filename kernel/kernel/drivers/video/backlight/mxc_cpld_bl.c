/*
 * Copyright 2007 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
/*!
 * @defgroup CPLD_BL MXC CPLD Backlight Driver
 */
/*!
 * @file mxc_cpld_bl.c
 *
 * @brief Backlight Driver for LCDC PWM on Freescale MXC/i.MX platforms.
 *
 * This file contains API defined in include/linux/clk.h for setting up and
 * retrieving clocks.
 *
 * Based on Sharp's Corgi Backlight Driver
 *
 * @ingroup CPLD_BL
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/clk.h>
#include <mach-mx25/machine.h>
#include <mach-mx25/iomux.h>

#define MXC_MAX_INTENSITY   255
#define MXC_DEFAULT_INTENSITY   255
#define MXC_INTENSITY_OFF   0

#define WEIM_CS3_CPLD_BACKLIGHT_ADDR 0xb2a00000
#define WEIM_CS3_FPGA_BACKLIGHT_ADDR 0xb2000070
#define WEIM_CS0_CPLD_BACKLIGHT_ADDR 0xa0b00000
#define WEIM_CS0_FPGA_BACKLIGHT_ADDR 0xa0000070

static volatile uint8_t *reg_backlight_cs;


extern void Backlight_On(unsigned long freq);
extern void Backlight_Off(void);
struct mxcbl_dev_data {
		int intensity;
};

static inline void set_brightness(uint8_t intensity)
{
	if(sand_machine()==SAND_PS4KV2)
	{
		iowrite8(ioread8(reg_backlight_cs)&(~(0x07)<<5),reg_backlight_cs);
		iowrite8(ioread8(reg_backlight_cs)|((intensity/32)<<5),reg_backlight_cs);
	}
	else if(sand_machine()==SAND_IPS420)
	{
		if(intensity)
		{
//		            //IO口控制背光
//                	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A14), 1);
                	
                  //PWM控制背光	
                  Backlight_On(0);
		}
		else
		{
			
//                	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A14), 0);
                
                 Backlight_Off();
		}		
	}
	else
	{
		if(intensity)
		{
			iowrite8(ioread8(reg_backlight_cs)|(1<<7),reg_backlight_cs);
		}
		else
		{
			iowrite8(ioread8(reg_backlight_cs)&(~(1<<7)),reg_backlight_cs);
		}
	}
}

static inline void lcd_enable(int f)
{
	if(sand_machine()==SAND_PS4KV2 || sand_machine()==SAND_PS400)
	{
		return;
	}
	else
	{
//		if(f)
//		{
//			iowrite8(ioread8(reg_backlight_cs)|(1<<6),reg_backlight_cs);
//		}
//		else
//		{
//			iowrite8(ioread8(reg_backlight_cs)&(~(1<<6)),reg_backlight_cs);
//		}
        
        if(f)
        {
            Backlight_On(0);
        }
        else
        {
            Backlight_Off();
        }            
	}
}

static int mxcbl_send_intensity(struct backlight_device *bd)
{
	int intensity = bd->props.brightness;
	struct mxcbl_dev_data *devdata = dev_get_drvdata(&bd->dev);

	if (bd->props.power != FB_BLANK_UNBLANK)
		intensity = 0;
	if (bd->props.fb_blank != FB_BLANK_UNBLANK)
		intensity = 0;

	if ((devdata->intensity == 0) && (intensity != 0))
		lcd_enable(true);

	set_brightness(intensity);

	if ((devdata->intensity != 0) && (intensity == 0))
		lcd_enable(false);
	
	devdata->intensity = intensity;

	return 0;
}

static int mxcbl_get_intensity(struct backlight_device *bd)
{
	struct mxcbl_dev_data *devdata = dev_get_drvdata(&bd->dev);
	return devdata->intensity;
}

static int mxcbl_check_fb(struct fb_info *info)
{
	if (strcmp(info->fix.id, "DISP0 BG") == 0) {
		return 1;
	}
	return 0;
}

static struct backlight_ops mxcbl_ops = {
	.get_brightness = mxcbl_get_intensity,
	.update_status = mxcbl_send_intensity,
	.check_fb = mxcbl_check_fb,
};

static int __init mxcbl_probe(struct platform_device *pdev)
{
	struct backlight_device *bd;
	struct mxcbl_dev_data *devdata;
	int ret = 0;

	devdata = kzalloc(sizeof(struct mxcbl_dev_data), GFP_KERNEL);
	if (!devdata)
		return -ENOMEM;

	if(sand_machine()==SAND_PS4KV2)
	{
		reg_backlight_cs = ioremap_nocache(WEIM_CS0_FPGA_BACKLIGHT_ADDR,1);
	}
	else if(sand_machine()==SAND_IPS420 || sand_machine() == SAND_PS400)
	{
//		mxc_request_iomux(MX25_PIN_A14, MUX_CONFIG_ALT5); //request gpio
//                gpio_request(IOMUX_TO_GPIO(MX25_PIN_A14),"A14");
//                gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A14), 1);
//                gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A14), 1);
//		reg_backlight_cs = ioremap_nocache(WEIM_CS0_CPLD_BACKLIGHT_ADDR,1);
		
		
		
	}
	else if(sand_machine()==SAND_DEMOBOARD)
	{
		reg_backlight_cs = ioremap_nocache(WEIM_CS3_CPLD_BACKLIGHT_ADDR,1);
	}
	else if(sand_machine()==SAND_PS4K)
	{
		reg_backlight_cs = ioremap_nocache(WEIM_CS3_FPGA_BACKLIGHT_ADDR,1);
	}
	else
	{
		pr_alert("MXC_CPLD_BL.C: UNKNOW BOARD!! \n");
	}

	bd = backlight_device_register(dev_name(&pdev->dev), &pdev->dev, devdata,
				       &mxcbl_ops);
	if (IS_ERR(bd)) {
		ret = PTR_ERR(bd);
		goto err0;
	}
	platform_set_drvdata(pdev, bd);

	bd->props.brightness = MXC_DEFAULT_INTENSITY;
	bd->props.max_brightness = MXC_MAX_INTENSITY;
	bd->props.power = FB_BLANK_UNBLANK;
	bd->props.fb_blank = FB_BLANK_UNBLANK;
	set_brightness(MXC_DEFAULT_INTENSITY);

    lcd_enable(true);

	printk("SAND Backlight Device %s Initialized.\n", dev_name(&pdev->dev));
	return 0;
      err0:
	kfree(devdata);
	return ret;
}

static int mxcbl_remove(struct platform_device *pdev)
{
	struct backlight_device *bd = platform_get_drvdata(pdev);

	bd->props.brightness = MXC_INTENSITY_OFF;
	backlight_update_status(bd);

	backlight_device_unregister(bd);

	return 0;
}

static struct platform_driver mxcbl_driver = {
	.probe = mxcbl_probe,
	.remove = mxcbl_remove,
	.driver = {
		   .name = "mxc_cpld_bl",
		   },
};

static int __init mxcbl_init(void)
{
	return platform_driver_register(&mxcbl_driver);
}

static void __exit mxcbl_exit(void)
{
	platform_driver_unregister(&mxcbl_driver);
}

module_init(mxcbl_init);
module_exit(mxcbl_exit);

MODULE_DESCRIPTION("SAND CPLD Backlight Driver");
MODULE_AUTHOR("SAND R&D Dept.2 zengshu");
MODULE_LICENSE("GPL");
