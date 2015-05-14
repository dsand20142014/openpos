#ifndef __SKB25_LCD__
#define SKB25_LCD extern
#else
#define SKB25_LCD
#endif



SKB25_LCD int OpenFontFile(void);
SKB25_LCD void LCD_Init();
SKB25_LCD void LCD_ClearOneLine(int Row,int Col,char SorD,char WorB);
SKB25_LCD void LCD_Clear(char WorB);
SKB25_LCD void LCD_DisplayOneLine( int Row,int Col,unsigned char str[],char SorD,char WorB,int ClrFlag );
SKB25_LCD int LCD_DisplayOneChr(int Row,int Col,unsigned char Hch,unsigned char Lch,char SorD,char WorB);






