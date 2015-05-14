#ifndef GSM_H_INCLUDED
#define GSM_H_INCLUDED

int GSM__dial_check(unsigned short uiTimeout);
int GSM__setParam_gprs(int _bands);
int GSM_set_APN(unsigned char* ucAPN);
void GSM_config_pap_chap(char *username, char *password);

#endif //GSM_H_INCLUDED
