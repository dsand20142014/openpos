#include <stdio.h>
#include <osdrv.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#include "lcd.h"



void OSMMI_Beep(void)
{
        NOTIMPLEMENT
}

DRVOUT *OSMMI_GetKey(void)
{
        NOTIMPLEMENT
}
unsigned char OSMMI_XGetKey(void)
{
        NOTIMPLEMENT
}
DRVOUT *OSMMI_GetKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
        NOTIMPLEMENT
}
unsigned char *OSMMI_XGetKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
        NOTIMPLEMENT
}
DRVOUT *OSMMI_GetFixKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
        NOTIMPLEMENT
}
unsigned char *OSMMI_XGetFixKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
        NOTIMPLEMENT
}
DRVOUT *OSMMI_GetPin(unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
        NOTIMPLEMENT
}
unsigned char *OSMMI_XGetPin(unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
        NOTIMPLEMENT
}
void OSMMI_Crypt(unsigned char *pucCryptKey)
{
        NOTIMPLEMENT
}
unsigned char OSMMI_BackLightCtl(unsigned char ucFlag)
{
        NOTIMPLEMENT
}
unsigned char OSMMI_GB2312Display(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr)
{
        NOTIMPLEMENT
}
unsigned char OSMMI_GraphDisplay(unsigned char ucType, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr,
                                 unsigned char ucLen)
{
        NOTIMPLEMENT
}
unsigned char OSMMI_NonStdDisplay(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr)
{
        NOTIMPLEMENT
}

#if 0
unsigned char OSMMI_DisplayASCII (unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char * Text)
{
        switch (ucFont)
        {
        case 0x31:
                Os__display(ucRow, ucCol, Text);
                break;
        case 0x30:
                //Os__display_6X8(ucRow, ucCol, Text);
                break;
        default:
                return 1;
        }

        return 0;
}


void OSMMI_ClrDisplay(unsigned char ucFont, unsigned char line)
{
        int lcd_fd;

        lcd_fd = open("/dev/lcd", O_RDWR);
        if (lcd_fd < 0)
        {
                Uart_Printf("open lcd error\n");
                return;
        }

        if ((line > 8) && (line != 0xFF))
        {
                close(lcd_fd);
                return;
        }

        switch (ucFont)
        {
        case 0x31:
                ioctl(lcd_fd, LCD_CLR_DISPLAY, &line);
                break;
        case 0x30:
                ioctl(lcd_fd, LCD_CLR_ONE_LINE, &line);
                break;
        default:
                close(lcd_fd);
                return;
        }

        close(lcd_fd);

        return;
}

#endif
//end
