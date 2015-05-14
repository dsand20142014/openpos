
#include "test.h"

#include "ostools.h"

//#define DBG

void _print11(char *fmt,...)
{
#ifdef DBG
    Uart_Printf(fmt);
    return ;
#else
    return ;
#endif
}


int tty_fd;
char *serialCom ="/dev/ttymxc1"; //1 for 420
#if 0
unsigned char PBOC_ISOSelectFilebyAID(unsigned char ucReader,
            unsigned char *pucAID,unsigned char ucAIDLen)
{
    unsigned char	ucResult;

//    Iso7816In.ucCla = 0x00;
//    Iso7816In.ucIns = 0xA4;
//    Iso7816In.ucP1  = 0x04;
//    Iso7816In.ucP2  = 00;

//    Iso7816In.ucP3 = ucAIDLen;
//    memcpy(&Iso7816In.aucDataIn[0],pucAID,ucAIDLen);
//    Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

//    PBOC_SetIso7816Out();
//    if( (ucResult = SMART_ISO(ucReader,
//                        SMART_ASYNC,
//                        (unsigned char *)&Iso7816In,
//                        Iso7816Out.aucData,
//                        &Iso7816Out.uiLen,
//                        TYPE4)) != OK)         //old is type4
//        return(ucResult);

    return(PBOC_ISOCheckReturn());
}
#endif

unsigned char SMART_NewWarmReset(
        unsigned char ucReader,
        unsigned char ucCardStandard,
        unsigned char * pucReset,
        unsigned short * puiLength)
{
    unsigned char ucCardType;

#if 0
    /* Power On */
    DRV_OUT* pdKey;

#ifdef GUI_PRO
    //while(  SMART_Detect(ucReader) == SMART_ABSENT);
#else
    Os__abort_drv(drv_mmi);
    pdKey = Os__get_key();
    while(  SMART_Detect(ucReader) == SMART_ABSENT)
    {
        if(pdKey->gen_status == DRV_ENDED)
        {
            if ( pdKey->xxdata[1] == KEY_CLEAR )
            {
                return (SMART_ABORT);
            }
            Os__abort_drv(drv_mmi);
            pdKey = Os__get_key();
        }
        }
#endif


    IccOrder.pt_order_in = &OrderType;
    IccOrder.ptout = pucReset;
    IccOrder.order = NEW_WARM_RESET;
    IccOrder.pt_order_in->new_power.card_standards = ucCardStandard;
    IccOrder.pt_order_in->new_power.GR_class_byte_00 = 1;
    IccOrder.pt_order_in->new_power.preferred_protocol = 0xFF;

    /* Send Command */
    pAnswer = Os__ICC_command (ucReader, &IccOrder);
    /* Answer Command */
    if ( pAnswer->drv_status != OK )
    {
        return (SMART_DRIVER_ERROR);
    }
    switch ( pAnswer->card_status )
    {
    case ICC_ASY:
        ucCardType = SMART_ASYNC;
        break;
    case ICC_SYN:
        ucCardType = SMART_SYNC;
        break;
    default:
        return (SMART_CARD_ERROR);
    }
    if ( pAnswer->Len > *puiLength )
    {
        return (SMART_OWERFLOW);
    }
    *puiLength = pAnswer->Len;
#endif
    return (ucCardType);
}

