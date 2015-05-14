
#include <stdio.h>

#include "py_input.h"
#include "py_input_config.h"
#include "asc_table.h"
#include "osdriver.h"
#include "toolslib.h"

#define SUCCESS								0x00
#define ERR_CANCEL							0x02

#ifndef		TRUE
#define		TRUE		1
#endif

#ifndef		true
#define		true		1
#endif

#ifndef		FALSE
#define		FALSE		0
#endif

#ifndef		false
#define		false		0
#endif

#define 	ONESECOND						100

#define PINYIN_TEST 1
typedef enum INPUT_TYPE
{
	INPUT_TYPE_PINYIN,
	INPUT_TYPE_NUMBER,
	INPUT_TYPE_UPPER_CASE,
	INPUT_TYPE_LOWER_CASE,		
	INPUT_TYPE_QUWEI
}InputType;

typedef enum INPUT_CHECK
{
	INPUT_FOUND,
	INPUT_NOT_FOUND,
	INPUT_INCLUDE
}InputCheck;

typedef struct PY_INDEX
{
	const char *m_py;
	const char *m_pyTable;
}PinYinIndex;

typedef struct PY_CHAR_INDEX
{
	const PinYinIndex *m_pyIndex;
	unsigned int m_tableSize;
}PinYinCharIndex;

/*=================全局变量===========================*/
//当前的输入法
static InputType sg_inputType;
//可用的输入法标志
static unsigned char sg_inputFlag;

//上一次按键
static unsigned char sg_ucPreviousKey;
//上一次输入的字符
static unsigned char sg_ucPreviousChar;

//输入时间(单位:10ms)
static unsigned int sg_uiTrueTimeOut;
//上一次的偏移量
static unsigned char sg_ucKeyTabOffset;

//拼音输入法汉字排列表,码表 (mb)
static PinYinIndex sg_indexLetterA[5];
static PinYinIndex sg_indexLetterB[16];
static PinYinIndex sg_indexLetterC[33];
static PinYinIndex sg_indexLetterD[20];
static PinYinIndex sg_indexLetterE[3];
static PinYinIndex sg_indexLetterF[9];
static PinYinIndex sg_indexLetterG[19];
static PinYinIndex sg_indexLetterH[19];
static PinYinIndex sg_indexLetterI[1];
static PinYinIndex sg_indexLetterJ[14];
static PinYinIndex sg_indexLetterK[18];
static PinYinIndex sg_indexLetterL[24];
static PinYinIndex sg_indexLetterM[19];
static PinYinIndex sg_indexLetterN[23];
static PinYinIndex sg_indexLetterO[2];
static PinYinIndex sg_indexLetterP[17];
static PinYinIndex sg_indexLetterQ[14];
static PinYinIndex sg_indexLetterR[14];
static PinYinIndex sg_indexLetterS[34];
static PinYinIndex sg_indexLetterT[19];
static PinYinIndex sg_indexLetterU[1];
static PinYinIndex sg_indexLetterV[1];
static PinYinIndex sg_indexLetterW[9];
static PinYinIndex sg_indexLetterX[14];
static PinYinIndex sg_indexLetterY[15];
static PinYinIndex sg_indexLetterZ[36];

//索引表，用来加速查找速度
static PinYinCharIndex sg_headLetterIndex[26];

/*===================常量定义=============================*/

static const unsigned char MAX_SIGNED_CHAR = 0x7F;

//区位码输入法开关
#define SI_JIAO		1

//每一行最长可显示的字符数
//绝不可以定义为奇数!!!
#define MAX_DISPLAY	24
/*=================内部函数定义===========================*/

//拼音系统初始化
void PY_Initial(void);

//获取下一个输入法
InputType PY_GetNextInputType
	(
	const InputType a_inputType
	);
//检查输入法是否可用
BOOL PY_CheckInputType
	(
	const InputType a_inputType
	);
//获取第一个可用的输入法
InputType PY_GetFirstInputType(void);

//检查输入的拼音
InputCheck PY_Check
	(
	const unsigned char *a_input
	);
//获取输入的拼音对应的中文字表
const unsigned char *PY_Ime
	(
	const unsigned char *a_input
	);

//显示输入的字符
void PY_Display
	(
	unsigned char ucLine,
	const unsigned char * aucInBuff
	);

//按键获取函数
unsigned char PY_GetKey(unsigned int a_ucWaitTime);

//获取用户选择的中文字
unsigned char PY_ChooseBuffer
	(
	const unsigned char *pucInData,
	unsigned char *pucOutData
	);

//输入单个字母和数字
unsigned char PY_InputLetter
	(
	unsigned char *pucBuf
	);

//插入用户输入的拼音字符，并检查
InputCheck PY_InsertPinYin
	(
	unsigned char *pucInput,
	const unsigned char ucInputOffset,
	const unsigned char *pucKeyTab,
	unsigned char *pucKeyTabOffset
	);
//拼音输入单个中文字
unsigned char PY_InputChinese
	(
	unsigned char *pucBuf
	);


#if SI_JIAO > 0
//区位码输入单个中文字
unsigned char PY_InputSiJiao
	(
	unsigned char *pucBuf
	);
#endif


unsigned char PY_Main
	(
	unsigned char *aucOut,
	unsigned char ucLine,
	const unsigned char ucMin,
	const unsigned char ucMax,
	const unsigned char ucInputFlag,
	const unsigned short usTimeOut
	)
{
	unsigned char ucResult;
	unsigned int uiLen;
	//一次输入字符最多为2个字节
	unsigned char aucBuf[4];
	unsigned char aucDisplayBuf[PINYIN_MAX_INPUT_NUMBER + 1];
	
	
//#if PINYIN_TEST > 0
	printf((char *)"Enter PinYin Main\n");
	printf((char *)"aucOut = %s\n",aucOut);
	printf((char *)"ucLine = %d, ucMin = %d, ucMax = %d\n", ucLine, ucMin ,ucMax);
	printf((char *)"ucInputFlag = %d, usTimeOut = %d\n", ucInputFlag, usTimeOut);
//#endif

/*
	//系统初始化,检查参数,设置变量
	if((ucInputFlag & (PY_INPUT_CHINESE
					| PY_INPUT_UPPER_CASE
					| PY_INPUT_LOWER_CASE
					| PY_INPUT_NUMBER
					| PY_INPUT_QUWEI) == 0x00)
		|| (ucMax > PINYIN_MAX)
		|| (ucMin > PINYIN_MAX)
		|| (ucMax < ucMin))
	{
//#if PINYIN_TEST > 0
		printf((char *)"PinYin Param Error\n");
//#endif
		return ERR_CANCEL;
	}*/

	PY_Initial();
printf((char *)"finish inital\n");

	sg_uiTrueTimeOut = usTimeOut * ONESECOND;
	sg_inputFlag = ucInputFlag;
	sg_inputType = PY_GetFirstInputType();

	memset(aucDisplayBuf, 0, sizeof(aucDisplayBuf));
	memset(aucBuf, 0, sizeof(aucBuf));

	uiLen = strlen((const char *)aucOut);
	if(uiLen > 0)
	{
		if(uiLen > PINYIN_MAX_INPUT_NUMBER)
		{
#if PINYIN_TEST > 0
			printf((char *)"aucOut is too long\n");
#endif
			uiLen = PINYIN_MAX_INPUT_NUMBER;
		}
		memcpy(aucDisplayBuf, aucOut, uiLen);
	}

#if PINYIN_TEST > 0
	printf((char *)"Length of aucOut = %d\n", uiLen);
#endif

	Os__timer_start(&sg_uiTrueTimeOut);

	//输入法主循环
	while(1)
	{
		ucResult = SUCCESS;

		PY_Display(ucLine, aucDisplayBuf);

		memset(aucBuf, 0x00, sizeof(aucBuf));

		switch(sg_inputType)
		{
		case INPUT_TYPE_PINYIN:
			//用户通过输入拼音后获取一个中文字符
			ucResult = PY_InputChinese(aucBuf);
			break;
		case INPUT_TYPE_LOWER_CASE:
		case INPUT_TYPE_UPPER_CASE:
		case INPUT_TYPE_NUMBER:
			//用户输入一个数字或字母
			ucResult = PY_InputLetter(aucBuf);
			break;
#if SI_JIAO > 0
		case INPUT_TYPE_QUWEI:
			ucResult = PY_InputSiJiao(aucBuf);
			break;
#endif
		default:
			//逻辑上应该不存在此判断
#if PINYIN_TEST > 0
			printf((char *)"/**************************/\n");
			printf((char *)"PinYin InputType Fatal Error\n");
			printf((char *)"/**************************/\n");
#endif
			sg_inputType = PY_GetFirstInputType();
			break;
		}

		//超时判断
		if(sg_uiTrueTimeOut == 0)
		{
			//返回应用程序之前一定要停止计时器(下同)!!!
#if PINYIN_TEST > 0
			printf((char *)"PinYin Warning: Timeout\n");
#endif
			Os__timer_stop(&sg_uiTrueTimeOut);
			return ERR_CANCEL;
		}
		
		uiLen = strlen((const char  *)aucDisplayBuf)/3;
		printf("\nuiLen %d %d\n",uiLen,strlen((const char  *)aucDisplayBuf));
		//输入后的返回判断
		switch(ucResult)
		{
		case SUCCESS:
			
			printf("\n%d\n",uiLen);
			if((uiLen >= ucMin) && (uiLen <= ucMax))
			{
				strcpy((char *)aucOut,(const char*)aucDisplayBuf);
				aucOut[uiLen*3]= '\0';
#if PINYIN_TEST > 0
				printf((char *)"PinYin Out Success: length of aucOut = %d\n", uiLen*3);
				printf((char *)"aucOut = %s\n", aucOut);
#endif
				Os__timer_stop(&sg_uiTrueTimeOut);
				return SUCCESS;
			}
			else
			{
#if PINYIN_TEST > 0
				if(uiLen < ucMin)
				{
					printf((char *)"PinYin Warning: Input length short\n");
				}
				else
				{
					printf((char *)"PinYin Warning: Input length long\n");
				}
#endif
				Os__beep();
			}
			break;
		case KEY_ENTER:
			//在已有字符后插入用户输入的字符
			if((uiLen + 1) <= ucMax)
			{
#if PINYIN_TEST > 0
				if(strlen((char *)aucBuf) != 0)
				{
					printf((char *)"Insert char = %s\n", aucBuf);
				}
#endif
				strcat((char *)aucDisplayBuf, (char *)aucBuf);
				
				
			}
			else
			{
#if PINYIN_TEST > 0
				printf((char *)"PinYin Warning: Input length long %d\n",ucMax);
#endif			
				sg_ucPreviousKey = 0;
				sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset = 0;
				Os__beep();
			}
			break;
		case KEY_CLEAR:
			if(uiLen > 0)
			{
#if PINYIN_TEST > 0
				printf((char *)"Clean Display\n");
#endif
				memset(aucDisplayBuf, 0x00, sizeof(aucDisplayBuf));
			}
			else
			{
#if PINYIN_TEST > 0
				printf((char *)"PinYin Cancel Out\n");
#endif
				Os__timer_stop(&sg_uiTrueTimeOut);
				return ERR_CANCEL;
			}
			break;
		case KEY_BCKSP:
			if(uiLen > 0)
			{
				--uiLen;
				//判断为中文字时删除3个字符
				if(aucDisplayBuf[uiLen] > MAX_SIGNED_CHAR)
				{
#if PINYIN_TEST > 0
					printf((char *)"Delete a chinese char\n");
#endif
				int i=0;
			
					
			//		printf("\n----->%d %x %x %x\n",uiLen,aucDisplayBuf[(uiLen+1)*3-1],aucDisplayBuf[(uiLen+1)*3-2],aucDisplayBuf[(uiLen+1)*3-3]);
					aucDisplayBuf[(uiLen+1)*3-1] = 0x00;
					aucDisplayBuf[(uiLen+1)*3-2] = 0x00;
					aucDisplayBuf[(uiLen+1)*3-3] = 0x00;
				}
				else
				{
#if PINYIN_TEST > 0
					printf((char *)"Delete a english char\n");
#endif
					aucDisplayBuf[uiLen] = 0x00;
				}
			}
			break;
		case KEY_PAPER_FEED:
			//切换输入法使用"走纸"键
			sg_ucPreviousKey = 0;
			sg_ucPreviousChar = 0;
			sg_ucKeyTabOffset=0;
			sg_inputType = PY_GetNextInputType(sg_inputType);
			break;
		case KEY_00_PT:
			aucDisplayBuf[uiLen-1]=aucBuf[0];
			break;
		default:
			break;
		}
	}
}

