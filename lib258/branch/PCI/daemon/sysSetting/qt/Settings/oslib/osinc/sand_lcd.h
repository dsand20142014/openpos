#ifndef __ZWJ_LCD__
#define __ZWJ_LCD__

#define CMS2753_MAGIC 'C'

struct display_struct
{
        unsigned char line;
        unsigned char column;
        unsigned char *pText;
        unsigned char text_len;
};

struct graph_display_struct
{
        unsigned char line;
        unsigned char column;
        unsigned char *Drawing;
        unsigned char size;
};

#define LCD_CLR_DISPLAY 	_IOW(CMS2753_MAGIC,1,unsigned char)
#define LCD_DISPLAY			_IOW(CMS2753_MAGIC,2,struct display_struct)
#define LCD_LIGHT_ON  		_IO(CMS2753_MAGIC,3)
#define LCD_LIGHT_OFF		_IO(CMS2753_MAGIC,4)
#define LCD_GB2312_DISP	_IOW(CMS2753_MAGIC,5,struct graph_display_struct)
#define LCD_GRAPH_DISPLAY	_IOW(CMS2753_MAGIC,6,struct graph_display_struct)
#define LCD_SET_CONTRAST	_IOW(CMS2753_MAGIC,7,unsigned char)
#define LCD_CLR_ONE_LINE    	_IOW(CMS2753_MAGIC,8,unsigned char)
#define LCD_DISPLAY_8X6    	_IOW(CMS2753_MAGIC,9,struct display_struct)

void Os__clr_display(unsigned char line);
//void Os__display(unsigned char line, unsigned char column, char *Text);
unsigned char Os__light_on(void);
unsigned char Os__light_off(void);
unsigned char Os__GB2312_disp(unsigned char Line, unsigned char * Text);
unsigned char Os__GB2312_display(unsigned char line, unsigned char column, unsigned char *Text);
unsigned char Os__graph_display(unsigned char line, unsigned char column, unsigned char *Drawing, unsigned char size);
//unsigned char *Os__conv_ascii_GB2312(unsigned char *ptAscii);

unsigned char OSMMI_DisplayASCII (unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char * Text);

void OSMMI_ClrDisplay(unsigned char ucFont, unsigned char line);

unsigned char  Os__GB2312_display_pp (unsigned char   Line, unsigned char   Column, unsigned char   * Text);
void Os__clr_display_pp (unsigned char Line);

unsigned char Os__light_on_pp(void);
unsigned char Os__light_off_pp(void);
void    Os__display_pp(unsigned char  line, unsigned char  col, unsigned char  *text);
void Os__lcd_test();
void Os__lcd_image(int x,int y,char *file);
void Os__lcd_modem_on_off(char flag);
void Os__lcd_bright(unsigned char bright);

void lcd_init();

#endif
