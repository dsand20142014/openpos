#ifndef OSTOOLS_H_INCLUDED
#define OSTOOLS_H_INCLUDED


#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup 系统工具函数
 * @author 熊剑
 * @brief 提供对各种格式的字符,数字等之间的相互转换
 * @{
 */

/**
 * @def FALSE
 * @brief 操作失败
 **/
#ifndef FALSE
#define FALSE 		0
#endif

/**
 * @def TRUE
 * @brief 操作成功
 **/
#ifndef TRUE
#define TRUE 			1
#endif

 
/**
 * @fn unsigned char *asc_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts, unsigned char Lgs)
 * @brief	转换ACSII码到BCD码，即取ASCII码的各个字节低4位，置于对应目的字符串的对应的字节位置;
 *			转换是将Pts的源字节串转换为Pts的半字节到目的串中
 *			目标字符串的长度：
 *		    如果Lgd>Lgs/2 ，则为LgDest的字节数
 *		    如果Lgd<Lgs/2 且Lgs为偶数，则为Lgs/2字节数
 *		    如果Lgd<Lgs/2 且Lgs为奇数，则为(Lgs＋1)/2字节数
 *        	如果Lgd>Lgs/2，目标串在转换前被置为0x00。
 *		    如果Lgs为奇数，转换将从目标串的第二个半字节开始(即第一字节的低4位)
 *
 * @param Ptd	目的字符串
 * @param Lgd	目的字符串长度
 * @param Pts	源字符串
 * @param Lgs	源字符串长度
 *
 * @return 		目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","12345");
 * 	
 * asc_bcd(Ptdest, 3, Ptsrc, 4);
 * for(i=0;i<3;i++)
 * printf("0x%x/",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_bcd(Ptdest, 2, Ptsrc, 4);
 * for(i=0;i<3;i++)
 * printf("0x%x/",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_bcd(Ptdest, 3, Ptsrc, 5);
 * for(i=0;i<3;i++)
 * printf("0x%x/",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_bcd(Ptdest, 2, Ptsrc, 5);
 * for(i=0;i<3;i++)
 * printf("0x%x/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = '12345' = 0x31/0x32/0x33/0x34/0x35
 * if Lgs = 4 and Lgd = 3        Ptd = 0x00/0x12/0x34
 * if Lgs = 4 and Lgd < Lgs/2  Ptd = 0x12/0x34
 * if Lgs = 5 and Lgd = 3        Ptd = 0x01/0x23/0x45
 * if Lgs = 5 and Lgd < Lgs/2  Ptd = 0x01/0x23/0x45
 * @endcode
 */
unsigned char *asc_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts, unsigned char Lgs);

/**
 * @fn unsigned long asc_long(unsigned char *Pts, unsigned char Ls)
 * @brief 	转换ASCII码到无符号长整形,扩展每个ASCII源字节到它的低4位,
 *			并将其转换为相应的无符号长整形目标数数值.
 *
 * @param Pts 	源字符串
 * @param Ls  	源字符串长度
 *
 * @return 		无符号长整形数.
 * @warning 	该函数转换的数应小于1000000000.
 *
 * 例
 * @code
 * unsigned char Ptsrc[10];
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","123");
 * printf("%u\n",asc_long(Ptsrc,3));
 * printf("%u\n",asc_long(Ptsrc,2));
 * //运行结果
 * Pts = "123"
 * if Ls = 3     return  123
 * if Ls = 2     return  12
 * @endcode
 */
unsigned long asc_long(unsigned char *Pts, unsigned char Ls);

/**
 * @fn unsigned char *asc_hex(unsigned char *Ptd, unsigned char Ld, unsigned char *Pts, unsigned char Ls)
 * @brief 	转换ASCII码到十六进制码;
 *			根据值的范围，每个ASCII源字节(Oi)将被转换到目的串的对应的半字节
 *        	for Oi = '0' to '9' (0x30 to 0x39) => 0 to 9
 *        	for Oi = 'A' to 'F' (0x41 to 0x46) => A to F
 *		    转换是将Pts的源字节串转换为Pts的半字节到目的串中
 *		    目标串(Ptd)在转换前通常被置为0x00。
 *		    如果Ls为奇数，转换将从目标串的第二个"半字节"开始(即第一字节的低4位)
 *
 * @param Ptd 目的字符串
 * @param Ld  目的字符串长度
 * @param Pts 源字符串
 * @param Ls  源字符串长度
 *
 * @return 	目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","1AC45");
 * asc_hex(Ptdest,3 , Ptsrc,4);
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_hex(Ptdest,2 , Ptsrc,4);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_hex(Ptdest,4 , Ptsrc,5);
 * for(i=0;i<4;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_hex(Ptdest,3 , Ptsrc,5);
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = '1AC45'
 * if Ls = 4 and Ld = 3        Ptd = 0x00/0x1A/0xC4
 * if Ls = 4 and Ld < Ls/2  Ptd = 0x1A/0xC4
 * if Ls = 5 and Ld = 4        Ptd = 0x00/0x01/0xAC/0x45
 * if Ls = 5 and Ld < Ls/2  Ptd = 0x01/0xAC/0x45
 * @endcode
 */
unsigned char *asc_hex(unsigned char *Ptd, unsigned char Ld, unsigned char *Pts, unsigned char Ls);