void PY_Initial(void)
{
	unsigned char index = 0;
	sg_ucPreviousKey = 0;
	sg_ucPreviousChar = 0;
	sg_ucKeyTabOffset=0;
	//初始化拼音列表
	/*a*/
	sg_indexLetterA[0].m_py = "";		sg_indexLetterA[0].m_pyTable = "阿啊";
	sg_indexLetterA[1].m_py = "i";		sg_indexLetterA[1].m_pyTable = "哎哀唉埃挨皑癌矮蔼艾爱隘碍";
	sg_indexLetterA[2].m_py = "n";		sg_indexLetterA[2].m_pyTable = "安氨鞍俺岸按案胺暗";
	sg_indexLetterA[3].m_py = "ng";		sg_indexLetterA[3].m_pyTable = "肮昂盎";
	sg_indexLetterA[4].m_py = "o";		sg_indexLetterA[4].m_pyTable = "凹敖熬翱袄傲奥澳懊";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterA;
	sg_headLetterIndex[index].m_tableSize = 5;
	++index;
	/*b*/
	sg_indexLetterB[0].m_py = "a";		sg_indexLetterB[0].m_pyTable = "八巴叭扒吧芭疤捌笆拔跋把靶坝爸罢霸";
	sg_indexLetterB[1].m_py = "ai";		sg_indexLetterB[1].m_pyTable = "白百佰柏摆败拜稗";
	sg_indexLetterB[2].m_py = "an";		sg_indexLetterB[2].m_pyTable = "扳班般颁斑搬板版办半伴扮拌绊瓣";
	sg_indexLetterB[3].m_py = "ang";	sg_indexLetterB[3].m_pyTable = "邦帮梆绑榜膀蚌傍棒谤磅镑";
	sg_indexLetterB[4].m_py = "ao";		sg_indexLetterB[4].m_pyTable = "包苞胞褒雹宝饱保堡报抱豹鲍暴爆剥薄瀑";
	sg_indexLetterB[5].m_py = "ei";		sg_indexLetterB[5].m_pyTable = "卑杯悲碑北贝狈备背钡倍被惫焙辈";
	sg_indexLetterB[6].m_py = "en";		sg_indexLetterB[6].m_pyTable = "奔本苯笨夯";
	sg_indexLetterB[7].m_py = "eng";	sg_indexLetterB[7].m_pyTable = "崩绷甭泵迸蹦";
	sg_indexLetterB[8].m_py = "i";		sg_indexLetterB[8].m_pyTable = "逼鼻比彼笔鄙币必毕闭庇毖陛毙敝痹蓖弊碧蔽壁避臂";
	sg_indexLetterB[9].m_py = "ian";	sg_indexLetterB[9].m_pyTable = "边编鞭贬扁卞便变遍辨辩辫";
	sg_indexLetterB[10].m_py = "iao";	sg_indexLetterB[10].m_pyTable = "彪标膘表";
	sg_indexLetterB[11].m_py = "ie";	sg_indexLetterB[11].m_pyTable = "憋鳖别瘪";
	sg_indexLetterB[12].m_py = "in";	sg_indexLetterB[12].m_pyTable = "宾彬斌滨濒摈";
	sg_indexLetterB[13].m_py = "ing";	sg_indexLetterB[13].m_pyTable = "冰兵丙秉柄炳饼并病";
	sg_indexLetterB[14].m_py = "o";		sg_indexLetterB[14].m_pyTable = "拨波玻钵脖菠播伯驳帛泊勃铂舶博渤搏箔膊卜";
	sg_indexLetterB[15].m_py = "u";		sg_indexLetterB[15].m_pyTable = "补哺捕不布步怖部埠簿";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterB;
	sg_headLetterIndex[index].m_tableSize = 16;
	++index;
	/*c*/
	sg_indexLetterC[0].m_py = "a";		sg_indexLetterC[0].m_pyTable = "擦";
	sg_indexLetterC[1].m_py = "ai";		sg_indexLetterC[1].m_pyTable = "猜才材财裁采彩睬踩菜蔡";
	sg_indexLetterC[2].m_py = "an";		sg_indexLetterC[2].m_pyTable = "参餐残蚕惭惨灿";
	sg_indexLetterC[3].m_py = "ang";	sg_indexLetterC[3].m_pyTable = "仓沧苍舱藏";
	sg_indexLetterC[4].m_py = "ao";		sg_indexLetterC[4].m_pyTable = "操糙曹槽草";
	sg_indexLetterC[5].m_py = "e";		sg_indexLetterC[5].m_pyTable = "册侧厕测策";
	sg_indexLetterC[6].m_py = "eng";	sg_indexLetterC[6].m_pyTable = "层蹭曾";
	sg_indexLetterC[7].m_py = "ha";		sg_indexLetterC[7].m_pyTable = "叉插查茬茶搽察碴岔诧差刹";
	sg_indexLetterC[8].m_py = "hai";	sg_indexLetterC[8].m_pyTable = "拆柴豺";
	sg_indexLetterC[9].m_py = "han";	sg_indexLetterC[9].m_pyTable = "掺搀谗馋缠蝉产铲阐颤";
	sg_indexLetterC[10].m_py = "hang";	sg_indexLetterC[10].m_pyTable = "昌猖肠尝偿常厂场敞畅倡唱";
	sg_indexLetterC[11].m_py = "hao";	sg_indexLetterC[11].m_pyTable = "抄钞超巢朝嘲潮吵炒绰";
	sg_indexLetterC[12].m_py = "he";	sg_indexLetterC[12].m_pyTable = "车扯彻掣撤澈";
	sg_indexLetterC[13].m_py = "hen";	sg_indexLetterC[13].m_pyTable = "郴尘臣忱沉辰陈晨衬趁";
	sg_indexLetterC[14].m_py = "heng";	sg_indexLetterC[14].m_pyTable = "称撑成呈承诚城乘惩程澄橙逞骋秤";
	sg_indexLetterC[15].m_py = "hi";	sg_indexLetterC[15].m_pyTable = "吃痴弛池驰迟持尺侈齿耻斥赤炽翅";
	sg_indexLetterC[16].m_py = "hong";	sg_indexLetterC[16].m_pyTable = "充冲虫崇宠";
	sg_indexLetterC[17].m_py = "hou";	sg_indexLetterC[17].m_pyTable = "抽仇绸畴愁稠筹酬踌丑瞅臭";
	sg_indexLetterC[18].m_py = "hu";	sg_indexLetterC[18].m_pyTable = "出初除厨滁锄雏橱躇础储楚处搐触矗畜";
	sg_indexLetterC[19].m_py = "huai";	sg_indexLetterC[19].m_pyTable = "揣";
	sg_indexLetterC[20].m_py = "huan";	sg_indexLetterC[20].m_pyTable = "川穿传船椽喘串";
	sg_indexLetterC[21].m_py = "huang";	sg_indexLetterC[21].m_pyTable = "闯疮窗床创";
	sg_indexLetterC[22].m_py = "hui";	sg_indexLetterC[22].m_pyTable = "吹炊垂捶锤";
	sg_indexLetterC[23].m_py = "hun";	sg_indexLetterC[23].m_pyTable = "春椿纯唇淳醇蠢";
	sg_indexLetterC[24].m_py = "huo";	sg_indexLetterC[24].m_pyTable = "戳";
	sg_indexLetterC[25].m_py = "i";		sg_indexLetterC[25].m_pyTable = "疵词茨瓷慈辞磁雌此次刺赐";
	sg_indexLetterC[26].m_py = "ong";	sg_indexLetterC[26].m_pyTable = "囱从匆葱聪丛";
	sg_indexLetterC[27].m_py = "ou";	sg_indexLetterC[27].m_pyTable = "凑";
	sg_indexLetterC[28].m_py = "u";		sg_indexLetterC[28].m_pyTable = "粗促醋簇";
	sg_indexLetterC[29].m_py = "uan";	sg_indexLetterC[29].m_pyTable = "蹿窜篡";
	sg_indexLetterC[30].m_py = "ui";	sg_indexLetterC[30].m_pyTable = "崔催摧脆淬瘁粹翠";
	sg_indexLetterC[31].m_py = "un";	sg_indexLetterC[31].m_pyTable = "村存寸";
	sg_indexLetterC[32].m_py = "uo";	sg_indexLetterC[32].m_pyTable = "搓磋撮挫措错";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterC;
	sg_headLetterIndex[index].m_tableSize = 33;
	++index;
	/*d*/
	sg_indexLetterD[0].m_py = "a";		sg_indexLetterD[0].m_pyTable = "搭达答瘩打大";     																																								
	sg_indexLetterD[1].m_py = "ai";		sg_indexLetterD[1].m_pyTable = "呆歹傣代带待怠殆贷袋逮戴";    																																								
	sg_indexLetterD[2].m_py = "an";		sg_indexLetterD[2].m_pyTable = "丹单担耽郸胆掸旦但诞弹惮淡蛋氮";    																																								
	sg_indexLetterD[3].m_py = "ang";	sg_indexLetterD[3].m_pyTable = "当挡党荡档";   																																								
	sg_indexLetterD[4].m_py = "ao";		sg_indexLetterD[4].m_pyTable = "刀导岛倒捣祷蹈到悼盗道稻";    																																								
	sg_indexLetterD[5].m_py = "e";		sg_indexLetterD[5].m_pyTable = "得德的";     																																								
	sg_indexLetterD[6].m_py = "eng";	sg_indexLetterD[6].m_pyTable = "灯登蹬等邓凳瞪";   																																								
	sg_indexLetterD[7].m_py = "i";		sg_indexLetterD[7].m_pyTable = "低堤滴狄迪敌涤笛嫡底抵地弟帝递第缔蒂";     																																								
	sg_indexLetterD[8].m_py = "ian";	sg_indexLetterD[8].m_pyTable = "掂滇颠典点碘电佃甸店垫惦淀奠殿靛";   																																								
	sg_indexLetterD[9].m_py = "iao";	sg_indexLetterD[9].m_pyTable = "刁叼凋碉雕吊钓掉";   																																								
	sg_indexLetterD[10].m_py = "ie";	sg_indexLetterD[10].m_pyTable = "爹跌迭谍叠碟蝶";   																																								
	sg_indexLetterD[11].m_py = "ing";	sg_indexLetterD[11].m_pyTable = "丁叮盯钉顶鼎订定锭";  																																								
	sg_indexLetterD[12].m_py = "iu";	sg_indexLetterD[12].m_pyTable = "丢";   																																								
	sg_indexLetterD[13].m_py = "ong";	sg_indexLetterD[13].m_pyTable = "东冬董懂动冻侗恫栋洞";  																																								
	sg_indexLetterD[14].m_py = "ou";	sg_indexLetterD[14].m_pyTable = "都兜斗抖陡豆逗痘";   																																								
	sg_indexLetterD[15].m_py = "u";		sg_indexLetterD[15].m_pyTable = "督毒读犊独堵赌睹妒杜肚度渡镀";    																																								
	sg_indexLetterD[16].m_py = "uan";	sg_indexLetterD[16].m_pyTable = "端短段断缎锻";  																																								
	sg_indexLetterD[17].m_py = "ui";	sg_indexLetterD[17].m_pyTable = "堆队对兑";   																																								
	sg_indexLetterD[18].m_py = "un";	sg_indexLetterD[18].m_pyTable = "吨敦墩蹲盾钝顿遁";   																																								
	sg_indexLetterD[19].m_py = "uo";	sg_indexLetterD[19].m_pyTable = "多哆夺掇朵垛躲剁堕舵惰跺";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterD;
	sg_headLetterIndex[index].m_tableSize = 20;
	++index;
	/*e*/
	sg_indexLetterE[0].m_py = "";		sg_indexLetterE[0].m_pyTable = "讹俄娥峨鹅蛾额厄扼恶饿鄂遏";
	sg_indexLetterE[1].m_py = "n";		sg_indexLetterE[1].m_pyTable = "恩";
	sg_indexLetterE[2].m_py = "r";		sg_indexLetterE[2].m_pyTable = "儿而尔耳洱饵二贰";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterE;
	sg_headLetterIndex[index].m_tableSize = 3;
	++index;
	/*f*/                                                  
	sg_indexLetterF[0].m_py = "a";		sg_indexLetterF[0].m_pyTable = "发乏伐罚阀筏法珐";
	sg_indexLetterF[1].m_py = "an";		sg_indexLetterF[1].m_pyTable = "帆番翻藩凡矾钒烦樊繁反返犯泛饭范贩";
	sg_indexLetterF[2].m_py = "ang";	sg_indexLetterF[2].m_pyTable = "方坊芳防妨房肪仿访纺放";
	sg_indexLetterF[3].m_py = "ei";		sg_indexLetterF[3].m_pyTable = "飞非啡菲肥匪诽吠废沸肺费";
	sg_indexLetterF[4].m_py = "en";		sg_indexLetterF[4].m_pyTable = "分吩纷芬氛酚坟汾焚粉份奋忿愤粪";
	sg_indexLetterF[5].m_py = "eng";	sg_indexLetterF[5].m_pyTable = "丰风枫封疯峰烽锋蜂冯逢缝讽凤奉";
	sg_indexLetterF[6].m_py = "o";		sg_indexLetterF[6].m_pyTable = "佛";
	sg_indexLetterF[7].m_py = "ou";		sg_indexLetterF[7].m_pyTable = "否";
	sg_indexLetterF[8].m_py = "u";		sg_indexLetterF[8].m_pyTable = "夫肤孵敷弗伏扶拂服俘氟浮涪符袱幅福辐抚甫府斧俯釜辅腑腐父讣付妇负附咐阜复赴副傅富赋缚腹覆";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterF;
	sg_headLetterIndex[index].m_tableSize = 9;
	++index;
	/*g*/
	sg_indexLetterG[0].m_py = "a";		sg_indexLetterG[0].m_pyTable = "嘎噶";
	sg_indexLetterG[1].m_py = "ai";		sg_indexLetterG[1].m_pyTable = "该改钙盖溉概";
	sg_indexLetterG[2].m_py = "an";		sg_indexLetterG[2].m_pyTable = "干甘杆肝柑竿秆赶敢感赣";
	sg_indexLetterG[3].m_py = "ang";	sg_indexLetterG[3].m_pyTable = "冈刚岗纲肛缸钢港杠";
	sg_indexLetterG[4].m_py = "ao";		sg_indexLetterG[4].m_pyTable = "皋羔高膏篙糕搞稿镐告";
	sg_indexLetterG[5].m_py = "e";		sg_indexLetterG[5].m_pyTable = "戈疙哥胳鸽割搁歌阁革格葛隔个各铬咯";
	sg_indexLetterG[6].m_py = "ei";		sg_indexLetterG[6].m_pyTable = "给";
	sg_indexLetterG[7].m_py = "en";		sg_indexLetterG[7].m_pyTable = "根跟";
	sg_indexLetterG[8].m_py = "eng";	sg_indexLetterG[8].m_pyTable = "更庚耕羹埂耿梗";
	sg_indexLetterG[9].m_py = "ong";	sg_indexLetterG[9].m_pyTable = "工弓公功攻供宫恭躬龚巩汞拱共贡";
	sg_indexLetterG[10].m_py = "ou";	sg_indexLetterG[10].m_pyTable = "勾沟钩狗苟构购垢够";
	sg_indexLetterG[11].m_py = "u";		sg_indexLetterG[11].m_pyTable = "估咕姑孤沽菇辜箍古谷股骨蛊鼓固故顾雇";
	sg_indexLetterG[12].m_py = "ua";	sg_indexLetterG[12].m_pyTable = "瓜刮剐寡挂褂";
	sg_indexLetterG[13].m_py = "uai";	sg_indexLetterG[13].m_pyTable = "乖拐怪";
	sg_indexLetterG[14].m_py = "uan";	sg_indexLetterG[14].m_pyTable = "关观官冠棺馆管贯惯灌罐";
	sg_indexLetterG[15].m_py = "uang";	sg_indexLetterG[15].m_pyTable = "光广逛";
	sg_indexLetterG[16].m_py = "ui";	sg_indexLetterG[16].m_pyTable = "归圭龟规闺硅瑰轨诡癸鬼刽柜贵桂跪";
	sg_indexLetterG[17].m_py = "un";	sg_indexLetterG[17].m_pyTable = "辊滚棍";
	sg_indexLetterG[18].m_py = "uo";	sg_indexLetterG[18].m_pyTable = "郭锅国果裹过";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterG;
	sg_headLetterIndex[index].m_tableSize = 19;
	++index;
	/*h*/
	sg_indexLetterH[0].m_py = "a";		sg_indexLetterH[0].m_pyTable = "蛤哈";
	sg_indexLetterH[1].m_py = "ai";		sg_indexLetterH[1].m_pyTable = "孩骸海亥骇害氦";
	sg_indexLetterH[2].m_py = "an";		sg_indexLetterH[2].m_pyTable = "酣憨含邯函涵寒韩罕喊汉汗旱悍捍焊憾撼翰";
	sg_indexLetterH[3].m_py = "ang";	sg_indexLetterH[3].m_pyTable = "杭航行";
	sg_indexLetterH[4].m_py = "ao";		sg_indexLetterH[4].m_pyTable = "毫豪嚎壕好郝号浩耗";
	sg_indexLetterH[5].m_py = "e";		sg_indexLetterH[5].m_pyTable = "呵喝禾合何和河阂核荷涸盒菏贺褐赫鹤";
	sg_indexLetterH[6].m_py = "ei";		sg_indexLetterH[6].m_pyTable = "黑嘿";
	sg_indexLetterH[7].m_py = "en";		sg_indexLetterH[7].m_pyTable = "痕很狠恨";
	sg_indexLetterH[8].m_py = "eng";	sg_indexLetterH[8].m_pyTable = "亨哼恒横衡";
	sg_indexLetterH[9].m_py = "ong";	sg_indexLetterH[9].m_pyTable = "轰哄烘弘红宏洪虹鸿";
	sg_indexLetterH[10].m_py = "ou";	sg_indexLetterH[10].m_pyTable = "侯喉猴吼后厚候";
	sg_indexLetterH[11].m_py = "u";		sg_indexLetterH[11].m_pyTable = "乎呼忽弧狐胡壶湖葫瑚糊蝴虎唬互户护沪";
	sg_indexLetterH[12].m_py = "ua";	sg_indexLetterH[12].m_pyTable = "花华哗滑猾化划画话";
	sg_indexLetterH[13].m_py = "uai";	sg_indexLetterH[13].m_pyTable = "怀徊淮槐坏";
	sg_indexLetterH[14].m_py = "uan";	sg_indexLetterH[14].m_pyTable = "欢还环桓缓幻宦唤换涣患焕痪豢";
	sg_indexLetterH[15].m_py = "uang ";	sg_indexLetterH[15].m_pyTable = "荒慌皇凰黄惶煌蝗磺簧恍晃谎幌";
	sg_indexLetterH[16].m_py = "ui";	sg_indexLetterH[16].m_pyTable = "灰恢挥辉徽回蛔悔卉汇会讳绘诲烩贿晦秽惠毁慧";
	sg_indexLetterH[17].m_py = "un";	sg_indexLetterH[17].m_pyTable = "昏荤婚浑魂混";
	sg_indexLetterH[18].m_py = "uo";	sg_indexLetterH[18].m_pyTable = "豁活火伙或货获祸惑霍";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterH;
	sg_headLetterIndex[index].m_tableSize = 19;
	++index;
	/*i*/
	sg_indexLetterI[0].m_py = "";		sg_indexLetterI[0].m_pyTable = "";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterI;
	sg_headLetterIndex[index].m_tableSize = 0;
	++index;
	/*j*/
	sg_indexLetterJ[0].m_py = "i";		sg_indexLetterJ[0].m_pyTable = "讥击饥圾机肌鸡迹姬积基绩缉畸箕稽激及吉汲级即极急疾棘集嫉辑籍几己挤脊计记伎纪妓忌技际剂季既济继寂寄悸祭蓟冀藉";
	sg_indexLetterJ[1].m_py = "ia";		sg_indexLetterJ[1].m_pyTable = "加夹佳枷家嘉荚颊甲贾钾价驾架假嫁稼挟";
	sg_indexLetterJ[2].m_py = "ian";	sg_indexLetterJ[2].m_pyTable = "奸尖坚歼间肩艰兼监笺缄煎拣俭柬茧捡减剪检硷简碱见件建饯剑荐贱健涧舰渐溅践鉴键箭";
	sg_indexLetterJ[3].m_py = "iang";	sg_indexLetterJ[3].m_pyTable = "江姜将浆僵疆讲奖桨蒋匠降酱";
	sg_indexLetterJ[4].m_py = "iao";	sg_indexLetterJ[4].m_pyTable = "交郊娇浇骄胶椒焦蕉礁角狡绞饺矫脚铰搅剿缴叫轿较教窖酵觉嚼";
	sg_indexLetterJ[5].m_py = "ie";		sg_indexLetterJ[5].m_pyTable = "阶皆接秸揭街节劫杰洁结捷睫截竭姐解介戒芥届界疥诫借";
	sg_indexLetterJ[6].m_py = "in";		sg_indexLetterJ[6].m_pyTable = "巾今斤金津筋襟仅紧谨锦尽劲近进晋浸烬禁靳";
	sg_indexLetterJ[7].m_py = "ing";	sg_indexLetterJ[7].m_pyTable = "京经茎荆惊晶睛粳兢精鲸井颈景警净径痉竞竟敬靖境静镜";
	sg_indexLetterJ[8].m_py = "iong";	sg_indexLetterJ[8].m_pyTable = "炯窘";
	sg_indexLetterJ[9].m_py = "iu";		sg_indexLetterJ[9].m_pyTable = "纠究揪九久灸玖韭酒旧臼咎疚厩救就舅";
	sg_indexLetterJ[10].m_py = "u";		sg_indexLetterJ[10].m_pyTable = "居拘狙驹疽鞠局桔菊咀沮举矩句巨拒具炬俱剧惧据距锯聚踞";
	sg_indexLetterJ[11].m_py = "uan";	sg_indexLetterJ[11].m_pyTable = "娟捐鹃卷倦绢眷";
	sg_indexLetterJ[12].m_py = "ue";	sg_indexLetterJ[12].m_pyTable = "撅决诀抉绝倔掘爵攫";
	sg_indexLetterJ[13].m_py = "un";	sg_indexLetterJ[13].m_pyTable = "军君均钧菌俊郡峻浚骏竣";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterJ;
	sg_headLetterIndex[index].m_tableSize = 14;
	++index;
	/*k*/
	sg_indexLetterK[0].m_py = "a";		sg_indexLetterK[0].m_pyTable = "咖喀卡";
	sg_indexLetterK[1].m_py = "ai";		sg_indexLetterK[1].m_pyTable = "开揩凯慨楷";
	sg_indexLetterK[2].m_py = "an";		sg_indexLetterK[2].m_pyTable = "槛刊勘堪坎砍看";
	sg_indexLetterK[3].m_py = "ang";	sg_indexLetterK[3].m_pyTable = "康慷糠扛亢抗炕";
	sg_indexLetterK[4].m_py = "ao";		sg_indexLetterK[4].m_pyTable = "考拷烤靠";
	sg_indexLetterK[5].m_py = "e";		sg_indexLetterK[5].m_pyTable = "坷苛柯科棵颗磕壳咳可渴克刻客课";
	sg_indexLetterK[6].m_py = "en";		sg_indexLetterK[6].m_pyTable = "肯垦恳啃";
	sg_indexLetterK[7].m_py = "eng";	sg_indexLetterK[7].m_pyTable = "吭坑";
	sg_indexLetterK[8].m_py = "ong";	sg_indexLetterK[8].m_pyTable = "空孔恐控";
	sg_indexLetterK[9].m_py = "ou";		sg_indexLetterK[9].m_pyTable = "抠口扣寇";
	sg_indexLetterK[10].m_py = "u";		sg_indexLetterK[10].m_pyTable = "枯哭窟苦库裤酷";
	sg_indexLetterK[11].m_py = "ua";	sg_indexLetterK[11].m_pyTable = "夸垮挎胯跨";
	sg_indexLetterK[12].m_py = "uai";	sg_indexLetterK[12].m_pyTable = "块快侩筷蒯脍";
	sg_indexLetterK[13].m_py = "uan";	sg_indexLetterK[13].m_pyTable = "宽款";
	sg_indexLetterK[14].m_py = "uang";	sg_indexLetterK[14].m_pyTable = "匡筐狂况旷矿框眶";
	sg_indexLetterK[15].m_py = "ui";	sg_indexLetterK[15].m_pyTable = "亏岿盔窥奎葵魁傀愧溃馈";
	sg_indexLetterK[16].m_py = "un";	sg_indexLetterK[16].m_pyTable = "坤昆捆困";
	sg_indexLetterK[17].m_py = "uo";	sg_indexLetterK[17].m_pyTable = "扩括阔廓";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterK;
	sg_headLetterIndex[index].m_tableSize = 18;
	++index;
	/*l*/
	sg_indexLetterL[0].m_py = "a";		sg_indexLetterL[0].m_pyTable = "垃拉啦喇腊蜡辣";
	sg_indexLetterL[1].m_py = "ai";		sg_indexLetterL[1].m_pyTable = "来莱赖";
	sg_indexLetterL[2].m_py = "an";		sg_indexLetterL[2].m_pyTable = "兰拦栏婪阑蓝谰澜篮览揽缆懒烂滥";
	sg_indexLetterL[3].m_py = "ang";	sg_indexLetterL[3].m_pyTable = "郎狼廊琅榔朗浪";
	sg_indexLetterL[4].m_py = "ao";		sg_indexLetterL[4].m_pyTable = "捞劳牢老佬姥涝烙酪";
	sg_indexLetterL[5].m_py = "e";		sg_indexLetterL[5].m_pyTable = "乐勒了";
	sg_indexLetterL[6].m_py = "ei";		sg_indexLetterL[6].m_pyTable = "雷镭垒磊蕾儡肋泪类累擂";
	sg_indexLetterL[7].m_py = "eng";	sg_indexLetterL[7].m_pyTable = "棱楞冷";
	sg_indexLetterL[8].m_py = "i";		sg_indexLetterL[8].m_pyTable = "厘梨狸离莉犁漓璃黎篱礼李里哩理鲤力历厉立吏丽利励沥例隶俐荔栗砾粒傈痢";
	sg_indexLetterL[9].m_py = "ian";	sg_indexLetterL[9].m_pyTable = "连帘怜涟莲联廉镰敛脸练炼恋链";
	sg_indexLetterL[10].m_py = "iang";	sg_indexLetterL[10].m_pyTable = "俩良凉梁粮粱两亮谅辆晾量";
	sg_indexLetterL[11].m_py = "iao";	sg_indexLetterL[11].m_pyTable = "潦辽疗聊僚寥廖撩燎镣料撂";
	sg_indexLetterL[12].m_py = "ie";	sg_indexLetterL[12].m_pyTable = "列劣烈猎裂";
	sg_indexLetterL[13].m_py = "in";	sg_indexLetterL[13].m_pyTable = "邻林临淋琳霖磷鳞凛吝赁拎";
	sg_indexLetterL[14].m_py = "ing";	sg_indexLetterL[14].m_pyTable = "伶灵岭玲凌铃陵羚菱零龄领令另";
	sg_indexLetterL[15].m_py = "iu";	sg_indexLetterL[15].m_pyTable = "溜刘流留琉硫馏榴瘤柳六";
	sg_indexLetterL[16].m_py = "ong";	sg_indexLetterL[16].m_pyTable = "龙咙笼聋隆窿陇垄拢";
	sg_indexLetterL[17].m_py = "ou";	sg_indexLetterL[17].m_pyTable = "娄楼搂篓陋漏";
	sg_indexLetterL[18].m_py = "u";		sg_indexLetterL[18].m_pyTable = "露卢庐芦炉颅卤虏掳鲁陆录赂鹿禄碌路戮潞麓";
	sg_indexLetterL[19].m_py = "uan";	sg_indexLetterL[19].m_pyTable = "孪峦挛滦卵乱";
	sg_indexLetterL[20].m_py = "ue";	sg_indexLetterL[20].m_pyTable = "掠略";
	sg_indexLetterL[21].m_py = "un";	sg_indexLetterL[21].m_pyTable = "抡仑伦沦纶轮论";
	sg_indexLetterL[22].m_py = "uo";	sg_indexLetterL[22].m_pyTable = "罗萝逻锣箩骡螺裸洛络骆落";
	sg_indexLetterL[23].m_py = "v";		sg_indexLetterL[23].m_pyTable = "滤驴吕侣旅铝屡缕履律虑率绿氯";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterL;
	sg_headLetterIndex[index].m_tableSize = 24;
	++index;
	/*m*/
	sg_indexLetterM[0].m_py = "a";		sg_indexLetterM[0].m_pyTable = "妈麻马玛码蚂骂吗嘛";
	sg_indexLetterM[1].m_py = "ai";		sg_indexLetterM[1].m_pyTable = "埋买迈麦卖脉";
	sg_indexLetterM[2].m_py = "an";		sg_indexLetterM[2].m_pyTable = "蛮馒瞒满曼谩慢漫蔓";
	sg_indexLetterM[3].m_py = "ang";	sg_indexLetterM[3].m_pyTable = "忙芒盲茫莽氓";
	sg_indexLetterM[4].m_py = "ao";		sg_indexLetterM[4].m_pyTable = "猫毛矛茅锚卯铆茂冒贸帽貌";
	sg_indexLetterM[5].m_py = "e";		sg_indexLetterM[5].m_pyTable = "么";
	sg_indexLetterM[6].m_py = "ei";		sg_indexLetterM[6].m_pyTable = "没枚玫眉梅媒煤酶霉每美镁妹昧媚寐";
	sg_indexLetterM[7].m_py = "en";		sg_indexLetterM[7].m_pyTable = "门们闷";
	sg_indexLetterM[8].m_py = "eng";	sg_indexLetterM[8].m_pyTable = "萌盟檬猛蒙锰孟梦";
	sg_indexLetterM[9].m_py = "i";		sg_indexLetterM[9].m_pyTable = "弥迷谜醚糜靡米眯泌觅秘密幂蜜";
	sg_indexLetterM[10].m_py = "ian";	sg_indexLetterM[10].m_pyTable = "眠绵棉免勉娩冕缅面";
	sg_indexLetterM[11].m_py = "iao";	sg_indexLetterM[11].m_pyTable = "苗描瞄秒渺藐妙庙";
	sg_indexLetterM[12].m_py = "ie";	sg_indexLetterM[12].m_pyTable = "灭蔑";
	sg_indexLetterM[13].m_py = "in";	sg_indexLetterM[13].m_pyTable = "民皿抿闽悯敏";
	sg_indexLetterM[14].m_py = "ing";	sg_indexLetterM[14].m_pyTable = "名明鸣铭螟命";
	sg_indexLetterM[15].m_py = "iu";	sg_indexLetterM[15].m_pyTable = "谬";
	sg_indexLetterM[16].m_py = "o";		sg_indexLetterM[16].m_pyTable = "貉摸摹模膜摩磨蘑魔抹末沫陌莫寞漠墨默";
	sg_indexLetterM[17].m_py = "ou";	sg_indexLetterM[17].m_pyTable = "牟谋某";
	sg_indexLetterM[18].m_py = "u";		sg_indexLetterM[18].m_pyTable = "母亩牡姆拇木目牧募墓幕睦慕暮穆";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterM;
	sg_headLetterIndex[index].m_tableSize = 19;
	++index;
	/*n*/
	sg_indexLetterN[0].m_py = "a";		sg_indexLetterN[0].m_pyTable = "拿哪那纳娜钠呐";
	sg_indexLetterN[1].m_py = "ai";		sg_indexLetterN[1].m_pyTable = "乃奶氖奈耐";
	sg_indexLetterN[2].m_py = "an";		sg_indexLetterN[2].m_pyTable = "男南难";
	sg_indexLetterN[3].m_py = "ang";	sg_indexLetterN[3].m_pyTable = "囊";
	sg_indexLetterN[4].m_py = "ao";		sg_indexLetterN[4].m_pyTable = "挠恼脑闹淖";
	sg_indexLetterN[5].m_py = "e";		sg_indexLetterN[5].m_pyTable = "呢";
	sg_indexLetterN[6].m_py = "ei";		sg_indexLetterN[6].m_pyTable = "内馁";
	sg_indexLetterN[7].m_py = "en";		sg_indexLetterN[7].m_pyTable = "嫩";
	sg_indexLetterN[8].m_py = "eng";	sg_indexLetterN[8].m_pyTable = "能";
	sg_indexLetterN[9].m_py = "i";		sg_indexLetterN[9].m_pyTable = "妮尼泥倪霓你拟逆匿溺腻";
	sg_indexLetterN[10].m_py = "ian";	sg_indexLetterN[10].m_pyTable = "拈年捻撵碾念蔫";
	sg_indexLetterN[11].m_py = "iang";	sg_indexLetterN[11].m_pyTable = "娘酿";
	sg_indexLetterN[12].m_py = "iao";	sg_indexLetterN[12].m_pyTable = "鸟尿";
	sg_indexLetterN[13].m_py = "ie";	sg_indexLetterN[13].m_pyTable = "捏涅聂啮镊镍孽";
	sg_indexLetterN[14].m_py = "in";	sg_indexLetterN[14].m_pyTable = "您";
	sg_indexLetterN[15].m_py = "ing";	sg_indexLetterN[15].m_pyTable = "宁拧狞柠凝泞";
	sg_indexLetterN[16].m_py = "iu";	sg_indexLetterN[16].m_pyTable = "牛扭纽钮";
	sg_indexLetterN[17].m_py = "ong";	sg_indexLetterN[17].m_pyTable = "农浓脓弄";
	sg_indexLetterN[18].m_py = "u";		sg_indexLetterN[18].m_pyTable = "奴努怒";
	sg_indexLetterN[19].m_py = "uan";	sg_indexLetterN[19].m_pyTable = "暖";
	sg_indexLetterN[20].m_py = "ue";	sg_indexLetterN[20].m_pyTable = "疟虐";
	sg_indexLetterN[21].m_py = "uo";	sg_indexLetterN[21].m_pyTable = "挪诺懦糯";
	sg_indexLetterN[22].m_py = "v";		sg_indexLetterN[22].m_pyTable = "女";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterN;
	sg_headLetterIndex[index].m_tableSize = 23;
	++index;
	/*o*/
	sg_indexLetterO[0].m_py = "";		sg_indexLetterO[0].m_pyTable = "哦";
	sg_indexLetterO[1].m_py = "u";		sg_indexLetterO[1].m_pyTable = "欧殴鸥呕偶藕沤";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterO;
	sg_headLetterIndex[index].m_tableSize = 2;
	++index;
	/*p*/
	sg_indexLetterP[0].m_py = "a";		sg_indexLetterP[0].m_pyTable = "趴啪爬耙琶帕怕";
	sg_indexLetterP[1].m_py = "ai";		sg_indexLetterP[1].m_pyTable = "拍徘排牌派湃";
	sg_indexLetterP[2].m_py = "an";		sg_indexLetterP[2].m_pyTable = "潘攀盘磐判叛盼畔";
	sg_indexLetterP[3].m_py = "ang";	sg_indexLetterP[3].m_pyTable = "乓庞旁耪胖";
	sg_indexLetterP[4].m_py = "ao";		sg_indexLetterP[4].m_pyTable = "抛刨咆炮袍跑泡";
	sg_indexLetterP[5].m_py = "ei";		sg_indexLetterP[5].m_pyTable = "呸胚陪培赔裴沛佩配";
	sg_indexLetterP[6].m_py = "en";		sg_indexLetterP[6].m_pyTable = "喷盆";
	sg_indexLetterP[7].m_py = "eng";	sg_indexLetterP[7].m_pyTable = "抨砰烹朋彭棚硼蓬鹏澎篷膨捧碰";
	sg_indexLetterP[8].m_py = "i";		sg_indexLetterP[8].m_pyTable = "辟批坯披砒劈霹皮毗疲啤琵脾匹痞屁僻譬";
	sg_indexLetterP[9].m_py = "ian";	sg_indexLetterP[9].m_pyTable = "片偏篇骗";
	sg_indexLetterP[10].m_py = "iao";	sg_indexLetterP[10].m_pyTable = "漂飘瓢票";
	sg_indexLetterP[11].m_py = "ie";	sg_indexLetterP[11].m_pyTable = "撇瞥";
	sg_indexLetterP[12].m_py = "in";	sg_indexLetterP[12].m_pyTable = "拼贫频品聘";
	sg_indexLetterP[13].m_py = "ing";	sg_indexLetterP[13].m_pyTable = "乒平评凭坪苹屏瓶萍";
	sg_indexLetterP[14].m_py = "o";		sg_indexLetterP[14].m_pyTable = "坡泼颇婆迫破粕魄";
	sg_indexLetterP[15].m_py = "ou";	sg_indexLetterP[15].m_pyTable = "剖";
	sg_indexLetterP[16].m_py = "u";		sg_indexLetterP[16].m_pyTable = "脯仆扑铺莆菩葡蒲朴圃埔浦普谱曝";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterP;
	sg_headLetterIndex[index].m_tableSize = 17;
	++index;
	/*q*/ 
	sg_indexLetterQ[0].m_py = "i";		sg_indexLetterQ[0].m_pyTable = "七沏妻柒凄栖戚期欺漆祁齐其奇歧祈脐崎畦骑棋旗乞企岂启起气讫迄弃汽泣契砌器";
	sg_indexLetterQ[1].m_py = "ia";		sg_indexLetterQ[1].m_pyTable = "掐恰洽";
	sg_indexLetterQ[2].m_py = "ian";	sg_indexLetterQ[2].m_pyTable = "千仟扦迁钎牵铅谦签前钱钳乾潜黔浅遣谴欠堑嵌歉";
	sg_indexLetterQ[3].m_py = "iang";	sg_indexLetterQ[3].m_pyTable = "呛羌枪腔强墙蔷抢";
	sg_indexLetterQ[4].m_py = "iao";	sg_indexLetterQ[4].m_pyTable = "悄敲锹橇乔侨桥瞧巧俏峭窍翘撬鞘";
	sg_indexLetterQ[5].m_py = "ie";		sg_indexLetterQ[5].m_pyTable = "切茄且怯窃";
	sg_indexLetterQ[6].m_py = "in";		sg_indexLetterQ[6].m_pyTable = "亲侵钦芹秦琴禽勤擒寝沁";
	sg_indexLetterQ[7].m_py = "ing";	sg_indexLetterQ[7].m_pyTable = "青氢轻倾卿清情晴氰擎顷请庆";
	sg_indexLetterQ[8].m_py = "iong";	sg_indexLetterQ[8].m_pyTable = "穷琼";
	sg_indexLetterQ[9].m_py = "iu";		sg_indexLetterQ[9].m_pyTable = "丘邱秋囚求泅酋球";
	sg_indexLetterQ[10].m_py = "u";		sg_indexLetterQ[10].m_pyTable = "区曲驱屈蛆躯趋渠取娶龋去趣";
	sg_indexLetterQ[11].m_py = "uan";	sg_indexLetterQ[11].m_pyTable = "圈全权泉拳痊醛颧犬劝券";
	sg_indexLetterQ[12].m_py = "ue";	sg_indexLetterQ[12].m_pyTable = "炔缺瘸却雀确鹊榷";
	sg_indexLetterQ[13].m_py = "un";	sg_indexLetterQ[13].m_pyTable = "裙群";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterQ;
	sg_headLetterIndex[index].m_tableSize = 14;
	++index;
	/*r*/
	sg_indexLetterR[0].m_py = "an";		sg_indexLetterR[0].m_pyTable = "然燃冉染";
	sg_indexLetterR[1].m_py = "ang";	sg_indexLetterR[1].m_pyTable = "瓤嚷壤攘让";
	sg_indexLetterR[2].m_py = "ao";		sg_indexLetterR[2].m_pyTable = "饶扰绕";  
	sg_indexLetterR[3].m_py = "e";		sg_indexLetterR[3].m_pyTable = "惹热";    
	sg_indexLetterR[4].m_py = "en";		sg_indexLetterR[4].m_pyTable = "人仁壬忍刃认任纫妊韧";  
	sg_indexLetterR[5].m_py = "eng";	sg_indexLetterR[5].m_pyTable = "扔仍";
	sg_indexLetterR[6].m_py = "i";		sg_indexLetterR[6].m_pyTable = "日";    
	sg_indexLetterR[7].m_py = "ong";	sg_indexLetterR[7].m_pyTable = "戎绒茸荣容溶蓉熔融冗";
	sg_indexLetterR[8].m_py = "ou";		sg_indexLetterR[8].m_pyTable = "柔揉肉";  
	sg_indexLetterR[9].m_py = "u";		sg_indexLetterR[9].m_pyTable = "如茹儒孺蠕汝乳辱入褥";    
	sg_indexLetterR[10].m_py = "uan";	sg_indexLetterR[10].m_pyTable = "阮软";
	sg_indexLetterR[11].m_py = "ui";	sg_indexLetterR[11].m_pyTable = "蕊锐瑞";  
	sg_indexLetterR[12].m_py = "un";	sg_indexLetterR[12].m_pyTable = "闰润";  
	sg_indexLetterR[13].m_py = "uo";	sg_indexLetterR[13].m_pyTable = "若弱"; 
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterR;
	sg_headLetterIndex[index].m_tableSize = 14;
	++index;
	/*s*/
	sg_indexLetterS[0].m_py = "a";		sg_indexLetterS[0].m_pyTable = "撒洒萨";
	sg_indexLetterS[1].m_py = "ai";		sg_indexLetterS[1].m_pyTable = "塞腮鳃赛";
	sg_indexLetterS[2].m_py = "an";		sg_indexLetterS[2].m_pyTable = "三叁伞散";
	sg_indexLetterS[3].m_py = "ang";	sg_indexLetterS[3].m_pyTable = "桑嗓丧";
	sg_indexLetterS[4].m_py = "ao";		sg_indexLetterS[4].m_pyTable = "搔骚扫嫂";
	sg_indexLetterS[5].m_py = "e";		sg_indexLetterS[5].m_pyTable = "色涩瑟";
	sg_indexLetterS[6].m_py = "en";		sg_indexLetterS[6].m_pyTable = "森";
	sg_indexLetterS[7].m_py = "eng";	sg_indexLetterS[7].m_pyTable = "僧";
	sg_indexLetterS[8].m_py = "ha";		sg_indexLetterS[8].m_pyTable = "杀沙纱砂莎傻啥煞厦";
	sg_indexLetterS[9].m_py = "hai";	sg_indexLetterS[9].m_pyTable = "筛晒";
	sg_indexLetterS[10].m_py = "han";	sg_indexLetterS[10].m_pyTable = "山删杉衫珊煽闪陕汕苫扇善缮擅膳赡栅";
	sg_indexLetterS[11].m_py = "hang";	sg_indexLetterS[11].m_pyTable = "伤商墒裳晌赏上尚";
	sg_indexLetterS[12].m_py = "hao";	sg_indexLetterS[12].m_pyTable = "捎梢烧稍勺芍韶少邵绍哨";
	sg_indexLetterS[13].m_py = "he";	sg_indexLetterS[13].m_pyTable = "奢赊舌蛇舍设社射涉赦慑摄";
	sg_indexLetterS[14].m_py = "hen";	sg_indexLetterS[14].m_pyTable = "申伸身呻绅娠砷深神沈审婶肾甚渗慎什";
	sg_indexLetterS[15].m_py = "heng";	sg_indexLetterS[15].m_pyTable = "升生声牲胜甥绳省圣盛剩";
	sg_indexLetterS[16].m_py = "hi";	sg_indexLetterS[16].m_pyTable = "匙尸失师虱诗施狮湿十石时识实拾蚀食史矢使始驶屎士氏世仕市示式事侍势视试饰室恃拭是柿适逝释嗜誓噬似";
	sg_indexLetterS[17].m_py = "hou";	sg_indexLetterS[17].m_pyTable = "收手守首寿受兽售授瘦";
	sg_indexLetterS[18].m_py = "hu";	sg_indexLetterS[18].m_pyTable = "书抒叔枢殊梳淑疏舒输蔬孰赎熟暑黍署鼠蜀薯曙术戍束述树竖恕庶数墅漱属";
	sg_indexLetterS[19].m_py = "hua";	sg_indexLetterS[19].m_pyTable = "刷耍";
	sg_indexLetterS[20].m_py = "huai";	sg_indexLetterS[20].m_pyTable = "衰摔甩帅";
	sg_indexLetterS[21].m_py = "huan";	sg_indexLetterS[21].m_pyTable = "拴栓";
	sg_indexLetterS[22].m_py = "huang";	sg_indexLetterS[22].m_pyTable = "双霜爽";
	sg_indexLetterS[23].m_py = "hui";	sg_indexLetterS[23].m_pyTable = "谁水税睡";
	sg_indexLetterS[24].m_py = "hun";	sg_indexLetterS[24].m_pyTable = "吮顺舜瞬";
	sg_indexLetterS[25].m_py = "huo";	sg_indexLetterS[25].m_pyTable = "说烁朔硕";
	sg_indexLetterS[26].m_py = "i";		sg_indexLetterS[26].m_pyTable = "丝司私思斯嘶撕死巳四寺伺饲嗣肆";
	sg_indexLetterS[27].m_py = "ong";	sg_indexLetterS[27].m_pyTable = "松怂耸讼宋诵送颂";
	sg_indexLetterS[28].m_py = "ou";	sg_indexLetterS[28].m_pyTable = "嗽搜艘擞";
	sg_indexLetterS[29].m_py = "u";		sg_indexLetterS[29].m_pyTable = "苏酥俗诉肃素速粟塑溯僳";
	sg_indexLetterS[30].m_py = "uan";	sg_indexLetterS[30].m_pyTable = "酸蒜算";
	sg_indexLetterS[31].m_py = "ui";	sg_indexLetterS[31].m_pyTable = "虽绥隋随髓岁祟遂碎隧穗";
	sg_indexLetterS[32].m_py = "un";	sg_indexLetterS[32].m_pyTable = "孙损笋";
	sg_indexLetterS[33].m_py = "uo";	sg_indexLetterS[33].m_pyTable = "唆梭蓑缩所索琐锁";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterS;
	sg_headLetterIndex[index].m_tableSize = 34;
	++index;
	/*t*/
	sg_indexLetterT[0].m_py = "a";		sg_indexLetterT[0].m_pyTable = "她他它塌塔獭挞踏蹋";
	sg_indexLetterT[1].m_py = "ai";		sg_indexLetterT[1].m_pyTable = "胎台抬苔太汰态泰酞";
	sg_indexLetterT[2].m_py = "an";		sg_indexLetterT[2].m_pyTable = "坍贪摊滩瘫坛谈痰谭潭檀坦袒毯叹炭探碳";
	sg_indexLetterT[3].m_py = "ang";	sg_indexLetterT[3].m_pyTable = "汤唐堂棠塘搪膛糖倘淌躺烫趟";
	sg_indexLetterT[4].m_py = "ao";		sg_indexLetterT[4].m_pyTable = "涛绦掏滔逃桃陶淘萄讨套";
	sg_indexLetterT[5].m_py = "e";		sg_indexLetterT[5].m_pyTable = "特";
	sg_indexLetterT[6].m_py = "eng";	sg_indexLetterT[6].m_pyTable = "疼腾誊藤";
	sg_indexLetterT[7].m_py = "i";		sg_indexLetterT[7].m_pyTable = "剔梯锑踢啼提题蹄体屉剃涕惕替嚏";
	sg_indexLetterT[8].m_py = "ian";	sg_indexLetterT[8].m_pyTable = "天添田恬甜填腆舔";
	sg_indexLetterT[9].m_py = "iao";	sg_indexLetterT[9].m_pyTable = "调挑条迢眺跳";
	sg_indexLetterT[10].m_py = "ie";	sg_indexLetterT[10].m_pyTable = "贴铁帖";
	sg_indexLetterT[11].m_py = "ing";	sg_indexLetterT[11].m_pyTable = "厅汀听烃廷亭庭停挺艇";
	sg_indexLetterT[12].m_py = "ong";	sg_indexLetterT[12].m_pyTable = "通同彤桐铜童酮瞳统捅桶筒痛";
	sg_indexLetterT[13].m_py = "ou";	sg_indexLetterT[13].m_pyTable = "偷头投透";
	sg_indexLetterT[14].m_py = "u";		sg_indexLetterT[14].m_pyTable = "凸秃突图徒涂途屠土吐兔";
	sg_indexLetterT[15].m_py = "uan";	sg_indexLetterT[15].m_pyTable = "湍团";
	sg_indexLetterT[16].m_py = "ui";	sg_indexLetterT[16].m_pyTable = "推颓腿退蜕褪";
	sg_indexLetterT[17].m_py = "un";	sg_indexLetterT[17].m_pyTable = "囤吞屯臀";
	sg_indexLetterT[18].m_py = "uo";	sg_indexLetterT[18].m_pyTable = "托拖脱驮陀驼鸵妥椭拓唾";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterT;
	sg_headLetterIndex[index].m_tableSize = 19;
	++index;
	/*u*/                                                     
	sg_indexLetterU[0].m_py = "";		sg_indexLetterU[0].m_pyTable = "";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterU;
	sg_headLetterIndex[index].m_tableSize = 0;
	++index;
	/*v*/
	sg_indexLetterV[0].m_py = "";		sg_indexLetterV[0].m_pyTable = "";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterV;
	sg_headLetterIndex[index].m_tableSize = 0;
	++index;
	/*w*/
	sg_indexLetterW[0].m_py = "a";		sg_indexLetterW[0].m_pyTable = "哇娃挖洼蛙瓦袜";
	sg_indexLetterW[1].m_py = "ai";		sg_indexLetterW[1].m_pyTable = "歪外";
	sg_indexLetterW[2].m_py = "an";		sg_indexLetterW[2].m_pyTable = "弯湾豌丸完玩顽烷宛挽晚婉惋皖碗万腕";
	sg_indexLetterW[3].m_py = "ang";	sg_indexLetterW[3].m_pyTable = "汪亡王网往枉妄忘旺望";
	sg_indexLetterW[4].m_py = "ei";		sg_indexLetterW[4].m_pyTable = "危威微巍为韦围违桅唯惟维潍伟伪尾纬苇委萎卫未位味畏胃尉谓喂渭蔚慰魏";
	sg_indexLetterW[5].m_py = "en";		sg_indexLetterW[5].m_pyTable = "温瘟文纹闻蚊吻紊稳问";
	sg_indexLetterW[6].m_py = "eng";	sg_indexLetterW[6].m_pyTable = "翁嗡瓮";
	sg_indexLetterW[7].m_py = "o";		sg_indexLetterW[7].m_pyTable = "挝涡窝蜗我沃卧握斡";
	sg_indexLetterW[8].m_py = "u";		sg_indexLetterW[8].m_pyTable = "乌污呜巫屋诬钨无毋吴吾芜梧五午伍坞武侮捂舞勿务戊物误悟晤雾";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterW;
	sg_headLetterIndex[index].m_tableSize = 9;
	++index;
	/*x*/
	sg_indexLetterX[0].m_py = "i";		sg_indexLetterX[0].m_pyTable = "夕汐西吸希昔析矽息牺悉惜烯硒晰犀稀溪锡熄熙嘻膝习席袭媳檄洗喜戏系细隙";
	sg_indexLetterX[1].m_py = "ia";		sg_indexLetterX[1].m_pyTable = "虾瞎匣侠峡狭暇辖霞下吓夏";
	sg_indexLetterX[2].m_py = "ian";	sg_indexLetterX[2].m_pyTable = "铣仙先纤掀锨鲜闲弦贤咸涎舷衔嫌显险县现线限宪陷馅羡献腺";
	sg_indexLetterX[3].m_py = "iang";	sg_indexLetterX[3].m_pyTable = "乡相香厢湘箱襄镶详祥翔享响想向巷项象像橡";
	sg_indexLetterX[4].m_py = "iao";	sg_indexLetterX[4].m_pyTable = "宵消萧硝销霄嚣淆小晓孝肖哮效校笑啸";
	sg_indexLetterX[5].m_py = "ie";		sg_indexLetterX[5].m_pyTable = "些楔歇蝎协邪胁斜谐携鞋写泄泻卸屑械谢懈蟹";
	sg_indexLetterX[6].m_py = "in";		sg_indexLetterX[6].m_pyTable = "心忻芯辛欣锌新薪信衅";
	sg_indexLetterX[7].m_py = "ing";	sg_indexLetterX[7].m_pyTable = "兴星惺猩腥刑邢形型醒杏姓幸性";
	sg_indexLetterX[8].m_py = "iong";	sg_indexLetterX[8].m_pyTable = "凶兄匈汹胸雄熊";
	sg_indexLetterX[9].m_py = "iu";		sg_indexLetterX[9].m_pyTable = "宿休修羞朽秀绣袖锈嗅";
	sg_indexLetterX[10].m_py = "u";		sg_indexLetterX[10].m_pyTable = "戌须虚嘘需墟徐许旭序叙恤绪续酗婿絮蓄吁";
	sg_indexLetterX[11].m_py = "uan";	sg_indexLetterX[11].m_pyTable = "轩宣喧玄悬旋选癣绚眩";
	sg_indexLetterX[12].m_py = "ue";	sg_indexLetterX[12].m_pyTable = "削靴薛穴学雪血";
	sg_indexLetterX[13].m_py = "un";	sg_indexLetterX[13].m_pyTable = "勋熏寻巡旬驯询循训讯汛迅逊殉"; 
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterX;
	sg_headLetterIndex[index].m_tableSize = 14;
	++index;
	/*y*/                                                  
	sg_indexLetterY[0].m_py = "a";		sg_indexLetterY[0].m_pyTable = "丫压呀押鸦鸭牙芽蚜崖涯衙哑雅亚讶";
	sg_indexLetterY[1].m_py = "an";		sg_indexLetterY[1].m_pyTable = "咽烟淹焉阉延严言岩沿炎研盐阎蜒颜奄衍掩眼演厌彦砚唁宴艳验谚堰焰雁燕";
	sg_indexLetterY[2].m_py = "ang";	sg_indexLetterY[2].m_pyTable = "央殃秧鸯扬羊阳杨佯疡洋仰养氧痒样漾";
	sg_indexLetterY[3].m_py = "ao";		sg_indexLetterY[3].m_pyTable = "侥妖腰邀尧姚窑谣摇遥瑶咬舀药要耀钥";
	sg_indexLetterY[4].m_py = "e";		sg_indexLetterY[4].m_pyTable = "椰噎爷耶也冶野业叶曳页夜掖液腋";
	sg_indexLetterY[5].m_py = "i";		sg_indexLetterY[5].m_pyTable = "一伊衣医依铱壹揖仪夷沂宜姨胰移遗颐疑彝乙已以矣蚁倚椅义亿忆艺议亦屹异役抑译邑易绎诣疫益谊翌逸意溢肄裔毅翼臆";
	sg_indexLetterY[6].m_py = "in";		sg_indexLetterY[6].m_pyTable = "因阴姻茵荫音殷吟寅淫银尹引饮隐印";
	sg_indexLetterY[7].m_py = "ing";	sg_indexLetterY[7].m_pyTable = "应英婴缨樱鹰迎盈荧莹萤营蝇赢颖影映硬";
	sg_indexLetterY[8].m_py = "o";		sg_indexLetterY[8].m_pyTable = "哟";
	sg_indexLetterY[9].m_py = "ong";	sg_indexLetterY[9].m_pyTable = "佣拥痈庸雍臃永咏泳勇涌恿蛹踊用";
	sg_indexLetterY[10].m_py = "ou";	sg_indexLetterY[10].m_pyTable = "优忧幽悠尤由犹邮油铀游友有酉又右幼佑诱釉";
	sg_indexLetterY[11].m_py = "u";		sg_indexLetterY[11].m_pyTable = "迂淤渝于予余盂鱼俞娱渔隅愉逾愚榆虞舆与宇屿羽雨禹语玉驭芋育郁狱峪浴预域欲喻寓御裕遇愈誉豫";
	sg_indexLetterY[12].m_py = "uan";	sg_indexLetterY[12].m_pyTable = "冤鸳渊元员园垣原圆袁援缘源猿辕远苑怨院愿";
	sg_indexLetterY[13].m_py = "ue";	sg_indexLetterY[13].m_pyTable = "曰约月岳悦阅跃粤越";
	sg_indexLetterY[14].m_py = "un";	sg_indexLetterY[14].m_pyTable = "云匀郧耘允陨孕运晕酝韵蕴";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterY;
	sg_headLetterIndex[index].m_tableSize = 15;
	++index;
	/*z*/                                                  
	sg_indexLetterZ[0].m_py = "a";		sg_indexLetterZ[0].m_pyTable = "匝杂砸咋";
	sg_indexLetterZ[1].m_py = "ai";		sg_indexLetterZ[1].m_pyTable = "灾哉栽宰载再在仔";
	sg_indexLetterZ[2].m_py = "an";		sg_indexLetterZ[2].m_pyTable = "咱攒暂赞";
	sg_indexLetterZ[3].m_py = "ang";	sg_indexLetterZ[3].m_pyTable = "赃脏葬";
	sg_indexLetterZ[4].m_py = "ao";		sg_indexLetterZ[4].m_pyTable = "遭糟凿早枣蚤澡藻灶皂造噪燥躁";
	sg_indexLetterZ[5].m_py = "e";		sg_indexLetterZ[5].m_pyTable = "则择泽责";
	sg_indexLetterZ[6].m_py = "ei";		sg_indexLetterZ[6].m_pyTable = "贼";
	sg_indexLetterZ[7].m_py = "en";		sg_indexLetterZ[7].m_pyTable = "怎";
	sg_indexLetterZ[8].m_py = "eng";	sg_indexLetterZ[8].m_pyTable = "增憎赠";
	sg_indexLetterZ[9].m_py = "ha";		sg_indexLetterZ[9].m_pyTable = "喳渣扎札轧闸铡眨乍诈炸榨柞";
	sg_indexLetterZ[10].m_py = "hai";	sg_indexLetterZ[10].m_pyTable = "斋摘宅翟窄债寨";
	sg_indexLetterZ[11].m_py = "han";	sg_indexLetterZ[11].m_pyTable = "沾毡粘詹瞻斩展盏崭辗占战栈站绽湛蘸";
	sg_indexLetterZ[12].m_py = "hang";	sg_indexLetterZ[12].m_pyTable = "长张章彰漳樟涨掌丈仗帐杖胀账障瘴";
	sg_indexLetterZ[13].m_py = "hao";	sg_indexLetterZ[13].m_pyTable = "招昭找沼召兆赵照罩肇爪";
	sg_indexLetterZ[14].m_py = "he";	sg_indexLetterZ[14].m_pyTable = "遮折哲蛰辙者锗这浙蔗着";
	sg_indexLetterZ[15].m_py = "hen";	sg_indexLetterZ[15].m_pyTable = "贞针侦珍真砧斟甄臻诊枕疹阵振镇震帧";
	sg_indexLetterZ[16].m_py = "heng";	sg_indexLetterZ[16].m_pyTable = "争征怔挣狰睁蒸拯整正证郑政症";
	sg_indexLetterZ[17].m_py = "hi";	sg_indexLetterZ[17].m_pyTable = "之支汁芝吱枝知织肢脂蜘执侄直值职植殖止只旨址纸指趾至志制帜治炙质峙挚秩致掷痔窒智滞稚置";
	sg_indexLetterZ[18].m_py = "hong";	sg_indexLetterZ[18].m_pyTable = "中忠终盅钟衷肿种仲众重";
	sg_indexLetterZ[19].m_py = "hou";	sg_indexLetterZ[19].m_pyTable = "州舟诌周洲粥轴肘帚咒宙昼皱骤";
	sg_indexLetterZ[20].m_py = "hu";	sg_indexLetterZ[20].m_pyTable = "朱诛株珠诸猪蛛竹烛逐主拄煮嘱瞩住助注贮驻柱祝著蛀筑铸";
	sg_indexLetterZ[21].m_py = "hua";	sg_indexLetterZ[21].m_pyTable = "抓";
	sg_indexLetterZ[22].m_py = "huai";	sg_indexLetterZ[22].m_pyTable = "拽";
	sg_indexLetterZ[23].m_py = "huan";	sg_indexLetterZ[23].m_pyTable = "专砖转撰篆";
	sg_indexLetterZ[24].m_py = "huang";	sg_indexLetterZ[24].m_pyTable = "妆庄桩装壮状幢撞";
	sg_indexLetterZ[25].m_py = "hui";	sg_indexLetterZ[25].m_pyTable = "追椎锥坠缀赘";
	sg_indexLetterZ[26].m_py = "hun";	sg_indexLetterZ[26].m_pyTable = "谆准";
	sg_indexLetterZ[27].m_py = "huo";	sg_indexLetterZ[27].m_pyTable = "卓拙捉桌灼茁浊酌啄琢";
	sg_indexLetterZ[28].m_py = "i";		sg_indexLetterZ[28].m_pyTable = "孜兹咨姿资淄滋籽子紫滓字自渍";
	sg_indexLetterZ[29].m_py = "ong";	sg_indexLetterZ[29].m_pyTable = "宗综棕踪鬃总纵";
	sg_indexLetterZ[30].m_py = "ou";	sg_indexLetterZ[30].m_pyTable = "邹走奏揍";
	sg_indexLetterZ[31].m_py = "u";		sg_indexLetterZ[31].m_pyTable = "租足卒族诅阻组祖";
	sg_indexLetterZ[32].m_py = "uan";	sg_indexLetterZ[32].m_pyTable = "赚纂钻";
	sg_indexLetterZ[33].m_py = "ui";	sg_indexLetterZ[33].m_pyTable = "嘴最罪醉";
	sg_indexLetterZ[34].m_py = "un";	sg_indexLetterZ[34].m_pyTable = "尊遵";
	sg_indexLetterZ[35].m_py = "uo";	sg_indexLetterZ[35].m_pyTable = "昨左佐作坐座做";
	sg_headLetterIndex[index].m_pyIndex = sg_indexLetterZ;
	sg_headLetterIndex[index].m_tableSize = 36;
	++index;
#if PINYIN_TEST > 0
	 printf((char *)"PinYin Initial Success\n");
#endif
}