unsigned char SMART_NewReset(
        unsigned char ucReader,unsigned char ucCardStandard,
        unsigned char * pucReset,
        unsigned short * puiLength)
{
    unsigned char ucCardType;
    ICC_ORDER IccOrder;
    ICC_ANSWER* pAnswer;
    union ORDER_IN_TYPE OrderType;

    /* Power On */
    IccOrder.pt_order_in = &OrderType;
    IccOrder.ptout = pucReset;
    IccOrder.order = NEW_POWER_ON;
    IccOrder.pt_order_in->new_power.card_standards = ucCardStandard;
    IccOrder.pt_order_in->new_power.GR_class_byte_00 = 1;
    IccOrder.pt_order_in->new_power.preferred_protocol = 0xFF;

    /* Send Command */
    pAnswer = Os__ICC_command(ucReader, &IccOrder);

    /* Answer Command */
    if ( pAnswer->drv_status != OK )
    {
        _print11("SMART_NewReset return for drv_status != OK \r\n");
        return (SMART_DRIVER_ERROR);
    }

    switch ( pAnswer->card_status )
    {
    case ICC_ASY:
        ucCardType = SMART_ASYNC;
        break;
    case ICC_SYN:
        ucCardType = SMART_SYNC;
        break;
    default:
        _print11("SMART_NewReset return for SMART_CARD_ERROR\r\n");
        return (SMART_CARD_ERROR);
    }

    if ( pAnswer->Len > *puiLength )
    {
        _print11("SMART_NewReset return for SMART_OWERFLOW\r\n");
        return (SMART_OWERFLOW);
    }


    *puiLength = pAnswer->Len;


    return (ucCardType);
}

#if 0
unsigned char OSGSM_openPort(void)
{
    tty_fd = open(serialCom, O_RDWR|O_NOCTTY|O_NDELAY);
    if(tty_fd == -1)
        return FAILED;
    else
        return SUCCESS;
}


unsigned char OSGSM_setupUart(int databits,int stopbits,int parity,int flow_control,int speed)
{
    int fd=tty_fd;
    struct termios options;

    if(tcgetattr(fd, &options) != 0){
        _print11("Com:   tcgetattr fail\n");
        return(FAILED);
    }

    options.c_cflag &= ~CSIZE;
    switch(databits){
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            _print11("Com:   Unsupported data size\n");
            return(FAILED);
    }

    switch(parity){
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 's':
        case 'S':
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            _print11("Com:   Unsupported parity\n");
            return(FAILED);
    }

    switch(stopbits){
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            _print11("Com:   Unsupported stop bits\n");
            return(FAILED);
    }

    /* if tht port is not for moden, then set it as raw model */
    options.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG|XCASE|ECHONL|NOFLSH);
    //options.c_oflag = 0;
    options.c_iflag &= ~(IGNBRK|IGNCR|INLCR|ICRNL|IUCLC|ISTRIP|BRKINT);
    options.c_iflag |= IGNPAR;
    options.c_oflag &= ~OPOST;
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cc[VTIME] = 20;
    options.c_cc[VMIN]  = 0;

    switch(flow_control){
        case 1://hwflow control
        _print11("hardware flow control.\n");
        options.c_cflag |= CRTSCTS;
        break;
        case 2://swf control
        options.c_iflag |= IXON | IXOFF;
        break;
        case 0://none flow control
        default:
        _print11("none flow control.\n");
        options.c_cflag &= ~CRTSCTS;
        options.c_iflag &= ~(IXON|IXOFF|IXANY);
        break;
    }

    /* Update the option and do it NOW */
    tcflush(fd, TCIFLUSH);
    if(tcsetattr(fd, TCSANOW, &options) != 0){
        _print11("Com:   Setup Serial Port\n");
        return (FAILED);
    }
    return(SUCCESS);



}


//?????
unsigned char OSGSM_init(void)
{
    unsigned char result;

    result  = OSGSM_openPort();

    if (result == FAILED)
        return FAILED;

    result=OSGSM_setupUart(8,1,'N',0,115200);

    if (result == FAILED)
        return	FAILED;
    else
        return	SUCCESS;
}


