#ifndef WPA_H
#define WPA_H
/*
 return  -2 : connect timeout
	< 0 :connect failed
	=10   success
*/

int connectB(unsigned char timers, unsigned char change_flags);

/*
  param : 1  switch AP
          0  same Ap(same MAC)

  return -1 : init failed
	 0 : success
*/

int conn_init(void);

/*
	return -1 : exit failed
	 0 : success
*/
int conn_exit(void);

#endif