/**
 * @fn unsigned char *bcd_asc(unsigned char *Ptd , unsigned char *Pts, unsigned char Lg)
 * @brief 	将BCD码转换为ASCII码；
 *			以1字节为单位，将源字节串的每个"半字节"(Qi)转换成ASCII值 
 *        	for Oi = '0' to '9' (0x30 to 0x39) => 0 to 9;
 *        	for Oi = 'A' to 'F' (0x41 to 0x46) => A to F;
 *		    转换是将Pts的源字节串中的每半字节转换到目的串中;
 *		    目标串(Ptd)在转换前通常被置为0x00
 *		    如果Lg为奇数,转换将从源字符串的第二个"半字节"开始(即第一字节的低4位)
 *
 * @param Ptd 目的字符串
 * @param Pts 源字符串
 * @param Lg  转换到目的字符串的长度
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * Ptsrc[0] |=0x12;
 * Ptsrc[1] |=0x34;
 * printf("0x%X/",Ptsrc[0]);
 * printf("0x%X/\n",Ptsrc[1]);
 * bcd_asc(Ptdest, Ptsrc, 1);
 * for(i=0;i<1;i++)
 * printf("%c",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * bcd_asc(Ptdest, Ptsrc, 2);
 * for(i=0;i<2;i++)
 * printf("%c",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * bcd_asc(Ptdest, Ptsrc, 3);
 * for(i=0;i<3;i++)
 * printf("%c",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * bcd_asc(Ptdest, Ptsrc, 4);
 * for(i=0;i<4;i++)
 * printf("%c",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = 0x12/0x34
 * if Lg = 1	ptDest = '2'
 * if Lg = 2  ptDest = '12'
 * if Lg = 3  ptDest = '234'
 * if Lg = 4  ptDest = '1234'
 * @endcode
 */
unsigned char *bcd_asc(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg);

/**
 * @fn unsigned long bcd_long(unsigned char *Pts, unsigned char Ls)
 * @brief	转换BCD码到无符号长整形;
 *			把源串的每个半字节转换为相应的数字,保存为无符号长整形目标数
 *		    转换是将Pts的源字节串中的每半字节转换到目的串中;
 *		    如果Ls为奇数,转换将从源字符串的第二个"半字节"开始(即第一字节的低4位)
 *
 * @param Pts 源字符串
 * @param Ls  源字符串长度
 *
 * @return 返回处理过的无符号长整形数据
 *
 * 例
 * @code
 * unsigned char Ptsrc[10];
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * Ptsrc[0] |=0x12;
 * Ptsrc[1] |=0x34;
 * printf("%u\n",bcd_long(Ptsrc,1));
 * printf("%u\n",bcd_long(Ptsrc,2));
 * printf("%u\n",bcd_long(Ptsrc,3));
 * printf("%u\n",bcd_long(Ptsrc,4));
 * //运行结果
 * Pts = 0x12/0x34
 * if Lg = 1	ptDest = '2'
 * if Lg = 2  ptDest = '12'
 * if Lg = 3  ptDest = '234'
 * if Lg = 4  ptDest = '1234'
 * @endcode
 */
unsigned long bcd_long(unsigned char *Pts, unsigned char Ls);

/**
 * @fn unsigned char *bcd_str( unsigned char *Ptd , unsigned char *Pts , unsigned char Lg )
 * @brief	将字符串每"半个字节"转换为一个字节的ASCII码(从0x30 到0x39),
 *			并且在所得字符串的结尾以'\0'字符结束;
 *	     	这个过程是将Lg个“半个字符”转换为Lg个字节长度的字符串;
 *	     	如果Lg是奇数,转换将会从源字符串的第二个“半个字符”开始(从第一个字节的低4位开始)
 *
 * @param Ptd 目的字符串
 * @param Pts 源字符串
 * @param Lg  源字符串长度
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * Ptsrc[0] |=0x12;
 * Ptsrc[1] |=0x34;
 * bcd_str(Ptdest,Ptsrc,1);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * bcd_str(Ptdest,Ptsrc,2);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * bcd_str(Ptdest,Ptsrc,3);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * bcd_str(Ptdest,Ptsrc,4);
 * printf("%s\n",Ptdest);
 * //运行结果
 * ptSrc = 0x12/0x34
 * if Lg = 1	ptDest = '2'
 * if Lg = 2  ptDest = '12'
 * if Lg = 3  ptDest = '234'
 * if Lg = 4  ptDest = '1234'
 * @endcode
 */
unsigned char *bcd_str(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg);

/**
 * @fn unsigned char *char_bcd(unsigned char *Ptd ,unsigned char Lgd,unsigned char *Pts)
 * @brief	将unsigned char类型(字符的10进制)数据转换为对应的BCD码字符串,
 *		    目标字符串(Ptd)在转换前总是赋值为0x00;
 *		    unsigned char类型的数据最大值有3个阿拉伯数字,
 *		    因此转换后的BCD数据最多是3个“半个字符”组成的字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * Ptsrc[0] = 'z';
 * char_bcd(Ptdest, 1, Ptsrc);
 * for(i=0;i<1;i++)
 * printf("0x%X",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * char_bcd(Ptdest, 4, Ptsrc);
 * for(i=0;i<4;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = 'z' = 0x7A = 122
 * if Lgd = 1 	Ptd = 0x22
 * if Lgd = 4 	Ptd = 0x00/0x00/0x01/0x22
 * @endcode
 */
unsigned char *char_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts);

/**
 * @fn unsigned char *char_asc(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
 * @brief	将unsigned char类型(字符的10进制)数据转换为对应的;
 *			ASCII码字符串(ASCII value = digit value + 0x30);
 *	     	目标字符串(Ptd)在转换前总是赋值为0x30(字符'0');
 *	     	unsigned char 类型的数据最大值有3个阿拉伯数字,
 *			因此转换后的字符串最多是3个字符的阿拉伯字符组成的ASCII 字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * Ptsrc[0] = 'z';
 * char_asc(Ptdest,2, Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * char_asc(Ptdest,5, Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 'z' = 0x7A = 122
 * if Lgd = 2 Ptd = 22
 * if Lgd = 5 Ptd = 00122
 * @endcode
 */
unsigned char *char_asc(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts);

