
#define	DATAGLO
#include <global.h>
#include <semaphore.h>
unsigned char ProgressFlag;// 线程结束标志
unsigned char jiaoyiCnt;
unsigned char g_ThreadFlag;// 是否是线程
unsigned char g_ProgressBarFlag; // 进度条显示
sem_t binSem1;
sem_t binSem2;
#undef DATAGLO