unsigned char OSGSM_ATCmd(unsigned char *cmd, unsigned char *rsp, unsigned short ms)
{
    int i;
     unsigned char c, str[200];
     unsigned char *pucPtr;

     memset(str, 0, sizeof(str));
     strcpy(str, cmd);
     i = 0;
     str[strlen(cmd)] = 13;
     str[strlen(cmd)+1] = '\0';
     i = write(tty_fd, str, strlen(str));
     if (i <= 0)  return FAILED;

     usleep(100000);
     memset(str, 0, sizeof(str));
     usleep(200000);
     i=0;
     while(1)
     {
     i = read(tty_fd, str, 200);
     if((i==-1)&&(errno == EINTR))
         continue;
     else
         break;
     }
     if (i <= 0)  return FAILED;

     strcpy(rsp,str);

     return SUCCESS;
}

unsigned char OSGSM_depRecv(unsigned char *ucRcv)
{
    unsigned char str[200];
    unsigned char *p;
    unsigned char *ret;
    int l=0;
    memset(str,0,sizeof(str));
    l=read(tty_fd, str, 200);
    _print11("str %s\n",str);
    usleep(100000);
    p=str;
    ret=strstr(p,ucRcv);
    if(p)
        return SUCCESS;
    else
        return FAILED;
}

unsigned char OSGSM_answerCalling(char *pText)
{
    char 	cmd[200];
      char 	rsp[200];
      unsigned char 	*pucPtr;
      unsigned char ucResult;


      ucResult=OSGSM_depRecv("RING");
      _print11("RING  %02x\n",ucResult);
      if(ucResult==SUCCESS)
      {

      //send 'ATA'
      memset(cmd, 0x00, sizeof(cmd));
      memset(rsp, 0x00, sizeof(rsp));
      _print11("ATA\n");

      strcpy(cmd, "ATA\r");
      OSGSM_ATCmd(cmd, rsp, 3000);
      pucPtr = (unsigned char *)strstr( rsp, "OK");
      _print11("[%s]\n",rsp);
      pText = rsp;
      _print11("pText==%s\n",pText);

      if(pucPtr)
          return	SUCCESS;
      }
      else
          pText = ".";

      return FAILED;
}

//?????ATH,????
unsigned char OSGSM_hangUp(void)
{
    char 	cmd[200];
    char 	rsp[200];
    unsigned char 	*pucPtr;

    memset(cmd, 0x00, sizeof(cmd));
    strcpy(cmd, "ATH");

    memset(rsp, 0x00, sizeof(rsp));
    OSGSM_ATCmd(cmd, rsp, 3000);

    pucPtr = (unsigned char *)strstr( rsp, "OK");
        if( pucPtr ){
        return	SUCCESS;
    }
    return FAILED;
}

//????
unsigned char OSGSM_exit(void)
{
    unsigned char 	result;

    result = close(tty_fd);
    if (result !=SUCCESS)
        return FAILED;
    return FAILED;
}

#endif


unsigned char OSGSM_openPort(void)
{
    tty_fd = open(serialCom, O_RDWR|O_NOCTTY|O_NDELAY);
    if(tty_fd == -1)
        return FAILED;
    else
        return SUCCESS;
}

unsigned char OSGSM_Recv_dep(unsigned char *ucRcv,unsigned char *ucBuf)
{
    unsigned char *ret;

    ret=strstr(ucRcv,ucBuf);
    if(ret)
        return SUCCESS;
    else
        return FAILED;
}

