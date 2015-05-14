#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/videodev.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <math.h>
#include <malloc.h>
#include "mxcfb.h"
#include "jpeglib.h"
#include "jerror.h"
#include "camera.h"
#include "sysparam.h"

#define CAMERA_DEVICE		"/dev/video0"
#define SCREEN_DEVICE		"/dev/fb0"
#define CAMFLASH_DEVICE		"/dev/camflash"
#define TFAIL				-1
#define TPASS				0
#define IMAGE_WIDTH			320
#define IMAGE_HEIGHT		240
#define CAMERA_FRAMERATE	0
#define IMAGE_PIXELFORMAT	V4L2_PIX_FMT_RGB565
#define IMAGE_BPP			16

#define BITMAP_HEAD_SIZE	54//bmp文件头大小

#define FLASH_CRITICAL_VALUE	8//闪光灯开启临界值

struct v4l2_format fmt;
static int fd_screen = 0;
static int fd_camera = 0;
static int flash = CAMERA_FLASH_AUTO;		//闪光灯标准,默认为自动

typedef struct
{
 	//14字节
 	unsigned short    bfType;            //文件标识 2字节 必须为BM 
    unsigned int      bfSize;                //文件大小 4字节 
    unsigned short    bfReserved1;        //保留，每字节以"00"填写 2字节 
    unsigned short    bfReserved2;        //同上 2字节 
    unsigned int      bfOffBits;            //记录图像数据区的起始位置(图象数据相对于文件头字节的偏移量)。 4字节

    //40字节
    unsigned int  biSize;                //表示本结构的大小 4字节 
    int    biWidth;                //位图的宽度  4字节 
    int    biHeight;               //位图的高度  4字节 
    unsigned short   biPlanes;            //永远为1 ,   2字节 
    unsigned short   biBitCount;        //位图的位数  分为1 4 8 16 24 32   2字节 
    unsigned int  biCompression;       //压缩说明   4字节 
    unsigned int  biSizeImage;            //表示位图数据区域的大小以字节为单位  4字节 
    int   biXPelsPerMeter;                //用象素/米表示的水平分辨率   4字节 
    int   biYPelsPerMeter;                //用象素/米表示的垂直分辨率   4字节 
    unsigned int  biClrUsed;            //位图使用的颜色索引数   4字节 
    unsigned int  biClrImportant;        //对图象显示有重要影响的颜色索引的数目  4字节  
 
}BMP;

/**
 * @fn int rgb565_2_rgb24(unsigned char *rgb565, int rgb565Len, unsigned char *rgb24, int rgb24Len)
 * @brief 	将RGB565图片格式转换成RGB888图片格式
 *
 * @param rgb565		RGB565数据地址
 * @param rgb565Len  	RGB565数据长度
 * @param rgb24			RGB888数据地址
 * @param rgb24Len  	RGB888字符串最大长度
 *
 * @return 		RGB888实际数据长度
 *
 */
int rgb565_2_rgb24(unsigned char *rgb565, int rgb565Len, unsigned char *rgb24, int rgb24Len)
{   
	int i = 0;
	if(rgb565Len*3/2 > rgb24Len)
		return -1;
		
	//因为ARM和普通电脑一般为小端模式,所以RGB565存储模式为"G2G1G0B4B3B2B1B0 R4R3R2R1R0G5G4G3"
	for(i=0;i<rgb565Len/2;i++)
	{
		rgb24[3*i] = (rgb565[2*i+1] & 0xF8) | ((rgb565[2*i+1] & 0x38)>>3);/*R4R3R2R1R0R2R1R0*/
	
		rgb24[3*i+1] = ((rgb565[2*i+1] & 0x07)<<5) | ((rgb565[2*i] & 0xE0)>>3) | ((rgb565[2*i] & 0x60)>>5);/*G5G4G3G2G1G0G1G0*/
	
		rgb24[3*i+2] = ((rgb565[2*i] & 0x1F)<<3) | (rgb565[2*i] & 0x07);;/*B4B3B2B1B0B2B1B0*/
	}
	return 3*i;
}

