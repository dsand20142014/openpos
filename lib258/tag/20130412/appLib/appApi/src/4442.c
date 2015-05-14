
/*
	Test System
--------------------------------------------------------------------------
	FILE  testsam.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-08-03		Xiaoxi Jiang
    Last modified :	2003-08-03		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

//#include <os.h>
#include <osdata.h>
#include <osicc.h>
#include <smart.h>
#include <newsync.h>
#include <stdio.h>
#include <string.h>

#define SUCCESS						0x00
#define ERR_POWERFAIL				0x40
#define ERR_READMEMORY				0x41
#define ERR_WRITEMEMORY				0x42
#define ERR_CHECKSECRET				0x43
#define ERR_MODIFYSECRET			0x44
#define ERR_CARDLOCK				0x45
#define ERR_CARDCRC					0x46
#define ERR_CARDPROXY				0x50

unsigned char SLE4442_ATR(unsigned char *pBuf);
unsigned char SLE4442_ReadMainMemory(unsigned char *pBuf,unsigned char ucAddr,unsigned char ucNbr);
unsigned char SLE4442_Read_Security_Memory(unsigned char *pBuf);
unsigned char SLE4442_Update_Main_Memory(unsigned char Input_Data,unsigned char ucAddr);
unsigned char SLE4442_Update_Security_Memory(unsigned char Input_Data,unsigned char ucAddr);
unsigned char SLE4442_Compare_Verification_Data(unsigned char Input_Data,unsigned char ucAddr);
unsigned char SLE4442_ResetAddress(unsigned char *code_buff);
unsigned char SLE4442_End_ATR(unsigned char *code_buff);
unsigned char SLE4442_Enter_Command_Mode(unsigned char *code_buff);
unsigned char SLE4442_Leave_Command_Mode(unsigned char *code_buff);
unsigned char SLE4442_Enter_Outgoing_Mode(unsigned char *code_buff);
unsigned char SLE4442_Send_Command(unsigned char *code_buff,unsigned char Command_Type,unsigned char Addr,unsigned char Input_Data);
unsigned char SLE4442_ReadBits(unsigned char *code_buff, unsigned char N_Bits);
unsigned char SLE4442_NReadBits(unsigned char *code_buff, unsigned char N_Bits);
unsigned char SLE4442_Erase_And_Write(unsigned char *code_buff);
unsigned char SLE4442_Compare_Process(unsigned char *code_buff);
unsigned char SLE4442_Break(unsigned char *code_buff);
unsigned char SLE4442_FinishCommand(unsigned char *code_buff);
unsigned char SLE4442_Check_Secret(unsigned char *pSecret_Code);
unsigned char SLE4442_Read(void);
unsigned char SLE_Read_Card_Info(void);

ICC_ORDER 	SLE4442_Order;
ICC_ANSWER *SLE4442_Answer;
union ORDER_IN_TYPE SLE4442_Type;
static unsigned char buff_index;
unsigned char SLE4442_Buffer[256];
unsigned char ucKey[4];

#define min(a,b) (((a)<(b))?(a):(b))

unsigned char SEL_SLE4442_Menu(void)
{
    unsigned char ucResult;
    unsigned char strBuf[100];

    ucResult = OK;

    /*Power On*/
    SLE4442_ATR(strBuf);

#if 1
    if (memcmp(strBuf,"\xA2\x13\x10\x91",4))
    {
        ucResult = ERR_POWERFAIL;
    }
#endif
    if (ucResult == SUCCESS)
    {
        ucResult = SLE_Read_Card_Info();
    }

#if 0
    if (ucResult == SUCCESS)
    {
        memset(strBuf,0,sizeof(strBuf));
        OSMMI_DisplayASCII(ASCIIFONT57,3,5,(unsigned char *)strBuf);
    }
#endif

    return(SUCCESS);

}

void gz_xor(unsigned char *a, unsigned char *b, char lg)
{
    while (lg--)
        *(b++) ^= *(a++);
}

