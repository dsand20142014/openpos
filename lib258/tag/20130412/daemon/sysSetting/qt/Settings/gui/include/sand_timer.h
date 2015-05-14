#ifndef __TIMER_H
#define __TIMER_H

#include <signal.h>

#ifdef __cplusplus 
extern "C" { 
#endif


#define MAXNUM_TIMER              25
#define MAXNUM_INTER_TIMER        10
#define TIMER_10MS                10000

#define TIMER_SUCCESS              0
#define TIMER_TIMERFULL           (-1)
#define TIMER_BADPARAM            (-2)

#define TIMERCOUNT_INGENICO        10

#define TIMER_TYPE_DECREASE       0x00
#define TIMER_TYPE_INGENICO       0x01

struct timer {
    unsigned char type;
    volatile unsigned int *ptimer;
};

struct inter_timer {
	volatile unsigned int *ptimer;
	void (*timer_proc)(void);
};

int  g_timer_cnt;
int  g_inter_timer_cnt;
struct timer timer_tbl[MAXNUM_TIMER];
struct inter_timer inter_timer_tbl[MAXNUM_INTER_TIMER];


void timer_init(void);
int  timer_start(unsigned char type,volatile unsigned int *ptimer);
int  timer_stop(volatile unsigned int *ptimer);
int  timer_Internal_start(volatile unsigned int *ptimer,void (*pfnProc)(void));
int  timer_Internal_stop(volatile unsigned int *ptimer);

//  system call
int  timer_sys_start(long sec,long usec);
int  timer_sys_stop(void);
int  timer_sys_action(void (*sigfunc)(int signum,siginfo_t *info,void *myact));

#ifdef __cplusplus 
}
#endif


#endif
