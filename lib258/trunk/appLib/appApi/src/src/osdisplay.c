#include "osdisplay.h"

//定义屏幕类型
#define DZ_SCREEN 0
#define TFT_SCREEN 1
//定义当前机型,以后可以写成从config.ini中读取
#define SCREEN_TYPE DZ_SCREEN


int Os__disPlay(unsigned char ucFont,unsigned char Line, unsigned char Column, char *Text)
{
	if(SCREEN_TYPE)
	{
		return _Os__displayDZ(ucFont, Line, Column, Text);
	}
	else
	{
		return _Os__displayTFT(ucFont, Line, Column, Text);
	}
}

void Os__clrDisplay(unsigned char ucFont,unsigned char Line)
{
	if(SCREEN_TYPE)
	{
		_Os__clearDZ(ucFont, Line);
	}
	else
	{
		_Os__clearTFT(ucFont, Line);
	}
}


int Os_graphDisplay(unsigned Line, unsigned char Colum, unsigned char *Drawing, unsigned char Size)
{
	if(SCREEN_TYPE)
	{
		return _Os__graphDZ(Line, Colum, Drawing, Size);
	}
	else
	{
		return Os__lcd_image(Line, Colum, Drawing);
	}
}