int rgb24_2_jpeg(unsigned char *rgb24, int rgb24Len, char *imageName, int imageWidth, int imageHeight)
{
	//如果rgb24的像素数/imageWidth小于图片的行数(即小于图片高度),函数退出
	if(rgb24Len/(3*imageWidth) < imageHeight)
		return -1;
	
	//调用JEPG库，将图片压缩成JPEG格式
	//申请并初始化jpeg压缩对象，同时要指定错误处理器
	FILE* jpgFile;
	struct jpeg_compress_struct jcs;
	//声明错误处理器，并赋值给jcs.err域
	struct jpeg_error_mgr jem;
	jcs.err = jpeg_std_error(&jem);
	jpeg_create_compress(&jcs);
	//指定压缩后的图像所存放的目标文件，注意，目标文件应以二进制模式打开
	jpgFile = fopen(imageName,"wb");
	if(jpgFile == NULL)
	{
		return -1;
	}
	jpeg_stdio_dest(&jcs, jpgFile);
	//设置压缩参数,主要参数有图像宽,高,色彩通道数(１:索引图像,３:其他),
	//色彩空间(JCS_GRAYSCALE表示灰度图,JCS_RGB表示彩色图像),压缩质量等,如下:
	
	jcs.image_width = imageWidth; //为图的宽和高,单位为像素
	jcs.image_height = imageHeight;
	
	jcs.input_components = 3; //色彩通道数 在此为1,表示灰度图,如果是彩色位图,则为3
	jcs.in_color_space = JCS_RGB; //色彩空间,JCS_GRAYSCALE表示灰度图,JCS_RGB表示彩色图像

	jpeg_set_defaults(&jcs);

	jpeg_set_quality (&jcs, 80, 1);//压缩质量
	//上面的工作准备完成后，就可以压缩了，压缩过程非常简单，首先调用 jpeg_start_compress，
	//然后可以对每一行进行压缩，也可以对若干行进行压缩，甚至可以对整个的图像进行一次压缩，如下：
	jpeg_start_compress(&jcs, 1);
	JSAMPROW row_pointer[1]; //一行位图
	int row_stride; //每一行的字节数
	row_stride = jcs.image_width*3; //如果不是索引图，此处需要乘以3//480
	
	//对每一行进行压缩
	while (jcs.next_scanline <imageHeight)  
	{
		row_pointer[0] = & rgb24[jcs.next_scanline * row_stride];
		jpeg_write_scanlines(&jcs, row_pointer, 1);
	}
	jpeg_finish_compress(&jcs);//压缩完成后，记得要调用jpeg_finish_compress函数
	
	//释放压缩工作过程中所申请的资源了，主要就是jpeg压缩对象
	jpeg_destroy_compress(&jcs);
	fclose(jpgFile);
	return 0;
}