unsigned char SLE4442_ATR(unsigned char *pBuf)
{

    buff_index = 0;

    buff_index += SLE4442_ResetAddress(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_NReadBits(SLE4442_Buffer+buff_index, 31);

    buff_index += SLE4442_End_ATR(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

    if ( buff_index )
    {
        SLE4442_Type.new_sync_order.Len = buff_index;
        SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
        SLE4442_Order.pt_order_in = &SLE4442_Type;
        SLE4442_Order.order = NEW_SYNC_ORDER;
        SLE4442_Order.ptout = SLE4442_Buffer;
        SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
        if ( SLE4442_Answer->drv_status != OK )
        {
            return (SMART_CARD_ERROR);
        }
        if ( SLE4442_Answer->card_status != ASY_OK )
        {
            return (SMART_CARD_ERROR);
        }
        memcpy(pBuf,SLE4442_Buffer,SLE4442_Answer->Len);
    }
    return(OK);
}

unsigned char SLE4442_ReadMainMemory(unsigned char *pBuf,unsigned char ucAddr,unsigned char ucNbr)
{
    int i, j;
    buff_index = 0;
    if ( ucNbr == 0)
        return(0);

    if ( ucAddr+ucNbr > 255)
        return(0);

    buff_index += SLE4442_Enter_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Send_Command(SLE4442_Buffer+buff_index,0x30,ucAddr,0);

    buff_index += SLE4442_Leave_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_NReadBits(SLE4442_Buffer+buff_index, ucNbr*8);

    buff_index += SLE4442_Break(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

    if ( buff_index )
    {
        SLE4442_Type.new_sync_order.Len = buff_index;
        SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;

        SLE4442_Order.pt_order_in = &SLE4442_Type;
        SLE4442_Order.order = NEW_SYNC_ORDER;
        SLE4442_Order.ptout = SLE4442_Buffer;
        SLE4442_Answer = Os__ICC_command(ICC1,&SLE4442_Order);

	//printf(" SLE4442_answer->drv_status = %d\n",  SLE4442_Answer->drv_status);

	//printf(" SLE4442_answer->card_status = %d\n",  SLE4442_Answer->card_status);

        if ( SLE4442_Answer->drv_status != OK )
        {
            return (ERR_READMEMORY);
        }
        if ( SLE4442_Answer->card_status != ASY_OK )
        {
            return (ERR_READMEMORY);
        }
        memcpy(pBuf,SLE4442_Buffer,SLE4442_Answer->Len);

    }
    return(OK);
}

unsigned char SLE4442_Read_Security_Memory(unsigned char *pBuf)
{

    buff_index = 0;

    buff_index += SLE4442_Enter_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Send_Command(SLE4442_Buffer+buff_index,0x31,0,0);

    buff_index += SLE4442_Leave_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_NReadBits(SLE4442_Buffer+buff_index, 32);

    buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

    if ( buff_index )
    {
        SLE4442_Type.new_sync_order.Len = buff_index;
        SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
        SLE4442_Order.pt_order_in = &SLE4442_Type;
        SLE4442_Order.order = NEW_SYNC_ORDER;
        SLE4442_Order.ptout = SLE4442_Buffer;
        SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
        if ( SLE4442_Answer->drv_status != OK )
        {
            return (SMART_CARD_ERROR);
        }
        if ( SLE4442_Answer->card_status != ASY_OK )
        {
            return (SMART_CARD_ERROR);
        }
        memcpy(pBuf,SLE4442_Buffer,SLE4442_Answer->Len);
    }
    return(OK);
}
unsigned char SLE4442_Update_Main_Memory(unsigned char Input_Data,unsigned char ucAddr)
{

    buff_index = 0;

    buff_index += SLE4442_Enter_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Send_Command(SLE4442_Buffer+buff_index,0x38,ucAddr,Input_Data);

    buff_index += SLE4442_Leave_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Erase_And_Write(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

    if ( buff_index )
    {
        SLE4442_Type.new_sync_order.Len = buff_index;
        SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
        SLE4442_Order.pt_order_in = &SLE4442_Type;
        SLE4442_Order.order = NEW_SYNC_ORDER;
        SLE4442_Order.ptout = SLE4442_Buffer;
        SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);

        if ( SLE4442_Answer->drv_status != OK )
        {
            return (ERR_WRITEMEMORY);
        }
        if ( SLE4442_Answer->card_status != ASY_OK )
        {
            return (ERR_WRITEMEMORY);
        }
    }
    return(OK);
}

unsigned char SLE4442_Update_Security_Memory(unsigned char Input_Data,unsigned char ucAddr)
{

    buff_index = 0;

    buff_index += SLE4442_Enter_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Send_Command(SLE4442_Buffer+buff_index,0x39,ucAddr,Input_Data);

    buff_index += SLE4442_Leave_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Erase_And_Write(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

    if ( buff_index )
    {
        SLE4442_Type.new_sync_order.Len = buff_index;
        SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
        SLE4442_Order.pt_order_in = &SLE4442_Type;
        SLE4442_Order.order = NEW_SYNC_ORDER;
        SLE4442_Order.ptout = SLE4442_Buffer;
        SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
        if ( SLE4442_Answer->drv_status != OK )
        {
            return (ERR_MODIFYSECRET);
        }
        if ( SLE4442_Answer->card_status != ASY_OK )
        {
            return (ERR_MODIFYSECRET);
        }

    }
    return(OK);
}

unsigned char SLE4442_Compare_Verification_Data(unsigned char Input_Data,unsigned char ucAddr)
{

    buff_index = 0;

    buff_index += SLE4442_Enter_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Send_Command(SLE4442_Buffer+buff_index,0x33,ucAddr,Input_Data);

    buff_index += SLE4442_Leave_Command_Mode(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_Compare_Process(SLE4442_Buffer+buff_index);

    buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

    if ( buff_index )
    {
        SLE4442_Type.new_sync_order.Len = buff_index;
        SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
        SLE4442_Order.pt_order_in = &SLE4442_Type;
        SLE4442_Order.order = NEW_SYNC_ORDER;
        SLE4442_Order.ptout = SLE4442_Buffer;
        SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
        if ( SLE4442_Answer->drv_status != OK )
        {
            return (SMART_CARD_ERROR);
        }
        if ( SLE4442_Answer->card_status != ASY_OK )
        {
            return (SMART_CARD_ERROR);
        }

    }
    return(OK);
}

unsigned char SLE4442_ResetAddress(unsigned char *code_buff)
{
    code_buff[0] = CLR_CLK_CODE;
    code_buff[1] = CLR_RST_CODE;
    code_buff[2] = SET_IO_CODE;
    code_buff[3] = SET_RST_CODE;
    code_buff[4] = CLK_5US_CODE;
    code_buff[5] = CLR_RST_CODE;
    code_buff[6] = IO_TO_LSB_CODE;
    return(7);
}

unsigned char SLE4442_End_ATR(unsigned char *code_buff)
{
    code_buff[0] = CLK_5US_CODE;
    return(1);
}


/*
* CLK          _
*		 _|  |_
*
* I/O      ___
*                 |__
*/
unsigned char SLE4442_Enter_Command_Mode(unsigned char *code_buff)
{
#if 1
    code_buff[0] = SET_IO_CODE;
    code_buff[1] = CLR_CLK_CODE;
    code_buff[2] = SET_CLK_CODE;
    code_buff[3] = CLR_IO_CODE;
    code_buff[4] = CLR_CLK_CODE;
    return(5);
#else //TODO 0825
    /*
    	code_buff[0] = SET_CLK_CODE;
    	code_buff[1] = CLK_5US_CODE;
    	code_buff[2] = CLR_CLK_CODE;
    	code_buff[3] = CLK_5US_CODE;
    	code_buff[4] = SET_CLK_CODE;
    //	code_buff[5] = CLK_10US_CODE;
    //	code_buff[6] = CLR_CLK_CODE;
    //	code_buff[7] = SET_IO_CODE;
    */
    code_buff[0] = SET_IO_CODE;
    return 1;

    code_buff[1] = CLR_IO_CODE;
    code_buff[2] = SET_IO_CODE;
    code_buff[3] = CLR_IO_CODE;
    code_buff[4] = SET_IO_CODE;
    /*
    	code_buff[3] = WT_200US_CODE;
    	code_buff[4] = CLR_IO_CODE;
    */
    /*
    	code_buff[5] = WT_10US_CODE;
    	code_buff[6] = SET_CLK_CODE;
    */
    return(5);
#endif

}

unsigned char SLE4442_Leave_Command_Mode(unsigned char *code_buff)
{
    code_buff[0] = SET_CLK_CODE;
    code_buff[1] = SET_IO_CODE;

    return(2);
}

unsigned char SLE4442_Enter_Outgoing_Mode(unsigned char *code_buff)
{
    code_buff[0] = SET_CLK_CODE;
    code_buff[1] = CLR_CLK_CODE;
    return(2);
}

unsigned char SLE4442_Send_Command(unsigned char *code_buff,unsigned char Command_Type,unsigned char Addr,unsigned char Input_Data)
{
    unsigned char Mask = 0x01;
    unsigned char i,j;

    j = 0;

    for (i=0; i<8; i++,Mask=Mask<<1)
    {
        if (Command_Type & Mask)
        {
            code_buff[j++] = SET_IO_CODE;
        }
        else
        {
            code_buff[j++] = CLR_IO_CODE;
        }
        code_buff[j++] = CLK_5US_CODE;
    }


//TODO 0825
    Mask = 0x01;
    for (i=0; i<8; i++,Mask=Mask<<1)
    {
        if (Addr & Mask)
        {
            code_buff[j++] = SET_IO_CODE;
        }
        else
        {
            code_buff[j++] = CLR_IO_CODE;
        }
        code_buff[j++] = CLK_5US_CODE;
    }
    Mask = 0x01;
    for (i=0; i<8; i++,Mask=Mask<<1)
    {
        if (Input_Data & Mask)
        {
            code_buff[j++] = SET_IO_CODE;
        }
        else
        {
            code_buff[j++] = CLR_IO_CODE;
        }
        code_buff[j++] = CLK_5US_CODE;
    }
    code_buff[j++] = CLR_IO_CODE;
    return(j);
}

unsigned char SLE4442_ReadBits(unsigned char *code_buff, unsigned char N_Bits)
{
    code_buff[0] = SET_CLK_CODE;
    code_buff[1] = CLR_CLK_CODE;
    code_buff[2] = MACRO_CODE;
    code_buff[3] = IO_TO_LSB_CODE;
    code_buff[4] = CLK_5US_CODE;
    code_buff[5] = MACRO_CODE;
    code_buff[6] = N_Bits;
    code_buff[7] = CLK_5US_CODE;
    return(8);
}

unsigned char SLE4442_NReadBits(unsigned char *code_buff, unsigned char N_Bits)
{
    code_buff[0] = MACRO_CODE;
    code_buff[1] = CLK_5US_CODE;
    code_buff[2] = IO_TO_LSB_CODE;
    code_buff[3] = MACRO_CODE;
    code_buff[4] = N_Bits;
    return(5);
}
unsigned char SLE4442_Erase_And_Write(unsigned char *code_buff)
{
    code_buff[0] = MACRO_CODE;
    code_buff[1] = CLK_10US_CODE;
    code_buff[2] = MACRO_CODE;
    code_buff[3] = 254;
    return(4);
}

unsigned char SLE4442_Compare_Process(unsigned char *code_buff)
{
    code_buff[0] = SET_CLK_CODE;
    code_buff[1] = SET_IO_CODE;
    code_buff[2] = CLR_CLK_CODE;
    code_buff[3] = CLR_IO_CODE;
    code_buff[4] = MACRO_CODE;
    code_buff[5] = CLK_10US_CODE;
    code_buff[6] = MACRO_CODE;
    code_buff[7] = 3;
    return(8);
}

unsigned char SLE4442_Break(unsigned char *code_buff)
{
    code_buff[0] = CLR_CLK_CODE;
    code_buff[1] = SET_RST_CODE;
    code_buff[2] = CLR_RST_CODE;
    return(3);
}

unsigned char SLE4442_FinishCommand(unsigned char *code_buff)
{
    code_buff[0] = EOC_CODE;
    return (1);
}
unsigned char SLE4442_Check_Secret(unsigned char *pSecret_Code)
{
    unsigned char strBuf[100];

    if ( SLE4442_Read_Security_Memory(strBuf) != OK)
    {
        return(ERR_READMEMORY);
    }
    switch (strBuf[0])
    {
    case 0x07:
        if ( SLE4442_Update_Security_Memory(0x06,0) != OK)
        {
            return(ERR_WRITEMEMORY);
        }
        break;
    case 0x06:
        if ( SLE4442_Update_Security_Memory(0x04,0) != OK)
        {
            return(ERR_WRITEMEMORY);
        }
        break;
    case 0x04:
        if ( SLE4442_Update_Security_Memory(0x00,0) != OK)
        {
            return(ERR_WRITEMEMORY);
        }
        break;
    default:
        return(ERR_CARDLOCK);
    }
    if ( SLE4442_Compare_Verification_Data(*(pSecret_Code),0x01) != OK)
    {
        return(ERR_CHECKSECRET);
    }
    if ( SLE4442_Compare_Verification_Data(*(pSecret_Code+1),0x02) != OK)
    {
        return(ERR_CHECKSECRET);
    }
    if ( SLE4442_Compare_Verification_Data(*(pSecret_Code+2),0x03) != OK)
    {
        return(ERR_CHECKSECRET);
    }

    if ( SLE4442_Update_Security_Memory(0xFF,0) != OK)
    {
        return(ERR_WRITEMEMORY);
    }

    if ( SLE4442_Read_Security_Memory(strBuf) != OK)
    {
        return(ERR_READMEMORY);
    }
    if ( strBuf[0] != 0x07)
    {
        return(ERR_CHECKSECRET);
    }
    return(OK);
}

unsigned char SLE4442_Read(void)
{
    unsigned char strBuf[100],disp[17];
    unsigned char ucResult,ucI,ucJ;

    memset(strBuf,0,sizeof(strBuf));
    SLE4442_ATR(strBuf);
    if (memcmp(strBuf,"\xA2\x13\x10\x91",4))
    {
        ucResult = ERR_POWERFAIL;
    }
    if (ucResult == SUCCESS )
    {
        for (ucI=0; ucI<7; ucI+=32)
        {
            ucResult=SLE4442_ReadMainMemory(strBuf,ucI,32);
            if (ucResult == SUCCESS )
            {
                for (ucJ=0; ucJ<4; ucJ++)
                {
                    memset(disp,0,sizeof(disp));
                    hex_asc(disp,&strBuf[ucJ*8],16);
                    disp[16] = 0;
                    /*
                    					OSMMI_DisplayASCII(ASCIIFONT57,ucJ,0,(unsigned char *)disp);
                    */
                    Uart_Printf("%d %02x\n", ucJ, disp);
                }
                /*
                				OSMMI_XGetKey();
                */
            }
            else
                break;
        }
    }
    return(ucResult);
}
unsigned char SLE_Read_Card_Info(void)
{
    unsigned char ucResult,ucI,ucJ;
    unsigned char pCust_Card[50];

    /*Read Card*/
    memset(pCust_Card,0,sizeof(pCust_Card));
    for (ucI=0,ucJ=32; ucI<40; ucI+=ucJ)
    {
        ucJ = min(ucJ,40-ucI);
        if ( ucResult == SUCCESS )
        {
            ucResult = SLE4442_ReadMainMemory(pCust_Card+ucI,32+ucI,ucJ);
        }
//			SLE4442_ATR(strBuf);
    }
#if 0
    for (ucI=0,ucJ=32; ucI<32; ucI+=ucJ)
    {
        ucJ = min(ucJ,40-ucI);
        if ( ucResult == SUCCESS )
        {
            ucResult=SLE4442_ReadMainMemory(pCust_Card+ucI,32+ucI,ucJ);
        }
    }
#endif
    Uart_Printf("Read card info begining at 32th byte\n");
    for (ucI=0; ucI<40; ucI++)
    {
        Uart_Printf("%02x ",pCust_Card[ucI]);
    }
    /*LILEI TEST*/
    return SUCCESS;

}



unsigned char SLE4442_ReadCardData(void)
{
    unsigned char ucResult,uctreat;
    unsigned char pCust_Card[300];
    int i,ucI,ucJ,j;
    /*Read Card*/
    ucResult=SUCCESS;//he:add
    memset(pCust_Card,0,sizeof(pCust_Card));
#if 0
    for (ucI=0,ucJ=32; ucI<256-32; ucI+=ucJ)
    {
        if ( ucResult == SUCCESS )
        {
            ucResult=SLE4442_ReadMainMemory(pCust_Card+ucI,32+ucI,ucJ);
        }
    }
#endif
    SLE4442_ReadMainMemory(pCust_Card,0, 32);
}


int test_sync()
{
    char buf[256];
	int i, j;

    memset(buf, 0, 100);

    SLE4442_ATR(buf);
#if 1
    Uart_Printf("ATR data:\n");
    for (i=0; i<4; i++)
        Uart_Printf("%02x ", buf[i]);
    Uart_Printf("\n");
#endif

    	memset(buf, 0, 256);
i=0;
    for (i=0; i<8; i++)
    {
        SLE4442_ReadMainMemory(buf+32*i,i*32,32);
    }
	
        for (j=0; j<256; j++)
            Uart_Printf("%02x ", buf[j]);
        Uart_Printf("\n");



}




//end
