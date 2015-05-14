#ifndef OSDISPLAY_H_INCLUDE
#define OSDISPLAY_H_INCLUDE

/**
 * @defgroup 系统显示函数
 * @author 熊剑
 * @brief 支持在点阵屏和TFT屏显示中英文和图像
 *
 * 点阵屏示例代码
 * @code
 * int main()
 * {
 * int i;
 * //银联图标
 * unsigned char  Drawing_1_1[32] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x00,0x7F,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0x9C,0x03,0x9C,0x03,0xBC};
 * unsigned char  Drawing_1_2[32] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x7F,0xFF,0xFF,0x83,0x70};
 * unsigned char  Drawing_1_3[32] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x81,0xFF,0x99,0xC1,0x99};
 * unsigned char  Drawing_1_4[32] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xF8,0xFF,0xFC,0xFF,0xFE,0xFF,0xFE,0xFF,0xFE,0xFF,0xFC,0xFF,0xFC,0xFF,0xFC,0xFF,0xFC,0xFF,0xFC,0x0D,0xC8};
 * unsigned char  Drawing_2_1[32] = { 0x03,0xBC,0x03,0xBD,0x03,0x3D,0x07,0x39,0x07,0x39,0x07,0x33,0x07,0x83,0x07,0xFF,0x07,0xFF,0x0F,0xFF,0x0F,0xFF,0x0F,0xFF,0x0F,0xFF,0x1F,0xFF,0x1F,0xFF,0x1F,0xFF};
 * unsigned char  Drawing_2_2[32] = { 0x9B,0x64,0xBB,0x4E,0xBA,0x4E,0x3A,0x5E,0x32,0x5C,0x32,0x4D,0x36,0xE1,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xCC,0xFF,0x9C,0xFF,0x80,0xFF,0x3C,0xFE,0x05,0xFF,0xDC};
 * unsigned char  Drawing_2_3[32] = { 0xC9,0xB9,0x9D,0xB3,0x99,0x87,0x99,0x3E,0xB9,0x3E,0xB9,0x3E,0xBB,0x3E,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x06,0x00,0x06,0x02,0xEF,0x69,0x0F,0x08,0xCF,0x4E,0x0E,0x4E};
 * unsigned char  Drawing_2_4[32] = { 0x64,0xD8,0xE4,0x98,0x0C,0xB8,0x6C,0x38,0xEE,0x70,0xCE,0x70,0x0E,0xF0,0xFE,0xF0,0xFC,0xF0,0xDD,0xF0,0xDF,0xE0,0x9F,0xE0,0x2F,0xE0,0x0F,0xE0,0x7F,0xC0,0x7F,0xC0};
 * unsigned char  Drawing_3_1[32] = { 0x1F,0xFF,0x1F,0xFF,0x3F,0xFF,0x3F,0xFF,0x3F,0xFF,0x7F,0xFF,0x7F,0xFF,0x7F,0xFF,0x3F,0xFF,0x1F,0xFF,0x0F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 * unsigned char  Drawing_3_2[32] = { 0xFF,0x18,0xFF,0x01,0xFF,0x8B,0xFF,0xA3,0xFF,0x88,0xFF,0x10,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 * unsigned char  Drawing_3_3[32] = { 0x7E,0x46,0x4E,0x10,0x3E,0xCC,0x3E,0x08,0x1C,0x9A,0x8F,0x83,0xFF,0x87,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 * unsigned char  Drawing_3_4[32] = { 0x7F,0xC0,0x1F,0xC0,0xFF,0xC0,0x7F,0xC0,0x3F,0x80,0x1F,0x80,0xBF,0x80,0xFF,0x80,0xFF,0x00,0xFE,0x00,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 * //清屏
 * Os__clr_display_imx(255);
 * //显示银联标识
 * OSMMI_GraphDisplay_imx(1,  0,  0, Drawing_1_1, 32);
 * OSMMI_GraphDisplay_imx(1,  0, 16, Drawing_1_2, 32);
 * OSMMI_GraphDisplay_imx(1,  0, 32, Drawing_1_3, 32);
 * OSMMI_GraphDisplay_imx(1,  0, 48, Drawing_1_4, 32);
 * OSMMI_GraphDisplay_imx(1, 16,  0, Drawing_2_1, 32);
 * OSMMI_GraphDisplay_imx(1, 16, 16, Drawing_2_2, 32);
 * OSMMI_GraphDisplay_imx(1, 16, 32, Drawing_2_3, 32);
 * OSMMI_GraphDisplay_imx(1, 16, 48, Drawing_2_4, 32);
 * OSMMI_GraphDisplay_imx(1, 32,  0, Drawing_3_1, 32);
 * OSMMI_GraphDisplay_imx(1, 32, 16, Drawing_3_2, 32);
 * OSMMI_GraphDisplay_imx(1, 32, 32, Drawing_3_3, 32);
 * OSMMI_GraphDisplay_imx(1, 32, 48, Drawing_3_4, 32);
 * sleep(2);
 * //清屏
 * OSMMI_ClrDisplay_imx(0x30,255);
 * //显示银联标识
 * Os__graph_display_imx( 0,  0,  Drawing_1_1, 32);
 * Os__graph_display_imx( 0, 16,  Drawing_1_2, 32);
 * Os__graph_display_imx( 0, 32,  Drawing_1_3, 32);
 * Os__graph_display_imx( 0, 48,  Drawing_1_4, 32);
 * Os__graph_display_imx(16,  0,  Drawing_2_1, 32);
 * Os__graph_display_imx(16, 16,  Drawing_2_2, 32);
 * Os__graph_display_imx(16, 32,  Drawing_2_3, 32);
 * Os__graph_display_imx(16, 48,  Drawing_2_4, 32);
 * Os__graph_display_imx(32,  0,  Drawing_3_1, 32);
 * Os__graph_display_imx(32, 16,  Drawing_3_2, 32);
 * Os__graph_display_imx(32, 32,  Drawing_3_3, 32);
 * Os__graph_display_imx(32, 48,  Drawing_3_4, 32);
 * sleep(2);
 * //清屏
 * _Os__clearDZ(0x31,255);
 * //显示银联图标
 * _Os__graphDZ( 0,  0, Drawing_1_1, 32);
 * _Os__graphDZ( 0, 16, Drawing_1_2, 32);
 * _Os__graphDZ( 0, 32, Drawing_1_3, 32);
 * _Os__graphDZ( 0, 48, Drawing_1_4, 32);
 * _Os__graphDZ(16,  0,  Drawing_2_1, 32);
 * _Os__graphDZ(16, 16,  Drawing_2_2, 32);
 * _Os__graphDZ(16, 32,  Drawing_2_3, 32);
 * _Os__graphDZ(16, 48,  Drawing_2_4, 32);
 * _Os__graphDZ(32,  0,  Drawing_3_1, 32);
 * _Os__graphDZ(32, 16,  Drawing_3_2, 32);
 * _Os__graphDZ(32, 32,  Drawing_3_3, 32);
 * _Os__graphDZ(32, 48,  Drawing_3_4, 32);
 * sleep(2);
 * //清屏
 * Os__clrDisplay(0x31,255);
 * Os_graphDisplay( 0,  0, Drawing_1_1, 32);
 * Os_graphDisplay( 0, 16, Drawing_1_2, 32);
 * Os_graphDisplay( 0, 32, Drawing_1_3, 32);
 * Os_graphDisplay( 0, 48, Drawing_1_4, 32);
 * Os_graphDisplay(16,  0,  Drawing_2_1, 32);
 * Os_graphDisplay(16, 16,  Drawing_2_2, 32);
 * Os_graphDisplay(16, 32,  Drawing_2_3, 32);
 * Os_graphDisplay(16, 48,  Drawing_2_4, 32);
 * Os_graphDisplay(32,  0,  Drawing_3_1, 32);
 * Os_graphDisplay(32, 16,  Drawing_3_2, 32);
 * Os_graphDisplay(32, 32,  Drawing_3_3, 32);
 * Os_graphDisplay(32, 48,  Drawing_3_4, 32);
 * sleep(2);
 * //整屏清除
 * _Os__clearDZ(0x30,255);
 * _Os__displayDZ(0x31,0,0,"上海杉德金卡有限公司");
 * _Os__displayDZ(0x31,1,0,"ABCDEFGHIJKLMNOP");
 * _Os__displayDZ(0x31,2,0,"QRSTUVWXYZ");
 * _Os__displayDZ(0x31,3,0,"中国银联");
 * sleep(2);
 * _Os__clearDZ(0x31,255);
 * Os__display_ascii5x7(0,0,'A');
 * Os__display_ascii(0,2,'B');
 * Os__display_zi_imx(3,0,"杉");
 * sleep(2);
 * for(i=0;i<8;i++)
 * {
 *  _Os__displayDZ(0x30,i,0,"ABCDEFGHIJKLMNOPQRSTU"); 
 * }
 * sleep(2);
 * for(i=0;i<8;i++)
 * {
 *  _Os__clearDZ(0x30,i);
 * 	sleep(1);
 * }
 * for(i=0;i<8;i++)
 * {
 * 	Os__display5x7(0,i,"ABCDEFGHIJKLMNOPQRSTU");
 * }
 * _Os__clearDZ(0x31,255);
 * for(i=0;i<8;i++)
 * {
 * 	Os__disPlay(0x30,i,0,"ABCDEFGHIJKLMNOPQRSTU");
 * }
 * sleep(2);
 * Os__clrDisplay(0x31,255);
 * for(i=0;i<4;i++)
 * {
 * Os__disPlay(0x31,i,0,"上海杉德信息技术");
 * }
 * return 0;
 * }
 * @endcode
 * 
 * TFT屏示例代码
 * @code
 * int main(void)
 * {	
 * int i;
 * //初始化TFT屏
 * lcd_init();
 * //清屏
 * _Os__clearTFT(0x30,255);
 * //显示信息
 * _Os__displayTFT(0x30,0,0,"ABCDEFGHIGKMLNOPQRSTU");
 * _Os__displayTFT(0x30,1,0,"MMMMMMMMMMMMMMMMMMMMM");
 * _Os__displayTFT(0x30,2,0,",<>?:;'\"\\/[]{}!~`");
 * _Os__displayTFT(0x30,3,0,"杉德金卡研发二部杉德");
 * _Os__displayTFT(0x30,4,0,"，。《》？/、！～·");
 * _Os__displayTFT(0x30,5,0,"ABCDEFGH杉德金卡研发");
 * _Os__displayTFT(0x30,6,0,"ABCDEFGH杉德金卡研发");
 * _Os__displayTFT(0x30,7,0,"ABCDEFGH杉德金卡研发");
 * sleep(3);
 * //清屏
 * for(i=0;i<8;i++)
 * {
 * 	_Os__clearTFT(0x30,i);
 * }
 * Os__display(0,0,"杉德金卡信息技术");
 * sleep(2);
 * Os__clr_display(0);
 * OSMMI_DisplayASCII(0x30,0,0,"上海杉德信息技术公司");
 * sleep(2);
 * OSMMI_ClrDisplay(0x30,0);
 * for(i=0;i<5;i++)
 * {
 * 	Os__disPlay(0x31,i,0,"杉德信息技术公司");
 * }
 * sleep(2);
 * Os__clrDisplay(0x30,255);
 * //显示图片,图片名称为logo.jpg
 * //需要先进图片保存到/mnt/pos/app/lcd/目录下
 * Os__lcd_image(159,0,"logo.jpg");
 * sleep(2);
 * Os__clrDisplay(0x30,255);
 * Os_graphDisplay(159,0,"1.jpg",10);
 * return 0;
 * }
 * @endcode
 * @{
 */

