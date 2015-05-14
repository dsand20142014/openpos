/*
*  Written by huzhigang (huzhigang@sand.cn), Qct. 29,2009
*
*Modified by Jiangtulin
*/		
#ifndef BAR_API_H
#define BAR_API_H

#define KEY_RELEASE	0x00
#define KEY_PRESS	0x01

#define PLATFORM	PS4000
#if PLATFORM==PS4000
#define DEVICE  "/dev/event0"
#elif PLATFORM==S3C2440
#define DEVICE  "/dev/event1"
#elif PLATFORM==PC
#define DEVICE  "/dev/input/event3"
#endif

#define DEV_TRIG	"/dev/bar_scanner_triger"
#define LOW_LEVEL	0
#define HIGH_LEVEL	1
#define BEEP_ON		2
#define BEEP_OFF	3

#define SUCCESS		0
#define FAILURE		-1
#define BUF_BAR_LEN 1850

//int Os__bar_scanner( char *code);


#endif

