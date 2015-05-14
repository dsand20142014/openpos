
#ifndef  PRINT_API_H
#define  PRINT_API_H

#ifdef __cplusplus
extern "C" {
#endif

int Os__prn_forward(unsigned char);
int Os__prn_xforward(unsigned char);
int Os__prn_backward (unsigned char);
int OSPRN_XReverseFeed(unsigned char);
int Os__linefeed(unsigned char);
int Os__xlinefeed(unsigned char);
//int Os__print(unsigned char *);
int Os__print(fmode_t ,unsigned char *);
int Os__xprint(unsigned char *);
int Os__GB2312_xprint(unsigned char *);
int Os__conv_ascii_GB2312 (unsigned char * ) ;
int Os__graph_xprint (unsigned char * , unsigned char ) ;
int Os__black_mark(void);
int halfstep_modify(unsigned char *);

#ifdef __cplusplus
}
#endif

#endif