/**
 * @def OSDISPLAY_OK
 * @brief 对屏幕显示清除字符图像成功
 **/
#define OSDISPLAY_OK				0
/**
 * @def OSDISPLAY_OPENDEVICEERR
 * @brief 打开屏幕设备出错
 **/
#define OSDISPLAY_OPENDEVICEERR		-1
/**
 * @def OSDISPLAY_MESSAGESENDERR
 * @brief 发送消息队列失败
 **/
#define OSDISPLAY_MESSAGESENDERR	-2
/**
 * @def OSDISPLAY_DISPLAYBEYOND
 * @brief 字符显示位置超出范围
 **/
#define OSDISPLAY_DISPLAYBEYOND		-3
/**
 * @def OSDISPLAY_TEXTISNULL
 * @brief 要显示的文本或图片内容为NULL
 **/
#define OSDISPLAY_TEXTISNULL        -4
/**
 * @def OSDISPLAY_OPENFONTERR
 * @brief 打开或读取字体库错误
 **/
#define OSDISPLAY_OPENFONTERR       -5
/**
 * @def OSDISPLAY_FONTERROR
 * @brief 字体不支持或错误
 **/
#define OSDISPLAY_FONTERROR			-6
 
/****************************FOR IMX258 DECLARE START*************************************/