/***************获取下一个输入法*******************
参数说明:
a_inputType :当前的输入法
返回值说明:下一个可用的输入法
***********************************************************/
InputType PY_GetNextInputType(const InputType a_inputType)
{
	InputType newInputType = a_inputType;
	do
	{
		if(newInputType == INPUT_TYPE_QUWEI)
		{
			newInputType = INPUT_TYPE_PINYIN;
		}
		else
		{
			newInputType = (InputType)(newInputType + 1);
		}
	}
	while(PY_CheckInputType(newInputType) == false);
#if PINYIN_TEST > 0
	 printf((char *)"Old InputType = %d\n", a_inputType);
	 printf((char *)"New InputType = %d\n", newInputType);
#endif
	return newInputType;
}

/*************检查输入法是否可用*****************
参数说明:
a_inputType :需要检查的输入法
返回值说明:
true :输入法可用
false :输入法不可用
***********************************************************/
BOOL PY_CheckInputType(const InputType a_inputType)
{
	bool check = false;
	if(((sg_inputFlag & PY_INPUT_CHINESE) != 0)
		&& (a_inputType == INPUT_TYPE_PINYIN))
	{
		check = true;
	}
	else if(((sg_inputFlag & PY_INPUT_LOWER_CASE) != 0)
		&& (a_inputType == INPUT_TYPE_LOWER_CASE))
	{
		check = true;
	}
	else if(((sg_inputFlag & PY_INPUT_UPPER_CASE) != 0)
		&& (a_inputType == INPUT_TYPE_UPPER_CASE))
	{
		check = true;
	}
	else if(((sg_inputFlag & PY_INPUT_NUMBER) != 0)
		&& (a_inputType == INPUT_TYPE_NUMBER))
	{
		check = true;
	}
	else if(((sg_inputFlag & PY_INPUT_QUWEI) != 0)
		&& (a_inputType == INPUT_TYPE_QUWEI))
	{
		check = true;
	}
#if PINYIN_TEST > 0
	printf((char *)"InputType = %d ", a_inputType);
	if(check != false)
	{
		printf((char *)"can use\n", a_inputType);
	}
	else
	{
		printf((char *)"can't use\n", a_inputType);
	}
#endif
	return check;
}

