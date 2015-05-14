
#define	GLOBAL_DATA
#include <test/Global.h>



#include <semaphore.h>

sem_t binSem1;
sem_t binSem2;

//void clearScreen()
//{
//    memset(myString,0x00,sizeof(myString));
//}

//void display_Info(int line,char* text,bool emitSignal)
//{
//    memcpy(myString[line],text,sizeof(text));

//    if(emitSignal)
//    {
//        sem_post(&binSem1);
//        sem_wait(&binSem2);
//    }
//}

#undef GLOBAL_DATA