/**
 * @fn void Os__display_ascii5x7(int x, int y, char c)
 * @brief 在终端上显示一个ASCII字符,字体宽*高=5*7,只用于IMX258
 *
 * @param x   信息显示所在的行号
 * @param y 信息显示所在的列号
 * @param c   所要显示的信息字符
 *
 * @return 无返回值
 *
 */
void Os__display_ascii5x7(int x, int y, char c);

/**
 * @fn void Os__display_ascii(int x, int y, char c)
 * @brief 在终端上显示一个ASCII字符,只用于IMX258
 *
 * @param x   信息显示所在的行号
 * @param y 信息显示所在的列号
 * @param c   所要显示的信息字符
 *
 * @return 无返回值
 *
 */
void Os__display_ascii(int x, int y, char c);

/**
 * @fn int Os__graph_display_imx(unsigned char line,unsigned char column,unsigned char *Drawing,unsigned char size)
 * @brief 在终端上进行图形显示(仅使用于图形终端),只用于IMX258
 *
 * @param line		图形显示的行号
 * @param column	图形显示的列号
 * @param Drawing	所要显示图像的首地址
 * @param size		图形大小
 *
 * @return 成功返回0,设备打开失败返回-1
 *
 */
int Os__graph_display_imx(unsigned char line,unsigned char column,unsigned char *Drawing,unsigned char size);