unsigned char OSGSM_setupUart(int databits,int stopbits,int parity,int flow_control,int speed)
{
    int fd=tty_fd;
    struct termios options;

    if(tcgetattr(fd, &options) != 0){
        _print11("Com:   tcgetattr fail\n");
        return(FAILED);
    }

    options.c_cflag &= ~CSIZE;
    switch(databits){
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            _print11("Com:   Unsupported data size\n");
            return(FAILED);
    }

    switch(parity){
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 's':
        case 'S':
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            _print11("Com:   Unsupported parity\n");
            return(FAILED);
    }

    switch(stopbits){
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            _print11("Com:   Unsupported stop bits\n");
            return(FAILED);
    }

    /* if tht port is not for moden, then set it as raw model */
    options.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG|XCASE|ECHONL|NOFLSH);
    //options.c_oflag = 0;
    options.c_iflag &= ~(IGNBRK|IGNCR|INLCR|ICRNL|IUCLC|ISTRIP|BRKINT);
    options.c_iflag |= IGNPAR;
    options.c_oflag &= ~OPOST;
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cc[VTIME] = 20;
    options.c_cc[VMIN]  = 0;

    switch(flow_control){
        case 1://hwflow control
        _print11("hardware flow control.\n");
        options.c_cflag |= CRTSCTS;
        break;
        case 2://swf control
        options.c_iflag |= IXON | IXOFF;
        break;
        case 0://none flow control
        default:
        _print11("none flow control.\n");
        options.c_cflag &= ~CRTSCTS;
        options.c_iflag &= ~(IXON|IXOFF|IXANY);
        break;
    }

    /* Update the option and do it NOW */
    tcflush(fd, TCIFLUSH);
    if(tcsetattr(fd, TCSANOW, &options) != 0){
        _print11("Com:   Setup Serial Port\n");
        return (FAILED);
    }
    return(SUCCESS);



}


//?????
unsigned char OSGSM_init(void)
{
    unsigned char result;

    result  = OSGSM_openPort();

    if (result == FAILED)
        return FAILED;

    result=OSGSM_setupUart(8,1,'N',0,115200);

    if (result == FAILED)
        return	FAILED;
    else
        return	SUCCESS;
}

//?????????????
unsigned char OSGSM_ATCmd(unsigned char *cmd, unsigned char *rsp, unsigned short ms)
{
    int i;
    unsigned char c, str[200];
    unsigned char *pucPtr;

    memset(str, 0, sizeof(str));
    strcpy(str, cmd);
    i = 0;
    str[strlen(cmd)] = 13;
    str[strlen(cmd)+1] = '\0';
    i = write(tty_fd, str, strlen(str));
    if (i <= 0)  return FAILED;

    usleep(100000);
    memset(str, 0, sizeof(str));
    usleep(200000);
    i=0;
    while(1)
    {
    i = read(tty_fd, str, 200);
    if((i==-1)&&(errno == EINTR))
        continue;
    else
        break;
    }
    if (i <= 0)  return FAILED;

    strcpy(rsp,str);

    return SUCCESS;
}


unsigned char OSGSM_depRecv(unsigned char *ucRcv)
{
    unsigned char str[200];
    unsigned char *p;
    unsigned char *ret;
    int l=0;
    memset(str,0,sizeof(str));
    l=read(tty_fd, str, 200);

    usleep(100000);
    p=str;
    ret=strstr(p,ucRcv);
    if(ret)
        return SUCCESS;
    else
        return FAILED;
}

unsigned char OSGSM_answerCalling(void)
{
    char 	cmd[200];
      char 	rsp[200];
      unsigned char 	*pucPtr;
      unsigned char ucResult;


      ucResult=OSGSM_depRecv("RING");
      _print11("RING  %02x\n",ucResult);
      if(ucResult==SUCCESS)
      {
      memset(cmd, 0x00, sizeof(cmd));
      memset(rsp, 0x00, sizeof(rsp));
      _print11("ATA\n");

      strcpy(cmd, "ATA\r");
      OSGSM_ATCmd(cmd, rsp, 3000);
      pucPtr = (unsigned char *)strstr( rsp, "OK");
      _print11("[%s]\n",rsp);
//      _print11("pText==%s\n",pText);

      if(pucPtr)
          return	SUCCESS;
      }

      return FAILED;
}

//?????ATH,????
unsigned char OSGSM_hangUp(void)
{
    char 	cmd[200];
    char 	rsp[200];
    unsigned char 	*pucPtr;

    memset(cmd, 0x00, sizeof(cmd));
    strcpy(cmd, "ATH");

    memset(rsp, 0x00, sizeof(rsp));
    OSGSM_ATCmd(cmd, rsp, 3000);

    pucPtr = (unsigned char *)strstr( rsp, "OK");
        if( pucPtr ){
        return	SUCCESS;
    }
    return FAILED;
}

