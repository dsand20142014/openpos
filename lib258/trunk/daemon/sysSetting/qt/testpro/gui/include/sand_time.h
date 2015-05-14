#ifndef SAND_TIME_H
#define SAND_TIME_H

#define TIME_EXTERN extern

#ifdef __cplusplus 
extern "C" { 
#endif



TIME_EXTERN unsigned char Os__xdelay (unsigned int Delay);
TIME_EXTERN void Os__read_date (unsigned char *ptDate);
TIME_EXTERN void Os__read_date_format (unsigned char *ptDate);
TIME_EXTERN unsigned char Os__read_date_time (unsigned char *ptDateTime);
TIME_EXTERN void Os__read_time (unsigned char *ptTime);
TIME_EXTERN void Os__read_time_format (unsigned char *ptTime);
TIME_EXTERN void Os__read_time_sec (unsigned char *ptTime);
TIME_EXTERN unsigned char Os__write_date (unsigned char *ptDate);
TIME_EXTERN unsigned char Os__write_time (unsigned char *ptTime);
TIME_EXTERN unsigned char Os__timer_start (unsigned int *ptTimer);
TIME_EXTERN void Os__timer_stop (unsigned int * ptTimer) ;
TIME_EXTERN unsigned int Os__timer_remain ();
TIME_EXTERN	unsigned char OS__timer_test();
TIME_EXTERN void Os_time_test();
TIME_EXTERN void OSDATE_ReadTimeSec(unsigned char *pucTime);

#ifdef __cplusplus 
}
#endif


#endif
