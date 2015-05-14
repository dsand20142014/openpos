#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/fb.h>
#include "signature.h"
#include "tslib.h"
#include "printer.h"

#define	DEV_FB	"/dev/fb0" 			//控制屏幕上的每一点的颜色的设备节点
#define DEV_TS	"/dev/input/ts0" 	//触摸屏设备节点

#define XORMODE	0x80000000
static int palette_sign[] = {0x000000, 0xffe080, 0xffffff, 0xe0c0a0};
#define NR_COLORS (sizeof (palette_sign) / sizeof (palette_sign [0]))

//四个变量分别为签名区域x和y坐标以及宽度和高度
static int sign_x, sign_y, sign_w, sign_h;
//触摸屏设备结构体指针
static struct tsdev *ts;
static struct ts_sample samp;
static int fb_fd;
static unsigned char line[320*48];
static unsigned char *fbuffer;
struct fb_fix_screeninfo fix;
static unsigned char **line_addr;
struct fb_var_screeninfo var;
int bytes_per_pixel;
unsigned colormap[256];
int xres, yres;
int xm,ym;
static int exit_flag = 0;

union multiptr
{
	unsigned char *p8;
	unsigned short *p16;
	unsigned long *p32;
};

int open_framebuffer(void)
{
	unsigned y, addr;

	fb_fd = open(DEV_FB, O_RDWR);
	if(fb_fd == -1)
	{
    	perror("open fbdevice");
    	return -1;
	}
	
    memset(&fix,0,sizeof(struct fb_fix_screeninfo));
	if(ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix) < 0)
	{
	    perror("ioctl FBIOGET_FSCREENINFO");
	    close(fb_fd);
	    return -1;
	}
	memset(&var,0,sizeof(struct fb_var_screeninfo));
	if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0)
	{
	    perror("ioctl FBIOGET_VSCREENINFO");
	    close(fb_fd);
	    return -1;
	}

	xres = var.xres;
	yres = var.yres;

	fbuffer = (unsigned char*)mmap(NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fb_fd, 0);
	if(fbuffer == (unsigned char *)-1)
	{
	    perror("mmap framebuffer");
	    close(fb_fd);
	    return -1;
	}

	bytes_per_pixel = (var.bits_per_pixel + 7) / 8;
	line_addr = (unsigned char**)malloc (sizeof (__u32) * var.yres_virtual);
	addr = 0;
	for(y = 0; y < var.yres_virtual; y++, addr += fix.line_length)
    	line_addr [y] = fbuffer + addr;

	return 0;
}

void setcolor(unsigned colidx, unsigned value)
{
	unsigned res;
	unsigned short red, green, blue;
	struct fb_cmap cmap;

	switch(0)
	{
		default:
		case 1:
			res = colidx;
			red = (value >> 8) & 0xff00;
			green = value & 0xff00;
			blue = (value << 8) & 0xff00;
			cmap.start = colidx;
			cmap.len = 1;
			cmap.red = &red;
			cmap.green = &green;
			cmap.blue = &blue;
			cmap.transp = NULL;
			if (ioctl(fb_fd, FBIOPUTCMAP, &cmap) < 0)
				perror("ioctl FBIOPUTCMAP");
			break;
		case 2:
		case 4:
			red = (value >> 16) & 0xff;
			green = (value >> 8) & 0xff;
			blue = value & 0xff;
			res = ((red >> (8 - var.red.length)) << var.red.offset) |
                      ((green >> (8 - var.green.length)) << var.green.offset) |
                      ((blue >> (8 - var.blue.length)) << var.blue.offset);
	}
	colormap [colidx] = res;
}


void __setpixel(union multiptr loc, unsigned xormode, unsigned color)
{
	switch(bytes_per_pixel)
	{
		case 1:
		default:
			if(xormode)
				*loc.p8 ^= color;
			else
				*loc.p8 = color;
			break;
		case 2:
			if(xormode)
				*loc.p16 ^= color;
			else
				*loc.p16 = color;
			break;
		case 4:
			if(xormode)
				*loc.p32 ^= color;
			else
				*loc.p32 = color;
			break;
	}
}