/**
 * @fn unsigned char *char_str(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
 * @brief	将unsigned  char 类型(字符的10进制)数据转换为对应的
 *			ASCII 码字符串 (ASCII value = digit value + 0x30);
 *	     	目标字符串(Ptd)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *      	unsigned char类型的数据最大值有3个阿拉伯数字,
 *			因此转换后的字符串最多是3个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址,转换后字符串以'\0'字符结束
 *
 * 例
 * @code
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * Ptsrc[0] = 'z';
 * char_str(Ptdest,2, Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * char_str(Ptdest,5, Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 'z' = 0x7A = 122
 * if Lgd = 2 	Ptd = "22"
 * if Lgd = 5 	Ptd = "00122"
 * @endcode
 */
unsigned char *char_str(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts);

/**
 * @fn unsigned short checksum(unsigned char *Pt, unsigned char *Ptf, unsigned short Val)
 * @brief	'checksum' 的计算(数组中所有字符的总和);
 * 			使用初始值'Val' 计算从数组的起始字节 'Pt'累加到最后一个字节'Ptf';

 *
 * @param Pt 数组起始字节
 * @param Ptf 数组最后一个字节
 * @param Val checksum初始值
 *
 * @return 计算后的Checksum值
 *
 * 例
 * @code
 * unsigned char Pt[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
 * unsigned short value = 0;
 * printf("0x%X\n",checksum(Pt, Pt+7,value));
 * //运行结果
 * Pt = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08}
 * value = 0
 * return 0x1C
 * @endcode
 */
unsigned short checksum(unsigned char *Pt, unsigned char *Ptf, unsigned short Val);

/**
 * @fn unsigned char *hex_asc(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg)
 * @brief 转换十六进制数到ASCII码;
 *        如果0 to 9 => '0' to '9' (0x30 to 0x39);
 *        如果 A to F => 'A' to 'F' (0x41 to 0x46). 
 *
 * @param Ptd 目的字符串
 * @param Lg  目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * Ptsrc[0] = 0x1A;
 * Ptsrc[1] = 0xC4;
 * hex_asc(Ptdest,Ptsrc,1);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * hex_asc(Ptdest,Ptsrc,2);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * hex_asc(Ptdest,Ptsrc,3);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * hex_asc(Ptdest,Ptsrc,4);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 0x1A/0xC4
 * if Lg = 1  Ptd = 'A'
 * if Lg = 2  Ptd = '1A'
 * if Lg = 3  Ptd = 'AC4'
 * if Lg = 4  Ptd = '1AC4'
 * @endcode
 */
unsigned char *hex_asc(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg);

/**
 * @fn unsigned char *hex_str(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg)
 * @brief 转换十六进制数到字符串;
 *        如果0 to 9 => '0' to '9' (0x30 to 0x39);
 *        如果 A to F => 'A' to 'F' (0x41 to 0x46). 
 *
 * @param Ptd 目的字符串
 * @param Lg  目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * Ptsrc[0] = 0x1A;
 * Ptsrc[1] = 0xC4;
 * hex_str(Ptdest,Ptsrc,1);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * hex_str(Ptdest,Ptsrc,2);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * hex_str(Ptdest,Ptsrc,3);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * hex_str(Ptdest,Ptsrc,4);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 0x1A/0xC4
 * if Lg = 1  Ptd = 'A'
 * if Lg = 2  Ptd = '1A'
 * if Lg = 3  Ptd = 'AC4'
 * if Lg = 4  Ptd = '1AC4'
 * @endcode
 */
unsigned char *hex_str(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg);

/**
 * @fn unsigned char *int_asc(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts)
 * @brief	将unsigned int型数据转换为ASCII码(从0~9)
 *			字符串(ASCII value = digit value + 0x30)
 *			目标字符串(Ptd)在转换前总是赋值为0x30(字符'0') 
 *			unsigned int类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的字符串最多是5个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned int Ptsrc = 12345;
 * int_asc(Ptdest, 2, &Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * int_asc(Ptdest, 7, &Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 12345
 * if Lgd = 2  Ptd = '45'
 * if Lgd = 7  Ptd = '0012345'
 * @endcode
 */
unsigned char *int_asc(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts);

/**
 * @fn unsigned char *int_bcd(unsigned char *Ptd ,unsigned char Lgd, unsigned int *Pts)
 * @brief	将unsigned int类型(字符的10进制)数据转换为对应的
 *			BCD码字符串,目标字符串（lgDest）在转换前总是赋值为0x00;
 *	    	unsigned int类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的BCD数据最多是5个"半个字符"组成的字符串. 
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned int Ptsrc = 12345;
 * int_bcd(Ptdest, 2, &Ptsrc);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * int_bcd(Ptdest, 5, &Ptsrc);
 * for(i=0;i<5;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = 12345 
 * if Lgd = 2 Ptd = 0x23/0x45 
 * if Lgd = 5 Ptd = 0x00/0x00/0x01/0x23/0x45 
 * @endcode
 */
unsigned char *int_bcd(unsigned char *Ptd ,unsigned char Lgd,unsigned int *Pts);

/**
 * @fn unsigned char *int_str(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts)
 * @brief	将unsigned int类型(字符的10进制)数据转换为对应的
 * 			ASCII码字符串(ASCII value = digit value + 0x30);
 *	   		目标字符串(lgDest)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *			unsigned int类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的字符串最多是5个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址,转换后字符串以'\0'字符结束
 *
 * 例
 * @code
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned int Ptsrc = 12345;
 * int_str(Ptdest, 2, &Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * int_str(Ptdest, 5, &Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 12345
 * if Lgd = 2  Ptd = '45'
 * if Lgd = 7  Ptd = '0012345'
 * @endcode
 */
unsigned char *int_str(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts);

