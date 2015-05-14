#ifndef __SKB25_POWER__
#define POWER_EXTERN extern
#else
#define POWER_EXTERN
#endif

POWER_EXTERN void powerchange_detect(int n);
POWER_EXTERN void powerchange_reset(int n);
POWER_EXTERN int set_powerchange_flag_irq(int sec);
POWER_EXTERN int OSCST_ChkExt_Power();
POWER_EXTERN void OSCST_Ext_PowerOff();