void pixel(int x, int y, unsigned colidx)
{
	unsigned xormode;
	union multiptr loc;
	
	int x1,y1;
	
	x1 = y;
	y1 = 240-x;
	
	if ((x1 < 0) || ((__u32)x1 >= var.xres_virtual) ||
			(y1 < 0) || ((__u32)y1 >= var.yres_virtual))
		return;

	xormode = colidx & XORMODE;
	colidx &= ~XORMODE;
	
	loc.p8 = line_addr [y1] + x1 * bytes_per_pixel;
	__setpixel(loc, xormode, colormap[colidx]);
}

void add_point(int x,int y,unsigned colidx)
{
	int i,j,m,n;
	for(i= -1;i<=1;i++)
		for(j= -1;j<=1;j++)
		{
			if(i==0 && j==0)
			{
				pixel(x+i,y+j,colidx);
			}
			else if(i==0 && j==-1)
			{
				pixel(x+i,y+j,colidx);
			}
			else if(i==1 && j==0)
			{
				pixel(x+i,y+j,colidx);
			}
			m = x+i+74;
			n = y+j;
			
			if(n<48)
				n = 48;
			if(m<0)
				m = 0;
		
			line[(n-48)*48+m/8] |= (0x1<<(7-m%8));
		}
}


void line2(int x1, int y1, int x2, int y2, unsigned colidx)
{
	static volatile int tmp;
	static volatile int dx;
	static volatile int dy;
	
	dx = x2 - x1;
	dy = y2 - y1;

	if(abs (dx) < abs (dy))
	{
		if(y1 > y2)
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while(y1 <= y2)
		{
			add_point(x1 >> 16,y1,colidx);
			//pixel (x1 >> 16,y1,colidx);
			x1 += dx;
			y1++;
		}
	}
	else
	{
		if (x1 > x2)
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2)
		{
			add_point(x1,y1 >>16,colidx);
			//pixel (x1,y1 >>16,colidx);
			y1 += dy;
			x1++;
		}
	}
}

int Os__init_sign(int x,int y,int width,int height)
{
	int i;
	fbuffer = NULL;
    line_addr = NULL;
    bytes_per_pixel = 0;
    xres= 0;
    yres = 0;
	ts = NULL;
    sign_x = x;
    sign_y = y;
    sign_w = width;
    sign_h = height;

    memset(line,0x00,sizeof(line));
    memset(colormap,0,sizeof(colormap));
    memset(&samp,0,sizeof(struct ts_sample));

    if(open_framebuffer() != 0)
    	return -1;

    for(i=0;i<NR_COLORS;i++)
		setcolor(i, palette_sign[i]);
		
	ts = ts_open(DEV_TS,1);
    if(!ts || ts_config(ts))
    {
    	Os__close_sign();
    	return -1;
    }

	return 0;
}

void Os__get_sign(void)
{
	int x,y;
	int mode = 1, ret = 0;
    exit_flag = 1;
    while(exit_flag)
    {
        ret = ts_read(ts, &samp, 1);

        if(ret < 0)
        	break;
        if(ret!=1)
        {
            usleep(20000);
            continue;
        }

        if(samp.pressure > 0)
        {
            //in sign zone
            if((samp.x>sign_y)&&(samp.x<sign_y+sign_h) && (240-samp.y>sign_x)&&(240-samp.y<sign_w+sign_x))
            {
                if(mode == 0x3)
                {
                    line2(x, y, 240-samp.y, samp.x,2);
                }
                else
                {
                    mode |= 0x2;
                }

                x = 240-samp.y;
                y = samp.x;
            }
            else
            {
                mode &= ~0x2;
            }
        }
        else
        {
            mode &= ~0x2;
        }
	}
}


void Os__close_sign(void)
{
	exit_flag = 0;
	if(fbuffer != NULL)
		munmap(fbuffer,fix.smem_len);
	if(fb_fd > 0)
		close(fb_fd);
	if(line_addr != NULL)
		free(line_addr);
	if(ts != NULL)
		ts_close(ts);
}


void Os__clear_sign(void)
{
	int i;
	for(i=0;i<240;i++)
		memset(fbuffer+50*2+i*640,0xFF,211*2);
	memset(line,0x00,sizeof(line));
}

int Os__print_sign(void)
{
    if(Os__checkpaper() == 0)
    {
        return Os__graph_xprint(line,sizeof(line));
    }
	return -2;
}
