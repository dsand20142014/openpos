#ifndef	YPINPUT_H
#define YPINPUT_H

#ifdef __cplusplus
extern "C" {
#endif

//输入法表
#define PY_INPUT_CHINESE		0x01
#define PY_INPUT_LOWER_CASE	0x02
#define PY_INPUT_UPPER_CASE	0x04
#define PY_INPUT_NUMBER		0x08
#define PY_INPUT_QUWEI	0x10
/*****************输入法主函数********************************
参数说明:
aucOut :输入确认后,保存用户输入的字符
ucLine :用户输入字符显示的行数,从0开始,
           拼音输入法可用时建议小等于1,
           拼音输入法不可用时建议小等于2,
ucMin :限制用户输入的最小字符,必须小等于
          ucMax和PINYIN_MAX_INPUT_NUMBER
ucMax :限制用户输入的最大字符,必须小等于
           PINYIN_MAX_INPUT_NUMBER
ucInputFlag :确定可用的输入法,
                   比如PY_INPUT_NUMBER | PY_INPUT_CHINESE表示只可以在
                   拼音和数字输入法之间切换
usTimeOut :输入的总时间,单位秒
返回值说明:
ERR_CANCEL :用户按取消或输入时间超过规定时间,
                     aucOut内容不会被修改
SUCCESS :用户输入后按确认并且符合规定的字符数,
                aucOut内容被修改为用户输入的字符
*********************************************************************/
unsigned char PY_Main
	(
	unsigned char *aucOut,
	unsigned char ucLine,
	const unsigned char ucMin,
	const unsigned char ucMax,
	const unsigned char ucInputFlag,
	const unsigned short usTimeOut
	);
unsigned char PY_Input
	(
	unsigned char *pucBuf
	);


#ifdef __cplusplus
}
#endif

#endif
