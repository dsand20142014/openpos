/*
	Operating System
--------------------------------------------------------------------------
	FILE  oslsr.h
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

#ifndef __OSLSR_H
#define __OSLSR_H

/* For transaction event first phase : EVE_ASK_...
    => application must indicate its level of acceptance of the card
       (remark : for smart card, if the application needs to dialogue with it,
        it must power it ON and OFF before answering to the monos).
   Incoming data : same data as the corresponding transaction event.
   Returned data MUST be :
   treatment_status     = TREATED
   application_status   = OK
   xxdata = point to a word indicating the uncertainty on the treatment of the
            card : .......BBBLLLLCC
               BBB  = number of 'F' in the BIN (0-6)
               LLLL = length interval (0-15)
               CCC  = number of 'F' in the service code (0-3)
            Example : If we read the following card :
                        bin : 432123
                        card number length = 18
                        service code = 101
                      And if the application accepts the following cards :
                        bin : 43FFFF
                        card number length = 17 to 19
                        service code = 1FF
                      So, we have the following uncertainty :
                        bin : b=4 (=100 binary)
                        card number length : l=2 (19-17) (=0010 binary)
                        service code : c=2 (=10 binary)
                      And the application must return :
                      static ushort value;
                        value = (b*0x40) + (l*4) + c; // binary:100001010
                        xxdata = (uchar*)&value;
            So, the best application will be the one giving the smallest
            value.
*/

/* For EVE_LSR : */
// Events numbers :
enum    slsr_event_nr {         /* constantes defining the lsr event numbers */
        LSR_CONFIGURATION,      // print application configuration (name)
        LSR_PLANNING,           // print application automatic treatment planning
        LSR_MODPARAM,           // request for common parameters modification
        LSR_ANOMALIES,          // application's treatment anomalies
        LSR_ASK_MPEV5_INFO      // OS request for MPEV5 Parameters
};

/* Transmitted data (defined in osevent.h) : 'e_lsr'
struct seve_lsr {               // structure for LSR specific events :
         uchar  lsr_event_nr;   // specific lsr event number ()
         uchar  lsr_event_id;   // complementary lsr event id
         uchar  xxdata[256];    // transmitted data
};
*/
typedef struct scurrency
{
    ushort num_code;      // Currency Code
    uchar  name[3];      // Currency Name
    uchar  decimal;      // Number of digits after the comma
}CURRENCY;

#ifdef SLAVE_TERMINAL
#define MAX_CURRENCY    10
#else
#define MAX_CURRENCY    7
#endif
typedef struct sos_mpev5_info
{
    uchar Nb_Currency;                 // Number of currency in the terminal
    CURRENCY Os_Currency[MAX_CURRENCY];// Array of currencies in the terminal
    uchar mpev5_appli[10];             // Flag that indicated if appli [i]
                                       // support MPEV5 configuration
    uchar Mpev5Flag[3];
#ifdef SLAVE_TERMINAL
    uchar FLAG_currency[3];             // not compatible with standalone OS
#endif
}OS_MPEV5_INFO;

// Fct to get the address of the OS_MPEV5_INFO structure of the MONOS
struct sos_mpev5_info *Os__get_mpev5_array(void);
struct sos_mpev5_info *Os__get_mpev5_info(void);

// List of common parameters for LSR_MODPARAM (data transmitted in xxdata is the new value)
enum    slsr_modparam {         /* constantes defining the lsr event numbers */
        LSR_DIALTYPE,           // modem dialling type '0':pulse(numeric),'1':tone(frequency) followed by 0
        LSR_PABX,               // PABX number
        LSR_PADX25,             // X25 PAD phone number for automatic download
        LSR_ONLYV22,            // forcing only V22 mode : '1'=only v22, '0'=try v22bis followed by 0
        LSR_OPTIONSTABLE,       // acquisition of a new table number (portable terminal only)
        LSR_TIME,               // manual change of the current time
        LSR_CLEMODEM,           // choice cle rnis='0' or internal modem='1' or external modem='2'
        LSR_CLEVITESSE,         // cle - modem speed : 0=1200,1=2400,2=4800,3=9600,4=19200
        LSR_DIALDURATION,       // dialing max duration in seconds
        LSR_DISABLETOTALS,      // 1=disable printing of totals
        LSR_CLENROCOM,          // RS232 used for RNIS or external modem : '0'=COM2,'1'=COM3
        LSR_DTRCLEMODEM,        // DTR management for external modem and RNIS : '0'=hard,'1'=soft
        LSR_ALLOWABORTDIAL,     // 1=allow abort when base is requested during dial (portable terminals only).
        LSR_DEFAULTTRANSACTION, // Default transaction 0:debit 1:credit
        LSR_DEFAULTCURRENCY     // Default currency 0:last 1:FRF 2:EUR
};



#endif