/**
 * @fn unsigned char *long_asc(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts)
 * @brief	将unsigned int类型(字符的10进制)数据转换为对应的
 *			ASCII 码字符串(ASCII value = digit value + 0x30); 
 *	    	目标字符串(Lgd)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *	    	unsigned int类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的字符串最多是5个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned long Ptsrc = 12345;
 * long_asc(Ptdest, 2, &Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * long_asc(Ptdest, 5, &Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 12345
 * if Lgd = 2  Ptd = '45'
 * if Lgd = 7  Ptd = '0012345'
 * @endcode
 */
unsigned char *long_asc(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts);

/**
 * @fn unsigned char *long_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts)
 * @brief	将unsigned long类型(字符的10进制)数据转换为对应的
 *			BCD码字符串(ASCII value = digit value + 0x30); 
 *	    	目标字符串(Lgd)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *	    	unsigned int类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的字符串最多是5个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned long Ptsrc = 1234567890;
 * long_bcd(Ptdest, 2, &Ptsrc);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * long_bcd(Ptdest, 7, &Ptsrc);
 * for(i=0;i<7;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * Pts = 1234567890
 * if Lgd = 2  Ptd = 0x78/0x90
 * if Lgd = 7  Ptd = 0x00/0x00/0x12/0x34/0x56/0x78/0x90
 * @endcode
 */
unsigned char *long_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts);

/**
 * @fn unsigned char *long_str(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts)
 * @brief	将unsigned int类型(字符的10进制)数据转换为对应的
 *			ASCII 码字符串(ASCII value = digit value + 0x30); 
 *	    	目标字符串(Lgd)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *	    	unsigned int类型的数据最大值有10个阿拉伯数字,
 *			因此转换后的字符串最多是10个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址,转换后字符串以0x00字符结束
 *
 * 例
 * @code
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned long Ptsrc = 12345;
 * long_str(Ptdest, 2, &Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * long_str(Ptdest, 5, &Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 12345
 * if Lgd = 2  Ptd = '45'
 * if Lgd = 7  Ptd = '0012345'
 * @endcode
 */
unsigned char *long_str(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts);

/**
 * @fn unsigned char *short_asc(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
 * @brief	将unsigned short类型(字符的10进制)数据转换为对应的
 *			ASCII 码字符串(ASCII value = digit value + 0x30); 
 *	    	目标字符串(Lgd)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *	    	unsigned short类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的字符串最多是5个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned short Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned long Ptsrc = 12345;
 * short_asc(Ptdest, 2, &Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * short_asc(Ptdest, 5, &Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 12345
 * if Lgd = 2  Ptd = '45'
 * if Lgd = 7  Ptd = '0012345'
 * @endcode
 */
unsigned char *short_asc(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts);

/**
 * @fn unsigned char *short_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
 * @brief	将unsigned short类型(字符的10进制)数据转换为对应的
 *			ASCII 码字符串(ASCII value = digit value + 0x30); 
 *	    	目标字符串(Lgd)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *	    	unsigned short类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的BCD数据最多是5个“半个字符”组成的字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned short Ptsrc = 12345;
 * short_bcd(Ptdest, 2, &Ptsrc);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * short_bcd(Ptdest, 5, &Ptsrc);
 * for(i=0;i<5;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = 12345 
 * if Lgd = 2 Ptd = 0x23/0x45 
 * if Lgd = 5 Ptd = 0x00/0x00/0x01/0x23/0x45 
 * @endcode
 */
unsigned char *short_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts);

/**
 * @fn unsigned char *short_str(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
 * @brief	将unsigned short类型(字符的10进制)数据转换为对应的
 *			ASCII 码字符串(ASCII value = digit value + 0x30); 
 *	    	目标字符串(Lgd)在转换前总是赋值为0x30(字符'0');
 *	    	在目标字符串的结尾会增加一个'\0'字符;
 *	    	unsigned short类型的数据最大值有5个阿拉伯数字,
 *			因此转换后的字符串最多是5个字符的阿拉伯字符组成的ASCII字符串.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * unsigned char Ptdest[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * unsigned short Ptsrc = 12345;
 * short_str(Ptdest, 2, &Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * short_str(Ptdest, 5, &Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = 12345
 * if Lgd = 2  Ptd = '45'
 * if Lgd = 7  Ptd = '0012345'
 * @endcode
 */
unsigned char *short_str(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts);

/**
 * @fn unsigned char *str_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
 * @brief	此函数和'asc_bcd'函数类似;
 *	    	他将截断每个ASCII码的低4位为对应目标字符串的"半个字节";
 *	    	如果Lgd > strlen(Pts)/2,目标字符串前补0x00;
 *	    	如果源字符串的长度是奇数,则转换将从第二个字符的"半个字节"开始.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","12345");
 * str_bcd(Ptdest,4, &Ptsrc);
 * for(i=0;i<4;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * str_bcd(Ptdest,2, &Ptsrc);
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = "12345" = 0x31/0x32/0x33/0x34/0x35/'\0'
 * if Lgd = 4 (> strlen(Pts)/2) 	Ptd = 0x00/0x01/0x23/0x45
 * if Lgd < strlen(Pts)/2 				Ptd = 0x01/0x23/0x45
 * @endcode
 */
unsigned char *str_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts);

