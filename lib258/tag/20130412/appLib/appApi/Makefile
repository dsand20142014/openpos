#author liuyifeng 20120912 

PRE		= arm-none-linux-gnueabi-
CC		= $(PRE)gcc
STRIP	= $(PRE)strip
AS		= $(PRE)as
LINK	= $(PRE)link

#编译优化等级
CFLAGS  = -O2
#源码路径
VPATH	= src src/lib src/src
#头文件路径
INCPATH	= -I./inc/gui/SDL -I./inc -I./inc/inc -I./src/src
#包含库的路径
LFLAGS = -L../rel 
#包含库
LIBS = -lm -liconv -lrt #-L../rel/ljpeg
#.o文件
OBJ		= ./obj
#生成库路径以及名称
APPNAME = ../rel/libapp.so
#版本
VER		= 1.1.4

LIBAPP	= $(APPNAME).$(VER)


###############################
#	可以直接注释选择功能	  #
###############################

#摄像头
OBJECTS	 += $(OBJ)/camera.o 
#外部存储
OBJECTS  += $(OBJ)/massstorage.o 
#屏幕亮度，led，蜂鸣器,需要ostime.h中的Os__xdelay支持
OBJECTS	 += $(OBJ)/modules.o
#条码枪
OBJECTS  += $(OBJ)/bar.o
#写文件方式操作文件
OBJECTS  += $(OBJ)/fileopt.o
#获取应用列表
OBJECTS  += $(OBJ)/applist.o
#配置文件操作
OBJECTS  += $(OBJ)/param.o
#工具函数
OBJECTS  += $(OBJ)/ostools.o
#各个设备状态
OBJECTS	 += $(OBJ)/devSignal.o
#api中需要开机初始化和获取系统参数的部分
OBJECTS	 += $(OBJ)/sysglobal.o
#gps
OBJECTS  += $(OBJ)/gps.o
#gsm
OBJECTS  += $(OBJ)/gsm.o
#算法
OBJECTS  += $(OBJ)/crc.o
OBJECTS  += $(OBJ)/md5.o
OBJECTS  += $(OBJ)/sha1.o
OBJECTS  += $(OBJ)/rsa.o 	#需要关联nn.c
OBJECTS	 += $(OBJ)/nn.o
OBJECTS	 +=	$(OBJ)/sha256.o

#卡相关,未整理，先归类
OBJECTS  += $(OBJ)/fsync_drvs.o
OBJECTS  += $(OBJ)/sand_osdrv.o
OBJECTS  += $(OBJ)/osdrv.o
OBJECTS  += $(OBJ)/osicc.o
OBJECTS  += $(OBJ)/sand_icc.o
OBJECTS  += $(OBJ)/pboc.o
OBJECTS  += $(OBJ)/osmifare.o
OBJECTS  += $(OBJ)/4442.o
OBJECTS  += $(OBJ)/classic.o
OBJECTS  += $(OBJ)/desfire.o
OBJECTS  += $(OBJ)/dicard.o

#串口
OBJECTS  += $(OBJ)/osuart.o

#加密
OBJECTS  += $(OBJ)/sand_encrypt.o
OBJECTS  += $(OBJ)/encrypt.o

#通讯
OBJECTS  += $(OBJ)/sand_network.o
OBJECTS  += $(OBJ)/sand_com.o
OBJECTS  += $(OBJ)/sand_modem.o
OBJECTS  += $(OBJ)/sand_gprs.o
OBJECTS  += $(OBJ)/network.o

#打印机
OBJECTS  += $(OBJ)/printer.o
OBJECTS  += $(OBJ)/zh_CN_Font.o

#计时器
OBJECTS  += $(OBJ)/ostime.o

#淘汰接口
OBJECTS +=$(OBJ)/oldapi.o







##################
#OBJECTS  += $(OBJ)/sand_time.o
#OBJECTS  += $(OBJ)/sand_timer.o

OBJECTS  += $(OBJ)/sand_main.o
OBJECTS  += $(OBJ)/sand_app.o
OBJECTS  += $(OBJ)/sand_lcd.o
#OBJECTS  += $(OBJ)/sand_debug.o
#OBJECTS  += $(OBJ)/sand_tools.o
#OBJECTS  += $(OBJ)/sand_modules.o
#OBJECTS	 += $(OBJ)/sys.o
#OBJECTS  += $(OBJ)/key.o

#OBJECTS  += $(OBJ)/new_drv.o
OBJECTS  += $(OBJ)/sand_key.o


OBJECTS  += $(OBJ)/toolslib.o
#OBJECTS  += $(OBJ)/osmem.o
#OBJECTS  += $(OBJ)/asc816.o
#OBJECTS  += $(OBJ)/asc1224.o
#OBJECTS  += $(OBJ)/classic.o
#OBJECTS  += $(OBJ)/sand_gprs.o
#OBJECTS  += $(OBJ)/sand_mmi.o
#OBJECTS  += $(OBJ)/sand_cfg.o #配置参数下载功能
#OBJECTS  += $(OBJ)/libaes.o  
#OBJECTS  += $(OBJ)/libconv.o
#OBJECTS  += $(OBJ)/libcrc.o
#OBJECTS  += $(OBJ)/libdate.o
#OBJECTS  += $(OBJ)/libmd5.o
#OBJECTS  += $(OBJ)/libmisc.o
#OBJECTS  += $(OBJ)/libnn.o
#OBJECTS  += $(OBJ)/librsa.o
#OBJECTS  += $(OBJ)/libsha1.o
#OBJECTS  += $(OBJ)/libtlv.o
#OBJECTS  += $(OBJ)/libzip.o
#OBJECTS  += $(OBJ)/libdate.o
#OBJECTS  += $(OBJ)/myconfig.o
#OBJECTS  += $(OBJ)/iwlib.o
#OBJECTS  += $(OBJ)/sand_adc.o
#OBJECTS  += $(OBJ)/jn24g.o
#OBJECTS  += $(OBJ)/ospad.o
#OBJECTS  += $(OBJ)/AES.o
#OBJECTS  += $(OBJ)/sand_usb.o
#OBJECTS  += $(OBJ)/sand_sd.o
#OBJECTS  += $(OBJ)/Os__barscanner_9600.o
#OBJECTS  += $(OBJ)/py_input.o
#OBJECTS  += $(OBJ)/asc_table.o
#OBJECTS  += $(OBJ)/uart.o
## for Poximity Card ##
#OBJECTS  += $(OBJ)/desfire.o
#OBJECTS  += $(OBJ)/dicard.o
#OBJECTS  += $(OBJ)/classic.o
#OBJECTS  += $(OBJ)/cmd_rf.o    #simple RF API for India
#OBJECTS +=$(OBJ)/lcd.o
#OBJECTS +=$(OBJ)/osdisplayLCD.o
#OBJECTS +=$(OBJ)/sand_security_monitor.o
#OBJECTS +=$(OBJ)/rubbish.o

TARGETS  = $(LIBAPP)

.PHONY:all
all:$(TARGETS)

$(LIBAPP): $(OBJECTS)
	$(CC) -shared $(LFLAGS) $(LIBS) -lpthread  $^ -o $@
	$(STRIP) $@
#	rm -rf $(OBJ)/*	
	@echo "End OSAPP"

$(OBJ)/%.o : %.c
	$(CC) -fPIC -c $(CFLAGS) $(INCPATH) -c $< -o $@

.PHONY:clean
clean:
	rm -rf $(OBJ)/*		   
	rm -rf $(APPNAME).*


