#ifndef __OSPLATFORM_H__
#define __OSPLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif
/** for imx258 below */
#define UART_MODEM	"/dev/ttymxc0"
#define UART_SCAN	"/dev/ttymxc0"
#define UART_GSM	"/dev/ttymxc1"	
#define UART_DEBUG	"/dev/ttymxc4"
#define UART_DNW	"/dev/ttymxc4"
#define UART_PINPAD	"/dev/ttymxc4"

#define UART0		"/dev/ttymxc0"
#define UART1		"/dev/ttymxc1"
#define UART2		"/dev/ttymxc2"
#define UART3		"/dev/ttymxc3"
#define UART4		"/dev/ttymxc4"

#define PRINTER		"/dev/printer"

#ifdef __cplusplus
}
#endif
#endif //__OSPLATFORM_H__

