/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  classic.h
--------------------------------------------------------------------------
*/

#ifndef __CLASSIC_H__
#define __CLASSIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <osmifare.h>

#define CLASSIC_SEND_MAX_LEN	32
#define CLASSIC_REV_MAX_LEN		32

#define CLASSIC_DELAY_READ		3
#define CLASSIC_DELAY_WRITE		60
#define CLASSIC_DELAY_INC		60
#define CLASSIC_DELAY_DEC		60
#define CLASSIC_DELAY_RESTORE	60
#define CLASSIC_DELAY_TRANSFER	60

#define CLASSIC_BLOCK_LEN		16
#define CLASSIC_VALUE_LEN		4

#define CLASSIC_RESPOND_ACK		0x0A

#define CLASSIC_ERR_SUCCESS		0x00
#define CLASSIC_ERR_READ		0x01
#define CLASSIC_ERR_WRITE		0x02
#define CLASSIC_ERR_INC			0x03
#define CLASSIC_ERR_DEC			0x04
#define CLASSIC_ERR_TRANSFER	0x05
#define CLASSIC_ERR_RESTORE		0x06


unsigned char CLASSIC_Process( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );

#ifdef __cplusplus
}
#endif

#endif
