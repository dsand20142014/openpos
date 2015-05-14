/*
 * acm_usb.h
 *
 *  Created on: Dec 30, 2011
 *      Author: root
 */

#ifndef ACM_USB_H_
#define ACM_USB_H_

#define GS_VERSION_STR         "v2.4"
#define GS_VERSION_NUM         0x2400

#define GS_LONG_NAME           "Gadget Serial"
#define GS_VERSION_NAME        GS_LONG_NAME " " GS_VERSION_STR

struct list_head{
	struct list_head *next, *prev;
};

/*USB_DT_ENDPOINT: Endpoint descriptor*/
struct usb_endpoint_descriptor {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
	u8 bRefresh;
	u8 bSynchAddress;
};


struct acm_ep_descs{
	struct usb_endpoint_descriptor *in;
	struct usb_endpoint_descriptor *out;
	struct usb_endpoint_descriptor *notify;

};

struct usb_gadget_ops{
	int (*get_frame)(struct usb_gadget *);
	int (*wakeup)(struct usb_gadget*);


};

struct usb_gadget{
	const struct usb_gadget_ops *ops;
	struct usb_ep   *ep0;
	struct list_head  ep_list;
	enum usb_device_speed speed;
	unsigned    is_dualspeed:1;
	unsigned    is_otg:1;
	unsigned    is_a_peripheral:1;
	unsigned    is_hnp_enable:1;
	unsigned    a_hnp_support:1;
	unsigned    a_alt_hnp_support:1;
	const char  *name;
};

struct usb_composite_dev{
	struct usb_gadget  *gadget;
	struct usb_request *req;
	unsigned     bufsiz;

	struct usb_configuration  *config;

	struct usb_device_descriptor desc;
	struct list_head     configs;
	struct usb_composite_driver    *driver;
	u8 next_string_id;

	unsigned deactivations;
};

struct usb_composite_driver{
	const char             *name;
	const struct usb_device_descriptor *dev;
	struct usb_gadget_strings           **strings;

	int   (*bind)(struct usb_composite_dev *);
	int   (*unbind)(struct usb_compsite_dev *);

	/*global suspend hooks*/
	void  (*suspend)(struct usb_composite_dev *);
	void (*resume)(struct usb_composite_dev *);
};


#define STRING_MANUFACTURER_IDX    0
#define STRING_PRODUCT_IDX         1
#define STRING_DESCRIPTION_IDX     2


static struct usb_string strings_dev[] = {
		[STRING_MANUFACTURER_IDX].s = manufacturer,
		[STRING_PRODUCT_IDX].s = GS_VERSION_NAME,
		[STRING_DESCRIPTION_IDX].s = NULL,
		{}
};

#define GS_VENDOR_ID         	0x0525   /*NetChip*/
#define GS_PRODUCT_ID        	0xa4a6	/*Linux-USB Serial */
#define GS_CDC_PRODUCT_ID    	0xa4a7
#define GS_CDC_OBEX_PRODUCT_ID  0Xa4a9

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO				1
#define USB_CLASS_COMM				2
#define USB_CLASS_HID				3
#define USB_CLASS_PHYSICAL			5
#define USB_CLASS_PRINTER			7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB				9
#define USB_CLASS_DATA				10
#define USB_CLASS_APP_SPEC			0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

/*
 * Descriptor types
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05

/* Descriptor sizes per descriptor type */
#define USB_DT_DEVICE_SIZE      18
#define USB_DT_CONFIG_SIZE      9
#define USB_DT_INTERFACE_SIZE   9
#define USB_DT_ENDPOINT_SIZE    7
#define USB_DT_ENDPOINT_AUDIO_SIZE  9	/* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE  7
#define USB_DT_HID_SIZE         9

struct usb_device_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x01 */
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} __attribute__ ((packed));

/* from config descriptor bmAttributes */
#define USB_CONFIG_ATT_ONE		(1 << 7)	/* must be set */
#define USB_CONFIG_ATT_SELFPOWER	(1 << 6)	/* self powered */
#define USB_CONFIG_ATT_WAKEUP		(1 << 5)	/* can wakeup */
#define USB_CONFIG_ATT_BATTERY		(1 << 4)	/* battery powered */

struct usb_configuration {
	const char  *label;
	u8 bLength;
	u8 bDescriptorType;	/* 0x2 */
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	u8 bmAttributes;
	u8 bMaxPower;
} __attribute__ ((packed));

#endif /* ACM_USB_H_ */