/**********获取第一个可用的输入法**************
参数说明:

返回值说明:第一个可用的输入法
***********************************************************/
InputType PY_GetFirstInputType(void)
{
	InputType firstInputType = INPUT_TYPE_PINYIN;

	for(firstInputType = INPUT_TYPE_PINYIN; PY_CheckInputType(firstInputType) == false; )
	{
		if(firstInputType == INPUT_TYPE_QUWEI)
		{
			firstInputType = INPUT_TYPE_PINYIN;
		}
		else
		{
			firstInputType = (InputType)(firstInputType + 1);
		}
	}
#if PINYIN_TEST > 0
	printf((char *)"FirstInputType = %d\n", firstInputType);
#endif
	return firstInputType;
}

/***************检查输入的拼音*********************
参数说明:
a_input :待检查的拼音字符串
返回值说明:
INPUT_NOT_FOUND :没有全匹配或部分匹配的拼音
INPUT_FOUND :全匹配的拼音
INPUT_INCLUDE :部分匹配的拼音
***********************************************************/
InputCheck PY_Check(const unsigned char *a_input)
{
	const PinYinIndex *cpHZ;
	unsigned int index;
	unsigned int maxIndex;
	unsigned int inputLen = strlen((const char *)a_input);
	InputCheck result = INPUT_NOT_FOUND;

#if PINYIN_TEST > 0
	 printf((char *)"Check PinYin = %s\n", a_input);
#endif

    if(inputLen != 0
	&& a_input[0] >= 'a'
	&& a_input[0] <= 'z')
    {
	    cpHZ = sg_headLetterIndex[a_input[0] - 'a'].m_pyIndex;/*查首字母索引*/
		maxIndex = sg_headLetterIndex[a_input[0] - 'a'].m_tableSize;/*设置指针界限*/

		for(index = 0; index < maxIndex; ++index, ++cpHZ)/*索引表不超界*/
	 	{
	 		//字母串部分匹配
			if(strncmp(cpHZ->m_py, (const char*)&a_input[1], inputLen - 1) == 0)
			{
				//字母串全匹配
				if(strcmp(cpHZ->m_py, (const char*)&a_input[1]) == 0)
				{
					result = INPUT_FOUND;
				}
				else
				{
					result = INPUT_INCLUDE;
				}
				break;
			}
	    }
    }
#if PINYIN_TEST > 0
	 printf((char *)"Check PinYin result = %d\n", result);
#endif
    return result;/*无果而终*/
}

