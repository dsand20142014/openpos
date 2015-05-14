#ifndef PY51_CONFIG_H
#define PY51_CONFIG_H
#ifdef __cplusplus
extern "C"
{
#endif
	#include "include.h"
#ifdef __cplusplus
}
#endif

//最大可以输入的字符
#define PINYIN_MAX_INPUT_NUMBER 100

static const unsigned int PINYIN_INPUT_INTERVAL = ONESECOND;
static const unsigned int LETTER_INPUT_INTERVAL = ONESECOND;

/*模块是否输出调试
0 :不输出
1 :输出*/
#define PINYIN_TEST 0

#endif