/**
 * @fn unsigned char *str_hex(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
 * @brief	此函数和 'asc_bcd'函数类似，lgSrc = strlen(ptSrc);
 *		    他将截断每个ASCII 码的低4位为对应目标字符串的“半个字节”;
 *		    如果 Oi = '0' to '9' (0x30 to 0x39) => 0 to 9
 *		    如果 Oi = 'A' to 'F' (0x41 to 0x46) => A to F
 *		    目标字符串(lgDest)在转换前总是赋值为0x00;
 *		    如果原字符串的长度是奇数,则转换将从第二个字符的“半个字节”开始.
 *
 * @param Ptd 目的字符串
 * @param Lgd 目的字符串长度
 * @param Pts 源字符串
 *
 * @return 目的字符串首地址
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","1AC45");
 * asc_hex(Ptdest,3 , Ptsrc,4);
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_hex(Ptdest,2 , Ptsrc,4);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_hex(Ptdest,4 , Ptsrc,5);
 * for(i=0;i<4;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 	
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * asc_hex(Ptdest,3 , Ptsrc,5);
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = '1AC45'
 * if Ls = 4 and Ld = 3        Ptd = 0x00/0x1A/0xC4
 * if Ls = 4 and Ld < Ls/2  Ptd = 0x1A/0xC4
 * if Ls = 5 and Ld = 4        Ptd = 0x00/0x01/0xAC/0x45
 * if Ls = 5 and Ld < Ls/2  Ptd = 0x01/0xAC/0x45
 * ptSrc = '1AC45'
 * if lgDest = 4 (> strlen(ptSrc)/2) ptDest = 0x00/0x01/0xAC/0x45
 * if lgDest < strlen(ptSrc)/2 ptDest = 0x01/0xAC/0x45.
 * @endcode
 */
unsigned char *str_hex(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts);

/**
 * @fn unsigned long str_long(unsigned char *Pts)
 * @brief	该函数和'asc_long'函数类似;它将截断每个字符
 *			的低4位，同时转换为unsigned long型对应的数据.
 *
 * @param Pts 源字符串
 *
 * @return 转换后的数据
 *
 * @warning 该函数目前使用的范围是:数据 < 1000000000;
 *
 * 例
 * @code
 * unsigned char Ptsrc[10];
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","123");
 * printf("%u\n",str_long(Ptsrc));
 * //运行结果
 * Pts = "123" => return  123
 * @endcode
 */
unsigned long str_long(unsigned char *Pts);

/**
 * @fn void format_date(unsigned char *Ptd, unsigned char *Pts)
 * @brief	将传入的六位日期字符串MMDDYY转换为MM/DD/YY格式,
 *
 * @param Pts 源日期字符串地址
 * @param Ptd 目的日期字符串地址
 *
 * @return 无返回值
 *
 * @warning 如果源日期字符串超出六位字符,目的字符串只截取前六位。
 *
 * 例
 * @code
 * unsigned char Ptdest[10],Ptsrc[10];
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","122311");
 * format_date(Ptdest,Ptsrc);
 * printf("%s\n",Ptdest);
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","122311");
 * format_date(Ptdest,Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = "122311" => Ptd = "12/23/11"
 * Pts = "122311095122" => Ptd = "12/23/11"
 * @endcode
 */
void format_date(unsigned char *Ptd, unsigned char *Pts);

/**
 * @fn unsigned char ebcdic_asc (unsigned char *Pts, unsigned char Lg)
 * @brief	将广义二进制编码的十进制交换码(EBCDIC)转化为ASCII码.
 *
 * @param Pts 源EBCDIC首地址
 * @param Lg  需要转换的EBCDIC的长度
 *
 * @return 成功转换返回1,否则返回0.
 *
 * 例
 * @code
 * unsigned char Ptsrc1[5] = {0xC1,0xD1,0xF0,0xF9,0x00};
 * unsigned char Ptsrc2[5] = {0xC1,0xD1,0xF0,0xF9,0x00};
 * ebcdic_asc(Ptsrc1,4);
 * *(Ptsrc1+4) = 0x00;
 * printf("%s\n",Ptsrc1);
 * ebcdic_asc(Ptsrc2,4);
 * *(Ptsrc2+4) = 0x00;
 * printf("%s\n",Ptsrc2);
 * //运行结果
 * Pts = 0xC1/0xD1/0xF0/0xF9
 * if Lg = 2, Pts = "AJ"
 * if Lg = 4, Pts = "AJ09"
 * @endcode
 */
unsigned char ebcdic_asc (unsigned char *Pts, unsigned char Lg);

/**
 * @fn unsigned char *asc_ebcdic(unsigned char *Pts, unsigned char Lg)
 * @brief 将ASCII码转换为广义二进制编码的十进制交换码(EBCDIC).
 *
 * @param Pts 源ASCII码首地址
 * @param Lg  需要转换的ASCII码的长度
 *
 * @return 返回转换成功的EBCDIC首地址.
 *
 * 例
 * @code
 * unsigned char Ptsrc1[] = "AJ09";
 * unsigned char Ptsrc2[] = "AJ09";
 *
 * asc_ebcdic(Ptsrc1,2);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptsrc1[i]);
 * printf("\n");
 * 
 * asc_ebcdic(Ptsrc2,4);
 * for(i=0;i<4;i++)
 * printf("0x%X/",Ptsrc2[i]);
 * printf("\n");
 * //运行结果
 * Pts = "AJ09"
 * if Lg = 2, Pts = 0xC1/0xD1
 * if Lg = 4, Pts = 0xC1/0xD1/0xF0/0xF9
 * @endcode
 */
unsigned char *asc_ebcdic(unsigned char *Pts, unsigned char Lg);

/**
 * @fn unsigned char *str_ebcdic(unsigned char *Pts)
 * @brief 将字符串转换为广义二进制编码的十进制交换码(EBCDIC).
 *
 * @param Pts 源字符串首地址
 *
 * @return 返回转换成功的EBCDIC首地址.
 *
 * 例
 * @code
 * unsigned char Ptsrc[] = "AJ09";
 * str_ebcdic(Ptsrc);
 * for(i=0;i<4;i++)
 * printf("0x%X/",Ptsrc[i]);
 * printf("\n");
 * //运行结果
 * Pts = "AJ09"
 * Pts = 0xC1/0xD1/0xF0/0xF9
 * @endcode
 */
unsigned char *str_ebcdic(unsigned char *Pts);

