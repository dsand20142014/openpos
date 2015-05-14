#ifndef __SAND_UART_MODEM__
#define __SAND_UART_MODEM__

#ifdef __LUXAPP_MODEM
#define EXT_MODEM extern
#else
#define EXT_MODEM
#endif

#include "osmodem.h"

#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#ifdef __cplusplus 
extern "C" { 
#endif

#define LOCK_SHM 	 	1
#define UNLOCK_SHM   	0
#define SERVER_READY 	0x26

#define SEED_SS 'S'    //Seed of Server Send
#define SEED_SR 'R'    //Seed of Server Rev

#define SHM_SEED 'B'

#define BUFF_SIZE 4096 //4096

#define LSI_INIT_MSG			  226
#define CLIENT_TO_SERVER_MSG     426

#define MODEM_SUCCESS 			0x00
#define MODEM_ERR        		       0xFF

#define MODEM_INIT_COM		 	  	100
#define MODEM_BGND_DIAL	 	 	101
#define MODEM_CHECK_DIAL 			102
#define MODEM_SEND_I_FRAME         	103
#define MODEM_SET_PARAM	       	104
#define MODEM_HANGUP                 		105
#define MODEM_SERVER_READY		106
#define MODEM_SMALL_ADJUST		107

enum modem_protocol {
    PROTCL_V92,
    PROTCL_V90,
    PROTCL_V34,
    PROTCL_BELL212A,
    PROTCL_V22,
    PROTCL_V22BIS,
    PROTCL_V29
 };

struct osdialparam {
	unsigned char	  ucProtocol;
	unsigned char   ucFastConnect;
	unsigned char   ucDialType;
	unsigned char	  ucCheckTone;
	unsigned short  uiDMOT;
	unsigned short  uiCommaDealy;
	unsigned short  uiToneKeepTime;
	unsigned short  uiToneOffTime;
	unsigned short  uiConnectTimeout;
};

struct shm_struct {
	unsigned int data_in;
	unsigned int data_out;
	unsigned int over_run;
	unsigned int connect_status;
	unsigned char info_data[BUFF_SIZE*2-16];
};

struct message_type {
       long type;  
       char buffer[BUFF_SIZE];	   	   
};

EXT_MODEM unsigned char Os__backgnd_dial (DIAL_PARAM *DialParameter);
EXT_MODEM unsigned char Os__check_backgnd_dial (unsigned short TimeOut);
EXT_MODEM unsigned char Os__dial (DIAL_PARAM *DialParameter);
EXT_MODEM void Os__hang_up (void) ;
EXT_MODEM unsigned char Os__init_com (COM_PARAM *ComParameter) ;
EXT_MODEM void Os__read_dial_param (DIAL_PARAM * DialParameter);
EXT_MODEM unsigned short Os__rxcar (unsigned short TimeOut);
EXT_MODEM unsigned char Os__txcar (unsigned char Charac);
EXT_MODEM unsigned char Os__set_modem_parameter (unsigned char ParamNb, unsigned char NewValue);
//EXT_MODEM unsigned char Os__set_modem_parameter (struct osdialparam *osdialparam_p);
//EXT_MODEM unsigned char Os__modem_small_adjust(struct osdialparam *osdialparam_p);
//EXT_MODEM void Os__modem_wild_adjust(unsigned char index);

#ifdef __cplusplus 
}
#endif

#endif
//end
