#ifndef   TRIMAG_SPI_H
#define   TRIMAG_SPI_H

#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

/**************** trimag serial command *****************/
#define			CMD_READTRACK1				0x01
#define			CMD_READTRACK2				0x02
#define			CMD_READTRACK3				0x03
#define			CMD_READALLTRACK			0x04
#define			CMD_ATR						0x05
#define			CMD_STATUS					0x06
#define			CMD_BYTEREADTRACK1			0x11
#define			CMD_BYTEREADTRACK2			0x12
#define			CMD_BYTEREADTRACK3			0x13
#define			CMD_BYTEREADTRACK			0x14
#define			CMD_CONTIUEREAD				0x15
#define			CMD_RESET					0x16
#define			CMD_NOOP					0xff


#define			TRIMAG_RAWTRACKLEN   		1+96*3+1
#define			TRIMAG_TRACK1LEN				75
#define			TRIMAG_TRACK2LEN				37
#define			TRIMAG_TRACK3LEN				104


#define			MAGLIB_LRC	1
#define			KZALLOC_FAILED  2

#define MAX_TK_BUFLen	140			//定义每磁道数据最大长度
typedef struct
{
    unsigned char ucErr;
    unsigned char ucTKLen;
    unsigned char aucTKData[MAX_TK_BUFLen];
}TK_DATA;
#endif