/**
 * @fn int Os__display_zi_imx(int x, int y, char *s)
 * @brief 在终端上显示汉字信息,只用于IMX258
 *
 * @param x   信息显示所在的行号
 * @param y 信息显示所在的列号
 * @param s   所要显示的信息的首地址
 *
 * @return 成功返回0,设备打开失败返回-1
 *
 */
int Os__display_zi_imx(int x, int y, char *s);

/**
 * @fn int Os__display_graph(int x, int y, char *s)
 * @brief 在终端上进行图形显示(仅使用于图形终端),只用于IMX258
 *
 * @param x		图形显示的行号
 * @param y		图形显示的列号
 * @param s		所要显示图像的首地址
 *
 * @return 成功返回0,设备打开失败返回-1
 *
 */
int Os__display_graph(int x, int y, char *s);

/**
 * @fn void _Os__clr_display(unsigned char line)
 * @brief 清除终端的指定行号,只用于IMX258
 *
 * @param line	要清除的行号,当line为255时,整屏清除
 *
 * @return 无返回类型
 *
 */
void _Os__clr_display(unsigned char line);

/**
 * @fn unsigned char Os__GB2312_display_imx(unsigned char column,unsigned char line,unsigned char *Text)
 * @brief 在终端上显示一串中文信息,只用于IMX258
 *
 * @param line   信息显示所在的行号
 * @param column 信息显示所在的列号
 * @param Text   所要显示的信息的首地址
 *
 * @return 成功返回0,失败返回非0
 *
 * @warning 显示文本信息,以行“Line”为起始端显示;
 *			终端显示国标码将占两行;
 *			line = 0是从起始开始显示.
 *
 */
unsigned char Os__GB2312_display_imx(unsigned char column,unsigned char line,unsigned char *Text);

/**
 * @fn unsigned char Os__display_imx(unsigned char Line, unsigned char Column, unsigned char *Text)
 * @brief 在终端上显示信息,从第一行至第三行,一行最多显示16个字符,只用于IMX258
 *
 * @param Line   信息显示所在的行号
 * @param Column 信息显示所在的列号
 * @param Text   所要显示的信息的首地址
 *
 * @return 无返回值
 *
 */
unsigned char Os__display_imx(unsigned char Line, unsigned char Column, unsigned char *Text);

/**
 * @fn unsigned char OSMMI_GB2312Display_imx(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr)
 * @brief 在终端上显示一串中文信息(仅适用于图形终端),只用于IMX258
 *
 * @param ucFont	字体类型
 * @param ucRow		信息显示所在的行号
 * @param ucCol 	信息显示所在的列号
 * @param pucPtr	所要显示的信息的首地址
 *
 * @return 成功返回0,失败返回非0
 *
 * @warning 显示文本信息,以行“Line”为起始端显示;
 *			终端显示国标码将占两行;
 *			line = 0是从起始开始显示.
 *
 */
unsigned char OSMMI_GB2312Display_imx(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr);

