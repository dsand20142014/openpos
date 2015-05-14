/*
*pinpad.h
*/




#ifndef __PINPAD_H_
#define __PINPAD_H_


#define UARTPAD_ACK			0xAA
#define UARTPADMAXLEN		1024	

#define CHAR_STX			0x02
#define CHAR_ETX			0x03

#define HALIICMAINADDR		0x60
#define HALIICMMIADDR			0x62
#define HALIICPADADDR			0x64
#define HALIICEEPADDR			0x66
#define HALIICMAGADDR			0x68
#define HALIICICCADDR			0x6A

#define HALDRVCMD_ABORT     0       /* abort driver     */
#define HALDRVCMD_EXECUTE   1       /* execute order    */
#define HALDRVCMD_STATEREQ  2       /* state request    */
#define HALDRVCMD_RESET     3       /* reset driver     */
#define HALDRVCMD_IOCTL    10       /* config driver    */

#define HALDRV_MAXNB        17      /* number of drivers */
#define HALDRVID_RESERVNB   17
#define HALDRVID_MMI        0       /* Man-Machine-Interface (Display-Keyboard) driver number */
#define HALDRVID_PRN        1       /* Printer driver number */
#define HALDRVID_PAD        2       /* PINPAD driver number */
#define HALDRVID_EEP        3       /* EEPROM driver number */
#define HALDRVID_MAG        4       /* magnetic card reader card driver number */
#define HALDRVID_DAT        5       /* DATe driver number */
#define HALDRVID_COM        6       /* COM driver number */
#define HALDRVID_ICC        7       /* Smart card driver number */
#define HALDRVID_I2C        8       /* I2C driver number */
#define HALDRVID_CHE        9       /* CHEQUE reader */
#define HALDRVID_PME        10      /* PME peripheral */
#define HALDRVID_GSM        11      /* GSM module */
#define HALDRVID_ADC        12      /* ADC module */
#define HALDRVID_MIFARE     14      /* MIFARE module */
#define HALDRVID_WIFI       16      /* WIFI module */

#define HALDRVSTATE_STOPED      0   /* Order execution terminated on error    */
#define HALDRVSTATE_ENDED       1   /* DRIVER TREATMENT ENDED OK              */
#define HALDRVSTATE_ABSENT      2   /* Peripheral does not answer             */
#define HALDRVSTATE_FREE        3   /* Driver free for ordering               */
#define HALDRVSTATE_WAIT        4   /* Order waiting for execution            */
#define HALDRVSTATE_RUNNING     5   /* Order is running                       */
#define HALDRVSTATE_FULL        6   /* Driver waiting stack is full           */
#define HALDRVSTATE_UNKNOWN     7   /* Unknown order                          */
#define HALDRVSTATE_REJECTED    8   /* Driver does not exist (wrong driver id)*/
#define HALDRVSTATE_NOABORT     9   /* Abort refused to the requester         */
#define HALDRVSTATE_RESET       10  /* Orders in waiting stack are reseted    */
#define HALDRVSTATE_CONTINUE    0xFF

#define PADCMD_CLRDISP					0x04
#define PADCMD_CLRDISPROW     	0x05
#define PADCMD_BEEP							0x07
#define PADCMD_DISPASCIIROW			0x08
#define PADCMD_DISPASCIICOL			0x09
#define PADCMD_GETKEY   				0x0A
#define PADCMD_GETNKEYS					0x0B
#define PADCMD_GETPIN						0x0C
#define PADCMD_CRYPT						0x0D
#define PADCMD_BACKLIGHTCTL			0x30
#define PADCMD_DISPGB2312				0x36
#define PADCMD_DISPGRAPH				0x37
#define PADCMD_DISPARABIC				0x38
#define PADCMD_MFCQUEST					0x1F


#define HALDRVERR_PROCESSPENDING    0xFF
#define HALDRVERR_SUCCESS           0x00

#define HALDOTERR_NOTGBCHAR			0x01
#define HALDOTERR_SUCCESS				0x00

#define ASCIIFONT57     	0x30
#define ASCIIFONT816    	0x31
#define ASCIIFONT612		0x32
#define ASCIIFONT1224		0x33
#define ASCIIFONT78	    0x35
#define ASCIIFONT1212		0x36
#define ASCIIFONTSPE1212	0x06
#define ASCIIFONT1212_BOX   0xC0
#define ASCIIFONT1212_UPLINK   0x80
#define ASCIIFONT1212_DOWNLINK   0x40
#define ASCIIFONT1212_CENTERLINK   0x00

#endif
