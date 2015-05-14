/*
	Operating System
--------------------------------------------------------------------------
	FILE  osicc.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __OSICC_H
#define __OSICC_H

// ICC and SIM reader numbers :
#define ICC1            0
#define SIM1            1
#define SIM2            2
#define SIM3            3
#define SIM4            4

// commands to execute in the ICC or SAM reader
#define POWER_ON        	0
#define POWER_OFF       	1
#define SYNC_ORDER      	2
#define ASYNC_ORDER     	3
#define ASYNC_PROTOCOL  	4
#define WARM_RESET      	5
#define NEW_SYNC_ORDER  	6
#define NEW_POWER_ON    	7
#define NEW_WARM_RESET		8
#define CHANGE_PROTOCOL 	9
#define SET_ICC_STANDARD 	10
#define EXCHANGE_BYTES  	100

#define ISO7816         0
#define EMV             1
#define MPEV5           2
#define VISA            3
#define MONDEX          4
#define CLA_IDEN        0
#define CLA_00          1
#define PROT_T0         0
#define PROT_T1         1
#define PROT_DFT        0xff

// execution status from ICC and SIM driver (drv_status)
#define OK              0
#define KO              1
#define NOICC           1
#define NOREADER        2
#define OVERRUN         6
#define DRVUNKNOWN      7
#define WRONGREADER     10
#define WRONGORDER      11
#define NULLPTOUT       12

#define ASY_OK          0x30      /* treatment ok */
#define ICC_REMOVED     0x31      /* smart card removed during the treatment */
#define VCC_ERROR       0x32      /* Vcc default */
#define VPP_ERROR       0x33      /* Vpp default */
#define ICC_DIALOG_ERR  0x34      /* error of dialog with the smart card */
#define ICC_ASY         0x35      /* asynchronous smart card detected */
#define ICC_ERR         0x36      /* driver unable to read the asynchronous smart card */
#define ICC_SYN         0x37      /* synchronous smart card detected */
#define ICC_MUTE        0x38      /* smart card dumb */
#define ICC_RUNNING     0xFF      /* treatment running */

// order for an ASYNCHRONOUS chip
typedef struct {
    unsigned char   Lc;
    unsigned char * ptin;
}type0;
typedef struct {
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
}type1;
typedef struct {
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
    unsigned char   Le;
}type2;
typedef struct {
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
    unsigned char   Lc;
    unsigned char * ptin;
}type3;
typedef struct {
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
    unsigned char   Lc;
    unsigned char * ptin;
    unsigned char   Le;
}type4;
typedef struct {
    unsigned short   Lc;
    unsigned char * ptin;
}type6;
typedef struct {
    unsigned char   pcb;
    unsigned char   Lc;
    unsigned char * ptin;
}type7;
union AS_ORDER_TYPE{
    type0   order_type0;
    type1   order_type1;
    type2   order_type2;
    type3   order_type3;
    type4   order_type4;
    type3   order_type5;
    type6   order_type6;
    type7   order_type7;
};
typedef struct {
    unsigned char   order_type;
    union AS_ORDER_TYPE as_order;       // => according to 'order_type' value
    unsigned char   NAD;
}ASYNC_ORDER_TYPE;

// order for a SYNCHRONOUS chip managed by the BIOS
typedef struct {
    unsigned char   card_type;  // chip ID
    unsigned char   INS;
    unsigned char   ADDH;
    unsigned char   ADDL;
    unsigned char   Len;  // data length in BITS
    unsigned char * ptin; // data transmitted
}SYNC_ORDER_TYPE;

// new function (execute a list of basic orders) for a SYNCHRONOUS chip
typedef struct {
    unsigned char   Len;    // length of the list of orders
    unsigned char * ptin;   // list of orders to execute
}NEW_SYNC_ORDER_TYPE;

// cold or warm reset with parameters for card standards and preferred protocol
typedef struct {
	 unsigned char card_standards;
	 unsigned char GR_class_byte_00;
	 unsigned char preferred_protocol;
}NEW_POWER_TYPE;

// send/receive exchange for PARAVANT card
typedef struct {
     unsigned short  send_len;        // number of bytes to send to ICC
     unsigned char   *ptin ;          // points to data to send
     unsigned short  rec_len;         // number of bytes to receive
     unsigned char   TC1;             // Extra Guard Time to be used for the sending
     unsigned char   check_status;    // check or not the first 2 received bytes (0x90/0x00)
}EXCHANGE_TYPE;

// order to be transmitted to the ICC or SIM
union ORDER_IN_TYPE {
    NEW_SYNC_ORDER_TYPE new_sync_order; // new functions for synchronous chip
    SYNC_ORDER_TYPE     sync_order;     // order to a synchronous chip
    ASYNC_ORDER_TYPE    async_order;    // order to an asynchronous chip
    unsigned char       async_protocol; // T=0 or T=1 protocol selection
    NEW_POWER_TYPE      new_power;      // power on with parameters
    EXCHANGE_TYPE       exchange;       // SEND/RECEIVE exchange (PARAVANT)
    unsigned char       icc_standard;   // ISO7816, EMV, MPEV5, VISA or MONDEX
};

// command to execute in the ICC or SIM reader
typedef struct {
    unsigned char   order;                      // order number to execute
    union ORDER_IN_TYPE * pt_order_in;  // order to be transmitted
    unsigned char * ptout;                      // output buffer
}ICC_ORDER;

// execution report
typedef struct {
    unsigned char   drv_status;
    unsigned char   card_status;
    unsigned short  Len;
}ICC_ANSWER;


// card detection in a smart card reader
unsigned char Os__ICC_detect( unsigned char reader_nb );

// execute a command of the ICC or SIM driver
ICC_ANSWER * Os__ICC_command( unsigned char reader_nb, ICC_ORDER * order );

DRV_OUT *Os__ICC_insert(void);

DRV_OUT *Os__ICC_Async_order(unsigned char *pucOrder,unsigned short uiLen);

DRV_OUT *Os__ICC_order(DRV_IN  *pDrvIn);

void Os__ICC_remove(void);

void Os__ICC_off(void);

#endif

