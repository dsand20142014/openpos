#include <hal.h>
#include <haldata.h>
#include <halprn.h>
#include <haldot.h>

#define M1103PRNDOTPORT	(*(unsigned char volatile *)(0x10100004))	//(*(unsigned char volatile *)(0x4000010))
#define M1103PRNMOTPORT	(*(unsigned char volatile *)(0x10100008))	//(*(unsigned char volatile *)(0x4000014))
#define M1103PRNSTAPORT	(*(unsigned char volatile *)(0x1010000c))	//(*(unsigned char volatile *)(0x4000018))  

#define M1103_HOME	((M1103PRNSTAPORT & (0x01<<4))>>4)
#define M1103_PAPER	((M1103PRNSTAPORT & (0x01<<5))>>5)
#define M1103_MARK	((M1103PRNSTAPORT & (0x01<<6))>>6)
#define M1103_TM		((M1103PRNSTAPORT & (0x01<<7))>>7)

#define M1103STATUS_IDLE			0x00
#define M1103STATUS_FEEDING		0x01
#define M1103STATUS_PRINTING		0x02
#define M1103STATUS_PRINTENDING	0x04


#define M1103PRN_EVENTSTEP			413
#define M1103PRN_PAPERMOTOPOWER	5000
#define M1103PRN_PAPEVENTSTEP		7000
#define M1103PRN_PAPACEVENTSTEP	1250
#define M1103PRN_INITEVENTSTEP		1050//1500
#define	M1103PRN_CHECKENDTIMEOUT	100

#define M1103PRN_LASH				4956

#define M1103PRN_BUFROWNB		10
#define M1103PRNLINE_DOTMAXNB	360

#define M1103TIMER_TYPE0			0x00
#define M1103TIMER_TYPE1			0x01
/*----------------------------------*/
#define M1103LSTEP_LTOR			0x00
#define M1103LSTEP_RTOL			0x01

/*----------------------------------*/
#define M1103LSTEP_FORW			0x00
#define M1103LSTEP_BACK			0x01
#define M1103LSTEP_POWERON		0x02
#define M1103LSTEP_MOTOMOVE		0x03
#define M1103LSTEP_CHECKMARK		0x04
#define	M1103STEP_PAPERACLASH		0x05
#define	M1103STEP_PAPERDCLASH		0x06

/*----------------------------------*/
#define M1103LSTEP_MOTOEN		0x00
#define M1103LSTEP_MOTOAC		0x01
#define M1103LSTEP_MOTODC		0x02
#define	M1103STEP_ACLASH		0x03
#define	M1103STEP_DCLASH		0x04
#define	M1103STEP_PRINT			0x05
#define M1103STEP_INITPHASE			0x06
#define	M1103STEP_DOT			0x07
#define M1103LSTEP_CONST		0x08



#define	M1103DOT1				0x01
#define	M1103DOT2				0x02
#define	M1103DOT3				0x03

/*----------------------------------*/
#define M1103LSTEP_NORMAL		0x00
#define M1103LSTEP_COMPRE		0x01
/*----------------------------------*/

#define M1103_POWEREN_ENABLE		1
#define M1103_POWEREN_DISABLE	0
#define M1103_LIMIT_ENABLE		1		
#define M1103_LIMIT_DISABLE		0
#define M1103_ST_ENABLE			0
#define M1103_ST_DISABLE			1

#define M1103_POWERSTATUS_POWON	1
#define M1103_POWERSTATUS_POWOFF	0
#define M1103_CARRIAGEMOTO_ON	1
#define M1103_CARRIAGEMOTO_OFF	0
#define M1103_PAPERMOTO_ON		1
#define M1103_PAPERMOTO_OFF		0

#define M1103_TEMPERATURE_HIGH	231		// -> 100度 (Rthermistor = 13.97K,TM1=2.310358106 V)
#define M1103_TEMPERATURE_LOW	221		// -> 120度 (Rthermistor = 15.17K,TM1=2.208317998 V)

#define M1103_TM_NORMAL			0
#define M1103_TM_OVERHEAT		1

#define	M1103_DEBUG				0

unsigned char ucM1103FirstMaskFlag = 0;
unsigned char ucM1103SaveFlag = 0;
unsigned char ucM1103DoFlag = 0;
unsigned char ucM1103EndFlag = 0;
extern unsigned char ucSuspendFlag;

typedef struct
{
volatile	unsigned char ucM1103Status;				/*当前打印机状态*/    																
	unsigned char ucM1103PrintEnd;			/*打印完成标志*/      																
	unsigned char ucM1103PrintNB;			/*当前添入Buffer的行数																*/
	unsigned char ucM1103PrintedNB;			/*被打印的行数*/      																
	unsigned char ucM1103PrintMove;			/*打印马达移动方式*/  																
	unsigned char ucM1103PaperMove;			/*走纸马达移动方式*/  																
	unsigned char ucM1103StepCount;			/*打印机马达加减速计数																*/
	unsigned char ucM1103PaperMode;	 		/*走纸马达模式定义*/  																
	unsigned char ucM1103PrintFunc;	 		/*打印马达功能定义*/  																
	unsigned char ucM1103PaperFunc; 			/*走纸马达功能定义*/  																
	unsigned char ucM1103PrintData;			/*打印电压,相位状态*/ 																
	unsigned char ucM1103PrintBufFull;		/*打印Buffer满标志*/  																
	unsigned char ucM1103PrintBufNotEmpty;	/*打印Buffer空标志*/
	unsigned char ucM1103MarkExistFlag;		/*黑标存在标志*/
	unsigned short uiM1103PrintStepNB;		/*打印机马达步进点数*/
	unsigned short uiM1103PrintedLen;		/*上次打印走过的行数*/
	unsigned short uiM1103PrintFactNB;		/*打印机实际走的步数*/
	unsigned short uiM1103PaperCount;		/*打印机走纸计数*/ 
	unsigned short uiM1103PaperedCount;		/*打印机走纸完成计数*/ 
	unsigned short uiLineDotBufLen;			/*单行数据长度*/
	unsigned short auiM1103DotBufLen[M1103PRN_BUFROWNB];	/*每行点阵的总数*/
	unsigned char aucM1103LineDotBuf[M1103PRNLINE_DOTMAXNB];	/*单行打印数据*/
	unsigned short auiM1103StepNB[M1103PRN_BUFROWNB];	/*记录每步打印后应走纸步数*/
	unsigned char aucM1103DotBuf[M1103PRN_BUFROWNB][M1103PRNLINE_DOTMAXNB];/*打印点阵数据*/
	volatile	unsigned char ucENStatus;					//匀速打印时的状态
	unsigned int uiLatchTime1;
	unsigned int uiHoldTime;
	unsigned int uiLatchTime2;
	volatile unsigned int uiCheckTimeout;
//	unsigned int uiMask;
	unsigned int uiEINTMASK;
	unsigned int uiINTMSK;
	unsigned int uiINTSUBMSK;
}M1103PRINTFUNC;




M1103PRINTFUNC  M1103_Func;
M1103PRINTFUNC  *pM1103Func;

unsigned short M1103_INIT_AC_Timer[] = {3960,2360,1890,1640,1490,1380,1300,1240,1190,1140};
unsigned short M1103_INIT_DC_Timer[] = {1080,1140,1220,1310,1420,1570,1780,2110,2750,6650};

unsigned short M1103_RL_AC_Timer[] = {3960,2448,1890,1586,1390,1252,1147,1065,998,942,894,855,826,826,826,826,826};
unsigned short M1103_RL_DC_Timer[] = {826,826,826,826,826,860,920,1000,1090,1200,1350,1560,1890,2530,6000};
#if 1
unsigned short M1103_LR_AC_Timer[] = {3960,2448,1890,1586,1390,1252,1147,1065,998,942,894,855,826,826};
unsigned short M1103_LR_DC_Timer[] = {826,826,826,826,826,826,826,826,860,920,1000,1090,1200,1350,1560,1890,2530,6000};

#else
unsigned short LR_AC_Timer[] =  {4900,1920,1470,1240,1090,990,910,850,826,826,826,826,826,826,826};
unsigned short LR_DC_Timer[] = {826,826,826,826,826,826,826,860,920,1000,1090,1200,1350,1560,1890,2530,6000};
#endif
unsigned short M1103_PAP_AC_Timer[] = {7000,3880,2980,2510,2210,2000,1840,1710,1610,1520,1450,1380,1330,1280};
unsigned short M1103_PAP_DC_Timer[] = {1670,2500};

#define		StepArrayNums(StepTimer)		sizeof((StepTimer))/sizeof((StepTimer)[0])
unsigned char gucM1103PowerStatus;
unsigned char gucM1103CarriageMotoStatus;
unsigned char gucM1103PaperMotoStatus;
static unsigned char ucPrintDot;

unsigned char M1103_Init(void);
unsigned char M1103_Feed(unsigned short uiLineNB);
unsigned char M1103_ReverseFeed(unsigned short uiLineNB);
unsigned char M1103_Mark(unsigned char ucFlag);
unsigned char M1103_CheckPaper(unsigned char ucFlag);
unsigned char M1103_Test(void);
unsigned char M1103_PrintGraph(unsigned char *pucStr,unsigned short uiLen);
unsigned char M1103_PrintStrGB2312(unsigned char *pucStr);
unsigned char M1103_PrintStrASCII(unsigned char *pucStr);
unsigned char M1103_PrintMultiLang(unsigned char *pucStr);
unsigned char M1103_TestPrintProtect(unsigned char ucFlag);	

void M1103_Config_POWEN(unsigned char ucValue);
void M1103_Config_LIMIT(unsigned char ucValue);
void M1103_Config_ST(unsigned char ucValue);

void M1103_SaveDot(unsigned char ucStepNB,unsigned char ucXor);
unsigned char M1103_RowPrint(unsigned char ucRowNum);
void __irq M1103_Timer0Interrupt(void);
void __irq M1103_Timer1Interrupt(void);
unsigned char M1103_PowerOn(void);
unsigned char M1103_PowerOff(void);
unsigned char M1103_TestHome(void);
void M1103_ACLash(void);
void M1103_DCLash(void);
void M1103_StartCarriageMotor(unsigned char ucFlag);
unsigned char M1103_WaitPrinterCoolHard(void);
void M1103_CalcLatchHoldTime(void);
void M1103_TrunckBlankSpace(unsigned char *pucStr);
void M1103_CheckPrintEndProc(void);
unsigned char M1103_bPrintComplete(void);
unsigned int M1103_CalcStepTime(unsigned short auiTimer[],unsigned char ucNum);
unsigned char M1103_CheckStatus(unsigned char ucFlag);
void __irq M1103_Timer2Interrupt(void);
void M1103_PrintDot(unsigned char ucDot);
void M1103_SaveDot1(unsigned char ucStepNB,unsigned char ucXor);
unsigned char M1103_PrintStrGB2312Dot12_1(unsigned char *pucStr);


PRNFUNC M1103Func =
{
	"EPSON M111SIII",
	M1103_Init,
	0,
	0,
//	0,
	M1103_CheckStatus,//M1103_Mark,
	M1103_CheckPaper,
	M1103_Feed,
	M1103_ReverseFeed,
	M1103_Test,
	M1103_PrintGraph,
	M1103_PrintStrGB2312,
	M1103_PrintStrASCII,
	M1103_PrintMultiLang,	
	M1103_TestPrintProtect
};

/*
	//=== PORT C GROUP
	//GPCCON-> 00:INPUT  01:OUTPUT  10:FUNC  11:RESERVED
	//Ports  : GPC2          GPC1          GPC0
	//Signal : (PRINT_POWEN) (PRINT_LIMIT) (PRINT_ST)	
	//Binary :	01 ,          01            01
	rGPCDAT = rGPCDAT |(0x01<<0)|(0x01<<1)&(~(0x01<<2));		//GPC0=1,GPC1=1,GPC2=0
	rGPCCON = 0x55555555;		
	rGPCUP	= 0x0;
*/
void __irq M1103_Timer1Test(void);
void __irq M1103_Timer0Test(void);
unsigned char M1103_Init(void)
{

	unsigned char ucRet =HALPRNERR_SUCCESS;

	//Disable TIMER0
	rINTMSK |= BIT_TIMER0;
	//Disable TIMER1
	rINTMSK |= BIT_TIMER1;
	
	//EN: Timer0&1 prescaler value is 25
	rTCFG0 = (rTCFG0 & 0xFFFFFF00) | 0x18;
	//EN: Timer0 Mux: 1/2
    rTCFG1 = (rTCFG1 & 0xFFFFFFF0) | 0x00;
	//EN: Timer1 Mux: 1/2
	rTCFG1 = (rTCFG1 & 0xFFFFFF0F) | 0x00;

	//Disable TIMER2
	rINTMSK |= BIT_TIMER2;

	/*EN: Timer2&3&4 prescaler value is 25 */
	rTCFG0 =  0x1800 + (rTCFG0 & 0xFFFF00FF);
	
//EN: Timer2 Mux: 1/2
	rTCFG1 = (rTCFG1 & 0xFFFFF0FF); 

	/*EN:
		Timer2 Config as 1us timer*/
		
	//rTCNTB2 = 0x3E8;	//(1/(PCLK/25/2))*0x3E8 = 0.001s (PCLK=50MHz)

	

	pISR_TIMER0 = (int)M1103_Timer0Interrupt;
	pISR_TIMER1 = (int)M1103_Timer1Interrupt;
	pISR_TIMER2 = (int)M1103_Timer2Interrupt;

	M1103PRNMOTPORT = 0x00;
	M1103PRNDOTPORT = 0xFF;
	/*rGPECON = (rGPECON&(~(0x03<<10)))|(0x01<<10); 
	rGPECON = (rGPECON&(~(0x03<<12)))|(0x01<<12); 
	rGPECON = (rGPECON&(~(0x03<<14)))|(0x01<<14); 
	rGPECON = (rGPECON&(~(0x03<<16)))|(0x01<<16); 
	rGPECON = (rGPECON&(~(0x03<<18)))|(0x01<<18); 
	rGPECON = (rGPECON&(~(0x03<<20)))|(0x01<<20); 
	rGPCCON = (rGPCCON&(~(0x03<<16)))|(0x01<<16); 
	rGPCCON = (rGPCCON&(~(0x03<<18)))|(0x01<<18); 
	rGPCUP	= rGPCUP&(~(0x01<<8))&(~(0x01<<9));
	rGPEUP	= rGPEUP&(~(0x01<<5))&(~(0x01<<6))&(~(0x01<<7))&(~(0x01<<8))&(~(0x01<<9))&(~(0x01<<10));
	rGPEDAT = rGPEDAT |(0x01<<5)|(0x01<<6)|(0x01<<7)|(0x01<<8)|(0x01<<9)|(0x01<<10);
	rGPCDAT = rGPCDAT |(0x01<<8)|(0x01<<9);*/

	pM1103Func = &M1103_Func;

	memset(pM1103Func,0,sizeof(M1103_Func));
	ucM1103SaveFlag = 1;
	//HALTIMER_InternalStart(&M1103_Func.uiCheckTimeout,M1103_CheckPrintEndProc);
	ucM1103SaveFlag = 0;
//	M1103_PowerOn();
//	ucRet =M1103_TestHome();
	M1103_PowerOff();

	return  HALDOT_VerifyGB2312Dot12();
}

