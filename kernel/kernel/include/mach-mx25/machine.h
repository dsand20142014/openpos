#ifndef SAND_MACHINE_H
#define SAND_MACHINE_H

#define SAND_DEMOBOARD	0
#define SAND_PS400		1
#define SAND_PS4K		2
#define SAND_IPS420		3
#define SAND_PS4KV2		4
#define SAND_IPS100N    5

extern int sand_machine(void);

/*
 * Define signals use for the aio.
 */
#define  KEY_SIG	(SIGIO)
#define  MAG_SIG	(SIGRTMIN+6)
#define  MFR_SIG	(SIGRTMIN+7)
#define  ICC_SIG  	(SIGRTMIN+8)
#define  APP_KEY_SIG	(SIGRTMIN+9)
#define  SECURITY_SIG	(SIGRTMIN+10)
#define  POWERKEY_SIG	(SIGRTMIN+11)

/*
 * Define the hardware control bit for 
 * the power management.
 * The corresponding bit:1=on 0=off.
 */
#define  GPRS_PWR	(1<<0)//GPRS POWER MARK
#define  GPS_PWR	(1<<1)//GPS POWER MARK
#define  BAR_PWR	(1<<2)//BAR CODE SCANNER POWER MARK
#define  WIFI_PWR	(1<<3)//WIFI POWER MARK
#define  LED1_PWR	(1<<4)//LED1 POWER MARK
#define  LED2_PWR	(1<<5)//LED2 POWER MARK
#define  LED3_PWR	(1<<6)//LED3 POWER MARK
#define  LED4_PWR	(1<<7)//LED4 POWER MARK

#endif//SAND_MACHINE_H