/**
 * @fn void Os__display5x7(unsigned line, unsigned column, char *Text)
 * @brief 在终端上显示信息,字体宽*高=5*7,只用于IMX258
 *
 * @param Line   信息显示所在的行号
 * @param Column 信息显示所在的列号
 * @param Text   所要显示的信息的首地址
 *
 * @return 无返回值
 *
 */
void Os__display5x7(unsigned line, unsigned column, char *Text);

/**
 * @fn int Os__light_on_imx(void)
 * @brief 打开显示背光开关,背光对显示无任何影响,只用于IMX258
 *
 * @param 无参数
 *
 * @return 成功返回0,打开设备失败返回-1
 *
 */
int Os__light_on_imx(void);

/**
 * @fn int Os__light_off_imx(void)
 * @brief 关闭显示背光开关,背光对显示无任何影响,只用于IMX258
 *
 * @param 无参数
 *
 * @return 成功返回0,打开设备失败返回-1
 *
 */
int Os__light_off_imx(void);

/**
 * @fn void Os__clr_display_imx(unsigned char line)
 * @brief 清除一个或EFT终端一行或多行显示,只用于IMX258
 *
 * @param line   所以清除的行号
 *
 * @return 无返回值
 *
 */
void Os__clr_display_imx(unsigned char line);

/**
 * @fn unsigned char OSMMI_GraphDisplay_imx(unsigned char ucType, unsigned char ucRow, unsigned char ucCol,unsigned char *pucPtr,unsigned char ucLen)
 * @brief 在终端上进行图形显示(仅使用于图形终端),只用于IMX258
 *
 * @param ucType 	字符类型
 * @param ucRow		图形显示的行号
 * @param ucCol		图形显示的列号
 * @param pucPtr	所要显示图像的首地址
 * @param ucLen		图形大小
 *
 * @return 成功返回0,失败返回非0
 *
 */
unsigned char OSMMI_GraphDisplay_imx(unsigned char ucType, unsigned char ucRow, unsigned char ucCol,unsigned char *pucPtr,unsigned char ucLen);

/**
 * @fn unsigned char OSMMI_DisplayASCII_imx (unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * Text)
 * @brief 在终端上显示ASCII码,只用于IMX258
 *
 * @param ucFont 字符类型
 * @param ucRow	图形显示的行号
 * @param ucCol	图形显示的列号
 * @param Text	要显示的信息的首地址
 *
 * @return 成功返回0,失败返回非0
 *
 */
unsigned char OSMMI_DisplayASCII_imx (unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * Text);

/**
 * @fn void OSMMI_ClrDisplay_imx(unsigned char ucFont, unsigned char line)
 * @brief 清除终端的指定行号,只用于IMX258
 *
 * @param ucFont 字符类型
 * @param line	要清除的行号,当line为255时,整屏清除
 *
 * @return 无返回类型
 *
 */
void OSMMI_ClrDisplay_imx(unsigned char ucFont, unsigned char line);
/****************************FOR IMX258 DECLARE END*************************************/

/**
 * @fn int Os__display_lcd(unsigned char line,unsigned char column,char *Text)
 * @brief 在终端上显示信息,从第一行至第三行,一行最多显示16个字符
 *
 * @param line   信息显示所在的行号
 * @param column 信息显示所在的列号
 * @param Text   所要显示的信息的首地址
 *
 * @return 成功返回0,打开设备错误返回-1,显示超出范围返回-3
 *
 */
int Os__display_lcd(unsigned char line,unsigned char column,char *Text);

/**
 * @fn int Os__light_on(void)
 * @brief 打开显示背光开关,背光对显示无任何影响
 *
 * @param 无参数
 *
 * @return 成功返回0,打开设备失败返回-1
 *
 */
int Os__light_on(void);

/**
 * @fn int Os__light_off(void)
 * @brief 关闭显示背光开关,背光对显示无任何影响
 *
 * @param 无参数
 *
 * @return 成功返回0,打开设备失败返回-1
 *
 */
int Os__light_off(void);

/**
 * @fn void Os__clr_display_lcd(unsigned char line)
 * @brief 清除一个或EFT终端一行或多行显示
 *
 * @param line 所要清除的行号
 *
 * @return 无返回值
 *
 *
 */
void Os__clr_display_lcd(unsigned char line);