void M1103_Config_POWEN(unsigned char ucValue)
{
#if 0
	Uart_Printf("M1103_Config_POWEN-->\n");
#endif	
	if(ucValue == M1103_POWEREN_DISABLE)
	{
		rGPCCON = (rGPCCON&(~(0x03<<4))|(0x01<<4));
		rGPCUP	= rGPCUP&(~(0x01<<2));
		rGPCDAT = rGPCDAT&(~(0x01<<2));		//GPC2=0
	}
	else	//M1103_POWEREN_ENABLE
	{
		rGPCCON = (rGPCCON&(~(0x03<<4))|(0x01<<4));
		rGPCUP	= rGPCUP&(~(0x01<<2));
		rGPCDAT = rGPCDAT|(0x01<<2);		//GPC2=1
	}
#if 0
	if(ucValue == M1103_POWEREN_DISABLE)
		Uart_Printf("	POW_OFF\n");
	else
		Uart_Printf("	POW_ON\n");
#endif
}

void M1103_Config_LIMIT(unsigned char ucValue)
{
#if 0
	Uart_Printf("M1103_Config_LIMIT-->\n");
#endif		
	if(ucValue == M1103_LIMIT_DISABLE)
	{
		rGPCCON = (rGPCCON&(~(0x03<<2))|(0x01<<2));
		rGPCUP	= rGPCUP&(~(0x01<<1));
		rGPCDAT = rGPCDAT&(~(0x01<<1));		//GPC1=0
	}
	else	//M1103_LIMIT_ENABLE
	{
		rGPCCON = (rGPCCON&(~(0x03<<2))|(0x01<<2));
		rGPCUP	= rGPCUP&(~(0x01<<1));
		rGPCDAT = rGPCDAT|(0x01<<1);		//GPC1=1
	}
#if 0
	if(ucValue == M1103_LIMIT_DISABLE)
		Uart_Printf("	LIMIT_OFF\n");
	else
		Uart_Printf("	LIMIT_ON\n");
#endif
}

void M1103_Config_ST(unsigned char ucValue)
{
#if 0
	Uart_Printf("M1103_Config_ST-->\n");
#endif	
	if(ucValue == M1103_ST_ENABLE)
	{
		rGPCCON = (rGPCCON&(~(0x03<<0))|(0x01<<0));
		rGPCUP	= rGPCUP&(~(0x01<<0));
		rGPCDAT = rGPCDAT&(~(0x01<<0));		//GPC0=0
	}
	else	//M1103_ST_DISABLE
	{
		rGPCCON = (rGPCCON&(~(0x03<<0))|(0x01<<0));
		rGPCUP	= rGPCUP&(~(0x01<<0));
		rGPCDAT = rGPCDAT|(0x01<<0);		//GPC0=1
	}
#if 0
	if(ucValue == M1103_ST_ENABLE)
		Uart_Printf("	ST_ON\n");
	else
		Uart_Printf("	ST_OFF\n");
#endif
}

unsigned char M1103_CheckHeat(unsigned char ucFlag)
{
	unsigned char ucHeatStatus;

	ucHeatStatus = M1103_TM;

#if 0
	Uart_Printf("M1103_CheckHeat-->\n");

	if( ucHeatStatus == 0 )
		Uart_Printf("	TM_OVERHEAT\n");
	else
		Uart_Printf("	TM_NORMAL\n");
#endif

	if( ucHeatStatus == 0 )
    	return(M1103_TM_OVERHEAT);
	else
		return(M1103_TM_NORMAL);
}

unsigned char M1103_PowerOn(void)
{
#if 0
	Uart_Printf("M1103_PowerOn-->\n");
#endif	
	M1103_Config_POWEN(M1103_POWEREN_ENABLE);
	M1103_Config_LIMIT(M1103_LIMIT_DISABLE);

	gucM1103PowerStatus 			= M1103_POWERSTATUS_POWON;
	gucM1103CarriageMotoStatus 	= M1103_CARRIAGEMOTO_OFF;
	gucM1103PaperMotoStatus		= M1103_PAPERMOTO_OFF;
	
	return HALPRNERR_SUCCESS;
}

unsigned char M1103_PowerOff(void)
{
#if 0
	Uart_Printf("M1103_PowerOff-->\n");
#endif		
	M1103_Config_POWEN(M1103_POWEREN_DISABLE);
	M1103_Config_LIMIT(M1103_LIMIT_ENABLE);
	M1103_Config_ST(M1103_ST_DISABLE);

	gucM1103PowerStatus 			= M1103_POWERSTATUS_POWOFF;
	gucM1103CarriageMotoStatus 	= M1103_CARRIAGEMOTO_OFF;
	gucM1103PaperMotoStatus		= M1103_PAPERMOTO_OFF;
	
	return HALPRNERR_SUCCESS;
}

unsigned char M1103_EnableStep(unsigned char ucTimerType)
{
#if 0
	Uart_Printf("M1103_EnableStep-->\n");
#endif
	switch(ucTimerType)
	{	
		case M1103TIMER_TYPE0:
			//Disable TIMER1
//			rINTMSK |= BIT_TIMER1;
			//EN: Stop Timer 0
			rTCON &= 0xFFFFFF0;
		    ClearPending(BIT_TIMER0);
			pM1103Func->ucM1103PrintFunc = M1103STEP_ACLASH;
			pM1103Func->uiLatchTime2 = M1103PRN_LASH*2;
			//EN: Reload Timer0 value
			rTCNTB0 = 50;				//100us
			//EN: Update Timer0
		    rTCON &= 0xFFFFFF0;
		    rTCON |= 0x0000002;
			//EN: Start Timer0, One-shot
			rTCON = 0x0000001 + (rTCON&0xFFFFFF0);
			rINTMSK &= ~BIT_TIMER0;
			break;

		case M1103TIMER_TYPE1:
			//Disable TIMER0
//			rINTMSK |= BIT_TIMER0;
			//EN: Stop Timer 1
			rTCON &= 0xFFFF0FF;
		    ClearPending(BIT_TIMER1);
		   
			//EN: Reload Timer1 value
			rTCNTB1 = 50;				//200us
			//EN: Update Timer1
		    rTCON &= 0xFFFF0FF;
		    rTCON |= 0x0000200;
			//EN: Start Timer1, One-shot
			rTCON = 0x0000100 + (rTCON&0xFFFF0FF);
			 rINTMSK &= ~BIT_TIMER1;
			break;
		
		default:
			break;
	}
	
#if 0
	if(ucTimerType == M1103TIMER_TYPE0)
		Uart_Printf("	TIMER0\n");
	else if(ucTimerType == M1103TIMER_TYPE1)
		Uart_Printf("	TIMER1\n");
	else
		Uart_Printf("	TIMER?\n");
#endif

	return(HALPRNERR_SUCCESS);
}

unsigned char M1103_WaitPrinterCool(void)
{
	unsigned short uiValue;
	unsigned char ucFlag,ucPrintFlag;
	unsigned int uiLowCount,uiHighCount,uiDispCounter;
	unsigned char ucDispCount,aucDispData[10];

#if M1103_DEBUG
	Uart_Printf("M1103_WaitPrinterCool-->\n");
#endif
	return M1103_WaitPrinterCoolHard();


	HALADC_Read(4,&uiValue);


	ucFlag = 0;
	uiLowCount = 0;
	uiHighCount = 0;
	ucPrintFlag = 0;
	ucDispCount = 0;
	uiDispCounter = 0;
	
	while(1)
	{
		HALADC_Read(5,&uiValue);
		
		uiValue = uiValue*330/1024;
//		uiValue = uiValue*251/200;
//		Uart_Printf( "E2053_WaitPrinterCool(): Uain4 = %04d , uiLowCount = %04d , uiHighCount = %04d \n",uiValue,uiLowCount,uiHighCount);
		if( ucFlag==0 )
		{
			if( uiValue<M1103_TEMPERATURE_LOW )
			{
				uiLowCount++;
				uiHighCount = 0;
				if( uiLowCount>10 )
					ucFlag = 1;
			}
			else
			{
				uiLowCount = 0;
				uiHighCount ++;
				if(uiHighCount>10)
					break;
			}
		}
		if( ucFlag==1 )
		{
			if( !ucPrintFlag )
			{
				ucPrintFlag = 1;	
				OSMMI_ClrDisplay(ASCIIFONT57,255);
				OSMMI_GB2312Display(ASCIIFONT57,2,0,(unsigned char *)"打印机过热请稍候");
			}
			if(!(uiDispCounter%5))
			{
				unsigned char ucI=0;

				if(ucDispCount==11)
					ucDispCount=0;
				memset(aucDispData,0,sizeof(aucDispData));
				for(ucI=0;ucI<ucDispCount;ucI++)
				{
					aucDispData[ucI]='.';
				}
				OSMMI_ClrDisplay(ASCIIFONT57,6);
				OSMMI_ClrDisplay(ASCIIFONT57,7);
				OSMMI_GB2312Display(ASCIIFONT57,3,0,aucDispData);
				ucDispCount++;				
//				Uart_Printf( "E2053_WaitPrinterCool(): U_TM = %04d	 (158,187)\n",uiValue);
			}
			if( uiValue>M1103_TEMPERATURE_HIGH )
				uiHighCount++;
			else
				uiHighCount = 0;
			if( uiHighCount>10 )
			{
				OSMMI_ClrDisplay(ASCIIFONT57,255);
				OSMMI_GB2312Display(ASCIIFONT57,2,0,(unsigned char *)"继续打印.....");
				break;	
			}
			uiDispCounter++;
			HALTIMER_Delay(1000);
		}
	}

	return(HALPRNERR_SUCCESS);
}

unsigned char M1103_WaitPrinterCoolHard(void)
{
	static unsigned char sucHardOverHeatFlag = M1103_TM_NORMAL;
	static unsigned char sucHardOverHeatStatus = M1103_TM_NORMAL;
	static unsigned int  suiHardNormalHeatCounter = 0;
	static unsigned char sucDispFlag = 0;
	static unsigned int  suiDispCounter = 0;
	static unsigned char sucDispCounter = 0;
	       unsigned char aucDispData[10];
#if 1
	while(1)
	{
		sucHardOverHeatStatus = M1103_CheckHeat(0);

		if(sucHardOverHeatFlag == M1103_TM_NORMAL)
		{
			if(sucHardOverHeatStatus == M1103_TM_NORMAL)
			{
				suiHardNormalHeatCounter = 0;
				sucDispFlag = 0;
				suiDispCounter = 0;
				sucDispCounter = 0;
				sucHardOverHeatFlag = M1103_TM_NORMAL;
				return(HALPRNERR_SUCCESS);
			}
			else if(sucHardOverHeatStatus == M1103_TM_OVERHEAT)
			{
				suiHardNormalHeatCounter = 0;
				sucDispFlag = 0;
				suiDispCounter = 0;
				sucDispCounter = 0;
				sucHardOverHeatFlag = M1103_TM_OVERHEAT;
			}
			else
			{
				sucHardOverHeatFlag = M1103_TM_NORMAL;
				sucHardOverHeatStatus = M1103_TM_NORMAL;
				suiHardNormalHeatCounter = 0;
				sucDispFlag = 0;
				suiDispCounter = 0;
				sucDispCounter = 0;
				return(HALPRNERR_SUCCESS);
			}
		}
		if(sucHardOverHeatFlag == M1103_TM_OVERHEAT)
		{
			if(sucHardOverHeatStatus == M1103_TM_NORMAL)
			{
				suiHardNormalHeatCounter++;
				if(suiHardNormalHeatCounter >= 10)
				{
					sucHardOverHeatFlag = M1103_TM_NORMAL;
					sucHardOverHeatStatus = M1103_TM_NORMAL;
					suiHardNormalHeatCounter = 0;
					sucDispFlag = 0;
					suiDispCounter = 0;
					sucDispCounter = 0;

					OSMMI_ClrDisplay(ASCIIFONT57,255);
					OSMMI_GB2312Display(ASCIIFONT57,2,0,(unsigned char *)"继续打印.....");
				}
			}
			else if(sucHardOverHeatStatus == M1103_TM_OVERHEAT)
			{
				if( !sucDispFlag )
				{
					sucDispFlag = 1;
					suiDispCounter = 0;
					sucDispCounter = 0;
					OSMMI_ClrDisplay(ASCIIFONT57,255);
					OSMMI_GB2312Display(ASCIIFONT57,2,0,(unsigned char *)"打印机过热请稍候");
					OSMMI_GB2312Display(ASCIIFONT57,3,0,(unsigned char *)"硬件保护中");
				}
				//if(!(suiDispCounter%200))	// PS400: 200Lines = 25MM
				if(!(suiDispCounter%10))	
				{
					unsigned char ucI=0;

					if(sucDispCounter==4)
						sucDispCounter=0;
					memset(aucDispData,0,sizeof(aucDispData));
					memset(aucDispData,' ',6);
					for(ucI=0;ucI<sucDispCounter;ucI++)
					{
						aucDispData[ucI]='.';
					}
					OSMMI_GB2312Display(ASCIIFONT57,3,5,aucDispData);
					sucDispCounter++;
				}
				suiDispCounter++;
			}
			else
			{
				sucHardOverHeatFlag = M1103_TM_NORMAL;
				sucHardOverHeatStatus = M1103_TM_NORMAL;
				suiHardNormalHeatCounter = 0;
				sucDispFlag = 0;
				suiDispCounter = 0;
				sucDispCounter = 0;
			}
		}
	}
#endif
	return(HALPRNERR_SUCCESS);
}

