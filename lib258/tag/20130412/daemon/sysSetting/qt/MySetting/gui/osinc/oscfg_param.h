#ifndef __OSCFG_PARAM_H
#define __OSCFG_PARAM_H

#ifndef _CFGDATA
#define CFGDATA extern
#else 
#define CFGDATA
#endif


#define CFG_PATH        "/mnt/pos/config/"
#define FILE_CFG_BASIC  CFG_PATH"osbasic.cfg"
#define FILE_CFG_BAT    CFG_PATH"osbat.cfg"
#define FILE_CFG_PRN    CFG_PATH"osprn.cfg"
#define FILE_CFG_MDM    CFG_PATH"osmdm.cfg"
#define FILE_CFG_GSM    CFG_PATH"osgsm.cfg"
#define FILE_CFG_WIF    CFG_PATH"oswif.cfg"
#define FILE_CFG_ETH    CFG_PATH"oseth.cfg"
#define FILE_CFG_PAD    CFG_PATH"ospad.cfg"
#define FILE_CFG_MAG    CFG_PATH"osmag.cfg"

#define IDX_BASIC   0
#define IDX_BAT     1
#define IDX_PRN     2
#define IDX_MDM     3
#define IDX_GSM     4
#define IDX_WIF     5
#define IDX_ETH     6

/* define for cfg_val_type */
#define V_TYPE_U8       1
#define V_TYPE_U16      2
#define V_TYPE_U32      3
#define V_TYPE_STR_DIG  4       /* 0-9       */
#define V_TYPE_STR_HEX  5       /* 0-9,A-F */
#define V_TYPE_STR_ASC  6       /* 26 a-z   */
#define V_TYPE_STR_IP   7       /* ip address format  */
#define V_TYPE_STR_MAC  8       /* MAC address format  */

/* onfigure  return value  */
#define ERRCFG_SUCCESS       0
#define ERRCFG_CLEAR        (-1)
#define ERRCFG_FILE         (-2)
#define ERRCFG_PRIVILEGE    (-3)
#define ERRCFG_OVERSCALE_VALUE    (-4)
#define ERRCFG_OVERSCALE_LENGTH   (-5)
#define ERRCFG_OVERSCALE_ANY      (-6)

/* define for privilege */
#define  PRIVI_NONE   0
#define  PRIVI_READ   1
#define  PRIVI_WRIT   2
#define  PRIVI_RDWR   3


/* device configure fields   */
struct _cfg_basic {
	unsigned char  b_model;
	unsigned char  b_main_ver;
	unsigned char  b_sub_ver[2+1];
	unsigned char  b_serial_no[16];
	unsigned char  b_pad_no[16];
	unsigned char  b_mdm_exist;
	unsigned char  b_icc_exist;
	unsigned char  b_rfc_exist;
	unsigned char  b_pad_exist;
	unsigned char  b_eth_exist;
	unsigned char  b_gsm_exist;
	unsigned char  b_wif_exist;
	unsigned char  b_usb_exist;
	unsigned char  b_lib_debug;
	unsigned char  b_crc[2];
};

struct _cfg_bat {
	unsigned short capacity;
	unsigned short voltage;
	unsigned short min_vol;
	unsigned short max_vol;
	unsigned char  crc[2];
};

struct _cfg_prn {
	unsigned char  version;
	unsigned short min_vol;
	unsigned short max_vol;
	unsigned short step_speed;
	unsigned short adjust_time;
	unsigned short strobe_num;
	unsigned char  crc[2];	
};

struct _cfg_mdm {
	unsigned char  version;
	unsigned char  de_flag;
	unsigned char  model;
	unsigned char  param[256];
	unsigned char  crc[2];	
};

struct _cfg_gsm {
	unsigned char  version;
	unsigned char  de_flag;
	unsigned char  uart_no;
	unsigned long  uart_speed;
	unsigned char  always_online;
	unsigned char  gsm_cdma;
	unsigned char  APN[32];
	unsigned char  username[40];
	unsigned char  password[20];
	unsigned char  manu_id[40];
	unsigned char  mode_id[40];
	unsigned char  vern_id[40];
	unsigned char  IMEI[40];
	unsigned char  CCID[40];
	unsigned char  crc[2];	
};

