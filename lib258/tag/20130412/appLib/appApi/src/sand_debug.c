#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <oscfg_param.h>
void Uart_Printf(char *fmt,...);

int _g_com2_openned=0;
int _g_enable_lib_debug=1;

void dump(int signo)
{
    char buf[1024];
    char cmd[1024];
    FILE *fh;

    Uart_Printf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
    if (!(fh = fopen(buf, "r")))
        exit(0);
    if (!fgets(buf, sizeof(buf), fh))
        exit(0);
    fclose(fh);
    if (buf[strlen(buf) - 1] == '\n')
        buf[strlen(buf) - 1] = '\0';
    Uart_Printf(cmd, sizeof(cmd), "gdb %s %d", buf, getpid());
    system(cmd);

    exit(0);
}

#define LIB_DEBUG
#ifdef LIB_DEBUG
void Uart_Printf(char *fmt,...)
{
    va_list ap;
    char string[512];

	if (strlen(fmt)>400)
		return;
    if (!_g_com2_openned && _g_enable_lib_debug ){
	    va_start(ap,fmt);
	    vsprintf(string,fmt,ap);
	    fprintf(stderr,string);
	    va_end(ap);
	}
}
#else
void Uart_Printf(char *fmt,...)
{
}
#endif

void OSDEG_enable_lib_debug(void)
{
	_g_enable_lib_debug = 1;
}

void OSDEG_disable_lib_debug(void)
{
	_g_enable_lib_debug = 0;
}

#define CFG_BASIC_FILE	"/mnt/pos/config/osbasic.cfg"


void OSCFG_decide_debug(void)
{
    int fd;
	struct _cfg_basic basic;

	basic.b_lib_debug = 1; /* Default */
	
    if ((fd = open(CFG_BASIC_FILE,O_RDONLY)) < 0) {
        sprintf(stderr,"Open the CFG_GSM_FILE failed!\n");
		goto exit;
    }
    if (read(fd, &basic, sizeof(struct _cfg_basic)) < 0) {
        sprintf(stderr,"Read the CFG_GSM_FILE failed!\n");
		goto exit;    }
exit:
	close(fd);
	
	if (basic.b_lib_debug)		
		OSDEG_enable_lib_debug();
	else
		OSDEG_disable_lib_debug();
}