unsigned char M1103_PrintStrASCII57(char *pcStr)
{
    unsigned char ucI,ucJ;
    unsigned char ucLen;
    unsigned int uiOffset;
	unsigned char aucDotBuf[6];
#if 0	

   memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
   ucI = 0x01;
   ucI = ucI<<((*pcStr)-0x31);
   memset(pM1103Func->aucM1103LineDotBuf,ucI,sizeof(pM1103Func->aucM1103LineDotBuf));
    pM1103Func->uiLineDotBufLen = 100;
	M1103_SaveDot(16,0);

    return(M1103_RowPrint(16));
	
#endif
    ucLen = strlen(pcStr);
    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
    uiOffset = 0;
    for(ucI=0;ucI<ucLen;ucI++)
    {
        HALPRN_ReadCharASCII57PrintColDot(*(pcStr+ucI),aucDotBuf);
        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++,uiOffset++)
        {
			pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf[ucJ];
//			pM1103Func->aucM1103LineDotBuf[uiOffset] = 0x00;
        }
    }
    pM1103Func->uiLineDotBufLen = uiOffset;
	M1103_SaveDot(16,0);

    return(M1103_RowPrint(16));
}

unsigned char M1103_PrintStrASCII816(char *pcStr)
{
    unsigned char ucI,ucJ;
    unsigned char ucLen;
    unsigned int uiOffset;
	unsigned char aucDotBuf[8];

#if 0
	Uart_Printf("M1103_PrintStrASCII816-->\n");
#endif

    ucLen = strlen(pcStr);
    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
    uiOffset = 0;
    for(ucI=0;ucI<ucLen;ucI++)
    {
        HALPRN_ReadCharASCII816PrintColDot(*(pcStr+ucI),aucDotBuf,0);
        for(ucJ=0;(ucJ<8)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
        {
			pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf[ucJ];
			pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
        }
    }
	pM1103Func->uiLineDotBufLen = uiOffset;
	//M1103_SaveDot1(16,0);
      M1103_SaveDot(16,0);
    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
    uiOffset = 0;
    for(ucI=0;ucI<ucLen;ucI++)
    {
        HALPRN_ReadCharASCII816PrintColDot(*(pcStr+ucI),aucDotBuf,1);
        for(ucJ=0;(ucJ<8)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
        {
			pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf[ucJ];
 			pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
       }
    }
	pM1103Func->uiLineDotBufLen = uiOffset;
	M1103_SaveDot(16,0);

    return(M1103_RowPrint(16));
}

unsigned char M1103_PrintStrGB2312Dot8(unsigned char *pucStr)
{
    unsigned char ucI,ucJ;
    unsigned char ucLen;
    unsigned char ucLoop;
    unsigned int uiOffset;
    unsigned char ucASCIIFont;
    unsigned char ucGB2312Font;
    unsigned char ucCh;
    unsigned char *pucPtr;
    unsigned char *pucDotBufPtr1;
    unsigned char *pucDotBufPtr2;
	unsigned char ucChar1,ucChar2;
	unsigned char aucDotBuf[32];
	unsigned char aucEvenOldBuf[17];
	
#if 0
	Uart_Printf("M1103_PrintStrGB2312Dot8-->\n");
#endif

    pucPtr = pucStr;
    ucASCIIFont = *pucPtr++;
    ucGB2312Font = *pucPtr++;
    ucLen = strlen((char *)pucPtr);

    for(ucLoop=0;ucLoop<2;ucLoop++)
    {
	    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
	    uiOffset = 0;
	    for(ucI=0;ucI<ucLen;)
	    {
		    if( *(pucPtr+ucI) >= 0xA0)
		    {
		        if(*(pucPtr+ucI+1) >= 0xA0)
		        {
	                HALDOT_ReadDisplayGB2312Dot16( *(pucPtr+ucI),*(pucPtr+ucI+1),aucDotBuf);
		        }
				memset(aucEvenOldBuf,0,sizeof(aucEvenOldBuf));
				for(ucJ=0;ucJ<16;ucJ++)
				{
					HALDOT_EvenOldDot(aucDotBuf,ucJ,ucLoop,&ucCh);
					aucEvenOldBuf[ucJ] = ucCh;
				}
#if 0
				for(ucJ=1;ucJ<16;ucJ++)
				{
					aucEvenOldBuf[ucJ] = (~(aucEvenOldBuf[ucJ-1]&aucEvenOldBuf[ucJ]))&aucEvenOldBuf[ucJ];
				}					
#else
				for(ucJ=0;ucJ<16;ucJ++)
				{
					if(ucJ%2)
					{	
						aucEvenOldBuf[ucJ-1] = aucEvenOldBuf[ucJ-1]|aucEvenOldBuf[ucJ];
						aucEvenOldBuf[ucJ] = 0;
					}	
				}					
#endif
				for(ucJ=0;ucJ<16;ucJ++)
				{				
					if( uiOffset < M1103PRNLINE_DOTMAXNB )
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucEvenOldBuf[ucJ];
					}
				}	
	            ucI += 2;
		    }else
		    {
				if(!ucLoop)
				{
			        HALPRN_ReadCharASCII57PrintColDot(*(pucPtr+ucI),aucDotBuf);
			        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
			        {
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf[ucJ];
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
			        }
				}else
				{
			        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
			        {
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
			        }
				}		
		        ucI += 1;
		    }
	    }
		pM1103Func->uiLineDotBufLen = uiOffset;

	    if(ucLoop)
	    {
	    	M1103_SaveDot(20,1);
	    }else
	    {
	    //	M1103_SaveDot1(1,1);	
	    M1103_SaveDot(1,1);
	    }	
	}
    return(M1103_RowPrint(16));
}

unsigned char M1103_PrintStrGB2312Dot12(unsigned char *pucStr)
{
    unsigned char ucI,ucJ;
    unsigned char ucLen;
    unsigned char ucLoop;
    unsigned int uiOffset;
    unsigned char ucASCIIFont;
    unsigned char ucGB2312Font;
    unsigned char ucCh;
    unsigned char *pucPtr;
    unsigned char *pucDotBufPtr;
    unsigned char ucDot1;
    unsigned char ucDot2;
	unsigned char aucDotBuf1[32];
	unsigned char aucDotBuf2[32];

#if 0
	Uart_Printf("M1103_PrintStrGB2312Dot12-->\n");
#endif

    pucPtr = pucStr;
    ucASCIIFont = *pucPtr++;
    ucGB2312Font = *pucPtr++;
    ucLen = strlen((char *)pucPtr);

    for(ucLoop=0;ucLoop<2;ucLoop++)
    {
	    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
	    uiOffset = 0;
	    for(ucI=0;ucI<ucLen;)
	    {
		    if( *(pucPtr+ucI) >= 0xA0)
		    {
		        if(*(pucPtr+ucI+1) >= 0xA0)
		        {
		        	memset(aucDotBuf1,0,sizeof(aucDotBuf1));
		        	memset(aucDotBuf2,0,sizeof(aucDotBuf2));
	                HALDOT_ReadGB2312Dot12( *(pucPtr+ucI),*(pucPtr+ucI+1),aucDotBuf1);
	                for(ucJ=0,ucCh=0;ucJ<18;ucJ+=3)
	                {
						ucDot1 = aucDotBuf1[ucJ];
						aucDotBuf2[ucCh++] = ucDot1;
						ucDot1 = aucDotBuf1[ucJ+1];
						aucDotBuf2[ucCh++] = ucDot1 & 0xF0;
						ucDot2 = aucDotBuf1[ucJ+2];
						aucDotBuf2[ucCh++] = ((ucDot1 & 0x0F)<<4)|((ucDot2&0xF0)>>4);
						aucDotBuf2[ucCh++] = (ucDot2&0x0F)<<4;
						if( ucCh == 8)
						{
							ucCh += 8;
						}
	                }
	                memset(aucDotBuf1,0,sizeof(aucDotBuf1));
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ] = aucDotBuf2[ucJ*2];
	                }
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ+8] = aucDotBuf2[ucJ*2+1];
	                }
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ+16] = aucDotBuf2[ucJ*2+16];
	                }
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ+24] = aucDotBuf2[ucJ*2+16+1];
	                }
		        }
		        pucDotBufPtr = &aucDotBuf1[16*ucLoop];
				for(ucJ=0;ucJ<8;ucJ++)
				{
					HALDOT_RotateDot(pucDotBufPtr,ucJ,&ucCh);
					if( uiOffset < M1103PRNLINE_DOTMAXNB )
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = ucCh;
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
					}
				}
				pucDotBufPtr += 8;
				for(ucJ=0;ucJ<4;ucJ++)
				{
					HALDOT_RotateDot(pucDotBufPtr,ucJ,&ucCh);
					if( uiOffset < M1103PRNLINE_DOTMAXNB )
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = ucCh;
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
					}
				}
	            ucI += 2;
		    }else
		    {
		        if( ucLoop )
		        {
                	memset(aucDotBuf1,0,sizeof(aucDotBuf1));
    		        HALPRN_ReadCharASCII57PrintColDot(*(pucPtr+ucI),aucDotBuf1);
    		        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
    		        {
    					pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf1[ucJ];
   			//		pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf1[ucJ];
   					pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
    		        }
    		    }else
    		    {
    		        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
    		        {
    					pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
    					pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
    		        }
    		    }
		        ucI += 1;
		    }
	    }
		pM1103Func->uiLineDotBufLen = uiOffset;
	    if( ucLoop )
	    {
			M1103_SaveDot(16,0);
	    }else
	    {
			//M1103_SaveDot1(8,0);
			M1103_SaveDot(8,0);
		}
		
	}
    return(M1103_RowPrint(12));
}



unsigned char M1103_PrintStrGB2312Dot12_1(unsigned char *pucStr)
{
    unsigned char ucI,ucJ;
    unsigned char ucLen;
    unsigned char ucLoop;
    unsigned int uiOffset;
    unsigned char ucASCIIFont;
    unsigned char ucGB2312Font;
    unsigned char ucCh;
    unsigned char *pucPtr;
    unsigned char *pucDotBufPtr;
    unsigned char ucDot1;
    unsigned char ucDot2;
	unsigned char aucDotBuf1[32];
	unsigned char aucDotBuf2[32];

#if 0
	Uart_Printf("M1103_PrintStrGB2312Dot12-->\n");
#endif

    pucPtr = pucStr;
    ucASCIIFont = *pucPtr++;
    ucGB2312Font = *pucPtr++;
    ucLen = strlen((char *)pucPtr);

    for(ucLoop=0;ucLoop<2;ucLoop++)
    {
	    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
	    uiOffset = 0;
	    for(ucI=0;ucI<ucLen;)
	    {
		    if( *(pucPtr+ucI) >= 0xA0)
		    {
		        if(*(pucPtr+ucI+1) >= 0xA0)
		        {
		        	memset(aucDotBuf1,0,sizeof(aucDotBuf1));
		        	memset(aucDotBuf2,0,sizeof(aucDotBuf2));
	                HALDOT_ReadGB2312Dot12( *(pucPtr+ucI),*(pucPtr+ucI+1),aucDotBuf1);
	                for(ucJ=0,ucCh=0;ucJ<18;ucJ+=3)
	                {
						ucDot1 = aucDotBuf1[ucJ];
						aucDotBuf2[ucCh++] = ucDot1;
						ucDot1 = aucDotBuf1[ucJ+1];
						aucDotBuf2[ucCh++] = ucDot1 & 0xF0;
						ucDot2 = aucDotBuf1[ucJ+2];
						aucDotBuf2[ucCh++] = ((ucDot1 & 0x0F)<<4)|((ucDot2&0xF0)>>4);
						aucDotBuf2[ucCh++] = (ucDot2&0x0F)<<4;
						if( ucCh == 8)
						{
							ucCh += 8;
						}
	                }
	                memset(aucDotBuf1,0,sizeof(aucDotBuf1));
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ] = aucDotBuf2[ucJ*2];
	                }
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ+8] = aucDotBuf2[ucJ*2+1];
	                }
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ+16] = aucDotBuf2[ucJ*2+16];
	                }
	                for(ucJ=0;ucJ<8;ucJ++)
	                {
	                	aucDotBuf1[ucJ+24] = aucDotBuf2[ucJ*2+16+1];
	                }
		        }
		        pucDotBufPtr = &aucDotBuf1[16*ucLoop];
				for(ucJ=0;ucJ<8;ucJ++)
				{
					HALDOT_RotateDot(pucDotBufPtr,ucJ,&ucCh);
					if( uiOffset < M1103PRNLINE_DOTMAXNB )
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = ucCh;
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
					}
				}
				pucDotBufPtr += 8;
				for(ucJ=0;ucJ<4;ucJ++)
				{
					HALDOT_RotateDot(pucDotBufPtr,ucJ,&ucCh);
					if( uiOffset < M1103PRNLINE_DOTMAXNB )
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = ucCh;
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
					}
				}
	            ucI += 2;
		    }else
		    {
		        if( ucLoop )
		        {
                	memset(aucDotBuf1,0,sizeof(aucDotBuf1));
    		        HALPRN_ReadCharASCII57PrintColDot(*(pucPtr+ucI),aucDotBuf1);
    		        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
    		        {
    					pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf1[ucJ];
//   					pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf1[ucJ];
	   					pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
    		        }
    		    }else
    		    {
    		        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
    		        {
    					pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
    					pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
    		        }
    		    }
		        ucI += 1;
		    }
	    }
		pM1103Func->uiLineDotBufLen = uiOffset;
	    if( ucLoop )
	    {
			M1103_SaveDot(16,0);
	    }else
	    {
			//M1103_SaveDot1(8,0);
			M1103_SaveDot(8,0);
		}
	}
    return(M1103_RowPrint(12));
}