/*****获取输入的拼音对应的中文字表*********
参数说明:
a_input :待检查的拼音字符串
返回值说明:
NULL :没有全匹配的拼音
其他:对应的中文字表
***********************************************************/
const unsigned char *PY_Ime
	(
	const unsigned char *a_input
	)
{
	const PinYinIndex *cpHZ;
	unsigned int index;
	unsigned int maxIndex;
	const unsigned char *result = NULL;

#if PINYIN_TEST > 0
	 printf((char *)"IME PinYin = %s\n", a_input);
#endif

	//如果输入空字符返回0
    if(strlen((const char *)a_input) != 0
	&& a_input[0] >= 'a'
	&& a_input[0] <= 'z')
	{
	    cpHZ = sg_headLetterIndex[a_input[0] - 'a'].m_pyIndex;/*查首字母索引*/
		maxIndex = sg_headLetterIndex[a_input[0] - 'a'].m_tableSize;/*设置指针界限*/

		for(index = 0; index < maxIndex; ++index, ++cpHZ)/*索引表不超界*/
		{
	    	if(strcmp(cpHZ->m_py,(const char*)&a_input[1]) == 0)/*字母串全匹配*/
	    	{
				result = (const unsigned char *)(cpHZ->m_pyTable);
	    	}
		}
	}
#if PINYIN_TEST > 0
	if(result == NULL)
	{
		 printf((char *)"No such pinyin\n");
	}
	else
	{
		 printf((char *)"PinYin Chinese Table = %s\n", result);
	}
#endif
    return result;/*无果而终*/
}

