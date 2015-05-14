#ifndef   SPLC501C_H
#define   SPLC501C_H

#ifndef HALLCDDATA
#define HALLCDDATA extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define HALLCD_SUCCESS				0x00
#define LCDDRVERR_LCDBUSY			0x91
#define LCDDRVERR_LCDUNKNOWNCHIP	0x92
#define LCDDRVERR_UNSUPPORTICON		0x93
#define LCDDRVERR_BADPARAM			0x94
#define LCDDRVERR_UNKNOWICONCMD		0x95
#define HALLCD_BADDATEFORMAT		0x96
#define HALLCDERR_LOCKED			0x9F

#define LCD_CSCHIP				0x4100004
#define LCD_R_CMD				0x01
#define LCD_W_CMD				0x00
#define LCD_R_DATA				0x03
#define LCD_W_DATA				0x02

#define LCDRETRY_CHECKBUSY		1000

#define LCDICONTYPE_MAXNB		6
#define LCDDATE_LEN				13
#define LCDDATE_LEN2			10
#define LCDICONTYPE_SIGNAL		0x00
#define LCDICONTYPE_TELEPHONE	0x01
#define LCDICONTYPE_MESSAGE		0x02
#define LCDICONTYPE_MAGCARD		0x03
#define LCDICONTYPE_ICCCARD		0x04
#define LCDICONTYPE_BATTERY		0x05
#define LCDICONTYPE_DATETIME	0x06

typedef struct
{
	unsigned char   ucIconCmd;
	unsigned char   aucSEG[9];
}ICONDEF;

typedef struct
{
	unsigned char	ucIconType;
	ICONDEF			*pIconDef;
	unsigned char	aucIconSEG[9];
}ICONTBL;

unsigned char HALLCD_Init(void);
unsigned char HALLCD_ClearAll(void);
unsigned char HALLCD_Display8DotInCol(unsigned char ucFlag,unsigned char ucY,unsigned char ucX,unsigned char *pucPtr);
unsigned char HALLCD_DisplayIcon(unsigned char ucIconType,unsigned char *pucIconData);
unsigned char HALLCD_DisplayNewDate(unsigned char *pucDate);
unsigned char HALLCD_ClearLine(unsigned char ucLineNb);
unsigned char HALLCD_ClearAddon(void);
unsigned char HALLCD_ClearIcon(unsigned char ucIconType);
unsigned char HALLCD_WriteCMDChip(unsigned char ucCmd);
unsigned char HALLCD_ReadData(unsigned char *pucData);
unsigned char HALLCD_WriteData(unsigned char ucData);
unsigned char HALLCD_CheckBusy(void);
unsigned char HALLCD_Lock(unsigned char ucWaitFlag);
unsigned char HALLCD_Unlock(void);
void HALLCD_ExchangeHighLowBit(unsigned char *pucCh);
unsigned char HALLCD_CheckDateValid(unsigned char *pucDate);
unsigned char HALLCD_Display1212DotInCol(unsigned char ucFlag,unsigned char ucY,unsigned char ucX,unsigned char ucCh);
unsigned char HALLCD_ClearLine1212(unsigned char ucLineNb);

//HALLCDDATA volatile unsigned char ucLCDUsedFlag;

#ifdef __cplusplus
}
#endif


#endif