unsigned char M1103_PrintStrGB2312Dot16(unsigned char *pucStr)
{
    unsigned char ucI,ucJ;
    unsigned char ucLen;
    unsigned char ucLoop;
    unsigned int uiOffset;
    unsigned char ucASCIIFont;
    unsigned char ucGB2312Font;
    unsigned char ucCh;
    unsigned char *pucPtr;
    unsigned char *pucDotBufPtr;
	unsigned char aucDotBuf[32];

#if 0
	Uart_Printf("M1103_PrintStrGB2312Dot16-->\n");
#endif

    pucPtr = pucStr;
    ucASCIIFont = *pucPtr++;
    ucGB2312Font = *pucPtr++;
    ucLen = strlen((char *)pucPtr);

    for(ucLoop=0;ucLoop<2;ucLoop++)
    {
	    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
	    uiOffset = 0;
	    for(ucI=0;ucI<ucLen;)
	    {
		    if( *(pucPtr+ucI) >= 0xA0)
		    {
		        if(*(pucPtr+ucI+1) >= 0xA0)
		        {
	                HALDOT_ReadDisplayGB2312Dot16( *(pucPtr+ucI),*(pucPtr+ucI+1),aucDotBuf);
		        }
		        pucDotBufPtr = &aucDotBuf[16*ucLoop];
				for(ucJ=0;ucJ<8;ucJ++)
				{
					HALDOT_RotateDot(pucDotBufPtr,ucJ,&ucCh);
					if( uiOffset < M1103PRNLINE_DOTMAXNB )
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = ucCh;
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
					}
				}
				pucDotBufPtr += 8;
				for(ucJ=0;ucJ<8;ucJ++)
				{
					HALDOT_RotateDot(pucDotBufPtr,ucJ,&ucCh);
					if( uiOffset < M1103PRNLINE_DOTMAXNB )
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = ucCh;
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
					}
				}
	            ucI += 2;
		    }else
		    {
	    	    switch( ucASCIIFont )
	    	    {
	            case ASCIIFONT816:
					HALPRN_ReadCharASCII816PrintColDot(*(pucPtr+ucI),aucDotBuf,ucLoop);
					for(ucJ=0;(ucJ<8)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
					{
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf[ucJ];
						pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
					}
	                break;
	            case ASCIIFONT57:
	            default:
	                if( ucLoop )
	                {
				        HALPRN_ReadCharASCII57PrintColDot(*(pucPtr+ucI),aucDotBuf);
				        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
				        {
							pM1103Func->aucM1103LineDotBuf[uiOffset++] = aucDotBuf[ucJ];
							pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
				        }
	                }else
	                {
				        for(ucJ=0;(ucJ<6)&&(uiOffset<M1103PRNLINE_DOTMAXNB);ucJ++)
				        {
							pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
							pM1103Func->aucM1103LineDotBuf[uiOffset++] = 0x00;
				        }
	                }
	                break;
	            }
		        ucI += 1;
		    }
	    }
		pM1103Func->uiLineDotBufLen = uiOffset;
	   if(ucLoop)
	        M1103_SaveDot(16,0);
	   else
	   	M1103_SaveDot(16,0);
	   	// M1103_SaveDot1(16,0);
	}
    return(M1103_RowPrint(16));
}

unsigned char M1103_PrintGraph(unsigned char *pucStr,unsigned short uiLen)
{
#if 0
	Uart_Printf("M1103_PrintGraph-->\n");
#endif

	M1103_WaitPrinterCool();
	
    memset(pM1103Func->aucM1103LineDotBuf,0,sizeof(pM1103Func->aucM1103LineDotBuf));
    if( uiLen > M1103PRNLINE_DOTMAXNB )
    	uiLen = M1103PRNLINE_DOTMAXNB;

    memcpy(pM1103Func->aucM1103LineDotBuf,pucStr,uiLen);
	pM1103Func->uiLineDotBufLen = uiLen;
	
	M1103_SaveDot(16,0);

    return(M1103_RowPrint(1));
}

unsigned char M1103_PrintStrGB2312(unsigned char *pucStr)
{
	unsigned char ucGBFont;

#if 0
	Uart_Printf("M1103_PrintStrGB2312-->\n");
#endif

	M1103_WaitPrinterCool();
	M1103_TrunckBlankSpace(pucStr+1);
	if((*(pucStr+2)) == 0x00)
	{
	      (*(pucStr+2)) = ' ';
		 (*(pucStr+3)) = 0x00;
	}
	ucGBFont = *(pucStr+1);
	
	switch(ucGBFont)
	{
	case GBFONT_SMALL:
		return(M1103_PrintStrGB2312Dot12(pucStr));
	case GBFONT_COMPRESS:
		return(M1103_PrintStrGB2312Dot8(pucStr));
	case GBFONT_DEFAULT:
	default:
    	return(M1103_PrintStrGB2312Dot16(pucStr));
    }
}

unsigned char M1103_PrintStrASCII(unsigned char *pucStr)
{
#if 0
	Uart_Printf("M1103_PrintStrASCII-->\n");
#endif

	M1103_WaitPrinterCool();
	M1103_TrunckBlankSpace(pucStr+1);
	if((*(pucStr+1)) == 0x00)
	{
	      (*(pucStr+1)) = ' ';
		 (*(pucStr+2)) = 0x00;
	}
    switch( *pucStr )
    {
    case ASCIIFONT816:
        return(M1103_PrintStrASCII816((char *)(pucStr+1)));
    case ASCIIFONT57:
    default:
        return(M1103_PrintStrASCII57((char *)(pucStr+1)));
	}
}

unsigned char M1103_PrintMultiLang(unsigned char *pucStr)
{
	/*
	***************************************************
	Wait to add
	***************************************************
	*/
	return(HALPRNERR_SUCCESS);
}

void M1103_SaveDot(unsigned char ucStepNB,unsigned char ucXor)
{
#if 0
	Uart_Printf("M1103_SaveDot-->\n");
#endif

       //     Uart_Printf("\n*%d,%d,%d",pM1103Func->ucM1103PrintNB,pM1103Func->ucM1103PrintedNB,ucStepNB);

	while( pM1103Func->ucM1103PrintBufFull );
    //  Uart_Printf("\n**%d,%d,%d",pM1103Func->ucM1103PrintNB,pM1103Func->ucM1103PrintedNB,ucStepNB);
	memcpy(&pM1103Func->aucM1103DotBuf[pM1103Func->ucM1103PrintNB][0],
		   &pM1103Func->aucM1103LineDotBuf[0],
		   M1103PRNLINE_DOTMAXNB);
   	pM1103Func->auiM1103DotBufLen[pM1103Func->ucM1103PrintNB] = pM1103Func->uiLineDotBufLen;
   	pM1103Func->auiM1103StepNB[pM1103Func->ucM1103PrintNB] = ucStepNB;
   	pM1103Func->ucM1103PrintNB ++;
   	if( pM1103Func->ucM1103PrintNB >= M1103PRN_BUFROWNB )
   		pM1103Func->ucM1103PrintNB = 0;
   	if( pM1103Func->ucM1103PrintNB == pM1103Func->ucM1103PrintedNB )
   		pM1103Func->ucM1103PrintBufFull = 1;
   	pM1103Func->ucM1103PrintBufNotEmpty = 1;
}

void M1103_SaveDot1(unsigned char ucStepNB,unsigned char ucXor)
{
#if 0
	Uart_Printf("M1103_SaveDot-->\n");
#endif

       //     Uart_Printf("\n*%d,%d,%d",pM1103Func->ucM1103PrintNB,pM1103Func->ucM1103PrintedNB,ucStepNB);

	while( pM1103Func->ucM1103PrintBufFull );
    //  Uart_Printf("\n**%d,%d,%d",pM1103Func->ucM1103PrintNB,pM1103Func->ucM1103PrintedNB,ucStepNB);
       pM1103Func->aucM1103DotBuf[pM1103Func->ucM1103PrintNB][0] = 0;
	memcpy(&pM1103Func->aucM1103DotBuf[pM1103Func->ucM1103PrintNB][1],
		   &pM1103Func->aucM1103LineDotBuf[0],
		   M1103PRNLINE_DOTMAXNB-1);
   	pM1103Func->auiM1103DotBufLen[pM1103Func->ucM1103PrintNB] = pM1103Func->uiLineDotBufLen;
   	pM1103Func->auiM1103StepNB[pM1103Func->ucM1103PrintNB] = ucStepNB;
   	pM1103Func->ucM1103PrintNB ++;
   	if( pM1103Func->ucM1103PrintNB >= M1103PRN_BUFROWNB )
   		pM1103Func->ucM1103PrintNB = 0;
   	if( pM1103Func->ucM1103PrintNB == pM1103Func->ucM1103PrintedNB )
   		pM1103Func->ucM1103PrintBufFull = 1;
   	pM1103Func->ucM1103PrintBufNotEmpty = 1;
}


void M1103_StartCarriageMotor(unsigned char ucFlag)
{
	
#if 0
	Uart_Printf("M1103_StartCarriageMotor(%d)-->\n",ucFlag);
#endif
	if(ucFlag)
	{
		pM1103Func->ucM1103PrintData |= 0x05;
		
	}else
	{
		pM1103Func->ucM1103PrintData = (pM1103Func->ucM1103PrintData&0xFA)|0x01;
	}	
	M1103PRNMOTPORT = pM1103Func->ucM1103PrintData;
	pM1103Func->ucM1103PrintEnd |= M1103STATUS_PRINTING;
	pM1103Func->ucM1103Status |= M1103STATUS_PRINTING;

	gucM1103CarriageMotoStatus = M1103_CARRIAGEMOTO_ON;
}

void M1103_StartPaperMotor(unsigned char ucFlag)
{

	if(ucFlag)
		pM1103Func->ucM1103PrintData |= 0x50;
	else
		pM1103Func->ucM1103PrintData =(pM1103Func->ucM1103PrintData&0xAF)|0x10;
	M1103PRNMOTPORT = pM1103Func->ucM1103PrintData;
	pM1103Func->ucM1103Status |= M1103STATUS_FEEDING;
	gucM1103PaperMotoStatus = M1103_PAPERMOTO_ON;
}

void M1103_StopCarriageMotor(void)
{
#if 0
	Uart_Printf("M1103_StopCarriageMotor-->\n");
#endif

	pM1103Func->ucM1103PrintData &= 0xF0;	/*完全关闭*/
	M1103PRNMOTPORT = pM1103Func->ucM1103PrintData;
	pM1103Func->ucM1103Status &= ~M1103STATUS_PRINTING;
	pM1103Func->ucM1103PrintEnd &= ~M1103STATUS_PRINTING;
	gucM1103CarriageMotoStatus = M1103_CARRIAGEMOTO_OFF;
}

void M1103_StoptPaperMotor(void)
{
#if 0
	Uart_Printf("M1103_StoptPaperMotor-->\n");
#endif

	pM1103Func->ucM1103PrintData &= 0x0F;
	M1103PRNMOTPORT = pM1103Func->ucM1103PrintData;
	pM1103Func->ucM1103Status &= ~M1103STATUS_FEEDING;

	gucM1103PaperMotoStatus = M1103_PAPERMOTO_OFF;
}

void M1103_Carriage_SetPhase(void)
{
	unsigned char ucI,ucChar;
	unsigned char aucPhaseTbale[]={0x00,0x02,0x0A,0x08,0x00};

	ucChar = pM1103Func->ucM1103PrintData&0x0A;

	
	
	if(pM1103Func->ucM1103PrintMove == M1103LSTEP_RTOL)	/*从右向左*/
	{
		for(ucI=0;ucI<4;ucI++)
		{
			if(ucChar == aucPhaseTbale[ucI])
			{
				ucChar = aucPhaseTbale[ucI+1];
				break;
			}		
		}	
	}else if(pM1103Func->ucM1103PrintMove == M1103LSTEP_LTOR) /*从左向右*/
	{
		for(ucI=4;ucI>0;ucI--)
		{
			if(ucChar == aucPhaseTbale[ucI])
			{
				ucChar = aucPhaseTbale[ucI-1];
				break;
			}		
		}	
	}
	pM1103Func->ucM1103PrintData = (pM1103Func->ucM1103PrintData&0xF5)|ucChar;
	M1103PRNMOTPORT = pM1103Func->ucM1103PrintData;

}

void M1103_Paper_SetPhase(void)
{
	unsigned char ucI,ucChar;
	unsigned char aucPhaseTbale[]={0x00,0x20,0xA0,0x80,0x00};
	ucChar = pM1103Func->ucM1103PrintData&0xA0;

	
	if(pM1103Func->ucM1103PaperMove == M1103LSTEP_BACK)	/*向后走纸*/
	{
		for(ucI=0;ucI<4;ucI++)
		{
			if(ucChar == aucPhaseTbale[ucI])
			{
				ucChar = aucPhaseTbale[ucI+1];
				break;
			}		
		}	
	}else if(pM1103Func->ucM1103PaperMove == M1103LSTEP_FORW) /*向前走纸*/
	{
		for(ucI=4;ucI>0;ucI--)
		{
			if(ucChar == aucPhaseTbale[ucI])
			{
				ucChar = aucPhaseTbale[ucI-1];
				break;
			}		
		}	
	}
	pM1103Func->ucM1103PrintData = (pM1103Func->ucM1103PrintData&0x5F)|ucChar;
	M1103PRNMOTPORT = pM1103Func->ucM1103PrintData;
}		

void M1103_InitPhaseSwitch(unsigned char ucMotoType,unsigned char ucSteps)
{
	unsigned  short uiTime;
	unsigned char ucACIndex,ucDCIndex,ucI;

	if(ucMotoType == M1103LSTEP_MOTOAC)
		ucACIndex =0;
	
	if(ucMotoType == M1103LSTEP_MOTODC)
		ucDCIndex =0;
	
	for(ucI=0;ucI<ucSteps;ucI++)
	{
		switch(ucMotoType)
		{
		
		case M1103LSTEP_MOTOEN:
			uiTime = M1103PRN_INITEVENTSTEP;
			break;
		case M1103LSTEP_MOTOAC:
			uiTime = M1103_INIT_AC_Timer[ucACIndex++];
//			Uart_Printf("\nMOTOAC %d",uiTime);
			if(ucACIndex>=10)//sizeof(INIT_AC_Timer)/sizeof(unsigned short))
				ucACIndex =0;
			break;
		case M1103LSTEP_MOTODC:
			uiTime = M1103_INIT_DC_Timer[ucDCIndex++];
//			Uart_Printf("\nMOTODC %d",uiTime);
			if(ucDCIndex>=10)//sizeof(INIT_DC_Timer)/sizeof(unsigned short))
				ucDCIndex =0;
			break;
		}
		M1103_Carriage_SetPhase();
//		HALTIMER_StartUSTimer(uiTime);
//		HALTIMER_WaitUSTimerEnd();
		HALTIMER_usDelay(uiTime);
	}
	return ;
}