int Os__camera_getBMP(unsigned char *rgb565, int rgb565Len, char *imageName, int imageWidth, int imageHeight)
{
	int rgb24Len = 0, i;
	unsigned char *rgb24Buf;
	int ret = -1;
	unsigned char ucBuf[100];
	BMP bitMap;
	
	memset(&bitMap,0x00,sizeof(BMP));
	//如果不存在，创建拍照文件夹
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",IMAGE_PATH);
	system(ucBuf);
	
	FILE* bmpFile;
	
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"%s/%s",IMAGE_PATH,imageName);
	bmpFile = fopen(ucBuf,"wb");
	if(bmpFile == NULL)
	{
		return ret;
	}
	
	//申请资源
	rgb24Len = (rgb565Len/2)*3;
	rgb24Buf = (unsigned char *) calloc(rgb24Len,1);
	
	if(rgb24Buf == NULL)
	{
		return ret;
	}
	//RGB565转成RGB24
	rgb24Len = rgb565_2_rgb24(rgb565,rgb565Len,rgb24Buf,rgb24Len);
	//因为ARM和普通电脑一般为小端模式,所以RGB565存储模式为"G2G1G0B4B3B2B1B0 R4R3R2R1R0G5G4G3"
	//对于BMP图片，图片数据使用BRG格式，且解析图片是倒序扫描
	for(i=0;i<rgb565Len/2;i++)
	{
		rgb24Buf[3*i+2] = (rgb565[2*i+1] & 0xF8) | ((rgb565[2*i+1] & 0x38)>>3);/*R4R3R2R1R0R2R1R0*/
	
		rgb24Buf[3*i+1] = ((rgb565[2*i+1] & 0x07)<<5) | ((rgb565[2*i] & 0xE0)>>3) | ((rgb565[2*i] & 0x60)>>5);/*G5G4G3G2G1G0G1G0*/
	
		rgb24Buf[3*i] = ((rgb565[2*i] & 0x1F)<<3) | (rgb565[2*i] & 0x07);/*B4B3B2B1B0B2B1B0*/
	}
	//配置bmp文件的头文件
	bitMap.bfType = 0x4D42;
	bitMap.bfSize = rgb24Len + BITMAP_HEAD_SIZE;
	bitMap.bfOffBits = BITMAP_HEAD_SIZE;
	
	bitMap.biSize = 40;
	bitMap.biWidth = imageWidth;
	bitMap.biHeight = imageHeight;
	bitMap.biPlanes = 1;
	bitMap.biBitCount = 24;
	bitMap.biSizeImage = rgb24Len;
	bitMap.biXPelsPerMeter = 3780;
	bitMap.biYPelsPerMeter = 3780;
	
	fseek(bmpFile,0L,0); //图像文件类型
	fwrite(&(bitMap.bfType),sizeof(short),1,bmpFile);
	
	fseek(bmpFile,2L,0); //图像文件大小
	fwrite(&(bitMap.bfSize),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,6L,0); //图像文件保留字1
	fwrite(&(bitMap.bfReserved1),sizeof(short),1,bmpFile);
	
	fseek(bmpFile,8L,0); //图像文件保留字2
	fwrite(&(bitMap.bfReserved2),sizeof(short),1,bmpFile);
	
	fseek(bmpFile,10L,0);//数据区的偏移量
	fwrite(&(bitMap.bfOffBits),sizeof(short),1,bmpFile);
	

	fseek(bmpFile,14L,0);//文件头结构大小
	fwrite(&(bitMap.biSize),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,18L,0);//图像的宽度
	fwrite(&(bitMap.biWidth),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,22L,0);//图像的高度
	fwrite(&(bitMap.biHeight),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,26L,0);//图像的面数
	fwrite(&(bitMap.biPlanes),sizeof(short),1,bmpFile);
	
	fseek(bmpFile,28L,0);//图像一个像素的字节数
	fwrite(&(bitMap.biBitCount),sizeof(short),1,bmpFile);
	
	fseek(bmpFile,30L,0);//图像压缩信息
	fwrite(&(bitMap.biCompression),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,34L,0);//图像数据区的大小
	fwrite(&(bitMap.biSizeImage),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,38L,0);//水平分辨率
	fwrite(&(bitMap.biXPelsPerMeter),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,42L,0);//垂直分辨率
	fwrite(&(bitMap.biYPelsPerMeter),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,46L,0);//颜色索引数
	fwrite(&(bitMap.biClrUsed),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,50L,0);//重要颜色索引数
	fwrite(&(bitMap.biClrImportant),sizeof(int),1,bmpFile);
	
	fseek(bmpFile,(long)(bitMap.bfOffBits),0);
	for(i=0;i<imageHeight;i++)
	{
		fwrite(&rgb24Buf[(imageHeight-1-i)*imageWidth*3],1,imageWidth*3,bmpFile);
	}
	fclose(bmpFile);
	free(rgb24Buf);
	ret = 0;
	return ret;
}

