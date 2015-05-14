#ifndef __OSICC_H__
#define __OSICC_H__
/**
 * @defgroup IC卡操作相关函数
 * @brief 提供对IC卡进行操作
 * @author 
 * @{
 */
#include "libicc.h"
#include "osdriver.h"

extern struct ic_device ICDEV;
extern int fd_icc;

extern unsigned char ucCardReader;
extern unsigned char ucSW1;
extern unsigned char ucSW2;
extern unsigned char ucCardRecvLen;
extern unsigned char ucCardRecvStatus;
extern unsigned char aucCardRecvBuf[256];//recv from IC card

extern unsigned char ucCardSendLen;
extern unsigned char aucCardSendBuf[256];


extern unsigned char uiRecvLen;
extern unsigned char uiRecvedLen;
extern unsigned char ucAPDUCase;
extern unsigned char ucAPDUOneByteFlag;
extern unsigned char uiAPDUSendLen;
extern unsigned char uiAPDUSentLen;
extern unsigned char uiAPDURecvLen;
extern unsigned char uiAPDURecvedLen;
extern volatile unsigned char ucATRLen;

extern volatile unsigned short uiT1FrameLastSendLen;
extern volatile unsigned short uiT1FrameSendLen;
extern volatile unsigned short uiT1FrameSentLen;



#define HALICC_RESET_COLD 0x00	//冷复位
#define HALICC_RESET_WARM 0x01	//热复位

#define ICCMETHOD_ASYNC					0x00
#define ICCMETHOD_SYNC					0x01


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
typedef struct
{
    unsigned char   Lc;
    unsigned char * ptin;
}type0;
typedef struct
{
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
}type1;
typedef struct
{
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
    unsigned char   Le;
}type2;
typedef struct
{
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
    unsigned char   Lc;
    unsigned char * ptin;
}type3;
typedef struct
{
    unsigned char   CLA;
    unsigned char   INS;
    unsigned char   P1;
    unsigned char   P2;
    unsigned char   Lc;
    unsigned char * ptin;
    unsigned char   Le;
}type4;
typedef struct
{
    unsigned short   Lc;
    unsigned char * ptin;
}type6;
typedef struct
{
    unsigned char   pcb;
    unsigned char   Lc;
    unsigned char * ptin;
}type7;
union AS_ORDER_TYPE
{
    type0   order_type0;
    type1   order_type1;
    type2   order_type2;
    type3   order_type3;
    type4   order_type4;
    type3   order_type5;
    type6   order_type6;
    type7   order_type7;
};
typedef struct
{
    unsigned char   order_type;
    union AS_ORDER_TYPE as_order;       // => according to 'order_type' value
    unsigned char   NAD;
}ASYNC_ORDER_TYPE;

// order for a SYNCHRONOUS chip managed by the BIOS
typedef struct
{
    unsigned char   card_type;  // chip ID
    unsigned char   INS;
    unsigned char   ADDH;
    unsigned char   ADDL;
    unsigned char   Len;  // data length in BITS
    unsigned char * ptin; // data transmitted
}SYNC_ORDER_TYPE;

// new function (execute a list of basic orders) for a SYNCHRONOUS chip
typedef struct
{
    unsigned char   Len;    // length of the list of orders
    unsigned char * ptin;   // list of orders to execute
}NEW_SYNC_ORDER_TYPE;

// cold or warm reset with parameters for card standards and preferred protocol
typedef struct
{
    unsigned char card_standards;
    unsigned char GR_class_byte_00;
    unsigned char preferred_protocol;
}NEW_POWER_TYPE;

// send/receive exchange for PARAVANT card
typedef struct
{
    unsigned short  send_len;        // number of bytes to send to ICC
    unsigned char   *ptin ;          // points to data to send
    unsigned short  rec_len;         // number of bytes to receive
    unsigned char   TC1;             // Extra Guard Time to be used for the sending
    unsigned char   check_status;    // check or not the first 2 received bytes (0x90/0x00)
}EXCHANGE_TYPE;

// order to be transmitted to the ICC or SIM
union ORDER_IN_TYPE
{
    NEW_SYNC_ORDER_TYPE new_sync_order; // new functions for synchronous chip
    SYNC_ORDER_TYPE     sync_order;     // order to a synchronous chip
    ASYNC_ORDER_TYPE    async_order;    // order to an asynchronous chip
    unsigned char       async_protocol; // T=0 or T=1 protocol selection
    NEW_POWER_TYPE      new_power;      // power on with parameters
    EXCHANGE_TYPE       exchange;       // SEND/RECEIVE exchange (PARAVANT)
    unsigned char       icc_standard;   // ISO7816, EMV, MPEV5, VISA or MONDEX
};