unsigned char M1103_CheckHPLevel(void)
{
	unsigned char ucLevel1,ucLevel2;
	unsigned char ucNums,ucI;

	
	ucNums =10;
	
	for(ucI=0;ucI<ucNums;ucI++)
	{
		ucLevel1=M1103_HOME;
//		HALTIMER_StartUSTimer(15);
//		HALTIMER_WaitUSTimerEnd();
		HALTIMER_usDelay(15);
		ucLevel2=M1103_HOME;
		
		if(ucLevel1==ucLevel2)
			break;
		else
		{
//			HALTIMER_StartUSTimer(15);
//			HALTIMER_WaitUSTimerEnd();
			HALTIMER_usDelay(15);
			if(M1103_DEBUG)
				Uart_Printf("\nCheck Level error");
		}
	}
	return ucLevel1;
}
	
unsigned char M1103_TestHome(void)
{

	unsigned char ucI,ucJ;
	unsigned char ucLevel;

	
#if 1
	M1103_StoptPaperMotor();
//	M1103_StartPaperMotor(0);
	M1103_StartCarriageMotor(0);
	HALTIMER_Delay(50);
	M1103_StartCarriageMotor(1);
	HALTIMER_Delay(50);
	
	ucLevel = M1103_CheckHPLevel();
	
	
	if(!ucLevel)
	{
		pM1103Func->ucM1103PrintMove = M1103LSTEP_LTOR;	/*从左向右*/
//		Uart_Printf("\n>=========>");
              M1103_InitPhaseSwitch(M1103LSTEP_MOTOAC,10);
		for(ucI=0;ucI<400;ucI++)	
		{
			M1103_InitPhaseSwitch(M1103LSTEP_MOTOEN,1);
			ucLevel = M1103_CheckHPLevel();
			if(ucLevel)
			{
				M1103_InitPhaseSwitch(M1103LSTEP_MOTOEN,22);
				break;
			}
		}
		M1103_InitPhaseSwitch(M1103LSTEP_MOTODC,10);
		if(ucI>=400)
			return HALPRNERR_NOSYNC;
	}
	
	pM1103Func->ucM1103PrintMove = M1103LSTEP_RTOL;
//	Uart_Printf("\n<==========<");
	M1103_InitPhaseSwitch(M1103LSTEP_MOTOAC,10);
	M1103_InitPhaseSwitch(M1103LSTEP_MOTOEN,32);
	M1103_InitPhaseSwitch(M1103LSTEP_MOTODC,10);

	ucLevel = M1103_CheckHPLevel();
//	Uart_Printf("\n Level222 %d",ucLevel);
	if(ucLevel)
		return HALPRNERR_NOSYNC;
	
	
	pM1103Func->ucM1103PrintMove = M1103LSTEP_LTOR;
//	Uart_Printf("\n>=========>");
	M1103_InitPhaseSwitch(M1103LSTEP_MOTOAC,10);
	for(ucI=0;ucI<50;ucI++)
	{
		M1103_InitPhaseSwitch(M1103LSTEP_MOTOEN,1);
		ucLevel = M1103_CheckHPLevel();
		if(ucLevel)
		{
//			Uart_Printf("\n ### Level %d ucI %d",ucLevel,ucI);
			break;
		}
	}
	if(ucI>=50)
		return HALPRNERR_NOSYNC;
	while(1)
	{
	     if((pM1103Func->ucM1103PrintData&0x0A) == 0x0A)
		  break;
	     M1103_InitPhaseSwitch(M1103LSTEP_MOTOEN,1);
	}
//	M1103_PrintDot(0xFF);
	//Uart_Printf("\n1 pM1103Func->ucM1103PrintData&0x0A=%02x", pM1103Func->ucM1103PrintData&0x0A);

	M1103_InitPhaseSwitch(M1103LSTEP_MOTOEN,12);
	M1103_InitPhaseSwitch(M1103LSTEP_MOTODC,10);
	M1103_StopCarriageMotor();

#endif
	pM1103Func->ucM1103PrintMove = M1103LSTEP_RTOL;
	pM1103Func->ucM1103StepCount = 0;
	pM1103Func->ucM1103PrintFunc = M1103LSTEP_MOTOAC;

	
	return(HALPRNERR_SUCCESS);

}


unsigned char M1103_RowPrint(unsigned char ucRowNum)
{
	unsigned char ucI,ucResult;
#if 0
	Uart_Printf("M1103_RowPrint-->\n");
	Uart_Printf("	pM1103Func->ucM1103PrintBufNotEmpty = %02x\n",pM1103Func->ucM1103PrintBufNotEmpty);
	Uart_Printf("	pM1103Func->ucM1103Status = %02x\n",pM1103Func->ucM1103Status);
	Uart_Printf("	pM1103Func->ucM1103PrintEnd = %02x\n",pM1103Func->ucM1103PrintEnd);
#endif
//	Uart_Printf("\nRow Print");
	M1103_Func.uiCheckTimeout = 0;
	if( pM1103Func->ucM1103PrintBufNotEmpty )
	{
		if( pM1103Func->ucM1103Status&M1103STATUS_PRINTING )
		{
			
			return(HALPRNERR_SUCCESS);
		}

//yangjun		while( pM1103Func->ucM1103Status );

		if(gucM1103PowerStatus != M1103_POWERSTATUS_POWON)
		{
			M1103_PowerOn();
		}
		
		if(1)
		{
                    if(!ucM1103FirstMaskFlag)
			  {
		             ucM1103FirstMaskFlag = 1;
				ucSuspendFlag = 1;
			     	pM1103Func->uiINTMSK = rINTMSK;
			     	rINTMSK = BIT_ALLMSK;//rINTMSK = BIT_ALLMSK &~BIT_IIC;  
				pM1103Func->uiINTSUBMSK = rINTSUBMSK;
			     	rINTSUBMSK = BIT_SUB_ALLMSK;
				pM1103Func->uiEINTMASK = rEINTMASK;
			     	rEINTMASK = BIT_ALLEINTMASK;
			  }

			for(ucI=0;ucI<5;ucI++)
			{
				ucResult=M1103_TestHome();
				if(!ucResult)
					break;
				else
				{
					Uart_Printf("\nTest Home Restart");
					M1103_PowerOff();
					M1103_PowerOn();
				}
			}
			if(ucI>=5)
			{
				M1103_StopCarriageMotor();
				return(HALPRNERR_MOTORSTART);
			}

			
			pM1103Func->ucM1103StepCount = 0;
			pM1103Func->ucM1103PrintFunc = M1103STEP_ACLASH;//M1103LSTEP_MOTOAC;
			pM1103Func->ucM1103PrintMove = M1103LSTEP_RTOL;
			pM1103Func->uiM1103PrintFactNB = M1103PRNLINE_DOTMAXNB+2;	
			pM1103Func->ucM1103PrintEnd |= M1103STATUS_PRINTENDING;

		 
			pM1103Func->uiM1103PaperCount = 24;
			pM1103Func->ucM1103PaperMove = M1103LSTEP_FORW;
			pM1103Func->ucM1103PaperMode = M1103STEP_PAPERACLASH;
			M1103_StartPaperMotor(0);
			M1103_EnableStep(M1103TIMER_TYPE1);		
			while(pM1103Func->uiM1103PaperCount);
			 rINTMSK |= BIT_TIMER2;
        }
	
	//	M1103_StartCarriageMotor(0);
		//HALTIMER_Delay(50);
		M1103_EnableStep(M1103TIMER_TYPE0);
		
	}
	return(HALPRNERR_SUCCESS);
}

unsigned char M1103_Feed(unsigned short uiRowNB)
{
	unsigned char ucM1103PrintedNB;

	if( !uiRowNB )
		return(HALPRNERR_SUCCESS);
	
	  M1103_Func.uiCheckTimeout = 0;
	//Uart_Printf("\n***%d,%d,%d,%d",pM1103Func->ucM1103PrintedNB,pM1103Func->ucM1103PrintNB,uiRowNB,pM1103Func->uiM1103PaperCount);
	while(pM1103Func->ucM1103PrintBufFull);
      if(pM1103Func->ucM1103PrintedNB != pM1103Func->ucM1103PrintNB)
      {
             if(pM1103Func->ucM1103PrintNB)
		 	ucM1103PrintedNB = pM1103Func->ucM1103PrintNB-1;
		 else
		 	ucM1103PrintedNB = M1103PRN_BUFROWNB -1;
		
		pM1103Func->auiM1103StepNB[ucM1103PrintedNB] += uiRowNB*2;
		
		return HALPRNERR_SUCCESS;
      }else if(pM1103Func->uiM1103PaperCount)
      	{
      	       pM1103Func->uiM1103PaperCount +=  uiRowNB*2;
		return HALPRNERR_SUCCESS;
      	}
	
	while(pM1103Func->uiM1103PaperCount);

	if(gucM1103PowerStatus != M1103_POWERSTATUS_POWON)
	{
		M1103_PowerOn();
	}
#if 0
	pM1103Func->uiM1103PaperedCount = 0;
	pM1103Func->uiM1103PaperCount = uiRowNB*2;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_FORW;
	if(pM1103Func->uiM1103PaperCount>16)	
		pM1103Func->ucM1103PaperFunc = M1103LSTEP_MOTOAC;	
	else
		pM1103Func->ucM1103PaperFunc = M1103LSTEP_CONST;
	pM1103Func->ucM1103PaperMode = M1103LSTEP_POWERON;
#else
	
	pM1103Func->uiM1103PaperCount = uiRowNB*2;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_FORW;
	pM1103Func->ucM1103PaperMode = M1103STEP_PAPERACLASH;
#endif
//	HALGSM_Suspend();
//      OSMODEM_Suspend();
//	pM1103Func->uiMask = rINTMSK;
//	rINTMSK = BIT_ALLMSK & ~(BIT_TICK | BIT_TIMER4 |BIT_IIC);
	//	rINTMSK = BIT_ALLMSK &~BIT_IIC;
	if(!ucM1103FirstMaskFlag)
      {
          ucM1103FirstMaskFlag = 1;
              ucSuspendFlag = 1;
		  pM1103Func->uiINTMSK = rINTMSK;
		     	rINTMSK = BIT_ALLMSK;//rINTMSK = BIT_ALLMSK &~BIT_IIC;  
			pM1103Func->uiINTSUBMSK = rINTSUBMSK;
		     	rINTSUBMSK = BIT_SUB_ALLMSK;
			pM1103Func->uiEINTMASK = rEINTMASK;
		     	rEINTMASK = BIT_ALLEINTMASK;
      }

	M1103_StartPaperMotor(0);
	M1103_EnableStep(M1103TIMER_TYPE1);	
	while(pM1103Func->uiM1103PaperCount);
	M1103_Func.uiCheckTimeout = M1103PRN_CHECKENDTIMEOUT;
	return(HALPRNERR_SUCCESS);
}

unsigned char M1103_ReverseFeed(unsigned short uiRowNB)
{
	
#if M1103_DEBUG
	Uart_Printf("M1103_ReverseFeed-->\n");
	Uart_Printf("	pM1103Func->ucM1103Status = %02x\n",pM1103Func->ucM1103Status);
#endif

	if( !uiRowNB )
		return(HALPRNERR_SUCCESS);
	
	M1103_Func.uiCheckTimeout = 0;
	while(pM1103Func->ucM1103PrintBufNotEmpty);

	if(gucM1103PowerStatus != M1103_POWERSTATUS_POWON)
	{
		M1103_PowerOn();
	}
#if 0
	pM1103Func->uiM1103PaperedCount = 0;
	pM1103Func->uiM1103PaperCount = uiRowNB*2;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_BACK;
	if(pM1103Func->uiM1103PaperCount>16)	
		pM1103Func->ucM1103PaperFunc = M1103LSTEP_MOTOAC;	
	else
		pM1103Func->ucM1103PaperFunc = M1103LSTEP_CONST;
	pM1103Func->ucM1103PaperMode = M1103LSTEP_POWERON;
#else
	pM1103Func->uiM1103PaperCount = uiRowNB*2+6;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_BACK;
	pM1103Func->ucM1103PaperMode = M1103STEP_PAPERACLASH;
#endif
//	HALGSM_Suspend();
//       OSMODEM_Suspend();
	if(!ucM1103FirstMaskFlag)
      {
          ucM1103FirstMaskFlag = 1;
	   ucSuspendFlag = 1;
          pM1103Func->uiINTMSK = rINTMSK;
		     	rINTMSK = BIT_ALLMSK;//rINTMSK = BIT_ALLMSK &~BIT_IIC;  
			pM1103Func->uiINTSUBMSK = rINTSUBMSK;
		     	rINTSUBMSK = BIT_SUB_ALLMSK;
			pM1103Func->uiEINTMASK = rEINTMASK;
		     	rEINTMASK = BIT_ALLEINTMASK;
      }	
	
	M1103_StartPaperMotor(0);
	
	M1103_EnableStep(M1103TIMER_TYPE1);	
//	Uart_Printf("RowNB %d.\n",uiRowNB);
	while(pM1103Func->uiM1103PaperCount);
//	Uart_Printf("111111.\n");
#if 1
		//pM1103Func->uiMask = rINTMSK;
//		rINTMSK = BIT_ALLMSK & ~(BIT_TICK | BIT_TIMER4 |BIT_IIC);
		//rINTMSK = BIT_ALLMSK &~BIT_IIC;
		

	pM1103Func->uiM1103PaperCount = 7;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_FORW;
	pM1103Func->ucM1103PaperMode = M1103STEP_PAPERACLASH;
	M1103_StartPaperMotor(0);
	M1103_EnableStep(M1103TIMER_TYPE1);		
	while(pM1103Func->uiM1103PaperCount);
	
#endif

	M1103_Func.uiCheckTimeout = M1103PRN_CHECKENDTIMEOUT;

//	M1103_EnableStep(0);

	return(HALPRNERR_SUCCESS);
}



unsigned char M1103_CheckStatus(unsigned char ucFlag)
{
     if(gucM1103PowerStatus == M1103_POWERSTATUS_POWON)
          return(HALPRNERR_BUSY);
     else
	   return(HALPRNERR_SUCCESS);

}