int OSGSM_signalDetect(void)
{
        int	detect_result = 0;
        char 	cmd[200]="AT+CSQ\r";
        char 	rsp[200];
        int signal_value=0;
        int rlen, pos,l;
        char *ptrb;
        char *ptre;
        char ucResult;

    ucResult=OSGSM_init();

    if(ucResult!=0x00)
    {
      _print11("GSM INI FAILED,PRESS OK TO REINITIALIZE!\n");
//      exit(-1);
      return -1;
    }

    memset(rsp,0,sizeof(rsp));
    OSGSM_ATCmd("AT", rsp, 3000);
    _print11("%s\n",rsp);






        memset(rsp, 0, sizeof(rsp));
//        rlen = _AT_cmd(cmd,rsp,sizeof(rsp),3);

        OSGSM_ATCmd(cmd,rsp,3000);
        _print11("-->%s\n",rsp);
        rlen=strlen(rsp);

//	    if (rlen <= 0)
 //           return 0;
        if(rlen>0)
        {
        ptrb=strpbrk(rsp,":");
        if(ptrb)
        {
           ptre=strpbrk(rsp,",");
           if(ptre)
           {
             l=ptre-ptrb;
             if(l>0)
             {
              ptrb+=2;
              signal_value=asc_long(ptrb,l-2);
              if(signal_value>31)
                signal_value=99;
             }
          }
        }
       // else
         //  return -1;
        }
        return signal_value;
}





//????
unsigned char OSGSM_exit(void)
{
    unsigned char 	result;

    result = close(tty_fd);
    if (result !=SUCCESS)
        return FAILED;
    return FAILED;
}


unsigned char SLE4442_FinishCommand(unsigned char *pucBuf)
{
    pucBuf[0] = EOC_CODE;	//0x00

    return (1);
}


unsigned char SLE4442_ResetAddress(unsigned char *pucBuf)
{
    pucBuf[0] = CLR_CLK_CODE;  	//0x08
    pucBuf[1] = CLR_RST_CODE;	//0x02
    pucBuf[2] = SET_IO_CODE;		//0x22
    pucBuf[3] = SET_RST_CODE;	//0x04
    pucBuf[4] = CLK_5US_CODE;	//0x0c
    pucBuf[5] = CLR_RST_CODE;	//0x02
    pucBuf[6] = IO_TO_LSB_CODE;	//0x2a

    return(7);
}

unsigned char SLE4442_NReadBits(unsigned char *pucBuf, unsigned char ucbitscnt)
{
    pucBuf[0] = MACRO_CODE;		//0xff
    pucBuf[1] = CLK_5US_CODE;	//0x0c
    pucBuf[2] = IO_TO_LSB_CODE;	//0x2a
    pucBuf[3] = MACRO_CODE;		//0xff
    pucBuf[4] = ucbitscnt;			//31
    return(5);
}

unsigned char SLE4442_End_ATR(unsigned char *pucBuf)
{
    pucBuf[0] = CLK_5US_CODE;	//0x0c

    return(1);
}