/**
 * @fn int Os__GB2312_display_lcd(unsigned char line,unsigned char column,unsigned char *Text)
 * @brief 在终端上显示一串中文信息
 *
 * @param line   信息显示所在的行号
 * @param column 信息显示所在的列号
 * @param Text   所要显示的信息的首地址
 *
 * @return 成功返回0,显示越界返回-3
 *
 * @warning 显示文本信息,以行“Line”为起始端显示;
 *			终端显示国标码将占两行;
 *			line = 0是从起始开始显示.
 *
 */
int Os__GB2312_display_lcd(unsigned char line,unsigned char column,unsigned char *Text);

/**
 * @fn void Os__set_contrast(unsigned char value)
 * @brief ?
 *
 * @param value 
 *
 * @return 无返回值
 *
 *
 */
void Os__set_contrast(unsigned char value);

/**
 * @fn void OSMMI_clr_Display(unsigned char ucFont,unsigned char line)
 * @brief 清除一个或EFT终端一行或多行显示.
 *
 * @param ucFont 字体类型
 * @param line   所要清除的行号
 *
 * @return 无返回值
 *
 * @warning ucFont为0x30或0x31,
 *			当为0x30时,行号显示为0到7,一次清除一行
 *			当为0x31时,行号显示为0到3,一次清除两行
 *			首行的数陈列是0,当代入为255,则将清除所有行.
 */
void OSMMI_clr_Display(unsigned char ucFont,unsigned char line);

/**
 * @fn int OSMMI_DisplayASCII_lcd(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * Text)
 * @brief 用ASCII字符在终端显示信息
 *
 * @param ucFont	字体类型
 * @param ucRow		显示ASCII信息所在的行号
 * @param ucCol		显示ASCII信息所在的列号
 * @param Text		所要显示的信息的首地址
 *
 * @return 成功返回0,设备打开失败返回-1,显示超出范围返回-3,字体错误或不支持返回-6
 *
 * @warning 当在行号前显示信息时,ucFont为0x30或0x31;
 *			当为0x30时,行号为0到7,8*6点阵将被显示一行最多有21个字符;
 *			当为0x31时,行号为0到3,8*16点阵将被显示,一行最多有16个字符.
 *			ucRow = 0 为首行显示.
 */
int OSMMI_DisplayASCII_lcd(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * Text);

/**
 * @fn int Os__graph_display_lcd(unsigned char line,unsigned char column,unsigned char *Drawing,unsigned char size)
 * @brief 在终端上进行图形显示(仅使用于图形终端)
 *
 * @param line		图形显示的行号
 * @param column	图形显示的列号
 * @param Drawing	所要显示图像的首地址
 * @param size		图形大小,定义小于32位图形将完全显示
 *
 * @return 成功返回0,设备打开失败显示-1,显示越界返回-3
 *
 * @warning 在终端按规定的行列,显示Drawing定义的图形;
 *			1为黑色像素 ，0为白色像素,像素按8位bit为一个byte进行编码,
 *			图形按16*16点阵进行画线,
 *			1st byte = 左边第一条线,
 *			2nd byte = 右边部分第一条线,
 *			…,
 *			31st byte = 左边部分第16条线,
 *			32nd byte = 右边部分第16条线,
 *			终端显示也可按2行7个字符16*16点阵进行显示;
 *			Line = 0 是第一行显示,
 *			Column = 0 是第一列显示,
 *			'Size' 是
 */
int Os__graph_display_lcd(unsigned char line,unsigned char column,unsigned char *Drawing,unsigned char size);

void Os__lcd_bright(unsigned char bright);

/**********************************FOR TFT-LCD SCREEN START************************************************************/

/**
 * @fn void lcd_init(void)
 * @brief 初始化LCD,适用于TFT屏
 *
 * @param 无参数
 *
 * @return 无返回值
 *
 */
void lcd_init(void);

/**
 * @fn int Os__GB2312_disp(unsigned char ucRow, unsigned char *pucPtr)
 * @brief 在终端上显示一串中文信息,只用于IMX258
 *
 * @param ucRow   信息显示所在的行号
 * @param pucPtr   所要显示的信息的首地址
 *
 * @return 成功返回0,消息队列发送失败返回-2,显示信息为NULL返回-4
 *
 * @warning 显示文本信息,以行“Line”为起始端显示;
 *			终端显示国标码将占两行;
 *			line = 0是从起始开始显示.
 *
 */