int Os__camera_getJPEG(unsigned char *rgb565, int rgb565Len, char *imageName, int imageWidth, int imageHeight)
{
	int rgb24Len = 0;
	unsigned char *rgb24Buf;
	int ret = 0;
	unsigned char ucBuf[100];
	//如果不存在，创建拍照文件夹
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",IMAGE_PATH);
	system(ucBuf);
	
	//申请资源
	rgb24Len = (rgb565Len/2)*3;
	rgb24Buf = (unsigned char *) calloc(rgb24Len,1);
	
	if(rgb24Buf == NULL)
	{
		ret = -1;
		return ret;
	}
	//RGB565转成RGB24
	rgb24Len = rgb565_2_rgb24(rgb565,rgb565Len,rgb24Buf,rgb24Len);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"%s/%s",IMAGE_PATH,imageName);	
	if(rgb24Len < 0 || rgb24_2_jpeg(rgb24Buf,rgb24Len,ucBuf,imageWidth,imageHeight) != 0)
		ret = -1;
		
	free(rgb24Buf);
	return ret;
}


int Os__camera_init(int width, int height)
{
	struct v4l2_streamparm parm;
	
	struct v4l2_framebuffer fb_v4l2;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	int overlay = 1;
	int ret = -1;
	
	memset(&fmt,0,sizeof(fmt));
	memset(&fb_v4l2, 0, sizeof(fb_v4l2));
	
	//打开camera
	fd_camera = open(CAMERA_DEVICE, O_RDWR, 0);

	if(fd_camera <= 0)
	{
		//printf("Unable to open %s\n", CAMERA_DEVICE);
		goto _exit;
	}
	//设置通过"获取数据格式"
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd_camera, VIDIOC_G_FMT, &fmt) < 0)
	{
		//printf("VIDIOC_G_FMT failed\n");
		goto _exit;
	}
	//设置流参数
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = CAMERA_FRAMERATE;
	parm.parm.capture.capturemode = 0;
	if(ioctl(fd_camera, VIDIOC_S_PARM, &parm) < 0)
	{
		//printf("VIDIOC_S_PARM failed\n");
		goto _exit;
	}
	//设置image的格式
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = IMAGE_PIXELFORMAT;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.sizeimage = fmt.fmt.pix.width *
				fmt.fmt.pix.height * IMAGE_BPP / 8;
	fmt.fmt.pix.bytesperline = width * IMAGE_BPP / 8;
	if(ioctl(fd_camera, VIDIOC_S_FMT, &fmt) < 0)
	{
		//printf("VIDIOC_S_FMT failed\n");
		goto _exit;
	}
	
	if((fd_screen = open(SCREEN_DEVICE, O_RDWR )) <= 0)
	{
		//printf("Unable to open frame buffer\n");
		goto _exit;
	}

	if (ioctl(fd_screen, FBIOGET_VSCREENINFO, &var) < 0)
	{
		//printf("FBIOGET_VSCREENINFO failed\n");
		goto _exit;
	}

	/* currently only support 1280*960 mode and 640*480 mode*/
	var.xres_virtual = width;
	var.yres_virtual = height;
	if(ioctl(fd_screen, FBIOPUT_VSCREENINFO, &var) < 0)   
	{
		//printf("FBIOPUT_VSCREENINFO failed\n");
		goto _exit;
	}

	if(ioctl(fd_screen, FBIOGET_VSCREENINFO, &var) < 0)
	{
        //printf("FBIOGET_VSCREENINFO failed\n");
        goto _exit;
    }
	
	//printf("var.xres_virtual = %d,\nvar.yres_virtual = %d\n",var.xres_virtual,var.yres_virtual);

	if(ioctl(fd_screen, FBIOGET_FSCREENINFO, &fix) < 0)
	{
		//printf("FBIOGET_FSCREENINFO failed\n");
		goto _exit;
	}

	fb_v4l2.base = (void *)fix.smem_start;
	if(ioctl(fd_camera, VIDIOC_S_FBUF, &fb_v4l2) < 0)
	{
		//printf("set framebuffer failed\n");
		goto _exit;
	}
	//start capture screen
	if(ioctl(fd_camera, VIDIOC_OVERLAY, &overlay) < 0)
	{
		//printf("VIDIOC_OVERLAY start failed\n");
		goto _exit;
	}

	ret = 0;
