#ifndef OSPRINTER_H_INCLUDED
#define OSPRINTER_H_INCLUDED

/**
 * @defgroup 打印机函数
 * @author 熊剑
 * @brief 提供打印字符,图片,走纸等功能
 * @{
 * @code
 *	int main()
 *	{
 *		int result = 0, i,j;
 *		unsigned char pixel[48*8];
 *	
 *		unsigned char type[5] = {0,0x1c,0x1d,0x1e,0x1b};
 *		char chineseBuf[][100] = {{"沁园春·雪  毛泽东"},
 *										{"北国风光，千里冰封，万里雪飘。"},
 *										{"望长城内外，惟余莽莽；"},
 *										{"大河上下，顿失滔滔。"},
 *										{"山舞银蛇，原驰蜡象，欲与天公试比高。"},
 *										{"须晴日，看红装素裹，分外妖娆。"},
 *										{"江山如此多娇,引无数英雄竞折腰。"},
 *										{"惜秦皇汉武,略输文采；"},
 *										{"唐宗宋祖,稍逊风骚。"},
 *										{"一代天骄，成吉思汗，"},
 *										{"只识弯弓射大雕。"},
 *										{"俱往矣,数风流人物，还看今朝。"},
 *										{"　、。·ˉˇ¨〃々—～‖"},
 *										{"…‘’“”〔〕〈〉《》「」"},
 *										{"『』〖〗【】±×÷∶∧∨"},
 *										{"∑∏∪∩∈∷√⊥∥∠⌒⊙"},
 *										{"∫∮≡≌≈∽∝≠≮≯≤≥"},
 *										{"∞∵∴♂♀°′″℃＄¤￠"},
 *										{"￡‰§№☆★○●◎◇◆□"},
 *										{"■△▲※→←↑↓〓"},
 *										{"ⅰⅱⅲⅳⅴⅵⅶⅷⅸⅹ⒈⒉"},
 *										{"⒊⒋⒌⒍⒎⒏⒐⒑⒒⒓⒔⒕"},
 *										{"⒖⒗⒘⒙⒚⒛⑴⑵⑶⑷⑸⑹"},
 *										{"⑺⑻⑼⑽⑾⑿⒀⒁⒂⒃⒄⒅"},
 *										{"⒆⒇①②③④⑤⑥⑦⑧⑨⑩"},
 *										{"㈠㈡㈢㈣㈤㈥㈦㈧㈨㈩Ⅰ"},
 *										{"ⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫ"},
 *										{"！＂＃￥％＆＇（）＊＋，"},
 *										{"－．／０１２３４５６７８９"},
 *										{"：；＜＝＞？＠ＡＢＣＤＥ"},
 *										{"ＦＧＨＩＪＫＬＭＮＯＰＱ"},
 *										{"ＲＳＴＵＶＷＸＹＺ［＼］"},
 *										{"＾＿｀ａｂｃｄｅｆｇｈｉ"},
 *										{"ｊｋｌｍｎｏｐｑｒｓｔｕ"},
 *										{"ｖｗｘｙｚ｛｜｝￣"}};
 *	
 *		for(i=0;i<40;i++)
 *		{
 *			if(i%2==0)
 *			{
 *				for(j=0;j<sizeof(pixel);j++)
 *				{
 *					if(j%48%2==0)
 *						pixel[j]=0xFF;
 *					else
 *						pixel[j]=0x00;
 *				}
 *			}
 *			else
 *			{
 *				for(j=0;j<sizeof(pixel);j++)
 *				{
 *					if(j%48%2==0)
 *						pixel[j]=0x00;
 *					else
 *						pixel[j]=0xFF;
 *				}
 *			}
 *			Os__graph_xprint(pixel,sizeof(pixel)) ;
 *		}
 *		//打印图片
 *		Os__sign_xprint("/daemon/image/printtest.bmp");
 *	
 *		//走纸
 *		result = Os__linefeed(10);
 *		for(i=0;i<5;i++)
 *		{
 *			Os__GB2312_xprint("0123456789",type[i]);
 *			Os__GB2312_xprint("abcdefghijklm",type[i]);
 *			Os__GB2312_xprint("nopqrstuvwxyz",type[i]);
 *			Os__GB2312_xprint("ABCDEFGHIJKLM",type[i]);
 *			Os__GB2312_xprint("NOPQRSTUVWXYZ",type[i]);
 *			Os__GB2312_xprint("!@#$%^&*()_+-=~`",type[i]);
 *			Os__GB2312_xprint("[]\\;',./{}|:\"<>?",type[i]);
 *			Os__linefeed(20);
 *		}
 *		for(j=0;j<5;j++)
 *		{
 *			for(i=0;i<35;i++)
 *			{
 *				Os__GB2312_xprint(chineseBuf[i],type[j]);
 *			}
 *			Os__linefeed(30);
 *		}
 *		return 0;
 *	}
 * @endcode
 */
 
 
 /**
 * @fn char Os__xprint(unsigned char *data)
 * @brief 	打印数据,打印类型为普通打印
 *
 * @param data		要打印的数据地址
 *
 * @return 		返回打印数据的长度
 *
 */
