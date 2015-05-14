/*
 * imx_udc.c
 *
 *  Created on: Jan 13, 2012
 *      Author: root
 */
static struct usb_device_instance *udc_device;
static int ep0state = EP0_IDLE;
struct clk *clk_get(const char *id)
{
	return &usbd_clk;
}
#define IS_ERR_VALUE(x) ((x) >= (unsigned long)-4095)
static long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}
int udc_init(void)
{
	struct clk *clk;
	udc_device = NULL;

	clk = clk_get("usbd_clk");
	clk->enable(clk);
	printf("USB CLK %d Hz", clk->rate);

	usb_init_data(imx_usb);

	imx_udc_init(imx_usb);


}
