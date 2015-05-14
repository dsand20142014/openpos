#ifndef __SAND_LCD_H__
#define __SAND_LCD_H__

#ifdef __cplusplus 
extern "C" { 
#endif

#define CMS2753_MAGIC 'C'
#define TEXTLEN 100

struct message_data {

    int api_id;

    int line;
    int colum;

    int gprs_d;
    int gprs_s;
    int modem_dial;
    int r1;
    /*int r2;
    int r3;
    int r4;*/

	int widget_type;
	int widget_key;
	int widget_info;
	
    char mtext[TEXTLEN];


};


struct lcd_message {

    long  mtype;
    struct message_data msg_data;

};


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
void Os__display(unsigned char line, unsigned char column, char *Text);
int Os__GB2312_disp(unsigned char Line, unsigned char * Text);
int Os__GB2312_display(unsigned char line, unsigned char column, unsigned char *Text);
int Os__graph_display(unsigned char line, unsigned char column, unsigned char *Drawing, unsigned char size);
unsigned char *Os__conv_ascii_GB2312(unsigned char *ptAscii);

int OSMMI_DisplayASCII (unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char * Text);

void OSMMI_ClrDisplay(unsigned char ucFont, unsigned char line);

int Os__GB2312_display_pp (unsigned char   Line, unsigned char   Column, unsigned char   * Text);
void Os__clr_display_pp (unsigned char Line);

unsigned char Os__light_on_pp(void);
unsigned char Os__light_off_pp(void);
void Os__display_pp(unsigned char  line, unsigned char  col, unsigned char  *text);
void Os__lcd_test();
void Os__lcd_image(int x,int y,char *file);
void Os__lcd_modem_on_off(char flag);

/*
 * name: gif file name. Os__gif_start() will show original gif size when (w,h)=(0,0) 
 * text: string 
 * textPosition = 1:text above gif  2:text on the righ 3:text under gif  4:text on the left
 */
void Os__gif_start(char *name, int x, int y, int w, int h); 
void Os__gif_text_auto_start(char *name, char *text, int textPosition, char *style);
void Os__gif_stop(void);

void Os__socket_client(char *content);
/*
 * @brief Show pin form and get pin key.
 * @author xiangliu 20130513
 * @param text The text show on screen
 * @param pin The pin key 
 * @param timeout timeout time(s)
 * @return lenth(>=0) of pin when success, <0 when timeout or error.
 * eg.
	int ret = 0;
	char cpin[10];

	ret = Os__get_PIN("请输入密码", cpin, 20);
	if ( ret < 0) {
	    //出错处理
	}
 */
int Os__get_PIN(char *text, char *pin, unsigned short timeout);

void lcd_init();
void lcd_exit();


#ifdef __cplusplus 
}
#endif

#endif