/*****************显示输入的字符*******************
参数说明:
ucLine :字符串显示的行数
a_input :需显示字符串
返回值说明:

***********************************************************/
void PY_Display
	(
	unsigned char ucLine,
	const unsigned char * aucInBuff
	)
{
	unsigned int uiLen;
	unsigned char aucBuf[MAX_DISPLAY + 1];
	unsigned char ucIndex;
	unsigned char ucDisplayIndex;
	unsigned char ucGb2312Count;

	uiLen = strlen((char  *)aucInBuff);
	memset(aucBuf, 0x00, sizeof(aucBuf));

#if PINYIN_TEST > 0
	printf((char *)"Input Buffer = %s\n", aucInBuff);
#endif

	if(uiLen < MAX_DISPLAY)
	{
		strcpy((char *)aucBuf, (const char *)aucInBuff);
	}
	else
	{
		//中英文混合显示,检查是否有半个中文字符
		ucGb2312Count = 0;
		for(ucIndex = 1; ucIndex < MAX_DISPLAY; ++ucIndex)
		{
			if(aucInBuff[uiLen - ucIndex] > MAX_SIGNED_CHAR)
			{
				++ucGb2312Count;
			}
		}
#if PINYIN_TEST > 0
		printf((char *)"GB2312 Count = %d\n", ucGb2312Count);
#endif
		if((ucGb2312Count & 0x01) == 0x01)
		{
			ucDisplayIndex = MAX_DISPLAY - 2;
		}
		else
		{
			ucDisplayIndex = MAX_DISPLAY - 1;
		}
		strncpy((char *)aucBuf, (const char *)&aucInBuff[uiLen - ucDisplayIndex], ucDisplayIndex);
	}
	//由于中文字库的下划线有问题，不使用，用英文的下划线代替
#if 0
	strcat((char *)aucBuf, "_");
#endif
	Os__clr_display(ucLine);
	Os__GB2312_display(ucLine, 0, aucBuf);
	Os__display(ucLine, strlen((const char*)aucBuf)/3*2, (unsigned char *)"_");
#if PINYIN_TEST > 0
	printf((char *)"Display In Screen = %s_\n", aucBuf);
#endif
}

/*****************按键获取函数*******************
参数说明:
a_ucWaitTime :输入的总时间,单位10毫秒
返回值说明:
NULL :等待按键超时
其他:用户按键
备注:与普通UTIL_GetKey函数有所区别,
            对于超时的返回值和参数的含义有区别
***********************************************************/
unsigned char PY_GetKey(unsigned int a_ucWaitTime)
{
	DRV_OUT *pdKey;

	pdKey = Os__get_key();
	do
	{
		--a_ucWaitTime;
		Os__xdelay(1);
	}
	while((pdKey->gen_status == DRV_RUNNING) && (a_ucWaitTime != 0));

	if(a_ucWaitTime == 0)
	{
		Os__abort_drv(drv_mmi);
		return NULL;
	}
	
#if PINYIN_TEST > 0
	 printf((char *)"PinYin get key = %02x\n", pdKey->xxdata[1]);
#endif
	
	return pdKey->xxdata[1];
}

/*****************显示输入的字符*******************
参数说明:
pucInData :输入的中文字表
pucOutData :用户选择的中文字
uiTimeout :输入的总时间,单位10毫秒
返回值说明:
ERR_CANCEL :用户输入取消 0x02
ERR_END :输入超时
SUCCESS :选择成功
备注:中文字表显示在第3行,
            选择的序号显示在第4行的上半行
***********************************************************/
unsigned char PY_ChooseBuffer
	(
	const unsigned char *pucInData,
	unsigned char *pucOutData
	)
{
	unsigned char    ucBuffIndex;
	unsigned char    ucChooseIndex;
	unsigned char    ucIndex;
	unsigned char    ucKey;
	unsigned int    uiChineseSize;
	unsigned char    aucDisplayBuf[MAX_DISPLAY + 1];
	BOOL choosed = false;
	unsigned char aucDisplayTmp[MAX_DISPLAY+1];
	unsigned char aucDisplayNumber[MAX_DISPLAY + 1];
	const unsigned int uiInLen = strlen((const char*)pucInData);

	//使用绘图函数清除第4行的上半行
	
	//ASC_DrawAscString(48, 0, 8, aucDisplayNumber);
	OSMMI_DisplayASCII(0x30, 4, 0,"                        ");
	ucBuffIndex = 0;
	while(choosed == false)
	{
		//显示中文字表
		memset(aucDisplayBuf, 0, sizeof(aucDisplayBuf));
		strncpy((char *)aucDisplayBuf, (const char *)(&pucInData[ucBuffIndex]), MAX_DISPLAY);
		Os__clr_display(2);
#if PINYIN_TEST > 0
		 printf((char *)"Chinese table can choose = %s\n", aucDisplayBuf);
		 printf((char *)"Chinese table index = %d\n", ucBuffIndex);
#endif
		//Os__GB2312_display(2, 0, aucDisplayBuf);
		memset(aucDisplayTmp,0,sizeof(aucDisplayTmp));
	
		sprintf(aucDisplayTmp,"%d/%d",(ucBuffIndex/24)+1,(uiInLen/24)+1);
		OSMMI_DisplayASCII(0x30, 3, 19,aucDisplayTmp);
		
		OSMMI_DisplayASCII(0x30, 3, 0,aucDisplayBuf);
		
		//显示可选择的序号
		uiChineseSize = strlen((const char *)aucDisplayBuf) / 3;
		
		 
		memset(aucDisplayTmp,0,sizeof(aucDisplayTmp));
		memcpy(aucDisplayTmp,"1 2 3 4 5 6 7 8 ",16);
		memset(aucDisplayNumber, 0, sizeof(aucDisplayNumber)); 
		
		memcpy(aucDisplayNumber,aucDisplayTmp,uiChineseSize*2);
		sprintf(aucDisplayNumber,"%s             ",aucDisplayNumber);

		OSMMI_DisplayASCII(0x30, 4, 0,aucDisplayNumber);
		//等待按键
		ucKey = PY_GetKey(sg_uiTrueTimeOut);
		if(ucKey == NULL)
		{
			ucKey = KEY_CLEAR;
		}
		
		switch(ucKey)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			ucChooseIndex = ucKey - '1';
			if(ucChooseIndex < uiChineseSize)
			{
				int i=0;
				printf("\n%d %s\n",ucChooseIndex,&aucDisplayBuf[0]);
				memcpy(pucOutData, &aucDisplayBuf[ucChooseIndex  * 3], 3);
				for(i=0;i<24;i++)
					printf(" %x ",aucDisplayBuf[i]);
				printf("\n");
				
				pucOutData[3] = 0;
#if PINYIN_TEST > 0
				 printf((char *)"Choose a chinese char = %s\n", pucOutData);
				 printf((char *)"Chinese char index = %d\n", ucChooseIndex * 3);
#endif
				choosed = true;
			}
			break;
		case KEY_F4:
			//向下翻页
			if(uiInLen > ucBuffIndex + MAX_DISPLAY)
			{
				ucBuffIndex += MAX_DISPLAY;
			}
			break ;
		case KEY_F3:
			//向上翻页
			if(ucBuffIndex >= MAX_DISPLAY)
			{
				ucBuffIndex -= MAX_DISPLAY;
			}
			break ;
		case KEY_CLEAR:
		case KEY_BCKSP:
			choosed = true;
			break;
		default:
			break;
		}
	}
	memset(aucDisplayNumber, 0, sizeof(aucDisplayNumber));
//	ASC_DrawAscString(48, 0, 8, aucDisplayNumber);
	OSMMI_DisplayASCII(0x30, 4, 0,"                        ");
	if(ucKey == KEY_CLEAR || ucKey == KEY_BCKSP)
	{
		return ERR_CANCEL;
	}
	else
	{
		return SUCCESS;
	}
}

