#ifndef __OSTEST_H
#define __OSTEST_H

struct __item {
	char string[20];
	unsigned char speed;
};

extern int OSTEST_menu_mmi(void);
extern int OSTEST_menu_printer(void);
extern int OSTEST_menu_keyboard(void);
extern int OSTEST_menu_ime(void);
extern int OSTEST_menu_magnetic(void);
extern int OSTEST_menu_icc(void);
extern int OSTEST_menu_sam(void);
extern int OSTEST_menu_uart(void);
extern int OSTEST_menu_pinpad(void);
extern int OSTEST_menu_modem(void);
extern int OSTEST_menu_gsm(void);
extern int OSTEST_menu_ethernet(void);
extern int OSTEST_menu_wifi(void);
extern int OSTEST_menu_memory(void);
extern int OSTEST_menu_barcode(void);

extern int OSTEST_menu_serialCom(void);
extern int OSTEST_menu_usb(void);
extern int OSTEST_menu_tf(void);
extern int OSTEST_menu_txrx(void);

extern int OSTEST_menu_power(void);
extern int OSTEST_power_suspend(void);
extern int OSTEST_power_bright(void);
extern int OSTEST_power_gprs(void);
extern int OSTEST_power_rc531(void);
extern int OSTEST_power_front_back(void);
extern int OSTEST_kms_management(void);// 1/12/2011 10:35a

//extern int OSTEST_menu_touch(void);

extern int OSTEST_WaitKey(unsigned char forever, unsigned int timeout);

extern unsigned char OSTEST_mfc_card(void);
extern int OSTEST_sam(void);
unsigned char OSTEST_Input(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask);

#endif

