/*
 * acm_serial.c
 *
 *  Created on: Dec 30, 2011
 *      Author: root
 */
//#include <usbdescriptors.h>
//#include <linux/usb/ch9.h>
//#include <usb_defs.h>
#include "acm_usb.h"

int acm_bind_config(struct usb_configuration *c, u8 port_num)
{
	struct f_acm *acm;
	int status;

	if (!can_support_cdc(c)) return -EINVAL;

	if (acm_string_defs[ACM_CTRL_IDX].id == 0){
		status = usb_string_id(c->cdev);
	  if (status < 0) return status;
	  acm_string_defs[ACM_CTRL_IDX].id = status;

	  acm_control_interface_desc.iInterface = status;

	  status = usb_string_id(c->cdev);
	  if (status < 0) return status;
	  acm_string_defs[ACM_DATA_IDX].id = status;

	  acm_data_interface_desc.iInterface = status;
	}

	acm = kzalloc(sizeof *acm, GFP_KERNEL);
	if (!acm) return -ENOMEM;

	//spin_lock_init(&acm->lock);

	acm->port_num = port_num;

	acm->port.connect = acm_connect;
	acm->port.discnnect = acm_disconnect;
	acm->port.send_break = acm_send_break;

	acm->port.func.name = "acm";
	acm->port.func.string = acm_strings;

	/*descriptors are per-instance copies*/
	acm->port.func.bind    = acm_bind;
	acm->port.func.unbind  = acm_unbind;
	acm->port.func.set_alt = acm_set_alt;
	acm->port.func.setup   = acm_setup;
	acm->port.func.disable = acm_disable;

	//
	//statusw = usb_add_function(c, &acm->port.func);
	if (status)  kfree(acm);

	return status;
}

static int serial_bind_config(struct usb_configuration *c)
{
	unsinged i;
	int status = 0;

	unsigned int n_ports = 1;

	//use acm
	for (i = 0; i < n_ports && status == 0; i++)
		status = acm_bind_config(c, i);
}

static struct usb_configuration serial_config_driver = {
		.bind = serial_bind_config,
		.bmAttributes = USB_CONFIG_ATT_SELFPOWER,
};

static struct usb_device_descriptor device_desc = {
		.bLength         = USB_DT_DEVICE_SIZE,
		.bDescriptorType = USB_DT_DEVICE,
		.bcdUSB          = 0x0200,
		.bDeviceSubClass = 0,
		.bDeviceProtocol = 0,
		.idVendor = GS_VENDOR_ID,
		.bNumConfigurations = 1,
};
static struct usb_composite_driver gserial_driver ={
		.name = "g_serial",
		.dev = &device_desc,
		.strings = dev_strings,
		.bind = gs_bind,

};

int usb_composite_register(struct usb_composite_driver *driver)
{
	if (!driver || !driver->dev || !driver->bind || composite)
		return -EINVAL;

	if (!driver->name)  driver->name = "composite";
	composite_driver.function = (char*)driver->name;
	composite_driver.driver.name = driver->name;
	composite = driver;

	//return usb_gadget_register_driver(&composite_driver);

}




static int init(void)
{
	//usb_acm
	serial_config_driver.label = "CDC ACM config";
	serial_config_driver.bConfigurationValue = 2;

	device_desc.bDeviceClass = USB_CLASS_COMM;
	device_desc.idProduct  = GS_CDC_PRODUCT_ID;

	strings_dev[STRING_DESCRIPTION_IDX].s = serial_config_driver.label;
	//return usb_composite_register(&gserial_driver);

}