/*************输入单个字母和数字*****************
参数说明:
pucBuf :用户输入的单个字符
返回值说明:
KEY_ENTER :确认插入字符
KEY_BCKSP :删除用户输入的最后一个字符
SUCCESS :用户希望退出并保存已输入的字符
KEY_CLEAR :用户按取消键,
                     删除已输入字符,个数为0则退出并且不保存
KEY_PAPER_FEED :切换输入法
备注:若为英文输入,则将用户可选的字符
            显示在第4行的下半行
***********************************************************/
unsigned char PY_InputLetter
	(
	unsigned char *pucBuf
	)
{
	static const unsigned char aucUpperCaseLeterTab[][21] =
	{{"0,.-!\"%\'()/:;<=>?*"},
	{"1QZ# +-"},
	{"2ABC"},
	{"3DEF"},
	{"4GHI"},
	{"5JKL"},
	{"6MNO"},
	{"7PRS"},
	{"8TUV"},
	{"9WXY"}};

	static const unsigned char aucLowerCaseLeterTab[][21] =
	{{"0,.-!\"%\'()/:;<=>?*"},
	{"1qz# +-"},
	{"2abc"},
	{"3def"},
	{"4ghi"},
	{"5jkl"},
	{"6mno"},
	{"7prs"},
	{"8tuv"},
	{"9wxy"}};

	static const unsigned char aucNumberTab[][2] =
	{{"0"},
	{"1"},
	{"2"},
	{"3"},
	{"4"},
	{"5"},
	{"6"},
	{"7"},
	{"8"},
	{"9"},
	{"0"}};
	static const unsigned char aucSymbolTab[] =
	{",.-!\"%\'()+/:;<=>?"};
	const unsigned char *(aucKeyTab[11]);

	//显示输入法名称
	unsigned char aucDisp[5];
	//当前输入的字符
	unsigned char ucCurrentInput;   
	unsigned char ucKey;
	unsigned char ucLastKey;
	unsigned char ucEnd;
	unsigned int uiTimeout;
	unsigned char index;
	unsigned char indexFont;
	unsigned char indexChar;
	unsigned char aucAscFont[ASC_SIZE + 1];
	//初始化变量
	memset(&aucDisp[0],0,sizeof(aucDisp));
	pucBuf[0] = 0;
	pucBuf[1] = 0;
	ucCurrentInput = 0;
	ucLastKey = 0;
	ucEnd = FALSE;
	//panmin
	#if 1
	sg_ucPreviousKey = 0;
	sg_ucPreviousChar = 0;
	sg_ucKeyTabOffset=0;
	#endif
	//根据输入法选择字符串表
	switch(sg_inputType)
	{
	case INPUT_TYPE_UPPER_CASE:
		for(index = 0; index < 10; ++index)
		{
			aucKeyTab[index] = aucUpperCaseLeterTab[index];
		}
		aucKeyTab[index] = aucSymbolTab;
		strcpy((char *)aucDisp,"ABC:");
		break;
	case INPUT_TYPE_LOWER_CASE:
		for(index = 0; index < 10; ++index)
		{
			aucKeyTab[index] = aucLowerCaseLeterTab[index];
		}
		aucKeyTab[index] = aucSymbolTab;
		strcpy((char *)aucDisp,"abc:");
		break;
	case INPUT_TYPE_NUMBER:
		for(index = 0; index < 11; ++index)
		{
			aucKeyTab[index] = aucNumberTab[index];
		}
		strcpy((char *)aucDisp,"123:");
		break;
	default:
		return KEY_CLEAR;
		break;
	}
	//显示当前输入法
	ASC_DrawAscString(56, 0, 6, aucDisp);

	//恢复用户上一次的按键
	if(sg_ucPreviousChar != 0 && sg_inputType != INPUT_TYPE_NUMBER)
	{
		ucCurrentInput = sg_ucPreviousChar;
		ucLastKey = sg_ucPreviousKey;
#if PINYIN_TEST > 0
		 printf((char *)"In sg_ucPreviousChar = %c\n", sg_ucPreviousChar);
		 printf((char *)"In sg_ucPreviousKey = %c\n", sg_ucPreviousKey);
#endif
	}

	do
	{
		//显示用户可选择的字符，
		//当前用户停留的字符用黑底白字显示
		// printf((char *)"ucflag3 = %02x\n", ucflag);

		//等待1秒按键(可修改停留时间)
		uiTimeout = LETTER_INPUT_INTERVAL;
		ucKey = PY_GetKey(sg_uiTrueTimeOut);
		//总输入时间超时
		if(sg_uiTrueTimeOut == 0)
		{
			sg_ucPreviousKey = 0;
			sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;

			pucBuf[0] = 0;
			ucKey = KEY_CLEAR;
			break;
		}

		if(ucKey == NULL)
		{
			//超时则把当前的选择结果保存并退出
			sg_ucPreviousKey = 0;
			sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;

			pucBuf[0] = ucCurrentInput;
			ucKey = KEY_ENTER;
			break;
		}
		// printf("ucKey= %02x\n",ucKey);
		if((ucKey >= '0') && (ucKey <= '9'))
		{
			if(sg_inputType != INPUT_TYPE_NUMBER)
			{
				{
#if PINYIN_TEST > 0
					 printf((char *)"Different key in letter\n");
#endif
#if 0
					sg_ucKeyTabOffset = 0;
					ucLastKey = ucKey;
					{
						//不同按键,且已输入过字符,
						//保存本次字符,插入上次字符
						sg_ucPreviousKey = ucKey;
						
						sg_ucPreviousChar = aucKeyTab[ucKey - '0'][sg_ucKeyTabOffset];
						pucBuf[0] = sg_ucPreviousChar;
						ucKey = KEY_ENTER;
						ucEnd = TRUE;
					}
#else
					sg_ucKeyTabOffset = 0;
					ucLastKey = ucKey;
					{
						//不同按键,且已输入过字符,
						//保存本次字符,插入上次字符
						sg_ucPreviousKey = ucKey;
						// printf((char *)"ucKey = %02x\n", ucKey);
						ASC_DrawAscString(56,24,6,(const unsigned char*)"");
						if(sg_ucPreviousKey != 0)
						{
							indexChar = sg_ucPreviousKey - '0';
#if PINYIN_TEST > 0
							 printf((char *)"Char table = %s\n", aucKeyTab[indexChar]);
							if(ucCurrentInput != 0)
							{
								 printf((char *)"Current char = %c\n", ucCurrentInput);
							}
#endif
							for(index = 0; aucKeyTab[indexChar][index] != 0; ++index)
							{
								 ASC_GetAscFont(aucKeyTab[indexChar][index], aucAscFont);
								if(sg_ucKeyTabOffset == index)
								{
									for(indexFont = 0; indexFont < ASC_SIZE; indexFont += 2)
									{
										aucAscFont[indexFont] = (~aucAscFont[indexFont]);
									}
								}
								Os__graph_display
									(
									56,
									index * 6 + 24,
									aucAscFont,
									ASC_SIZE
									);
							}
						}

						while (1)
						{
						ucKey = PY_GetKey(sg_uiTrueTimeOut);
						if((ucKey >= '0') && (ucKey <= '9')||(ucKey=KEY_CLEAR))	
							{
							break;
							}
						}
						if(ucKey==KEY_CLEAR)
							{
							return ERR_CANCEL;
							}
						{
						sg_ucPreviousChar = aucKeyTab[sg_ucPreviousKey - '0'][ucKey-'1'];
							
						pucBuf[0] = sg_ucPreviousChar;
						}
						ucKey = KEY_ENTER;
						ucEnd = TRUE;
					}
#endif					
				}
				
			}
			else
			{
				//数字输入法下的数字按键总是退出(为了直接显示)
				pucBuf[0] = aucKeyTab[ucKey - '0'][0];
				ucKey = KEY_ENTER;
				ucEnd = TRUE;
			}
		}
		else 
		if(ucKey == KEY_00_PT)
		{
	//		#if PINYIN_TEST > 0
			// printf((char *)"sg_inputType= %d\n", sg_inputType);
	//		#endif	
			if(sg_inputType != INPUT_TYPE_NUMBER)
			{
			#if 0
	//	#if PINYIN_TEST > 0
					// printf((char *)"sg_ucPreviousChar= %d\n", sg_ucPreviousChar);
	//	#endif	
				if(sg_ucPreviousChar != 0)
				{
					//00按键,切换到下个和可选字符
					if(sg_ucKeyTabOffset < strlen((char *)aucKeyTab[sg_ucPreviousKey-'0']) - 1)
					{
						sg_ucKeyTabOffset++;
					}
					else
					{
						sg_ucKeyTabOffset = 0;
					}
#if PINYIN_TEST > 0
					 printf((char *)"Same key in letter, offset = %d\n", sg_ucKeyTabOffset);
#endif
					ucCurrentInput = aucKeyTab[sg_ucPreviousKey - '0'][sg_ucKeyTabOffset];
					pucBuf[0] = ucCurrentInput;
					ucEnd = TRUE;
				}
			#endif	
			}
			else
			{
				strcpy((char *)pucBuf, "00");
				ucKey = KEY_ENTER;
				ucEnd = TRUE;
			}
		}
		else
		{
			//处理各种功能键
			ucLastKey = ucKey;
			switch(ucKey)
			{
			case KEY_ENTER:
				ucKey = SUCCESS;
				sg_ucPreviousKey = 0;
				sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;
				ucEnd = TRUE;
				break;
			case KEY_CLEAR:
			case KEY_BCKSP:
				sg_ucPreviousKey = 0;
				sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;
				ucEnd = TRUE;
				break;
			case KEY_PAPER_FEED:
				ucEnd = TRUE;
				break;
			default :
				break;
			}
		}
	// printf((char *)"ucflag2 = %02x\n", ucflag);
	}
	while(ucEnd == FALSE);    
#if PINYIN_TEST > 0
	if(sg_ucPreviousChar != 0)
	{
		 printf((char *)"Out sg_ucPreviousChar = %c\n", sg_ucPreviousChar);
		 printf((char *)"Out sg_ucPreviousKey = %c\n", sg_ucPreviousKey);
		 printf((char *)"Out sg_ucKeyTabOffset = %d\n", sg_ucKeyTabOffset);
	}
#endif
	return(ucKey);
}

/*****插入用户输入的拼音字符，并检查******
参数说明:
pucInput :待插入的拼音
ucInputOffset :插入的偏移量
pucKeyTab :可插入的字符表
pucKeyTabOffset :字符表的起始偏移量,
                           改变后的偏移量需要被使用,用指针传送
返回值说明:同PY_Check的返回值说明
***********************************************************/
InputCheck PY_InsertPinYin
	(
	unsigned char *pucInput,
	const unsigned char ucInputOffset,
	const unsigned char *pucKeyTab,
	unsigned char *pucKeyTabOffset
	)
{
	unsigned char index;
	InputCheck inputCheck;
	int maxKeyLen = strlen((char *)pucKeyTab);
	//从字符表的起始偏移量开始将字符逐个插入检查
	for(index = 0; index < maxKeyLen; ++index)
	{
#if PINYIN_TEST > 0
		 printf((char *)"KeyTabOffset = %d\n", *pucKeyTabOffset);
#endif
		pucInput[ucInputOffset] = pucKeyTab[*pucKeyTabOffset];
		inputCheck = PY_Check(pucInput);
		if(inputCheck == INPUT_NOT_FOUND)
		{
			if(((*pucKeyTabOffset) + 1) < maxKeyLen)
			{
				++(*pucKeyTabOffset);
			}
			else
			{
				(*pucKeyTabOffset) = 0;
			}
		}
		else
		{
			break;
		}
	}
	return inputCheck;
}

/***************拼音输入单个中文字*******************
参数说明:
pucBuf :用户输入的单个中文字
返回值说明:
KEY_ENTER :确认插入字符
KEY_BCKSP :删除用户输入的最后一个字符
SUCCESS :用户希望退出并保存已输入的字符
KEY_CLEAR :用户按取消键,
                     删除已输入字符,个数为0则退出并且不保存
KEY_PAPER_FEED :切换输入法
备注:用户输入的拼音显示在第4行的下半行
***********************************************************/
unsigned char PY_InputChinese
	(
	unsigned char *pucBuf
	)
{
	const unsigned char aucKeyTab[][7] =
	{
		{"qz"},
		{"abc"},
		{"def"},
		{"ghi"},
		{"jkl"},
		{"mno"},
		{"prs"},
		{"tuv"},
		{"wxy"},
	};
	unsigned char   aucDisp[25];
	unsigned char   aucInput[20];
	unsigned char   ucKey;
	unsigned char   ucLastKey;
	unsigned char   ucKeyTabOffset;
	unsigned char   ucEnd;
	unsigned char   ucChooseResult;
	unsigned char   ucRedraw;
	unsigned char   ucCount;
	unsigned char   ucOffset;
	unsigned int uiLastTime;
	InputCheck inputCheck;
	const unsigned char *pucChinese;
	unsigned int maxKeyLen;
	unsigned char ucLastChar;

	memset(aucDisp,0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = TRUE;
	uiLastTime = sg_uiTrueTimeOut;
	ucCount = 0;

	do
	{
		if(ucRedraw == TRUE)
		{
			//显示拼音
			Os__clr_display(2);
			memset(aucDisp,0,sizeof(aucDisp));

			strcpy((char *)aucDisp, "PinYin:");
			strcat((char *)aucDisp, (const char *)aucInput);
		
#if PINYIN_TEST > 0
			printf((char *)"Input PinYin = %s\n", aucDisp);
#endif
			strcat((char *)aucDisp, "_");
		//ASC_DrawAscString(56, 0, 6, aucDisp);
			OSMMI_DisplayASCII(0x30, 6, 0,"                        ");
			OSMMI_DisplayASCII(0x30, 6, 0,aucDisp);
			pucChinese = PY_Ime(aucInput);
			if(pucChinese != NULL)
			{
				//若有匹配的拼音则显示中文
				memset(aucDisp,0,sizeof(aucDisp));
				strncpy((char *)aucDisp, (const char*)pucChinese, 24);
			//	Os__GB2312_display(2,0,aucDisp);
				OSMMI_DisplayASCII(0x30, 3, 0,aucDisp);
			}
			ucRedraw = FALSE;
		}

		//等待按键
		uiLastTime = sg_uiTrueTimeOut;
		ucKey = PY_GetKey(sg_uiTrueTimeOut);
		if(ucKey == NULL)
		{
			ucKey = KEY_CLEAR;
			break;
		}

		if((ucKey >= '1') && (ucKey <= '9'))
		{
			if(ucLastKey != ucKey)
			{
				//不同按键尝试插入新字符
				ucKeyTabOffset = 0;
				inputCheck = PY_InsertPinYin(aucInput, ucCount, aucKeyTab[ucKey - '1'], &ucKeyTabOffset);
#if PINYIN_TEST > 0
				printf((char *)"Different key = %c\n", ucKey);
#endif
				if(inputCheck == INPUT_NOT_FOUND)
				{
					//所有的字符均不可插入
					ucLastKey = 0;
					ucRedraw = FALSE;
					ucKeyTabOffset = 0;
				}
				else
				{
					//找到第一个可插入的字符
					ucLastKey = ucKey;
					ucRedraw = TRUE;
					++ucCount;
				}
				aucInput[ucCount] = 0;
			}
			else
			{
				if((uiLastTime - sg_uiTrueTimeOut) < PINYIN_INPUT_INTERVAL)
				{
					if(ucCount)
					{
						ucOffset = ucCount - 1;
					}
					else
					{
						ucOffset = 0;
					}
					maxKeyLen = strlen((char *)aucKeyTab[ucKey - '1']) - 1;
					if(ucKeyTabOffset < maxKeyLen)
					{
						++ucKeyTabOffset;
					}
					else
					{
						ucKeyTabOffset = 0;
					}
					ucLastChar = aucInput[ucOffset];
					//在这种情况中总是存在一个可插入的字符
					//所以不对inputCheck做判断
					inputCheck = PY_InsertPinYin(aucInput, ucOffset, aucKeyTab[ucKey - '1'], &ucKeyTabOffset);
#if PINYIN_TEST > 0
					printf((char *)"Same key = %c\n",ucKey);
#endif
					//只有一种组合时将时间增加,
					//目的是将相同按键转变作为下一个按键
					if(ucLastChar == aucInput[ucOffset])
					{
						uiLastTime = sg_uiTrueTimeOut + PINYIN_INPUT_INTERVAL;
					}
					else
					{
						ucRedraw = TRUE;
					}
				}
				if((uiLastTime - sg_uiTrueTimeOut) >= PINYIN_INPUT_INTERVAL)
				{
					//处理方法同两次不同按键
					ucKeyTabOffset = 0;
					inputCheck = PY_InsertPinYin(aucInput, ucCount, aucKeyTab[ucKey - '1'], &ucKeyTabOffset);
#if PINYIN_TEST > 0
					 printf((char *)"Same key = %c, timeout\n",ucKey);
#endif
					if(inputCheck == INPUT_NOT_FOUND)
					{
						ucLastKey = 0;
						ucRedraw = FALSE;
						ucKeyTabOffset = 0;
					}
					else
					{
						ucLastKey = ucKey;
						ucRedraw = TRUE;
						++ucCount;
					}
					aucInput[ucCount] = 0;
				}
			}
		}
		else
		{
			//处理各种功能键
			ucLastKey = ucKey;
			switch(ucKey)
			{
			case KEY_ENTER:
				if(ucCount == 0)
				{
					ucKey = SUCCESS;
					ucEnd = TRUE;
				}
				else if((pucChinese = PY_Ime(aucInput)) != NULL)
				{
					ucChooseResult = PY_ChooseBuffer(pucChinese,pucBuf);
					if(sg_uiTrueTimeOut == 0)
					{
						ucEnd = TRUE;
						ucKey = KEY_CLEAR;
					}
					else if(ucChooseResult == SUCCESS)
					{
						ucEnd = TRUE;
					}
					else if(ucChooseResult == ERR_CANCEL)
					{
							OSMMI_DisplayASCII(0x30, 3, 19,"    ");
					}
					
				}
				else
				{
					Os__beep();
				}
				break;
			case KEY_CLEAR:
				if(ucCount)
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					
					ucRedraw = TRUE;
				}
				else
				{
					ucEnd = TRUE;
				}
				break;
			case KEY_BCKSP:
				if(ucCount)
				{
					ucCount--;
					aucInput[ucCount] = 0;
					
					ucRedraw = TRUE;
				}
				else
				{
					ucEnd = TRUE;
				}
				break;
			case KEY_PAPER_FEED:
				ucEnd = TRUE;
				break;
			default :
				break;
			}
		}
	}
	while(ucEnd == FALSE);

	return(ucKey);
}