int Os__GB2312_disp(unsigned char ucRow, unsigned char *pucPtr);

/**
 * @fn int Os__GB2312_display(unsigned char line, unsigned char column, unsigned char *text)
 * @brief 在终端上显示一串中文信息
 *
 * @param line   信息显示所在的行号
 * @param column 信息显示所在的列号
 * @param text   所要显示的信息的首地址
 *
 * @return 成功返回0,消息队列发送失败返回-2,显示信息为NULL返回-4
 *
 * @warning 显示文本信息,以行“Line”为起始端显示;
 *			终端显示国标码将占两行;
 *			line = 0是从起始开始显示.
 *
 */
int Os__GB2312_display(unsigned char line, unsigned char column, unsigned char *text);

/**
 * @fn int Os__display(unsigned line, unsigned colum, char *text)
 * @brief 在终端上显示信息,从第一行至第三行,一行最多显示16个字符
 *
 * @param line   信息显示所在的行号
 * @param column 信息显示所在的列号
 * @param text   所要显示的信息的首地址
 *
 * @return 成功返回0,消息队列发送失败返回-2,显示信息为NULL返回-4
 *
 */
int Os__display(unsigned line, unsigned colum, char *text);

/**
 * @fn void Os__clr_display(unsigned char line)
 * @brief 清除终端的指定行号
 *
 * @param line	要清除的行号,当line为255时,整屏清除
 *
 * @return 无返回类型
 *
 */
void Os__clr_display(unsigned char line);


/**
 * @fn int Os__graph_display(unsigned line, unsigned char colum, unsigned char *drawing, unsigned char size)
 * @brief 在终端指定位置显示图形
 *
 * @param line		显示图形文件所在的行
 * @param column	显示图形文件所在的列
 * @param file		图片名称
 *
 * @return 成功返回0,消息队列发送失败返回-2,图片不存在返回-4
 * @warning	只是用于TFT屏幕,Drawing为图片名称,且图片必须保存在/mnt/pos/app/lcd/目录下.
 */
int Os__graph_display(unsigned line, unsigned char colum, unsigned char *drawing, unsigned char size);

/**
 * @fn int OSMMI_DisplayASCII(unsigned char ucFont, unsigned char line, unsigned char colum, unsigned char * text)
 * @brief 用ASCII字符在终端显示信息
 *
 * @param ucFont	字体类型
 * @param line		显示ASCII信息所在的行号
 * @param colum		显示ASCII信息所在的列号
 * @param text		所要显示的信息的首地址
 *
 * @return 成功返回0,消息队列发送失败返回-2,显示信息为NULL返回-4
 *
 * @warning 当在行号前显示信息时,ucFont为0x30,为其他值时同样显示小字体
 *			当为0x30时,行号为0到7,8*6点阵将被显示一行最多有21个字符;
 *			ucRow = 0 为首行显示.
 */
int OSMMI_DisplayASCII(unsigned char ucFont, unsigned char line, unsigned char colum, unsigned char * text);

/**
 * @fn int OSMMI_ClrDisplay(unsigned char ucFont, unsigned char line)
 * @brief 清除终端的指定行号
 *
 * @param ucFont 字符类型
 * @param line	要清除的行号,当line为255时,整屏清除
 *
 * @return 成功返回0,失败返回-2
 *
 */
int OSMMI_ClrDisplay(unsigned char ucFont, unsigned char line);


/**
 * @fn int Os__lcd_image(int line,int column,char *file)
 * @brief 在终端指定位置显示图形
 *
 * @param line		显示图形文件所在的行
 * @param column	显示图形文件所在的列
 * @param file		图形文件首地址
 *
 * @return 成功返回0,消息队列发送失败返回-2,图片不存在返回-4
 * @warning	只是用于TFT屏幕,Drawing为图片名称,且图片必须保存在/mnt/pos/app/lcd/目录下.
 */
int Os__lcd_image(int line,int column,char *file);
/**********************************FOR TFT-LCD SCREEN END************************************************************/

/**
 * @fn int Os__disPlay(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text)
 * @brief 在终端上显示信息,跟据所使用的机型和字体类型显示,支持在全机型下的中文和英文显示.
 *
 * @param ucFont 显示信息的字体
 * @param line   显示信息所在的行号
 * @param column 显示信息所在的列号
 * @param Text   所要显示信息的首地址
 *
 * @return 成功返回0,消息队列发送失败返回-2,超出显示范围返回-3,显示信息为NULL返回-4
 *
 */