char Os__xprint(unsigned char *data);

 /**
 * @fn int OSPRN_XPrintGB2312(unsigned char ucASCIIFont, unsigned char ucGBFont, unsigned char * Text)
 * @brief 	根据传入的ASC字符串字体和GB2312字符串的字体打印数据
 *
 * @param ucASCIIFont	ASC字符串字体
 * @param ucGBFont		GB2312字符串字体
 *
 * @return		成功返回0,失败返回-1
 *
 */
//int OSPRN_XPrintGB2312(unsigned char ucASCIIFont, unsigned char ucGBFont, unsigned char * Text);

 /**
 * @fn int Os__sign_xprint(unsigned char *ucPath)
 * @brief 	打印图片函数,图片必须为bmp格式的1位图,位图宽度必须为384,高度最大为320,图片大小不能超过20k
 *
 * @param ucPath	要打印的bmp图片路径
 *
 * @return		成功返回0,失败返回-1
 *
 */
int Os__sign_xprint(unsigned char *ucPath);

 /**
 * @fn int Os__linefeed(unsigned int line)
 * @brief   打印机走纸
 *
 * @param line	需要走纸的行数
 *
 * @return		成功返回0,失败返回-1
 *
 */
int Os__linefeed(unsigned int line);

 /**
 * @fn char Os__GB2312_xprint(unsigned char * pdata, unsigned char ucFont)
 * @brief   在终端上打印一串中文信息,打印应为国标码.
 *
 * @param pdata		要打印的数据地址
 * @param ucFont	打印字体类型	0: 普通
 *									0x1C: 双倍宽度
 *									0x1D: 双倍高度
 *									0x1E: 双倍尺寸（双倍高度及双倍宽度）
 *									0x1B: 等比尺寸（等高等宽）
 *
 * @return		成功返回0,失败返回-1
 *
 */
char Os__GB2312_xprint(unsigned char * pdata, unsigned char ucFont);

 /**
 * @fn int Os__graph_xprint(unsigned char *pixel_line, int size)
 * @brief   在打印纸上打印图形,打印一行PixelLine定义的行像素
 *
 * @param pixel_line	'PixelLine' 行像素定义指针
 *									黑色像素为 1
 *									白色像素为 0
 *									像素由8bit构成一个byte,
 *
 * @param size	行定义尺寸,图形行像素最大是384像素，所以最大PixLine是48个byte
 *
 * @return		成功返回0,失败返回-1
 *
 */
int Os__graph_xprint(unsigned char *pixel_line, int size);

 /**
 * @fn int Os__checkpaper(void)
 * @brief   检纸
 *
 *
 * @return		有纸返回0,无纸返回1
 */
int Os__checkpaper(void);

 /**
 * @fn int Os__check_printBuf(void)
 * @brief   查看打印机buf是否有数据
 *
 *
 * @return	buf非空返回0,buf中无数据返回1,查询失败返回负值
 */
int Os__check_printBuf(void);

 /**
 * @fn int Os__clear_printbuf(void)
 * @brief   清空打印机中正在打印的数据
 *
 * @return	成功清空打印机返回0,失败返回-1
 */
int Os__clear_printBuf(void);
#endif  //OSPRINTER_H_INCLUDED
/** @} end of etc_fns */