#if SI_JIAO > 0
/***************区位码输入单个中文字*******************
参数说明:
pucBuf :用户输入的单个中文字
返回值说明:
KEY_ENTER :确认插入字符
KEY_BCKSP :删除用户输入的最后一个字符
SUCCESS :用户希望退出并保存已输入的字符
KEY_CLEAR :用户按取消键,
                     删除已输入字符,个数为0则退出并且不保存
KEY_PAPER_FEED :切换输入法
备注:用户输入的区位码显示在第4行的下半行
***********************************************************/
unsigned char PY_InputSiJiao
	(
	unsigned char *pucBuf
	)
{
	unsigned char aucDispBuf[5];
	unsigned char ucKey;
	unsigned char ucEnd;
	unsigned char ucCount;
	unsigned long qm;
	unsigned long wm;

	ucEnd = FALSE;
	ucCount = 0;
	memset(aucDispBuf, 0, sizeof(aucDispBuf));

	do
	{
		ASC_DrawAscString(56, 0, 6, (const unsigned char*)"QuWei:");
		ASC_DrawAscString(56, 36, 6, aucDispBuf);
		ASC_DrawAscString(56, 36 + ucCount * 6, 6, (const unsigned char*)"_");
		ucKey = PY_GetKey(sg_uiTrueTimeOut);
		switch(ucKey)
		{
		case KEY_ENTER:
			if(ucCount >= 4)
			{
				qm = asc_long(&aucDispBuf[0], 2);
				wm = asc_long(&aucDispBuf[2], 2);
				qm = qm + 0xA0;
				wm = wm + 0xA0;
				long_tab(&pucBuf[0], 1, &qm);
				long_tab(&pucBuf[1], 1, &wm);
				pucBuf[2] = 0;
				ucCount = 0;
				ucEnd = TRUE;
			}
			else if(ucCount == 0)
			{
				ucKey = SUCCESS;
				ucEnd = TRUE;
			}
			break;
		case KEY_BCKSP:
			if(ucCount != 0)
			{
				--ucCount;
				aucDispBuf[ucCount] = 0x00;
			}
			else
			{
				ucEnd = TRUE;
			}
			break;
		case KEY_CLEAR:
			if(ucCount != 0)
			{
				ucCount = 0;
				memset(aucDispBuf, 0, sizeof(aucDispBuf));
			}
			else
			{
				ucEnd = TRUE;
			}
			break;
		case KEY_PAPER_FEED:
			ucEnd = TRUE;
			break;
		default:
			if((ucKey >= '0')
			&& (ucKey <= '9'))
			{
				if(//超出范围
				(ucCount >= 4)
				//前两位不可以同时为0
				|| ((ucCount == 1)
					&& (aucDispBuf[ucCount - 1] == '0')
					&& (ucKey == '0'))
				//后两位不可以同时为0
				|| ((ucCount == 3)
					&& (aucDispBuf[ucCount - 1] == '0')
					&& (ucKey == '0')))
				{
					Os__beep();
				}
				else
				{
					aucDispBuf[ucCount] = ucKey;
					ucCount++;
				}
			}
			break;
		}
	}
	while(ucEnd == FALSE);

	return(ucKey);
}
#endif

unsigned char PY_Input
	(
	unsigned char *pucBuf
	)
{
	static const unsigned char aucUpperCaseLeterTab[][21] =
	{{"0!\"%\'()./:;<=>?*,"},
	{"1QZ# +-"},
	{"2ABC"},
	{"3DEF"},
	{"4GHI"},
	{"5JKL"},
	{"6MNO"},
	{"7PRS"},
	{"8TUV"},
	{"9WXY"}};

	static const unsigned char aucLowerCaseLeterTab[][21] =
	{{"0!\"%\'()./:;<=>?*,"},
	{"1qz# +-"},
	{"2abc"},
	{"3def"},
	{"4ghi"},
	{"5jkl"},
	{"6mno"},
	{"7prs"},
	{"8tuv"},
	{"9wxy"}};

	static const unsigned char aucNumberTab[][2] =
	{{"0"},
	{"1"},
	{"2"},
	{"3"},
	{"4"},
	{"5"},
	{"6"},
	{"7"},
	{"8"},
	{"9"},
	{"0"}};
	static const unsigned char aucSymbolTab[] =
	{"!\"%\'()+-./:;<=>?"};
	const unsigned char *(aucKeyTab[11]);

	//显示输入法名称
	unsigned char aucDisp[5];
	//当前输入的字符
	unsigned char ucCurrentInput;   
	unsigned char ucKey;
	unsigned char ucLastKey;
	unsigned char ucEnd;
	unsigned int uiTimeout;
	unsigned char index;
	unsigned char indexFont;
	unsigned char indexChar;
	unsigned char aucAscFont[ASC_SIZE + 1];
	unsigned char ucflag=0;
	//初始化变量
	memset(&aucDisp[0],0,sizeof(aucDisp));
	pucBuf[0] = 0;
	pucBuf[1] = 0;
	ucCurrentInput = 0;
	ucLastKey = 0;
	ucEnd = FALSE;
	//panmin
	#if 1
	sg_ucPreviousKey = 0;
	sg_ucPreviousChar = 0;
	sg_ucKeyTabOffset=0;
	#endif
	//根据输入法选择字符串表
	switch(sg_inputType)
	{
	case INPUT_TYPE_UPPER_CASE:
		for(index = 0; index < 10; ++index)
		{
			aucKeyTab[index] = aucUpperCaseLeterTab[index];
		}
		aucKeyTab[index] = aucSymbolTab;
		strcpy((char *)aucDisp,"ABC:");
		break;
	case INPUT_TYPE_LOWER_CASE:
		for(index = 0; index < 10; ++index)
		{
			aucKeyTab[index] = aucLowerCaseLeterTab[index];
		}
		aucKeyTab[index] = aucSymbolTab;
		strcpy((char *)aucDisp,"abc:");
		break;
	case INPUT_TYPE_NUMBER:
		for(index = 0; index < 11; ++index)
		{
			aucKeyTab[index] = aucNumberTab[index];
		}
		strcpy((char *)aucDisp,"123:");
		break;
	default:
		return KEY_CLEAR;
		break;
	}
	//显示当前输入法
	ASC_DrawAscString(56, 0, 6, aucDisp);

	//恢复用户上一次的按键
	if(sg_ucPreviousChar != 0 && sg_inputType != INPUT_TYPE_NUMBER)
	{
		ucCurrentInput = sg_ucPreviousChar;
		ucLastKey = sg_ucPreviousKey;
#if PINYIN_TEST > 0
		 printf((char *)"In sg_ucPreviousChar = %c\n", sg_ucPreviousChar);
		 printf((char *)"In sg_ucPreviousKey = %c\n", sg_ucPreviousKey);
#endif
	}

	do
	{
		//显示用户可选择的字符，
		//当前用户停留的字符用黑底白字显示
		 printf((char *)"ucflag3 = %02x\n", ucflag);

	

		//等待1秒按键(可修改停留时间)
		uiTimeout = LETTER_INPUT_INTERVAL;
		ucKey = PY_GetKey(sg_uiTrueTimeOut);
		//总输入时间超时
		if(sg_uiTrueTimeOut == 0)
		{
			sg_ucPreviousKey = 0;
			sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;

			pucBuf[0] = 0;
			ucKey = KEY_CLEAR;
			break;
		}

		if(ucKey == NULL)
		{
			//超时则把当前的选择结果保存并退出
			sg_ucPreviousKey = 0;
			sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;

			pucBuf[0] = ucCurrentInput;
			ucKey = KEY_ENTER;
			break;
		}

		if((ucKey >= '0') && (ucKey <= '9'))
		{
			if(sg_inputType != INPUT_TYPE_NUMBER)
			{
				{
#if PINYIN_TEST > 0
					 printf((char *)"Different key in letter\n");
#endif
#if 0
					sg_ucKeyTabOffset = 0;
					ucLastKey = ucKey;
					{
						//不同按键,且已输入过字符,
						//保存本次字符,插入上次字符
						sg_ucPreviousKey = ucKey;
						
						sg_ucPreviousChar = aucKeyTab[ucKey - '0'][sg_ucKeyTabOffset];
						pucBuf[0] = sg_ucPreviousChar;
						ucKey = KEY_ENTER;
						ucEnd = TRUE;
					}
#else
						sg_ucKeyTabOffset = 0;
					ucLastKey = ucKey;
					{
						//不同按键,且已输入过字符,
						//保存本次字符,插入上次字符
						sg_ucPreviousKey = ucKey;
						 printf((char *)"ucKey = %02x\n", ucKey);
						ASC_DrawAscString(56,24,6,(const unsigned char*)"");
						if(sg_ucPreviousKey != 0)
						{
							indexChar = sg_ucPreviousKey - '0';
#if PINYIN_TEST > 0
							 printf((char *)"Char table = %s\n", aucKeyTab[indexChar]);
							if(ucCurrentInput != 0)
							{
								 printf((char *)"Current char = %c\n", ucCurrentInput);
							}
#endif
							for(index = 0; aucKeyTab[indexChar][index] != 0; ++index)
							{
								 ASC_GetAscFont(aucKeyTab[indexChar][index], aucAscFont);
								if(sg_ucKeyTabOffset == index)
								{
									for(indexFont = 0; indexFont < ASC_SIZE; indexFont += 2)
									{
										aucAscFont[indexFont] = (~aucAscFont[indexFont]);
									}
								}
								Os__graph_display
									(
									56,
									index * 6 + 24,
									aucAscFont,
									ASC_SIZE
									);
							}
						}
/*
						switch(ucflag)
						{
						case 0:
							ucflag=1;
						break;
						case 1:
							ucflag=2;
						break;
						case 2:
							ucflag=1;
						break;
						default :
						break;
						}
						 printf((char *)"ucflag = %02x\n", ucflag);
						if(ucflag==1)
							{
							continue;
							}
							*/
						while (1)
						{
						ucKey = PY_GetKey(sg_uiTrueTimeOut);
						if((ucKey >= '0') && (ucKey <= '9')||(ucKey=KEY_CLEAR))	
							{
							break;
							}
						}
						if(ucKey==KEY_CLEAR)
							{
							return ERR_CANCEL;
							}
						{
						sg_ucPreviousChar = aucKeyTab[sg_ucPreviousKey - '0'][ucKey-'1'];
							
						pucBuf[0] = sg_ucPreviousChar;
						}
						ucKey = KEY_ENTER;
						ucEnd = TRUE;
					}
#endif
					
				}
				
			}
			else
			{
				//数字输入法下的数字按键总是退出(为了直接显示)
				pucBuf[0] = aucKeyTab[ucKey - '0'][0];
				ucKey = KEY_ENTER;
				ucEnd = TRUE;
			}
		}
		else if(ucKey == KEY_00_PT)
		{
			if(sg_inputType != INPUT_TYPE_NUMBER)
			{
			#if 1
		#if PINYIN_TEST > 0
					 printf((char *)"sg_ucPreviousChar= %d\n", sg_ucPreviousChar);
		#endif	
				if(sg_ucPreviousChar != 0)
				{
					//00按键,切换到下个和可选字符
					if(sg_ucKeyTabOffset < strlen((char *)aucKeyTab[sg_ucPreviousKey-'0']) - 1)
					{
						sg_ucKeyTabOffset++;
					}
					else
					{
						sg_ucKeyTabOffset = 0;
					}
#if PINYIN_TEST > 0
					 printf((char *)"Same key in letter, offset = %d\n", sg_ucKeyTabOffset);
#endif
					ucCurrentInput = aucKeyTab[sg_ucPreviousKey - '0'][sg_ucKeyTabOffset];
					pucBuf[0] = ucCurrentInput;
					ucEnd = TRUE;
				}
			#endif	
			}
			else
			{
				strcpy((char *)pucBuf, "00");
				ucKey = KEY_ENTER;
				ucEnd = TRUE;
			}
		}
		else
		{
			//处理各种功能键
			ucLastKey = ucKey;
			switch(ucKey)
			{
			case KEY_ENTER:
				ucKey = SUCCESS;
				sg_ucPreviousKey = 0;
				sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;
				ucEnd = TRUE;
				break;
			case KEY_CLEAR:
			case KEY_BCKSP:
				sg_ucPreviousKey = 0;
				sg_ucPreviousChar = 0;
				sg_ucKeyTabOffset=0;
				ucEnd = TRUE;
				break;
			case KEY_PAPER_FEED:
				ucEnd = TRUE;
				break;
			default :
				break;
			}
		}
	 printf((char *)"ucflag2 = %02x\n", ucflag);

	}
	while(ucEnd == FALSE);    
#if PINYIN_TEST > 0
	if(sg_ucPreviousChar != 0)
	{
		 printf((char *)"Out sg_ucPreviousChar = %c\n", sg_ucPreviousChar);
		 printf((char *)"Out sg_ucPreviousKey = %c\n", sg_ucPreviousKey);
		 printf((char *)"Out sg_ucKeyTabOffset = %d\n", sg_ucKeyTabOffset);
	}
#endif
	return(ucKey);
}
