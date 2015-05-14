/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  dicard.h
--------------------------------------------------------------------------
*/

#ifndef __DICARD_H__
#define __DICARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DICARD_SEND_MAX_LEN	512
#define DICARD_REV_MAX_LEN 512

#define DICARD_DELAY_PURCHASE		100
#define DICARD_DELAY_CREDITLOAD		150
#define DICARD_DELAY_APPENDRECORD	50
#define DICARD_DELAY_UPDATAERECORD	50
#define DICARD_DELAY_UPDATAEBINARY	50
#define DICARD_DELAY_NORMAL			20

unsigned char DICARD_Process( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );

#ifdef __cplusplus
}
#endif

#endif