_exit:
	if(ret < 0)
		Os__camera_close();
	return ret;
}

int Os__camera_capture(char *cameraBuf, int bufLen)
{
	int ret = -1;
	int fd_camflash = 0;
	int overlay = 1;
	unsigned char brightValue;
	SYS_SETTING setting;
	
	//获取闪光灯类型
	memset(&setting,0,sizeof(SYS_SETTING));
	strcpy(setting.section,"CAMERA_SETTING");
	strcpy(setting.key,"FLASH_LIGHT");
	if(PARAM_setting_get(&setting) > 0)
		flash = atoi(setting.value);

	if(fd_camera > 0 && fd_screen > 0)
	{
		
		if(bufLen < fmt.fmt.pix.sizeimage)
		{
			printf("cameraBuf is short!\n");
			return ret;
		}
		
		ioctl(fd_camera, VIDIOC_OVERLAY, &overlay);
		
 		//获取外界亮度值//根据亮度值判断是否开启闪光灯
		ioctl(fd_camera, VIDIOC_G_OUTPUT, &brightValue);


		//开启闪光灯
		if(flash == CAMERA_FLASH_ON || (flash == CAMERA_FLASH_AUTO && brightValue <= FLASH_CRITICAL_VALUE))
		{
			fd_camflash = open(CAMFLASH_DEVICE,O_WRONLY);
			if(fd_camflash > 0)
			{
				write(fd_camflash,"1",1);
				usleep(1000*300);
			}
		}
		//修改read描述符由fd_screen变为fd_camera,直接从摄像头读取,不再从屏幕截取
		ret = read(fd_camera, cameraBuf, fmt.fmt.pix.sizeimage);
		
		overlay = 0;
		ioctl(fd_camera, VIDIOC_OVERLAY, &overlay);
		
		if(fd_camflash > 0)
		{
			write(fd_camflash,"0",1);
			close(fd_camflash);
		}
		overlay = 1;
		usleep(1000*500);
		ioctl(fd_camera, VIDIOC_OVERLAY, &overlay);
	}
	return ret;
}

void Os__camera_close(void)
{
	if(fd_camera > 0)
		close(fd_camera);

	if(fd_screen > 0)
		close(fd_screen);
}

void CAMERA_get_picsize(int *imgWidth, int *imgHeight)
{
	SYS_SETTING setting;
	int index = 0, cn = 0;
	char *point = NULL;
	char strTmp[50];

	//获取图片大小
	memset(&setting,0,sizeof(SYS_SETTING));
	strcpy(setting.section,"CAMERA_SETTING");
	strcpy(setting.key,"PICTURE_SIZE");
	if((cn = PARAM_setting_get(&setting)) > 0)
	{
		index = atoi(setting.value);
		if(index > 0 && index <= cn)
		{
			point = strchr(setting.briefList[index-1],'*');
			if(point != NULL)
			{
				memset(strTmp,0,sizeof(strTmp));
				strncpy(strTmp,setting.briefList[index-1],point-setting.briefList[index-1]);
				*imgWidth = atoi(strTmp);
				
				memset(strTmp,0,sizeof(strTmp));
				strncpy(strTmp,point+1,strlen(setting.briefList[index-1])-(point-setting.briefList[index-1]) -1);
				*imgHeight = atoi(strTmp);
				return;
			}
		}
	}
	//取值失败使用默认值
	*imgWidth = 800;
	*imgHeight = 600;
}
