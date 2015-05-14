/********************************************************************
* timer functions 
*********************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include <error.h>
#include <sys/time.h>

       #include <sys/types.h>

	   
#include <pthread.h>

#include "sand_timer.h"

static int   timer_new_thread(void);
static void* timer_handle(void*);

static inline void os_sleep(__time_t sec,__suseconds_t us)
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = us;
    select(0, NULL, NULL, NULL, &tv);
}

void timer_init(void)
{
    int i;
	
	g_timer_cnt = 0;
    for(i=0;i<MAXNUM_TIMER;i++){
        timer_tbl[i].ptimer = 0;
    }
	
    g_inter_timer_cnt = 0;
    for(i=0; i<MAXNUM_INTER_TIMER; i++){
    	inter_timer_tbl[i].ptimer = 0;
    	inter_timer_tbl[i].timer_proc = 0;
    }
    timer_new_thread();
}

void timer_print(void)
{
    int i;
	
	Uart_Printf("g_timer_cnt=%d\n",g_timer_cnt);
	
    for(i=0;i<MAXNUM_TIMER;i++){
		Uart_Printf("timer_tbl[i].ptimer =%d\n",*(timer_tbl[i].ptimer));
    }
	
	Uart_Printf("g_inter_timer_cnt=%d\n",g_inter_timer_cnt);        
	
    for(i=0; i<MAXNUM_INTER_TIMER; i++){
		Uart_Printf("inter_timer_tbl[i].ptimer =%d\n",*(inter_timer_tbl[i].ptimer));
    }

}

int timer_start(unsigned char type,volatile unsigned int *ptimer)
{
    unsigned char i;
//printf("%s %d\n", __func__, getpid());
    if( !ptimer ){
        return(TIMER_BADPARAM);
    }

    for(i=0;i<MAXNUM_TIMER;i++){
        if( timer_tbl[i].ptimer == ptimer ){
            return(0);
        }
    }
    for(i=0;i<MAXNUM_TIMER;i++){
        if( !timer_tbl[i].ptimer ){
            timer_tbl[i].type = type;
            timer_tbl[i].ptimer = ptimer;
            return (0);
        }
    }
    return (-1);
}

int timer_stop(volatile unsigned int *ptimer)
{
    unsigned char i;
//printf("%s %d\n", __func__, getpid());

    if( !ptimer ){
        return(TIMER_BADPARAM);
    }
    for(i=0;i<MAXNUM_TIMER;i++){
        if(timer_tbl[i].ptimer == ptimer){
            timer_tbl[i].ptimer = 0;
            break;
        }
    }
    return(0);
}

int timer_Internal_start(volatile unsigned int *ptimer,void (*pfnProc)(void))
{
    unsigned char i;

    if( !ptimer ){
        return(TIMER_BADPARAM);
    }

	//timer_stop(ptimer);
    for(i=0;i<MAXNUM_INTER_TIMER;i++){
    	if( inter_timer_tbl[i].ptimer == ptimer ){
    		inter_timer_tbl[i].timer_proc = pfnProc;
				goto start;
		}
    	if( !inter_timer_tbl[i].ptimer ){
    		inter_timer_tbl[i].ptimer = ptimer;
    		inter_timer_tbl[i].timer_proc = pfnProc;
    		g_inter_timer_cnt ++;
    		goto start;
    	}
    }
	return(TIMER_TIMERFULL);
start:
	//timer_start(0,ptimer);
	return(TIMER_SUCCESS);

}

int timer_Internal_stop(volatile unsigned int *ptimer)
{
    unsigned char i;

    if( !ptimer ){
        return(TIMER_BADPARAM);
    }
    //timer_stop(ptimer);
    for(i=0;i<g_inter_timer_cnt;i++){
        if(inter_timer_tbl[i].ptimer == ptimer ){
    		if( i != g_inter_timer_cnt-1 ){
				inter_timer_tbl[i].ptimer = inter_timer_tbl[g_inter_timer_cnt-1].ptimer;
				inter_timer_tbl[i].timer_proc = inter_timer_tbl[g_inter_timer_cnt-1].timer_proc;
	    		inter_timer_tbl[g_inter_timer_cnt-1].ptimer = 0;
	    		inter_timer_tbl[g_inter_timer_cnt-1].timer_proc = 0;
    		}else{
	    		inter_timer_tbl[i].ptimer = 0;
	    		inter_timer_tbl[i].timer_proc = 0;
    		}
    		g_inter_timer_cnt --;
        	break;
        }
    }
	//timer_start(0,ptimer);
    return(TIMER_SUCCESS);
}

static int timer_new_thread(void)
{
    pthread_t ptid;

	if (pthread_create(&ptid,NULL,timer_handle,NULL)< 0){
		perror("thread fail!");
		exit(-1);
	}
	return 0;
}

static void* timer_handle(void *p)
{
    unsigned char i;
    unsigned int timer_value;

    while(1){
		for(i=0;i<g_inter_timer_cnt;i++){
			if( inter_timer_tbl[i].ptimer ){
				timer_value = *inter_timer_tbl[i].ptimer;
				if( timer_value ){
					timer_value --;
					*inter_timer_tbl[i].ptimer = timer_value;
					if( !timer_value ){
						if( inter_timer_tbl[i].timer_proc ){
			        	    (*(inter_timer_tbl[i].timer_proc))();
						}
					}
				}
			}
		}
	    g_timer_cnt++;
	    for(i=0; i<MAXNUM_TIMER; i++){
	        if( timer_tbl[i].ptimer ){
	            timer_value = *(timer_tbl[i].ptimer);
	            if( timer_value )
	                timer_value --;
	            *(timer_tbl[i].ptimer) = timer_value;
	            if( !timer_value ){
	            	timer_tbl[i].ptimer = 0;
	            }
	        }
	    }
	    if( g_timer_cnt >= TIMERCOUNT_INGENICO ){
	        g_timer_cnt = 0;
	    }
		//Uart_Printf("thread (%d)\n",getpid());
		os_sleep(0,10000);
	}
}
//
unsigned char Os__timer_start (unsigned int *ptTimer)
{
	//*ptTimer = (*ptTimer)*10/15;
	return timer_start(0,ptTimer);
}

void Os__timer_stop (unsigned int * ptTimer)
{
    timer_stop(ptTimer);
}
unsigned char OSTIMER_Start(unsigned int *puiTimeout)
{
    return Os__timer_start(puiTimeout);
}

void OSTIMER_Stop(unsigned int *puiTimeout)
{
    return Os__timer_stop (puiTimeout);
}	

unsigned char OSTIMER_Delay(unsigned int uiTimeout)
{
    usleep(10000);
}	
///////////////////////   END ////////////////////////////

