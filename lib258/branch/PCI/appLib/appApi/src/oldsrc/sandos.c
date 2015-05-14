#include <stdio.h>

extern void Uart_Printf(char *fmt,...);
extern int mag_test();
extern int smart_main(char argc, char *argv[]);
extern int pboc_main(char argc, char *argv[]);
extern int fileops_main(char argc, char *argv[]);
extern int icc_main();
extern int key_main(int argc,char *argv[]);
extern int jn24_test(char argc, char **argv);
extern int barscan_test();
extern void usb_test(void);

int manager(void *p){}//2010-05-18
int main(char argc, char *argv[])
{
    Uart_Printf("SandOS demo function\n");

#ifdef DEMO_USB
	usb_test();
#endif

#ifdef DEMO_RFCMD
	cmd_rf_test();
#endif

#ifdef DEMO_RFCLASSIC
	classic_test(argc,argv);
#endif

#ifdef DEMO_BARSCAN
	barscan_test();
#endif


#ifdef DEMO_MF
	mifare_test(argc, argv);
#endif

#ifdef DEMO_RF
	rf_test(argc, argv);
#endif

#ifdef DEMO_MODULES
	modules_test(argc, argv);
#endif


#ifdef DEMO_JN24
	jn24_test(argc, argv);
#endif


#ifdef DEMO_4442
    test_sync();
#endif


#ifdef DEMO_MAG
    mag_test();
#endif

#ifdef DEMO_SMART
    smart_main(argc, argv);
#endif

#ifdef DEMO_FILE
    fileops_main(argc, argv);
#endif

#ifdef DEMO_ICC
    icc_main();
#endif



#ifdef DEMO_KEY
    key_main(argc, argv);
#endif
    return 0;
}