/**
 * @fn unsigned char *long_tab(unsigned char *Ptd ,unsigned char Lgd,unsigned long *Pts)
 * @brief 将无符号长整型数转化为16进制数组.
 *
 * @param Ptd 目的16进制数组地址
 * @param Lgd 需要转换的16进制数组长度
 * @param Pts 源长整型数首地址
 *
 * @return 返回无符号16进制数组首地址.
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10];
 * unsigned long Ptsrc = 1234608850L;
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * long_tab(Ptdest,2,&Ptsrc);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * long_tab(Ptdest,6,&Ptsrc);
 * for(i=0;i<6;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = 1234608850 = 0x49/0x96/0xA2/0xD2
 * if Lgd = 2, Ptd = 0xA2/0xD2
 * if Lgd = 6, Ptd = 0x00/0x00/0x49/0x96/0xA2/0xD2
 * @endcode
 */
unsigned char *long_tab(unsigned char *Ptd ,unsigned char Lgd,unsigned long *Pts);

/**
 * @fn unsigned long tab_long(unsigned char *Pts, unsigned char Ls)
 * @brief	把16进制数组转化成无符号的长整形数;'tab_long'函数
 *			和'long_tab'函数在功能上是相反的,它将ptSrc里面的
 *			16进制的数据,转换为unsigned long类型的数据.
 *
 * @param Pts 源16进制数组地址
 * @param Ls  需要转换的16进制数组长度
 *
 * @return 返回成功转换后的无符号长整型数.
 *
 * @warning 该函数目前计算出的数据 < 1000000000(Ls 在1~4之间).
 * 例
 * @code
 * unsigned char Ptsrc[10] = {0x49,0x96,0x02,0xD2};
 * printf("%u\n",tab_long(Ptsrc,2));
 * printf("%u\n",tab_long(Ptsrc,4));
 * //运行结果
 * Pts = 0x49/0x96/0x02/0xD2 (= '1234567890')
 * if Ls = 2, return = 0x4996 = 18838
 * if Ls = 4, return = 0x499602D2 = 1234567890
 * @endcode
 */
unsigned long tab_long(unsigned char *Pts,unsigned char Ls);

/**
 * @fn unsigned char *short_tab(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
 * @brief 将无符号短整型数转化为16进制数组.
 *
 * @param Ptd 目的16进制数组地址
 * @param Lgd 需要转换的16进制数组长度
 * @param Pts 源短整型数首地址
 *
 * @return 返回无符号16进制数组首地址.
 *
 * @warning 无符号短整型占两个字节,最大值为65535,
 *					超出系统自动截取!
 * 
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10];
 * unsigned short Ptsrc = 41393;
 * printf("%u\n",Ptsrc);
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * short_tab(Ptdest,1,&Ptsrc);
 * for(i=0;i<1;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * short_tab(Ptdest,3,&Ptsrc);
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = 41393 = 0xA1/0xB1
 * if Lgd = 1, Ptd = 0xB1
 * if Lgd = 3, Ptd = 0x00/0xA1/0xB1
 * @endcode
 */
unsigned char *short_tab(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts);

/**
 * @fn unsigned char *int_tab(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts)
 * @brief 将无符号整型数转化为16进制数组.
 *
 * @param Ptd 目的16进制数组地址
 * @param Lgd 需要转换的16进制数组长度
 * @param Pts 源整型数首地址
 *
 * @return 返回无符号16进制数组首地址.
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptdest[10];
 * unsigned int Ptsrc = 1234608850;
 * printf("%u\n",Ptsrc);
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * int_tab(Ptdest,2,&Ptsrc);
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * 
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * int_tab(Ptdest,4,&Ptsrc);
 * for(i=0;i<4;i++)
 * printf("0x%X/",Ptdest[i]);
 * printf("\n");
 * //运行结果
 * Pts = 1234608850 = 0x49/0x96/0xA2/0xD2
 * if Lgd = 2, Ptd = 0xA2/0xD2
 * if Lgd = 4, Ptd = 0x49/0x96/0xA2/0xD2
 * @endcode
 */
unsigned char *int_tab(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts);

/**
 * @fn unsigned char is_asc_digit ( unsigned char *Pts, unsigned char Lgs )
 * @brief 判断字符串Pts前Lgs个字符是否全为数字.
 *
 * @param Pts 源字符串
 * @param Lgs 需要判断的字符串长度
 *
 * @return 前Lgs个字符全为数字返回1,否则返回0.
 *
 * 例
 * @code
 * unsigned char Ptsrc[10];
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","1234ABC");
 * printf("%u\n",is_asc_digit(Ptsrc,4));
 * printf("%u\n",is_asc_digit(Ptsrc,5));
 * //运行结果
 * Pts = "1234ABC"
 * if Lgs = 4, return 1
 * if Lgs = 5, return 0
 * @endcode
 */
unsigned char is_asc_digit ( unsigned char *Pts, unsigned char Lgs );

/**
 * @fn unsigned char is_str_digit ( unsigned char *Pts )
 * @brief 判断字符串Pts是否全为数字.
 *
 * @param Pts 源字符串
 *
 * @return 前Lgs个字符全为数字返回1,否则返回0.
 *
 * 例
 * @code
 * unsigned char Ptsrc[10];
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","1234ABC");
 * printf("%u\n",is_str_digit(Ptsrc));
 * //运行结果
 * Pts = "1234ABC" return 0
 * @endcode
 */
unsigned char is_str_digit ( unsigned char *Pts );

/**
 * @fn unsigned char addbcd( unsigned char Oc1, unsigned char Oc2)
 * @brief 将两个BCD码相加.
 *
 * @param Oc1 被加数
 * @param Oc2 加数
 *
 * @return Oc1和Oc2的和.
 *
 * @warning Oc1和Oc2的高四位和低四位各表示
 * 					一个0~9的数,且相加采用十进制.
 *
 * 例
 * @code
 * unsigned char Oc1 = 0x38, Oc2 = 0x24;
 * printf("0x%X\n",addbcd(Oc1,Oc2));
 * //运行结果
 * Oc1 = 0x38, Oc2 = 0x24
 * return 0x62
 * @endcode
 */