struct _cfg_wifi {
	unsigned char  version;
	unsigned char  de_flag;
	unsigned char  param[256];
	unsigned char  crc[2];	
};

struct _cfg_eth {
	unsigned char  version;
	unsigned char  type;
	unsigned char  mac[6];
	unsigned char  filter;
	unsigned char  dest_mac[6];
	unsigned char  ip_addr[4];
	unsigned char  ip_mask[4];
	unsigned char  gw_addr[4];
	unsigned char  f_dhcp;
	unsigned char  f_dns;
	unsigned char  f_broadcast;
	unsigned char  dhcp_addr[4];
	unsigned char  dns_addr[4];
	unsigned char  crc[2];	
};

struct _cfg_mag {
	unsigned char  version;
    unsigned char  de_flag;
	unsigned char  param[20];
	unsigned char  crc[2];	
};

/* end of device configure fields */

struct _cfg_item {
	char  *name;
	unsigned char  privi;
	char  *prompt1;
	char  *prompt2;
	unsigned long  val_min;
	unsigned long  val_max;
	unsigned char  val_type; 
	unsigned long  val_addr;
	unsigned long  val_length;
	char  val_default[20]; // char array 
};

struct _cfg_info {
	char  filename[30];
	unsigned long  var_addr;
	unsigned long  var_length;
	struct _cfg_item  *item;
};

union _u_value {
	unsigned char  byte;
	unsigned short word;
	unsigned int   dword;
};
int  OSCFG_load_configs(void);
int  OSCFG_restore_to_defaults(void);

int OSCFG_config(unsigned char idx);
int OSCFG_config_basic(void);
int OSCFG_config_battery(void);
int OSCFG_config_printer(void);
int OSCFG_config_modem(void);
int OSCFG_config_gsm(void);
int OSCFG_config_wifi(void);
int OSCFG_config_ethenet(void);
void OSCFG_decide_debug(void);

unsigned char g_r_shift;
unsigned char g_cfg_mode;     /* = SYS ,MTC,TST,USR */
unsigned char g_timeout;

#define RSFT_SYS  6
#define RSFT_MTS  4
#define RSFT_TST  2
#define RSFT_USR  0

#define MODE_SYS (3<<RSFT_SYS)          /*1100 0000 , gbl_r_shift = 6 */
#define MODE_MTS (3<<RSFT_MTS)          /*0011 0000,  gbl_r_shift = 4 */
#define MODE_TST (3<<RSFT_TST)          /*0000 1100,  gbl_r_shift = 2 */
#define MODE_USR (3<<RSFT_USR)          /*0000 0011,  gbl_r_shift = 0 */


struct _cfg_basic g_cfg_basic;
struct _cfg_bat   g_cfg_bat;
struct _cfg_prn   g_cfg_prn;
struct _cfg_mdm   g_cfg_mdm;
struct _cfg_gsm   g_cfg_gsm;
struct _cfg_wifi  g_cfg_wif;
struct _cfg_eth   g_cfg_eth;
struct _cfg_mag   g_cfg_mag;

extern const struct _cfg_item cfg_item_basic[];
extern const struct _cfg_item cfg_item_bat[];
extern const struct _cfg_item cfg_item_prn[];
extern const struct _cfg_item cfg_item_mdm[];
extern const struct _cfg_item cfg_item_gsm[];
extern const struct _cfg_item cfg_item_wif[];
extern const struct _cfg_item cfg_item_eth[];
extern const struct _cfg_item cfg_item_mag[];


int OSCFG_save_basic(void);
int OSCFG_save_battery(void);
int OSCFG_save_printer(void);
int OSCFG_save_modem(void);
int OSCFG_save_gsm(void);
int OSCFG_save_wifi(void);
int OSCFG_save_ethenet(void);
int OSCFG_save(unsigned char idx);
int OSCFG_load_tmsparam(int *piflag);
char* OSCFG_input_passwd(unsigned int mdigit, char *pcptr);
int   OSCFG_is_save(void);
int   OSCFG_error_message(int err_value);

#endif

