#ifndef _BARSCANNER_H
#define _BARSCANNER_H

int bar_scan_9600(const char *szComDesc,char *buf,int len,char exitcode);
int Os__barscanner(char *buf,int barlen,char exitcode,int iretry);
#endif