unsigned char M1103_Mark(unsigned char ucFlag)
{
    // Uart_Printf("\ncheck paper %02x",gucM1103PowerStatus);
     if(gucM1103PowerStatus == M1103_POWERSTATUS_POWON)
          return(HALPRNERR_BUSY);
     else
	   return(HALPRNERR_SUCCESS);
#if 0
	while(pM1103Func->ucM1103Status);

	pM1103Func->uiM1103PaperCount = 200*8;
	pM1103Func->ucM1103MarkExistFlag = 0;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_FORW;
	pM1103Func->ucM1103PaperFunc = M1103LSTEP_MOTOAC;	
	pM1103Func->ucM1103PaperMode = M1103LSTEP_CHECKMARK;
	M1103_StartPaperMotor(1);
	M1103_EnableStep(M1103TIMER_TYPE1);		

	while(!pM1103Func->ucM1103MarkExistFlag);

	if(pM1103Func->ucM1103MarkExistFlag == 0xFF)
	{
		return(HALPRNERR_SUCCESS);
	}else
	{	
		return(HALPRNERR_NOMARK);	
	}
#endif
}

unsigned char M1103_CheckPaper(unsigned char ucFlag)
{

      unsigned char ucI;

#if M1103_DEBUG
	Uart_Printf("M1103_CheckPaper-->\n");
#endif	     
      if(gucM1103PowerStatus != M1103_POWERSTATUS_POWON)
	{
		M1103_Config_POWEN(M1103_POWEREN_ENABLE);
		ucI = M1103_PAPER;
		HALTIMER_Delay(1);
		ucI = M1103_PAPER;
		M1103_Config_POWEN(M1103_POWEREN_DISABLE);
		if(ucI)
		   return(HALPRNERR_SUCCESS);
	      return(HALPRNERR_NOPRINTER);
	}else
	{
	     if(M1103_PAPER)
		return(HALPRNERR_SUCCESS);
	     return(HALPRNERR_NOPRINTER);
	}

	
}

void M1103_PrintDot(unsigned char ucDot)
{
	unsigned char ucI;

	ucPrintDot = ucDot & ~ucPrintDot;
	M1103PRNDOTPORT = ucPrintDot;
	M1103_Config_ST(M1103_ST_ENABLE);
	M1103_Config_ST(M1103_ST_DISABLE);


	/*ucPrintDot = ucDot & ~ucPrintDot;
	if(ucPrintDot&0x80)
	{
	       rGPCDAT = rGPCDAT |(0x01<<9);
	}else
	{
	       rGPCDAT = rGPCDAT &(~(0x01<<9));
	}
	if(ucPrintDot&0x40)
	{
	       rGPCDAT = rGPCDAT |(0x01<<8);
	}else
	{
	       rGPCDAT = rGPCDAT &(~(0x01<<8));
	}
      if(ucPrintDot&0x20)
	{
	       rGPEDAT = rGPEDAT |(0x01<<10);
	}else
	{
	       rGPEDAT = rGPEDAT &(~(0x01<<10));
	}
	if(ucPrintDot&0x10)
	{
	       rGPEDAT = rGPEDAT |(0x01<<9);
	}else
	{
	       rGPEDAT = rGPEDAT &(~(0x01<<9));
	}
	if(ucPrintDot&0x08)
	{
	       rGPEDAT = rGPEDAT |(0x01<<8);
	}else
	{
	       rGPEDAT = rGPEDAT &(~(0x01<<8));
	}	if(ucPrintDot&0x04)
	{
	       rGPEDAT = rGPEDAT |(0x01<<7);
	}else
	{
	       rGPEDAT = rGPEDAT &(~(0x01<<7));
	}
       if(ucPrintDot&0x02)
	{
	       rGPEDAT = rGPEDAT |(0x01<<6);
	}else
	{
	       rGPEDAT = rGPEDAT &(~(0x01<<6));
	}
	if(ucPrintDot&0x01)
	{
	       rGPEDAT = rGPEDAT |(0x01<<5);
	}else
	{
	       rGPEDAT = rGPEDAT &(~(0x01<<5));
	}
//	HALTIMER_Delay(1);
	M1103_Config_ST(M1103_ST_ENABLE);
	
	M1103_Config_ST(M1103_ST_DISABLE);*/
}

unsigned char M1103_Test(void)
{
	unsigned short uiI;
#if 0
      M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57(( char *)"1");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57((char *)"2");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57(( char *)"3");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57((char *)"4");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57(( char *)"5");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57(( char *)"6");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57(( char *)"7");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57(( char *)"8");
#endif
#if 1
//	M1103_PrintStrASCII816("   0.01   0.01   0.01");
//	M1103_WaitPrinterCool();
//	M1103_PrintStrGB2312Dot16((unsigned char *)"00杉德公司打印测试ABC");
	/*M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试11");
	M1103_ReverseFeed(32);
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试22");
	M1103_ReverseFeed(48);
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试33");
	M1103_Feed(32);
	return 0;*/


    //   for(uiI=0;uiI<50;uiI++)
	//   	M1103_PrintStrGB2312Dot12((unsigned char *)"001");


    

	M1103_PrintStrGB2312Dot12((unsigned char *)"00abc");
	
	M1103_PrintStrGB2312Dot12((unsigned char *)"00abcdefghijklmnopqrstu");
	
	M1103_PrintStrGB2312Dot12((unsigned char *)"00abcdefghijklmnopq");
	
	M1103_PrintStrGB2312Dot12((unsigned char *)"00abcdefghijklmnopqrstuvwx");
	
	M1103_PrintStrGB2312Dot12((unsigned char *)"00abcdefghijklmnopqr");
	
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	
	M1103_PrintStrGB2312Dot12((unsigned char *)"00系统打印机自检测试");
	
	M1103_PrintStrGB2312Dot16((unsigned char *)"00系统打印机自检测试");

	M1103_PrintStrGB2312Dot16((unsigned char *)"00系统打印机自检测试");
	M1103_PrintStrASCII57("12345678901234");
	M1103_PrintStrASCII57("abcdefghijklmnopqr");
	M1103_PrintStrASCII57("123456");
	M1103_PrintStrASCII57("abc");
	M1103_PrintStrASCII816("ABCDEFGHIJKLMNOPQRSTUV");
	M1103_PrintStrASCII816("1234567890123456789012");
	M1103_PrintStrASCII57("HHHHHHHHHHHHHHHHHHHH");
	M1103_PrintStrASCII57("HHHHHHHHHHHHHHHHHHHHH");
	M1103_PrintStrASCII57("HHHHHHH");
	M1103_Feed(32);

#else
	unsigned char ucI;
	unsigned char aucPrintBuf[30];

	
	M1103_WaitPrinterCool();

	M1103_PrintStrGB2312Dot16((unsigned char *)"00杉德公司打印测试ABC");


	M1103_PrintStrGB2312Dot12((unsigned char *)"00杉德公司打印测试ABC");
  	M1103_PrintStrASCII57("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHH");
	M1103_PrintStrASCII57("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHH");
	M1103_PrintStrGB2312Dot12((unsigned char *)"00杉德公司打印测试ABC");
	
	M1103_PrintStrASCII816("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	M1103_PrintStrASCII816("   0.01   0.01   0.01");
	M1103_PrintStrASCII57("abcdefghijklmnopqrstuvwxyz");
	for(ucI=0;ucI<10;ucI++)
	{
		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,ucI+'0',20);
		M1103_PrintStrASCII57((char *)aucPrintBuf);
	}
	
	M1103_Feed(40);

	while(M1103_Mark(0));

#endif
	

	return(HALPRNERR_SUCCESS);
}

unsigned char M1103_TestPrintProtect(unsigned char ucFlag)
{
	unsigned short uiI;

#if M1103_DEBUG
	Uart_Printf("M1103_TestPrintProtect-->\n");
#endif

	if(!ucFlag)
	{
		M1103_WaitPrinterCool();
	}
	else
	{
		M1103_WaitPrinterCoolHard();
	}
#if 0
	memset(pM1103Func->aucM1103LineDotBuf,0xFF,sizeof(pM1103Func->aucM1103LineDotBuf));
	pM1103Func->uiLineDotBufLen = M1103PRNLINE_DOTMAXNB;
	M1103_SaveDot(16,0);
    M1103_RowPrint(1);
#else
	M1103_PrintStrASCII57("HHHHHHHHHHHHHHHHHHHH");
	M1103_PrintStrASCII57("HHHHHHHHHHHHHHHHHHHHH");
	M1103_PrintStrASCII57("HHHHHHHHHHHHHHHHHHHHH");
#endif
	

	return(HALPRNERR_SUCCESS);
}

unsigned char M1103_PaperPrint(void)
{

//	M1103_StartCarriageMotor(0);
	pM1103Func->uiM1103PaperCount = pM1103Func->auiM1103StepNB[pM1103Func->ucM1103PrintedNB];
	
		pM1103Func->ucM1103PrintedNB++;
	if(pM1103Func->ucM1103PrintedNB >= M1103PRN_BUFROWNB)
		pM1103Func->ucM1103PrintedNB = 0;
#if 1
	if(pM1103Func->ucM1103PrintedNB == pM1103Func->ucM1103PrintNB)
	{	
		pM1103Func->ucM1103PrintEnd &= ~M1103STATUS_PRINTING;
//yangjun		pM1103Func->ucM1103PrintBufNotEmpty = 0;
	}
#endif
	M1103_StartPaperMotor(1);
	pM1103Func->ucM1103PaperFunc = M1103LSTEP_MOTOEN;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_FORW;
	pM1103Func->ucM1103PaperMode = M1103LSTEP_MOTOMOVE;
	M1103_EnableStep(M1103TIMER_TYPE1);		
}

unsigned char  M1103_Print_Proc(void)
{
	unsigned short uiPrintLine;
	unsigned short uiPrintingLine;
	unsigned char ucPrintedNB;
	unsigned char ucDot,bPrintFlag=FALSE;

	ucPrintedNB = pM1103Func->ucM1103PrintedNB;
	uiPrintLine = pM1103Func->auiM1103DotBufLen[ucPrintedNB];
	
	switch(pM1103Func->ucM1103PrintMove)
	{
	case M1103LSTEP_RTOL:
		if(M1103_DEBUG)
			Uart_Printf("\n<-------Print Dot %d",pM1103Func->uiM1103PrintStepNB);
		
		if((pM1103Func->uiM1103PrintFactNB-pM1103Func->uiM1103PrintStepNB) < uiPrintLine)
		{
			ucDot = pM1103Func->aucM1103DotBuf[pM1103Func->ucM1103PrintedNB][pM1103Func->uiM1103PrintFactNB-pM1103Func->uiM1103PrintStepNB];
			M1103_PrintDot(ucDot);
			bPrintFlag = TRUE;
		}else
		{
		      M1103_PrintDot(0x00);
		}
		pM1103Func->uiM1103PrintStepNB++;
		if( pM1103Func->uiM1103PrintFactNB-pM1103Func->uiM1103PrintStepNB ==8)
		{
			M1103_CalcLatchHoldTime();
			M1103_EnableStep(M1103TIMER_TYPE1);
		}
		if(pM1103Func->uiM1103PrintStepNB > pM1103Func->uiM1103PrintFactNB)
		{
			if(M1103_DEBUG)
			{
				Uart_Printf("\nChange Directory ");
				Uart_Printf("\nFact %d  Len %d",pM1103Func->uiM1103PrintFactNB,uiPrintLine);
			}
			pM1103Func->uiM1103PrintStepNB = 0;
			pM1103Func->ucM1103PrintFunc = M1103LSTEP_MOTODC;
			pM1103Func->uiM1103PrintFactNB = M1103PRNLINE_DOTMAXNB+2;
			
		}
		break;
	case M1103LSTEP_LTOR:
		if(M1103_DEBUG)
			Uart_Printf("\n------->Print Dot %d   PrintLine %d",pM1103Func->uiM1103PrintStepNB,uiPrintLine);
		
		if(pM1103Func->uiM1103PrintStepNB<uiPrintLine)
		{
		       if(pM1103Func->uiM1103PrintStepNB>=2)
		       {
		             ucDot = pM1103Func->aucM1103DotBuf[pM1103Func->ucM1103PrintedNB][pM1103Func->uiM1103PrintStepNB-2];
			      M1103_PrintDot(ucDot);
		       }else
			{
			      M1103_PrintDot(0x00);
			}
			bPrintFlag = TRUE;
		}	
		pM1103Func->uiM1103PrintStepNB++;
		if(pM1103Func->uiM1103PrintStepNB ==1)
		{
			if(++ucPrintedNB >= M1103PRN_BUFROWNB)
				ucPrintedNB = 0;
			if(ucPrintedNB != pM1103Func->ucM1103PrintNB)
			{
				uiPrintingLine = pM1103Func->auiM1103DotBufLen[ucPrintedNB];
					
				if(uiPrintLine<uiPrintingLine)
					pM1103Func->uiM1103PrintFactNB = uiPrintingLine;
				else
					pM1103Func->uiM1103PrintFactNB = uiPrintLine;
			}
			else 
				pM1103Func->uiM1103PrintFactNB = uiPrintLine;
			switch(pM1103Func->uiM1103PrintFactNB%8)
			{
			       case 0:
				  	pM1103Func->uiM1103PrintFactNB += 2;
				  	break;
				 case 1:
				  	pM1103Func->uiM1103PrintFactNB += 1;
				  	break;
				 case 2:
				  	pM1103Func->uiM1103PrintFactNB += 0;
				  	break;
				case 3:
				  	pM1103Func->uiM1103PrintFactNB += 7;
				  	break;
				case 4:
				  	pM1103Func->uiM1103PrintFactNB += 6;
				  	break;
				case 5:
				  	pM1103Func->uiM1103PrintFactNB += 5;
				  	break;
				case 6:
				  	pM1103Func->uiM1103PrintFactNB += 4;
				  	break;
			       case 7:
				  	pM1103Func->uiM1103PrintFactNB += 3;
				  	break;
				default:
					break;
			}
			if(pM1103Func->uiM1103PrintFactNB>(M1103PRNLINE_DOTMAXNB+2))
				pM1103Func->uiM1103PrintFactNB -= 8;
		}

		if(pM1103Func->uiM1103PrintFactNB-pM1103Func->uiM1103PrintStepNB ==8)
		{
			M1103_CalcLatchHoldTime();
			M1103_EnableStep(M1103TIMER_TYPE1);
		}
		
		if(pM1103Func->uiM1103PrintStepNB > pM1103Func->uiM1103PrintFactNB)
		{
			if(M1103_DEBUG)
			{
				Uart_Printf("\nChange Directory ");
				Uart_Printf("\n -------> Fact %d  Len %d",pM1103Func->uiM1103PrintFactNB,uiPrintLine);
			}
			pM1103Func->uiM1103PrintStepNB = 0;
			pM1103Func->ucM1103PrintFunc = M1103LSTEP_MOTODC;
		}	
		break;
	default:
		Uart_Printf("\nMove Error");
		break;
	}
	return bPrintFlag;
}	