unsigned char SLE4442_ATR(unsigned char *pucBuf)
{
    unsigned char aucSle4442[256];
    unsigned char ucIndex;

    ICC_ORDER 	SLE4442_order;
    ICC_ANSWER *pSLE4442_answer;
    union ORDER_IN_TYPE SLE4442_type;

    ucIndex = 0;
    ucIndex += SLE4442_ResetAddress(aucSle4442 + ucIndex);	//return 7
    ucIndex += SLE4442_NReadBits(aucSle4442 + ucIndex, 31);	//return 5
    ucIndex += SLE4442_End_ATR(aucSle4442 + ucIndex);		//return 1
    ucIndex += SLE4442_FinishCommand(aucSle4442 + ucIndex);	//return 1

    if (ucIndex)		//ucIndex=14
    {
        SLE4442_type.new_sync_order.Len = ucIndex;
        SLE4442_type.new_sync_order.ptin = aucSle4442;
        SLE4442_order.pt_order_in = &SLE4442_type;
        SLE4442_order.order = NEW_SYNC_ORDER;
        SLE4442_order.ptout = aucSle4442;
        pSLE4442_answer = Os__ICC_command(0,&SLE4442_order);
        if (pSLE4442_answer->drv_status != OK )
        {
            //return (SMART_CARD_ERROR);
            _print11("SLE4442 DRV_STATUS ERROR\n");
            return 1;
        }
        if( pSLE4442_answer->card_status != ASY_OK )
        {
            //return (SMART_CARD_ERROR);
            _print11("SLE4442 CARD_STATUS ERROR\n");
            return 1;
        }
        memcpy(pucBuf,aucSle4442,pSLE4442_answer->Len);
    }

    return(OK);
}


unsigned char SLE4442_ReadMainMemory(unsigned char *pucBuf,
                                                                             unsigned char ucaddr,
                                                                             unsigned char ucsize)
{
    unsigned char aucSle4442[256];
    unsigned char ucIndex;

    ICC_ORDER 	SLE4442_order;
    ICC_ANSWER *pSLE4442_answer;
    union ORDER_IN_TYPE SLE4442_type;

    ucIndex = 0;

    if (ucsize == 0)
    {
        return 0;
    }
    if (ucaddr + ucsize > 255)
    {
        return 0;
    }

    ucIndex += SLE4442_Enter_Command_Mode(aucSle4442 + ucIndex);//return 5
    ucIndex += SLE4442_Send_Command(aucSle4442 + ucIndex, 0x30, ucaddr, 0);//rturn 48
    ucIndex += SLE4442_Leave_Command_Mode(aucSle4442 + ucIndex);//return 2
    ucIndex += SLE4442_NReadBits(aucSle4442 + ucIndex, ucsize * 8);//return 5
    ucIndex += SLE4442_Break(aucSle4442 + ucIndex);
    ucIndex += SLE4442_FinishCommand(aucSle4442 + ucIndex);//return 1

    if (ucIndex)
    {
        SLE4442_type.new_sync_order.Len = ucIndex;
        SLE4442_type.new_sync_order.ptin = aucSle4442;
        SLE4442_order.pt_order_in = &SLE4442_type;
        SLE4442_order.order = NEW_SYNC_ORDER;
        SLE4442_order.ptout = aucSle4442;
        pSLE4442_answer = Os__ICC_command(0,&SLE4442_order);

        _print11(" SLE4442_answer->drv_status = %d\n",  pSLE4442_answer->drv_status);

        _print11(" SLE4442_answer->card_status = %d\n",  pSLE4442_answer->card_status);


        if( pSLE4442_answer->drv_status != OK )
        {
            //return (ERR_READMEMORY);
            _print11("SLE4442 DRV_STATUS ERROR\n");
            return 1;
        }
        if( pSLE4442_answer->card_status != ASY_OK )
        {
            //return (ERR_READMEMORY);
            _print11("SLE4442 CARD_STATUS ERROR\n");
            return 1;
        }
        memcpy(pucBuf, aucSle4442,pSLE4442_answer->Len);
    }

    return(OK);
}

/************************** printer **************************************/



unsigned long asc_long(unsigned char *Pts, unsigned char Ls)
{
    unsigned char I;
    unsigned long Lg1,
    Lg2;

    Lg1 = 0 ;
    Lg2 = 1 ;
    Pts += Ls ;
    for (I = 0; I< Ls ; I++)
    {
        Lg1 += (Lg2 * (*--Pts & 0x0F) ) ;
        Lg2 *= 10 ;
    }

    return (Lg1) ;
}


/******************  wifi *********************************/

