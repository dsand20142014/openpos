#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

/**
 * @defgroup 照相机拍照图片转换函数
 * @author 熊剑
 * @author edit by 熊剑 20130115 
 *				添加rgb565转为bmp格式图片
 * @brief 提供对RGB565转换为jpg格式的图片
 * @{
 */

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif


#define CAMERA_FLASH_OFF		0
#define CAMERA_FLASH_ON			1
#define CAMERA_FLASH_AUTO		2

#define IMAGE_PATH "./image" 

/**
 * @fn int rgb565_2_jpeg(unsigned char *rgb565, int rgb565Len, char *imageName, int imageWidth, int imageHeight)
 * @brief 	将RGB565图片格式转换成jpg图片格式
 *
 * @param rgb565		RGB565数据地址
 * @param rgb565Len  	RGB565数据长度
 * @param imageName		图片保存路径和名称
 * @param imageWidth	图片宽度
 * @param imageHeight	图片高度
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int  Os__camera_getJPEG(unsigned char *rgb565, int rgb565Len, char *imageName, int imageWidth, int imageHeight);

/**
 * @fn int Os__camera_getBMP(unsigned char *rgb565, int rgb565Len, char *imageName, int imageWidth, int imageHeight)
 * @brief 	将RGB565图片格式转换成bmp图片格式
 *
 * @param rgb565		RGB565数据地址
 * @param rgb565Len  	RGB565数据长度
 * @param imageName		图片保存路径和名称
 * @param imageWidth	图片宽度
 * @param imageHeight	图片高度
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int Os__camera_getBMP(unsigned char *rgb565, int rgb565Len, char *imageName, int imageWidth, int imageHeight);

/**
 * @fn int Os__camera_init(int width, int height)
 * @brief 	初始化camera
 *
 * @param width		图片宽度
 * @param height	图片高度
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int Os__camera_init(int width, int height);

/**
 * @fn void Os__camera_close(void)
 * @brief 	关闭camera
 *
 * @param rgb565	无参数
 *
 * @return 		无返回值
 *
 */
void Os__camera_close(void);

/**
 * @fn int Os__camera_capture(char *cameraBuf, int bufLen)
 * @brief 	捕捉图像,将rgb565数据保存在cameraBuf中
 *
 * @param cameraBuf		rgb565数据保存地址
 * @param bufLen		cameraBuf的最大长度
 *
 * @return 		成功返回实际rgb565数据长度,失败返回-1
 *
 * @code
 * int main(void)
 * {
 *	int fd = 0;
 *	char *buf = NULL;
 *	int imgWidth = 320;
 *	int imgHeight = 240;
 *	int i, imgSize, bufSize;
 *	bufSize = imgWidth*imgHeight*2;
 *	buf = (char *)calloc(bufSize,1);
 *	if(buf == NULL)
 *		return -1;
 *	if((fd = Os__camera_init(imgWidth,imgHeight)) < 0)
 *	{
 *		printf("camer init is fail!\n");
 *		return -1;
 *	}
 *	printf("camera init is OK!\n");
 *	
 *
 *	imgSize = Os__camera_capture(buf,bufSize);
 *	if(imgSize > 0)
 *	{
 *		if(Os__camera_getJPEG(buf,imgSize,"IMG_20130105.jpg",imgWidth,imgHeight) != 0)
 *		{
 *			printf("RGB24 to JPG error\n");
 *			return -1;
 *		}
 *		//如果需要生产BMP图片使用如下函数
 *		//if(Os__camera_getJPEG(buf,imgSize,"IMG_20130105.bmp",imgWidth,imgHeight) != 0)
 *		//{
 *		//	printf("RGB24 to BMP error\n");
 *		//	return -1;
 *		//}
 *	}
 *	free(buf);
 *	printf("camer test is OK!\n");
 *	return 0;
 *}
 * @endcode
 */
int Os__camera_capture(char *cameraBuf, int bufLen);

/**
 * @fn void CAMERA_get_picsize(int *imgWidth, int *imgHeight)
 * @brief 	获取系统设置中的图片宽高
 *
 * @param imgWidth	图片宽度地址
 * @param imgHeight	图片高度地址
 *
 * @return 		无返回值
 *
 */
void CAMERA_get_picsize(int *imgWidth, int *imgHeight);

#endif  //CAMERA_H_INCLUDED
/** @} end of etc_fns */
