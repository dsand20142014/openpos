#ifndef __OSCFG_H
#define __OSCFG_H

#include "tools.h"



#define LTSTR(s)    (unsigned char *)(s)
#define fU8(fn)     (unsigned char  (*)())(fn)
#define fU16(fn)    (unsigned short (*)())(fn)
#define fU32(fn)    (unsigned long  (*)())(fn)

#define MAX_DOWN_APP_BYTES  2*1024*1024

#define OSAPPID_OS              0x00

#define OSFILE_FILENAMELEN      24

#define OSDOWN_APPFILEID        "%DWN-1.0-SAND%"
#define FILEFS_ATTRIBLEN        10

#define OSFILEATT_OWNERID       0x00
#define OSFILEATT_TYPE          0x01
#define OSFILEATT_APPID         0x02
#define OSFILEATT_READ          0x04
#define OSFILEATT_WRITE         0x05

#define OSAPPTAG_FILE           0x40
#define OSAPPTAG_TYPE           0x41
#define OSAPPTAG_ID             0x42
#define OSAPPTAG_NAME           0x43
#define OSAPPTAG_LABEL          0x44
#define OSAPPTAG_SELFVERSION		0x45
#define OSAPPTAG_ZIP_TYPE       0x46
#define OSAPPTAG_ATTR						0x47
#define OSAPPTAG_CREATETIME     0x48
#define OSAPPTAG_OPERATE				0x4A
#define OSAPPTAG_VERSION				0x4B
#define OSAPPTAG_MD5SUM					0x4C
#define OSAPPTAG_ORIGINALSIZE   0x4D
#define OSAPPTAG_CRC            0x4E
#define OSAPPTAG_DATA           0x4F

#define OSAPP_MAXAPPNB          10
#define OSAPP_FILENAMELEN       21
#define OSAPP_LABELLEN          40



/*OSAPPTAG_TYPE 0x41 */
#define OSAPPTAG_TYPE_OS     0x01
#define OSAPPTAG_TYPE_APP    0x02
#define OSAPPTAG_TYPE_FONT   0x03
#define OSAPPTAG_TYPE_DATA   0x04
#define OSAPPTAG_TYPE_LIB    0x05
#define OSAPPTAG_TYPE_DRIVER 0x06
#define OSAPPTAG_TYPE_ZIMAGE 0x07


struct _file_info 
{
	char name[30];
	int  length;
};

struct _file_attr {
	unsigned char  type; // 01-OS 02-APP 03-FONT 04-DATA 5-lib 6-driver 7-zimage
	unsigned short  id;
	unsigned char  op;
	char  name[20];
	char  lable[20];
	unsigned char ctime[14];
	unsigned char version[5];
	unsigned char md5sum[32];
	unsigned char  zip_type;
	unsigned char *zip_addr;
	unsigned int   ori_size;
	unsigned int   zip_size;
	unsigned char  attr; //0 显示，其他不显示
	unsigned int  sort; //排序
};




int OSCFG_main(void);
int OSCFG_configure(void);
int OSCFG_datetime(void);
int OSCFG_maintenance(void);
int OSCFG_test(void);
int OSCFG_restore_to_defaults(void);

int OSCFG_mtc_save(unsigned char **ppstr, 
										unsigned int uisize, 
										unsigned char ucFlag, 
										unsigned char *pucAppID,
										unsigned int *puiAppCnt);
int OSCFG_mtc_all_down(void);
//int OSCFG_mtc_app_dele(void);
int OSCFG_mtc_lcdfont_down(void);
int OSCFG_mtc_prnfont_down(void);
int OSCFG_mtc_filedelete(char *pcinfo, char *pcname, struct _file_attr *pattr);
void OSCFG_init_appinfo(void);

int OSCFG_tms(void);

int OSCFG_test_lcd(void);
int OSCFG_test_printer(void);
int OSCFG_test_magnetic(void);
int OSCFG_test_icc(void);
int OSCFG_test_rfcard(void);

int  OSCFG_create_downfile(char *pucfix, struct _file_attr *file_attr);
int  OSCFG_get_attr(TLV *tlv, int tlv_nums, struct _file_attr *file_attr);
int  OSCFG_put_attr(char *pathname, struct _file_attr *file_attr);
int  OSCFG_save_apporder(struct _file_attr *pattr);
void OSCFG_init_posdir(void);
int OSCFG_dump_directory(const char *dname,int recursive);
extern void OSDEG_enable_lib_debug(void);
extern void OSDEG_disable_lib_debug(void);
extern int OSTEST_digit_input(int row, int col, int nums, char *dst);
extern int OSCFG_config_password();

void OSCFG_ReadParam_ADS(unsigned char * cp);

#endif