// command to execute in the ICC or SIM reader
typedef struct
{
    unsigned char   order;                      // order number to execute
    union ORDER_IN_TYPE * pt_order_in;  		// order to be transmitted
    unsigned char * ptout;                      // output buffer
}ICC_ORDER;

// execution report
typedef struct
{
    unsigned char   drv_status;
    unsigned char   card_status;
    unsigned short  Len;
}ICC_ANSWER;

extern unsigned char aucT1FrameSendBuf[LIBICC_MAXCOMMBUF];
extern unsigned char aucATR[LIBICC_MAXATRBUF];
extern unsigned char aucAPDURecvBuf[LIBICC_MAXCOMMBUF];


void    OSICC_SetReport(unsigned char ucICCStatus);

unsigned char OSICC_NewPowerOn(unsigned char ucType);

unsigned char OSICC_ChangeReader(unsigned char ucReader);
unsigned char OSICC_PowerOn(unsigned char ucResetType);
unsigned char OSICC_Detect(unsigned char ucReader);
unsigned char OSICC_ResetUart(unsigned char ucReader);
unsigned char OSICC_SetClock(unsigned char ucReader, unsigned char ucStopFlag,unsigned char ucLevel);
unsigned char OSICC_SetMethod(unsigned char ucReader,unsigned char ucMethod);
unsigned char OSICC_SetBaud(unsigned char ucReader, unsigned char ucBaudMulti);
unsigned char OSICC_SetDi(unsigned char ucReader, unsigned char ucDi);
unsigned char OSICC_SetProtocol(unsigned char ucReader,unsigned char ucProtocol);
unsigned char OSICC_SetConvention(unsigned char ucReader, unsigned char ucConvention);
unsigned char OSICC_SetPowerType(unsigned char ucReader, unsigned char ucPowerType);
unsigned char OSICC_SetPEC(unsigned char ucReader, unsigned char ucPEC);
/**
 * @fn ICC_ANSWER * Os__ICC_command( unsigned char reader_nb, ICC_ORDER * order )
 * @brief 发送读写卡命令SMART卡或内部SIM卡
 * @param reader_nb       IC卡卡槽编号
 * @param order           是指向ICC_ORDER数据结构指针   
 *  *
 * @return IC卡结构体指针 
 */ 
ICC_ANSWER * Os__ICC_command( unsigned char reader_nb, ICC_ORDER * order );
ICC_ANSWER *OSICC_Command(unsigned char ucReader, ICC_ORDER *pOrder);
void    OSICC_SetReport(unsigned char ucICCStatus);
unsigned char OSICC_ChangeReader(unsigned char ucReader);
unsigned char OSICC_Activate(unsigned char ucReader);
unsigned char OSICC_PowerOn(unsigned char ucResetType);
unsigned char OSICC_SetWaitTime(unsigned char ucReader,
                                unsigned char ucWaitType,
                                unsigned char ucWaitValue0,unsigned char ucWaitValue1,
                                unsigned char ucWaitValue2,unsigned char ucWaitValue3);
void OSICC_Init(void);

/* Smart cards and SIMs reader-writer */
/**
 * @fn unsigned char OSICC_Detect(unsigned char ucReader)
 * @brief 检测ic卡或sam卡是否已经插入卡槽
 *  *  *
 * @return 0 有ic卡或sam卡在卡槽中.
           1 没有ic卡或sam卡在卡槽中
 */
unsigned char OSICC_Detect(unsigned char ucReader);
/**
 * @fn DRV_OUT *OSICC_Insert(void)
 * @brief 等待插卡并对卡上电，该函数仅适用于外部主读卡头
 * @param ucReader   IC卡卡槽编号
 *  *  *
 * @return 上电返回数据.
 */
DRV_OUT *OSICC_Insert(void);
DRV_OUT *OSICC_OrderAsync(unsigned char *pucOrder,unsigned short uiLen);
//DRV_OUT *OSICC_Order(DRV_IN  *pDrvIn);
/**
 * @fn void    void OSICC_Remove(void)
 * @brief IC卡等待拔卡下电函数,仅适用于外部读卡头
 *  *  *
 * @return 无返回数据.
 */
void OSICC_Remove(void);
/**
 * @fn void void OSICC_PowerOff(void)
 * @brief 关闭IC卡，仅适用于外部读卡头
 *  *  *
 * @return 无返回数据.
 */
void OSICC_PowerOff(void);
#endif
/** @} end of etc_fns */
