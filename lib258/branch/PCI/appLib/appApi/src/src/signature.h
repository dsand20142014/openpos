#ifndef SIGNATURE_H_INCLUDED
#define SIGNATURE_H_INCLUDED
/**
 * @defgroup 签名打印接口函数
 * @author 熊剑
 * @brief 提供签名的初始化和关闭函数以及对签名进行打印
 * @{
 */

/**
 * @fn void Os__clear_sign(void)
 * @brief 	清除签名数据
 *
 * @param 无参数
 *
 * @return 	无返回值
 *
 */
void Os__clear_sign(void);

/**
 * @fn void Os__close_sign(void)
 * @brief 	关闭签名初始化时打开的节点和开辟的内存空间,
 * 			使Os__get_sign()函数退出死循环
 *
 * @param 无参数
 *
 * @return 	无返回值
 *
 */
void Os__close_sign(void);

/**
 * @fn void Os__get_sign(void)
 * @brief 	获取签名的轨迹,需要放在线程里,
 * 			此函数是一个死循环直到调用Os__close_sign()为止
 *
 * @param 无参数
 *
 * @return 	无返回值
 *
 */
void Os__get_sign(void);

/**
 * @fn int Os__init_sign(int x,int y,int width,int height)
 * @brief 	初始化签名并设置签名区域
 *
 * @param x			签名区域的x坐标
 * @param y			签名区域的y坐标
 * @param width		签名区域的宽度
 * @param height	签名区域的高度
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int Os__init_sign(int x,int y,int width,int height);

/**
 * @fn int Os__print_sign(void)
 * @brief 	打印签名
 *
 * @param 无参数
 *
 * @code
 * int main(void)
 * {
 * 		if(Os__init_sign(0,50,240,211) == 0)
 * 		{
 * 			//此处应该使用线程来调用此函数
 * 			Os__get_sign();
 * 			//关闭
 * 			Os__close_sign();
 * 		}
 * 		return 0;
 * }
 * @endcode
 * @return 		成功返回0,无纸返回-2,失败返回-1
 *
 */
int Os__print_sign(void);
#endif  //SIGNATURE_H_INCLUDED
/** @} end of etc_fns */