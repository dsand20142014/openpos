#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <linux/ioctl.h>
#include <asm/param.h>

#include "sand_types.h"

#include "sand_com.h"
#include "sand_debug.h"
//#include "sand_fileops.h"
#include "sand_gprs.h"
#include "sand_icc.h"
#include "sand_modem.h"
#include "sand_lcd.h"
#include "sand_key.h"
#include "sand_print.h"
#include "sand_power.h"
#include "sand_sd.h"
#include "sand_time.h"
#include "sand_tools.h"
#include "sand_usb.h"

/***************************************************
 *
 * includes related to Sand OS
 *
 ***************************************************/
//#include "osdriver.h"
#include "osmodem.h"
#include "osicc.h"
#include "osinc.h"

#include "osgraph.h"
#include "typelib.h"
#include "type_uns.h"

#include "sand_tools.h"