void __irq M1103_Timer0Interrupt(void)
{
	unsigned char ucI;
	static unsigned short uiDelta;
	static unsigned char  ucDotIndex,ucFlag;

   
          ClearPending(BIT_TIMER0);

	rINTMSK |= BIT_TIMER2;
	
	switch(pM1103Func->ucM1103PrintMove)	
	{
		case M1103LSTEP_RTOL:
			switch(pM1103Func->ucM1103PrintFunc)
			{
				case M1103LSTEP_MOTOAC:
					M1103_Carriage_SetPhase();
					if(pM1103Func->ucM1103StepCount<StepArrayNums(M1103_RL_AC_Timer))
					{
						if(M1103_DEBUG)
							Uart_Printf("\nACStep Count %d  %d",pM1103Func->ucM1103StepCount,M1103_RL_AC_Timer[pM1103Func->ucM1103StepCount]);
						rTCNTB0 = M1103_RL_AC_Timer[pM1103Func->ucM1103StepCount];
						pM1103Func->ucM1103StepCount++;
						if(pM1103Func->ucM1103StepCount>=StepArrayNums(M1103_RL_AC_Timer))
						{
							pM1103Func->ucM1103StepCount = 0;
							pM1103Func->uiM1103PrintStepNB = 0;
							pM1103Func->ucM1103PrintFunc = M1103LSTEP_MOTOEN;
							pM1103Func->ucENStatus = M1103STEP_INITPHASE;
							ucPrintDot = 0x00;
						}	
					}
					break;
				case M1103LSTEP_MOTODC:
					if(M1103_DEBUG)
						Uart_Printf("\nDCStep Count %d  %d",pM1103Func->ucM1103StepCount,M1103_RL_DC_Timer[pM1103Func->ucM1103StepCount]);
					
					if(pM1103Func->ucM1103StepCount<StepArrayNums(M1103_RL_DC_Timer))
					{
						M1103_Carriage_SetPhase();
						rTCNTB0 = M1103_RL_DC_Timer[pM1103Func->ucM1103StepCount];
						pM1103Func->ucM1103StepCount++;
					}
					else 
					{
							pM1103Func->ucM1103StepCount = 0;
							pM1103Func->ucM1103PrintMove = M1103LSTEP_LTOR;
							pM1103Func->ucM1103PrintFunc = M1103STEP_DCLASH;
							rTCNTB0 = pM1103Func->uiLatchTime1;
					}		
					
					break;
				case M1103LSTEP_MOTOEN:
					switch(pM1103Func->ucENStatus)
					{
						case M1103STEP_INITPHASE:
							if(M1103_DEBUG)
								Uart_Printf("\nPHASE change");

							M1103_Carriage_SetPhase();
//							if(pM1103Func->uiM1103PrintStepNB==0)
							{
								rTCNTB0 =375-100;
								pM1103Func->ucENStatus =M1103STEP_DOT;
								ucDotIndex=M1103DOT1;
								
							}							
							break;
						case M1103STEP_DOT:
							if(ucDotIndex != M1103DOT3)
							{
							     M1103_Print_Proc();
							    // uii2++;
							}
							if(ucDotIndex==M1103DOT1)
							{
								if(pM1103Func->ucM1103PrintFunc==M1103LSTEP_MOTOEN)
								{
								       rTCNTB0 = M1103PRN_EVENTSTEP;
								      ucDotIndex=M1103DOT2;
								}else
								{
								       rTCNTB0 = M1103PRN_EVENTSTEP+138;
								     ucDotIndex=M1103DOT2;
									// Uart_Printf("\n fuck3");
								}
							}
							else   if(ucDotIndex==M1103DOT2)
							{
							       rTCNTB0 = 138;
								ucDotIndex=M1103DOT3;
							//	if(pM1103Func->ucM1103PrintFunc!=M1103LSTEP_MOTOEN)
								//	Uart_Printf("\n fuck 1");
							}else
							{
								if(pM1103Func->ucM1103PrintFunc==M1103LSTEP_MOTOEN)
								{
									M1103_Carriage_SetPhase();
									rTCNTB0 =375-100;
									ucDotIndex=M1103DOT1;
								}else
								{ 
								   ; //  Uart_Printf("\n fuck2");
								}
							}
							break;
						
					}
					break;
				case M1103STEP_ACLASH:
					M1103_ACLash();
					break;
				case M1103STEP_DCLASH:
					M1103_DCLash();
					break;
				default:
					rTCNTB0 = 0;
					Uart_Printf("\ndefault RTOL");
					break;			
			}
			break;
		case M1103LSTEP_LTOR:
			switch(pM1103Func->ucM1103PrintFunc)
			{
				case M1103LSTEP_MOTOAC:
					if(M1103_DEBUG)
						Uart_Printf("\nACStep Count %d  %d",pM1103Func->ucM1103StepCount,M1103_LR_AC_Timer[pM1103Func->ucM1103StepCount]);
					M1103_Carriage_SetPhase();
					if(pM1103Func->ucM1103StepCount<StepArrayNums(M1103_LR_AC_Timer))
					{
						rTCNTB0 = M1103_LR_AC_Timer[pM1103Func->ucM1103StepCount];
						pM1103Func->ucM1103StepCount++;
						if(pM1103Func->ucM1103StepCount>=StepArrayNums(M1103_LR_AC_Timer))
						{
							pM1103Func->ucM1103StepCount = 0;
							pM1103Func->uiM1103PrintStepNB = 0;
							pM1103Func->ucM1103PrintFunc = M1103LSTEP_MOTOEN;
							pM1103Func->ucENStatus = M1103STEP_INITPHASE;
							pM1103Func->uiM1103PrintFactNB = pM1103Func->auiM1103DotBufLen[pM1103Func->ucM1103PrintedNB];
							ucPrintDot = 0x00;
						}	
					}
					break;
				case M1103LSTEP_MOTODC:
					if(M1103_DEBUG)
						Uart_Printf("\nDCStep Count %d   %d",pM1103Func->ucM1103StepCount,M1103_LR_DC_Timer[pM1103Func->ucM1103StepCount]);
					
					if(pM1103Func->ucM1103StepCount<StepArrayNums(M1103_LR_DC_Timer))
					{
						M1103_Carriage_SetPhase();
						rTCNTB0 = M1103_LR_DC_Timer[pM1103Func->ucM1103StepCount];
						pM1103Func->ucM1103StepCount++;
					}
					else
					{
							rTCNTB0 = 0;
							pM1103Func->ucM1103StepCount = 0;
							pM1103Func->ucM1103PrintMove = M1103LSTEP_RTOL;
							pM1103Func->ucM1103PrintFunc = M1103STEP_DCLASH;
							rTCNTB0 = pM1103Func->uiLatchTime1;
					}	
					
					break;
				case M1103LSTEP_MOTOEN:
					switch(pM1103Func->ucENStatus)
					{
						case M1103STEP_INITPHASE:
							if(M1103_DEBUG)
								Uart_Printf("\nPHASE change");
							M1103_Carriage_SetPhase();
//							if(pM1103Func->uiM1103PrintStepNB==0)
							{
								rTCNTB0 =347;
								ucFlag = FALSE;
								ucDotIndex=M1103DOT1;
								pM1103Func->ucENStatus =M1103STEP_DOT;
							}
							
						
							break;
						case M1103STEP_DOT:
						/*	if(ucFlag == TRUE)
							{
								M1103_Print_Proc();
								if(ucDotIndex==M1103DOT1)
								{
									if(pM1103Func->ucM1103PrintFunc==M1103LSTEP_MOTOEN)
									{
										rTCNTB0 = M1103PRN_EVENTSTEP;
										ucDotIndex=M1103DOT2;
									}
									else
									{
										rTCNTB0 = 479;
									}
								}
								else
								{
									
										HALTIMER_usDelay(66);
										M1103_Carriage_SetPhase();
										rTCNTB0 =347;
										ucDotIndex=M1103DOT1;
								}
								
							}
							else
							{
								ucFlag = TRUE;
								rTCNTB0 = M1103PRN_EVENTSTEP;
								ucDotIndex=M1103DOT2;
							}*/
							if(ucFlag)
							{
							     if(ucDotIndex != M1103DOT3)
								{
								     M1103_Print_Proc();
								 //    uii2++;
								}
							}
							ucFlag = TRUE;
							if(ucDotIndex==M1103DOT1)
							{
								if(pM1103Func->ucM1103PrintFunc==M1103LSTEP_MOTOEN)
								{
								       rTCNTB0 = M1103PRN_EVENTSTEP;
								      ucDotIndex=M1103DOT2;
								}else
								{
								       rTCNTB0 = M1103PRN_EVENTSTEP+66;
								     ucDotIndex=M1103DOT2;
									// Uart_Printf("\n fuck6");
								}
							}
							else   if(ucDotIndex==M1103DOT2)
							{
							       rTCNTB0 = 66;
								ucDotIndex=M1103DOT3;
							//	if(pM1103Func->ucM1103PrintFunc!=M1103LSTEP_MOTOEN)
								//	Uart_Printf("\n fuck 4");
							}else
							{
								if(pM1103Func->ucM1103PrintFunc==M1103LSTEP_MOTOEN)
								{
									M1103_Carriage_SetPhase();
									rTCNTB0 = 347;
									ucDotIndex=M1103DOT1;
								}else
								{ 
								    ;//  Uart_Printf("\n fuck5");
								}
							}
							break;
						
					}
					break;
				case M1103STEP_ACLASH:
					M1103_ACLash();
					break;
				case M1103STEP_DCLASH:
					M1103_DCLash();
				
					break;
				default:
					rTCNTB0 = 0;
					Uart_Printf("\ndefault LTOR");
					break;			
			}
			break;
		
		default:
			rTCNTB0 = 0;
			break;			
	}	
	if( rTCNTB0 )
	{	//EN: Update and Start Timer0
		
	    rTCON &= 0xFFFFFF0;
	    rTCON |= 0x0000002;
	  	rTCON = 0x0000001 + (rTCON&0xFFFFFF0);
		rINTMSK &= ~BIT_TIMER0;
	}else
	{
		rINTMSK |= BIT_TIMER0;
		pM1103Func->ucM1103Status &= ~M1103STATUS_PRINTING;
		if(!(pM1103Func->ucM1103Status&(M1103STATUS_FEEDING|M1103STATUS_PRINTING)))
		{
		     M1103_Func.uiCheckTimeout = M1103PRN_CHECKENDTIMEOUT;
		     rTCNTB2 = 1000;
		     rTCON &= 0xFFF0FFF;
	            rTCON |= 0x0002000;
	    	     rTCON = 0x0001000 + (rTCON&0xFFF0FFF);
			//EN: Update and Start Timer2
			rINTMSK &= ~BIT_TIMER2;
		}
		
	}
}

void __irq M1103_Timer1Interrupt(void)
{
	
	ClearPending(BIT_TIMER1);

//	Uart_Printf("\n <<");


	rINTMSK |= BIT_TIMER2;

	if(M1103_DEBUG)
		Uart_Printf("\n %d %d %d",pM1103Func->ucM1103PaperMode,pM1103Func->uiM1103PaperCount,pM1103Func->uiM1103PaperedCount);
	switch(pM1103Func->ucM1103PaperMode)
	{
		case M1103LSTEP_POWERON:
//			Uart_Printf("\nPOWERON %d",pM1103Func->uiM1103PaperedCount);
			if(pM1103Func->uiM1103PaperedCount<2)
			{
				
				pM1103Func->uiM1103PaperedCount++;
				if(pM1103Func->uiM1103PaperedCount == 2)
				{
					M1103_StartPaperMotor(1);
					pM1103Func->ucM1103PaperMode = M1103LSTEP_MOTOMOVE;
					pM1103Func->uiM1103PaperedCount = 0;
				}
				rTCNTB1 = M1103PRN_PAPERMOTOPOWER;
				
			}	
			break;
		case M1103LSTEP_CHECKMARK:
			if( !(M1103_MARK) )
			{
				pM1103Func->ucM1103MarkExistFlag = 0xFF;
				pM1103Func->uiM1103PaperedCount = 0;
				pM1103Func->uiM1103PaperCount = 0;
				M1103_StoptPaperMotor();
				rTCNTB1 = 0;
				break; 	
			}	
		case M1103STEP_PAPERACLASH:
			if(M1103_DEBUG)
				Uart_Printf("\n PAPER ACLASH");
			M1103_StartPaperMotor(1);
			rTCNTB1 = M1103PRN_LASH;
//			Uart_Printf("\n AAAA stepnb  %d",pM1103Func->auiM1103StepNB[pM1103Func->ucM1103PrintedNB]);
			if(pM1103Func->uiM1103PaperCount >=16)
//			if(pM1103Func->auiM1103StepNB[pM1103Func->ucM1103PrintedNB]>=16)
				pM1103Func->ucM1103PaperFunc= M1103LSTEP_MOTOAC;
			else
			{
				pM1103Func->ucM1103PaperFunc = M1103LSTEP_CONST;
				if(pM1103Func->uiM1103PaperCount)
					pM1103Func->uiM1103PaperCount--;
			}
			pM1103Func->ucM1103PaperMode = M1103LSTEP_MOTOMOVE;
			break;
		case M1103STEP_PAPERDCLASH:
			if(M1103_DEBUG)
				Uart_Printf("\n PAPER DCLASH");
			M1103_StartPaperMotor(0);
			pM1103Func->uiM1103PaperedCount = 0;
			pM1103Func->uiM1103PaperCount = 0;	
			rTCNTB1 = 0;
			break;
		case M1103LSTEP_MOTOMOVE:
			if(pM1103Func->uiM1103PaperedCount >=pM1103Func->uiM1103PaperCount)
			{
				rTCNTB1 = M1103PRN_LASH;
				pM1103Func->ucM1103PaperMode= M1103STEP_PAPERDCLASH;
			}else
			{
				M1103_Paper_SetPhase();
				switch(pM1103Func->ucM1103PaperFunc)
				{
					case M1103LSTEP_MOTOEN:
//						Uart_Printf("\nMOTOEN %d",pM1103Func->uiM1103PaperedCount);
						rTCNTB1 = M1103PRN_PAPACEVENTSTEP;
						if(pM1103Func->uiM1103PaperCount>=16)
						{
							if(pM1103Func->uiM1103PaperCount-pM1103Func->uiM1103PaperedCount<=3)
								pM1103Func->ucM1103PaperFunc = M1103LSTEP_MOTODC;
						}
						break;
					case M1103LSTEP_MOTOAC:
//						Uart_Printf("\nMOTOAC %d",PAP_AC_Timer[pM1103Func->uiM1103PaperedCount]);
						if(pM1103Func->uiM1103PaperedCount<14)
							rTCNTB1 = M1103_PAP_AC_Timer[pM1103Func->uiM1103PaperedCount];
						else 
						{
						
							if(pM1103Func->uiM1103PaperCount-pM1103Func->uiM1103PaperedCount<=2)
							{
								pM1103Func->ucM1103PaperFunc= M1103LSTEP_MOTODC;
								rTCNTB1 = M1103_PAP_DC_Timer[2+pM1103Func->uiM1103PaperedCount-pM1103Func->uiM1103PaperCount];
							//	Uart_Printf("\n<2");
							}else if((pM1103Func->uiM1103PaperCount-pM1103Func->uiM1103PaperedCount)==3)
							{
							       rTCNTB1 = M1103PRN_PAPACEVENTSTEP;	
							}
							else
							{
								pM1103Func->ucM1103PaperFunc= M1103LSTEP_MOTOEN;
								rTCNTB1 = M1103PRN_PAPACEVENTSTEP;	
							}

						}
						break;
					case M1103LSTEP_MOTODC:
						rTCNTB1 = M1103_PAP_DC_Timer[2+pM1103Func->uiM1103PaperedCount-pM1103Func->uiM1103PaperCount];
//						Uart_Printf("\nMOTODC %d",rTCNTB1);
						break;
					
					case M1103LSTEP_CONST:
						rTCNTB1 = M1103PRN_PAPEVENTSTEP;
						break;
					default:
						rTCNTB1 = 0;
						break;	
				}
				pM1103Func->uiM1103PaperedCount++;
			}
			break;
		default:
			rTCNTB1 = 0;
			break;	
	}
	
	if( rTCNTB1 )
	{	
        rTCON &= 0xFFFF0FF;
        rTCON |= 0x0000200;
    	rTCON = 0x0000100 + (rTCON&0xFFFF0FF);
		//EN: Update and Start Timer1
		rINTMSK &= ~BIT_TIMER1;
	}else
	{
	    rINTMSK |= BIT_TIMER1;
		pM1103Func->ucM1103Status &= ~M1103STATUS_FEEDING;

		if(!(pM1103Func->ucM1103Status&(M1103STATUS_FEEDING|M1103STATUS_PRINTING)))
		{
		 /*   M1103_Func.uiCheckTimeout = M1103PRN_CHECKENDTIMEOUT;
		    ucM1103DoFlag = 1;
		    rINTMSK &=~BIT_TIMER4;*/
		     M1103_Func.uiCheckTimeout = M1103PRN_CHECKENDTIMEOUT;
		     rTCNTB2 = 1000;
		     rTCON &= 0xFFF0FFF;
	            rTCON |= 0x0002000;
	    	     rTCON = 0x0001000 + (rTCON&0xFFF0FFF);
			//EN: Update and Start Timer2
			rINTMSK &= ~BIT_TIMER2;
		}

	}
}