int Os__disPlay(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text);

/**
 * @fn void Os__clrDisplay(unsigned char ucFont,unsigned char Line)
 * @brief 清除一个或EFT终端一行或多行显示,支持在全机型下清除中文和英文.
 *
 * @param ucFont 字体类型
 * @param line   所要清除的行号
 *
 * @return 无返回值
 *
 */
void Os__clrDisplay(unsigned char ucFont,unsigned char Line);

/**
 * @fn int Os_graphDisplay(unsigned Line, unsigned char Colum, unsigned char *Drawing, unsigned char Size)
 * @brief 在终端上进行图形显示(仅使用于图形终端),函数支持所有机型.
 *
 * @param line		图形显示的行号
 * @param column	图形显示的列号
 * @param drawing	所要显示图像的首地址
 * @param size		图形大小,定义小于32位图形将完全显示
 *
 * @return 成功返回0,消息队列发送失败返回-2,TFT屏图片不错在时返回-4
 *
 * @warning 在终端按规定的行列,显示Drawing定义的图形;
 *			1为黑色像素 ，0为白色像素,像素按8位bit为一个byte进行编码,
 *			图形按16*16点阵进行画线,
 *			1st byte = 左边第一条线,
 *			2nd byte = 右边部分第一条线,
 *			…,
 *			31st byte = 左边部分第16条线,
 *			32nd byte = 右边部分第16条线,
 *			终端显示也可按2行7个字符16*16点阵进行显示;
 *			Line = 0 是第一行显示,
 *			Column = 0 是第一列显示,
 *			对于TFT屏幕,Drawing为图片名称,且图片必须保存在/mnt/pos/app/lcd/目录下,
 *			对于点阵屏,只能使用图形矩阵.
 */
int Os_graphDisplay(unsigned Line, unsigned char Colum, unsigned char *Drawing, unsigned char Size);


int _Os__graphDZ(unsigned char Line, unsigned char Column, unsigned char *Drawing, unsigned char Size);

/**
 * @fn void _Os__clearDZ(unsigned char ucFont, unsigned char Line)
 * @brief 清除终端屏幕上的内容,用于点阵屏幕
 *
 * @param ucFont 字体类型,字体类型不同每行的行高不同
 * @param Line	 要清除的行的行号,当line为255时,整屏清除
 *
 * @return 无返回类型
 *
 */
void _Os__clearDZ(unsigned char ucFont, unsigned char Line);

/**
 * @fn int _Os__displayDZ(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text)
 * @brief 在终端上显示信息,从第一行至第三行,一行最多显示16个字符
 *
 * @param ucFont 字体类型0x30为小字体,0x31为大字体
 * @param Line   信息显示所在的行号
 * @param Column 信息显示所在的列号
 * @param Text   所要显示的信息的首地址
 *
 * @return  成功返回0,消息队列发送失败返回-2,超出显示范围返回-3,显示信息为NULL返回-4
 * @warning 小字体(0x30),可显示8行,每行最多21个字符,不支持中文显示;
 *			大字体(0x31),默认字体,可显示5行,每行最多16个字符.
 *
 */
int _Os__displayDZ(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text);

/**
 * @fn void _Os__clearTFT(unsigned char ucFont, unsigned char Line)
 * @brief 清除终端屏幕上的内容
 *
 * @param ucFont 字体类型,字体类型不同每行的行高不同
 * @param Line	 要清除的行的行号,当Line为255时,整屏清除
 *
 * @return 无返回类型
 *
 */
void _Os__clearTFT(unsigned char ucFont, unsigned char Line);

/**
 * @fn int _Os__displayTFT(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text)
 * @brief 在终端上显示信息,从第一行至第三行,一行最多显示16个字符
 *
 * @param ucFont 字体类型0x30为小字体,0x31为大字体
 * @param Line   信息显示所在的行号
 * @param Column 信息显示所在的列号
 * @param Text   所要显示的信息的首地址
 *
 * @return  成功返回0,消息队列发送失败返回-2,超出显示范围返回-3,显示信息为NULL返回-4
 * @warning 小字体(0x30),可显示8行,每行最多21个字符;
 *			大字体(0x31),默认字体,可显示5行,每行最多16个字符.
 *
 */
int _Os__displayTFT(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text);


#endif