unsigned char addbcd( unsigned char Oc1, unsigned char Oc2);

/**
 * @fn unsigned char subbcd ( unsigned char Oc1, unsigned char Oc2 )
 * @brief 将两个BCD码相减.
 *
 * @param Oc1 被减数
 * @param Oc2 减数
 *
 * @return Oc1和Oc2的差.
 *
 * @warning Oc1和Oc2的高四位和低四位各表示
 * 					一个0~9的数,且相减采用十进制.
 *
 * 例
 * @code
 * unsigned char Oc1 = 0x38, Oc2 = 0x24;
 * printf("0x%X\n",subbcd(Oc1,Oc2));
 * //运行结果
 * Oc1 = 0x38, Oc2 = 0x24
 * return 0x14
 * @endcode
 */
unsigned char subbcd ( unsigned char Oc1, unsigned char Oc2 );

/**
 * @fn unsigned char sub_heure (unsigned char *Ptr, unsigned char *Pt1, unsigned char *Pt2 )
 * @brief 两个使用BCD码表示的时间相减.
 *
 * @param Ptr 时间差值
 * @param Pt1 作为被减数的时间地址
 * @param Pt2 作为减数的时间地址
 *
 * @return 如果Pt1的时间早于Pt2的时间,返回1,否则返回0.
 *
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptr[2] = {0x00,0x00}, Pt1[2] = {0x12,0x24}, Pt2[2] = {0x08,0x18};
 * printf("%u\n",sub_heure(Ptr,Pt1,Pt2));
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptr[i]);
 * printf("\n");
 * //运行结果
 * Pt1 = 0x12/0x24(相当于时间 12:24)
 * Pt2 = 0x08/0x18(相当于时间 08:18)
 * Ptr = 0x04/0x06(时间差值为4小时6分钟)
 * return 0
 * @endcode
 */
unsigned char sub_heure (unsigned char *Ptr, unsigned char *Pt1, unsigned char *Pt2 );

/**
 * @fn unsigned char add_date_day( unsigned char *Ptr, unsigned char B_ov)
 * @brief 将一定的天数加到以BCD码表示的年月日格式的日期中
 *
 * @param Ptr 以BCD码表示的年月日格式的日期
 * @param B_ov 要增加的天数
 *
 * @return 如果日期相加后使增加一年,返回1,否则返回0
 *
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptr[] = {0x11,0x12,0x27};
 * printf("%u\n",add_date_day(Ptr,0x25));
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptr[i]);
 * printf("\n");
 * //运行结果
 * Ptr = 0x11/0x12/0x27 (相当于日期 2011年12月27日)
 * B_ov = 0x25(相当于25天)
 * //函数运行后
 * Ptr = 0x12/0x01/0x21 (相当于日期 2012年01月21日)
 * return 1
 * @endcode
 */
unsigned char add_date_day( unsigned char *Ptr, unsigned char B_ov);

/**
 * @fn unsigned char add_heure_attente (unsigned char *Ptr, unsigned char *Pt2)
 * @brief 将一定的时间加到以BCD码表示的小时分钟格式的时间中
 *
 * @param Ptr 以BCD码表示的小时分钟格式的时间
 * @param Pt2 要增加的时间
 *
 * @return 如果时间相加后使增加一天,返回1,否则返回0
 *
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptr[] = {0x22,0x01}, Pt2[] = "1022";
 * printf("%u\n",add_heure_attente(Ptr,Pt2));
 * for(i=0;i<2;i++)
 * printf("0x%X/",Ptr[i]);
 * printf("\n");
 * //运行结果
 * Ptr = 0x22/0x01 (相当于时间 22:01)
 * B_ov = "1022"(相当于时间10小时22分钟)
 * //函数运行后
 * Ptr = 0x08/0x23 (相当于时间 08:23)
 * return 1
 * @endcode
 */
unsigned char add_heure_attente (unsigned char *Ptr, unsigned char *Pt2 );

/**
 * @fn unsigned char sub_date (unsigned char *Ptr, unsigned char *Pt1, unsigned char *Pt2, unsigned char B_ov)
 * @brief 将以BCD码表示的日期(Pt1)减去以BCD码表示的日期(Pt2),再减去天数(B_ov)
 *
 * @param Ptr 以BCD码表示的年月日格式的日期的差值
 * @param Pt1 以BCD码表示的日期作为被减数
 * @param Pt2 以BCD码表示的日期作为减数
 * @param B_ov 要减去的天数
 *
 * @return 如果Pt1的年值小于Pt2的年值且Pt1大于50,返回1,否则返回0
 *
 * 例
 * @code
 * int i;
 * unsigned char Ptr[] = {0x00,0x00,0x00}, Pt1[] = {0x11,0x12,0x27},Pt2[] = {0x10,0x05,0x22};
 * printf("%u\n",sub_date(Ptr,Pt1,Pt2,0x06));
 * for(i=0;i<3;i++)
 * printf("0x%X/",Ptr[i]);
 * printf("\n");
 * //运行结果
 * Pt1 = 0x11/0x12/0x27 (相当于日期 2011年12月27日)
 * Pt2 = 0x10/0x05/0x22 (相当于日期 2010年05月22日)
 * B_ov = 0x06(相当于6天)
 * //函数运行后
 * Ptr = 0x01/0x06/0x29 (日期差值为1年6月29天)
 * return 0
 * @endcode
 */
unsigned char sub_date (unsigned char *Ptr, unsigned char *Pt1, unsigned char *Pt2, unsigned char B_ov);

