#ifndef __SKB25_DISPLAY__
#define DISPLAY_EXTERN extern
#else
#define DISPLAY_EXTERN
#endif


DISPLAY_EXTERN void //Os__clr_display ( unsigned char line );
DISPLAY_EXTERN void Os__display(unsigned char Line, unsigned char Column, unsigned char *Text);
DISPLAY_EXTERN unsigned char //Os__GB2312_display(unsigned char Line, unsigned char Column, unsigned char *Text);
DISPLAY_EXTERN unsigned char Os__light_on ();
DISPLAY_EXTERN unsigned char Os__light_off ();
DISPLAY_EXTERN void kx_je_dfj(long int *b);
DISPLAY_EXTERN void Os__display_recvdata(unsigned char Line, unsigned char Column, unsigned char *rec,  	int len);
DISPLAY_EXTERN unsigned char //Os__GB2312_display(unsigned char,unsigned char,unsigned char * pt_msg);

DISPLAY_EXTERN  int FileMode_to_ANSI(unsigned char ucMode);

DISPLAY_EXTERN  int lcd192_main();