unsigned char M1103_Print_ConvChar(unsigned char ucCh)
{
	unsigned char ucChar;
	ucChar = 0;
	ucChar |= (ucCh&0x80)>>7;
	ucChar |= (ucCh&0x40)>>5;
	ucChar |= (ucCh&0x20)>>3;
	ucChar |= (ucCh&0x10)>>1;
	ucChar |= (ucCh&0x08)<<1;
	ucChar |= (ucCh&0x04)<<3;
	ucChar |= (ucCh&0x02)<<5;
	ucChar |= (ucCh&0x01)<<7;
	return(ucChar);
}	

void M1103_ACLash(void)
{
	if(M1103_DEBUG)
		Uart_Printf("\n AC LASH");

	if(pM1103Func->uiLatchTime2)
	{
		rTCNTB0 = pM1103Func->uiLatchTime2;
		M1103_StartCarriageMotor(1);
	}
	else
		if(pM1103Func->ucM1103PrintBufNotEmpty)
			rTCNTB0 =50+M1103PRN_LASH;
		else
			rTCNTB0 = 0;
		
	pM1103Func->ucM1103PrintFunc = M1103LSTEP_MOTOAC;
}

void M1103_DCLash(void)
{
	if(M1103_DEBUG)
		Uart_Printf("\n DC LASH");
	
	if(pM1103Func->uiHoldTime)
	{
		M1103_StartCarriageMotor(0);
		if(pM1103Func->uiHoldTime>=0xffff)
		{
			rTCNTB0 = 0xffff;
			pM1103Func->uiHoldTime -= 0xffff;
		}
		else
		{
			pM1103Func->ucM1103PrintedNB++;
			pM1103Func->ucM1103PrintBufFull = 0;
			if(pM1103Func->ucM1103PrintedNB >= M1103PRN_BUFROWNB)
				pM1103Func->ucM1103PrintedNB = 0;
			if(pM1103Func->ucM1103PrintedNB == pM1103Func->ucM1103PrintNB)	
			{
//yangjun				pM1103Func->ucM1103PrintBufNotEmpty = 0;
				M1103_StartCarriageMotor(0);
				rTCNTB0 = 0;
				return;
			}
			rTCNTB0 = pM1103Func->uiHoldTime;
			pM1103Func->ucM1103PrintFunc = M1103STEP_ACLASH;
		}
		
	}
	else
	{
		pM1103Func->ucM1103PrintedNB++;
		pM1103Func->ucM1103PrintBufFull = 0;
		if(pM1103Func->ucM1103PrintedNB >= M1103PRN_BUFROWNB)
			pM1103Func->ucM1103PrintedNB = 0;
		if(pM1103Func->ucM1103PrintedNB == pM1103Func->ucM1103PrintNB)	
		{
//yangjun			pM1103Func->ucM1103PrintBufNotEmpty = 0;
			M1103_StartCarriageMotor(0);
			rTCNTB0 = 0;
			return;
		}
		rTCNTB0 =50;
		pM1103Func->ucM1103PrintFunc = M1103STEP_ACLASH;
	}
}


void M1103_CalcLatchHoldTime(void)
{
	unsigned char ucM1103PrintedNB,bEndPrint;
	unsigned int  uiTPF,uiTCR1,uiTCR2,uiTDEF;
	unsigned short uiPaperNums;

#if 1
	bEndPrint = M1103_bPrintComplete();
#endif
	pM1103Func->uiHoldTime = pM1103Func->uiLatchTime1 = pM1103Func->uiLatchTime2 =0;
	if(!bEndPrint)
	{
		uiPaperNums =pM1103Func->auiM1103StepNB[pM1103Func->ucM1103PrintedNB];
		            
		if(uiPaperNums>=16)
		{
			uiTPF = M1103_CalcStepTime(M1103_PAP_AC_Timer,StepArrayNums(M1103_PAP_AC_Timer))
					+M1103_CalcStepTime(M1103_PAP_DC_Timer,StepArrayNums(M1103_PAP_DC_Timer))
					+ (uiPaperNums-16)*M1103PRN_PAPACEVENTSTEP+M1103PRN_LASH*2;
		}
		else
		{
			uiTPF = M1103PRN_LASH*2 + uiPaperNums * M1103PRN_PAPEVENTSTEP;
		}

		if(pM1103Func->ucM1103PrintMove == M1103LSTEP_RTOL)
		{
			uiTCR1 = M1103_CalcStepTime(M1103_RL_DC_Timer,StepArrayNums(M1103_RL_DC_Timer))+M1103PRN_EVENTSTEP*8;      //22530 + 3304;
			uiTCR2 = M1103_CalcStepTime(M1103_LR_AC_Timer,StepArrayNums(M1103_LR_AC_Timer));		//19152 ;
		}
		else
		{
			uiTCR1 = M1103_CalcStepTime(M1103_LR_DC_Timer,StepArrayNums(M1103_LR_DC_Timer))+M1103PRN_EVENTSTEP*8;
			uiTCR2 = M1103_CalcStepTime(M1103_RL_AC_Timer,StepArrayNums(M1103_RL_AC_Timer));
		}

		if(uiTPF > uiTCR1 + uiTCR2)
		{
			uiTDEF = uiTPF -(uiTCR1 + uiTCR2);
			if(uiTDEF < M1103PRN_LASH)
				pM1103Func->uiLatchTime1 =M1103PRN_LASH;
			else if(uiTDEF <M1103PRN_LASH*2)
				pM1103Func->uiLatchTime1 = uiTDEF;
			else
			{
				pM1103Func->uiLatchTime1 =pM1103Func->uiLatchTime2 =M1103PRN_LASH;
				pM1103Func->uiHoldTime = uiTDEF - M1103PRN_LASH*2;
			}
				
		}
		else
		{
			pM1103Func->uiLatchTime1 =M1103PRN_LASH;
		}
	}
	else
	{
		pM1103Func->uiLatchTime1 = pM1103Func->uiHoldTime =M1103PRN_LASH;
	}
	
                         	pM1103Func->ucM1103PaperMode = M1103STEP_PAPERACLASH;
	pM1103Func->ucM1103PaperFunc = M1103LSTEP_MOTOEN;
	pM1103Func->ucM1103PaperMove = M1103LSTEP_FORW;
	pM1103Func->uiM1103PaperedCount =0;
	pM1103Func->uiM1103PaperCount = pM1103Func->auiM1103StepNB[pM1103Func->ucM1103PrintedNB];

	if(M1103_DEBUG)
	{
		Uart_Printf("\nCount %d  %d  %d",pM1103Func->uiM1103PaperCount,pM1103Func->ucM1103PrintedNB,pM1103Func->ucM1103PrintNB);
		Uart_Printf("\nP %d %d %d %d",uiPaperNums,uiTPF,uiTCR1,uiTCR2);
		Uart_Printf("\nT %d %d %d",pM1103Func->uiLatchTime1,pM1103Func->uiHoldTime,pM1103Func->uiLatchTime2);
	}
}

void M1103_TrunckBlankSpace(unsigned char *pucStr)
{
	unsigned char ucLen,*p;

	ucLen = strlen((char *)pucStr);
	
	if(ucLen <=2)
		return ;
	
	p = pucStr +ucLen -1;
	
	pucStr += 2;
	
	while(p>pucStr)
	{
		if(*p == ' ')
		{
			*p = 0x00;
			p--;
		}
		else if ((*p == 0xA1) && (*(p-1) ==0xA1))
		{
			*p = 0x00;
			*(p-1) = 0x00;
			p -= 2;
			
		}
		else break;
	}
	return;
}

void M1103_CheckPrintEndProc(void)
{
	
	if(pM1103Func->ucM1103PrintNB==pM1103Func->ucM1103PrintedNB)
	{
		pM1103Func->ucM1103PrintBufNotEmpty = 0;//yangjun
		if(M1103_DEBUG)
			Uart_Printf("\nStop and Poweroff");
		M1103_StopCarriageMotor();
		M1103_StoptPaperMotor();
		M1103_PowerOff();
#if 0
		HALGSM_Resume();
		OSMODEM_Resume();
#else
		rINTMSK = pM1103Func->uiINTMSK;
             rINTSUBMSK = pM1103Func->uiINTSUBMSK;
		rEINTMASK = pM1103Func->uiEINTMASK;

             ucM1103FirstMaskFlag = 0;
		ucM1103EndFlag = 0;
		ucSuspendFlag = 0;
#endif
	}
//	else
//		M1103_EnableStep(M1103TIMER_TYPE0);
}
unsigned char M1103_bPrintComplete(void)
{
	unsigned char ucM1103PrintedNB;
	
	ucM1103PrintedNB = pM1103Func->ucM1103PrintedNB+1;
	if(ucM1103PrintedNB >= M1103PRN_BUFROWNB)
		ucM1103PrintedNB = 0;
	
	if(ucM1103PrintedNB == pM1103Func->ucM1103PrintNB)
		return TRUE;
	else
		return FALSE;
}
unsigned int M1103_CalcStepTime(unsigned short auiTimer[],unsigned char ucNum)
{
	unsigned char ucI;
	unsigned int  uiTimer =0;

	for(ucI =0;ucI<ucNum;ucI++)
		uiTimer += auiTimer[ucI];
	return uiTimer;
}


void __irq M1103_Timer2Interrupt(void)
{
	
	ClearPending(BIT_TIMER2);

  //    Uart_Printf("\n*** %d",M1103_Func.uiCheckTimeout);

	
	 if(M1103_Func.uiCheckTimeout)
	 {
	       M1103_Func.uiCheckTimeout --;
		if(!M1103_Func.uiCheckTimeout )
		{
		        if(pM1103Func->ucM1103PrintNB==pM1103Func->ucM1103PrintedNB)
			{
				pM1103Func->ucM1103PrintBufNotEmpty = 0;//yangjun
				if(M1103_DEBUG)
					Uart_Printf("\nStop and Poweroff");
				M1103_StopCarriageMotor();
				M1103_StoptPaperMotor();
				M1103_PowerOff();
				rINTMSK = pM1103Func->uiINTMSK;
		             rINTSUBMSK = pM1103Func->uiINTSUBMSK;
				rEINTMASK = pM1103Func->uiEINTMASK;
                             rINTMSK |= BIT_TIMER2;
		             ucM1103FirstMaskFlag = 0;
				ucSuspendFlag = 0;

		//		Uart_Printf("\n*********************************");

			//	iiiii = 0;
			//	uii2 = 0;
			//	uii1 = 0;
			/*	pM1103Func->ucM1103PrintedNB ++;
				
				pM1103Func->ucM1103PrintNB++;
				if(pM1103Func->ucM1103PrintedNB >= M1103PRN_BUFROWNB)
				pM1103Func->ucM1103PrintedNB = 0;
				if(pM1103Func->ucM1103PrintNB >= M1103PRN_BUFROWNB)
				pM1103Func->ucM1103PrintNB = 0;*/
			}
		}
	 }
	  rTCON &= 0xFFF0FFF;
        rTCON |= 0x0002000;
	 rTCON = 0x0001000 + (rTCON&0xFFF0FFF);

}