/**
 * @fn unsigned char memnul( unsigned char *Pt, unsigned int Lg )
 * @brief 验证一个数据域是否为NULL
 *
 * @param Pt 目标字符串
 * @param Lg 需要验证的字符串的长度
 *
 * @return 如果字符串全为NULL,返回1,否则返回0
 *
 * 例
 * @code
 * unsigned char Ptr[] = {0x00,0x00,0x00,0x01};
 * printf("%u\n",memnul(Ptr,3));
 * printf("%u\n",memnul(Ptr,4));
 * //运行结果
 * Pt = 0x00/0x00/0x00/0x01
 * if Lg = 3  return 1
 * if Lg = 4  return 0
 * @endcode
 */
unsigned char memnul( unsigned char *Pt, unsigned int Lg );

/**
 * @fn unsigned char lrc_l( unsigned char *Pts, unsigned short Lgs, unsigned char Val_init )
 * @brief 对目标字符串Pts进行冗余计算
 *
 * @param Pts 目标字符串首地址
 * @param Lgs 目标字符串长度
 * @param Val_init 初始化值
 *
 * @return 返回冗余计算结果值
 *
 */
unsigned char lrc_l( unsigned char *Pts, unsigned short Lgs, unsigned char Val_init );

/**
 * @fn unsigned char  lrc( unsigned char *pointchar1, unsigned char *pointchar2, unsigned char valeur)
 * @brief 对两个字符串进行冗余计算
 *
 * @param pointchar1 字符串1首地址
 * @param pointchar2 字符串2首地址
 * @param valeur 值
 *
 * @return 返回冗余计算结果值
 *
 */
unsigned char  lrc( unsigned char *pointchar1, unsigned char *pointchar2, unsigned char valeur);

/**
 * @fn void move_inc_ptr ( unsigned char **Ptd, unsigned char *Pts , unsigned char Lg )
 * @brief	由Pts所指内存区域复制Lg个字节到*Ptd所指内存区域,
 *			并将*Ptd所指向的指针移到*Ptd+Lg
 *
 * @param Ptd 目标字符串地址的首地址
 * @param Pts 源字符串首地址
 * @param Lg  要复制的字节数
 *
 * @return  无返回值
 * @warning 函数运行后,*Ptd指向(*Ptd+Lg)
 *
 * @code
 * unsigned char *Ptdest,Ptsrc[10];
 * Ptdest = (unsigned char *)malloc(10);
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","1234ABC");
 * move_inc_ptr(&Ptdest,Ptsrc,5);
 * printf("%s\n",Ptdest-5);
 * free(Ptdest-5);
 * //运行结果
 * Pts = "1234ABC"
 * if Lg = 5  (Ptd-Lg) = "1234A"
 * @endcode
 */
void move_inc_ptr ( unsigned char **Ptd, unsigned char *Pts , unsigned char Lg );

/**
 * @fn unsigned char *move_255 ( unsigned char *Ptd, unsigned char *Pts , unsigned char Lg )
 * @brief 由Pts所指内存区域复制Lg个字节到Ptd所指内存区域
 *
 * @param Ptd 目标字符串首地址
 * @param Pts 源字符串首地址
 * @param Lg  要复制的字节数
 *
 * @return 目标字符串首地址
 *
 * @code
 * unsigned char *Ptdest,Ptsrc[10];
 * Ptdest = (unsigned char *)malloc(10);
 * memset(Ptdest,0x00,sizeof(Ptdest));
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%s","1234ABC");
 * move_255(Ptdest,Ptsrc,5);
 * printf("%s\n",Ptdest);
 * free(Ptdest);
 * //运行结果
 * Pts = "1234ABC"
 * if Lg = 5  Ptd = "1234A"
 * @endcode
 */
unsigned char *move_255 ( unsigned char *Ptd, unsigned char *Pts , unsigned char Lg );

/**
 * @fn unsigned char *mov_to_str(unsigned char *Ptd, unsigned char *Pts )
 * @brief 从Pts+1所指内存区域复制*Pts个字节到Ptd所指内存区域
 *
 * @param Ptd 目标字符串首地址
 * @param Pts 源字符串首地址
 *
 * @return 目标字符串首地址
 *
 * @warning Pts采用长度+字符串,即*Pts为要复制的字符串长度
 *
 * @code
 * unsigned char *Ptdest,Ptsrc[10];
 * Ptdest = (unsigned char *)malloc(10);
 * memset(Ptdest,0x00,10);
 * memset(Ptsrc,0x00,sizeof(Ptsrc));
 * sprintf(Ptsrc,"%u%s",7,"1234ABC");
 * mov_to_str(Ptdest,Ptsrc);
 * printf("%s\n",Ptdest);
 * //运行结果
 * Pts = "71234ABC"
 * Ptd = "1234ABC"
 * @endcode
 */
unsigned char *mov_to_str(unsigned char *Ptd, unsigned char *Pts );

/**
 * @fn void Uart_Printf(char *fmt,...)
 * @brief	Uart_Printf可以作为调试终端的内存变量值的手段,
 *			根据RS232的请求,将信息输出到DWN的注释窗口中.
 *
 * @param fmt 参数输出的格式,与函数printf的参数一样.
 *
 * @return 无返回值
 *
 * @code
 * unsigned char *a = "Hello SAND!";
 * unsigned int b = 10, c = 0xAA;
 * Uart_Printf("%s\n%d\n0x%X\n%s\n",a,b,c,"end");
 * //运行结果
 * Hello SAND!
 * 10
 * 0xAA
 * end
 * @endcode
 */
void Uart_Printf(char *fmt,...);



int gb2312_utf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);

int utf8_gb2312(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);

#ifdef __cplusplus
}
#endif

#endif  //OSTOOLS_H_INCLUDED
/** @} end of etc_fns */
