PRE		= arm-none-linux-gnueabi-
CC		= $(PRE)gcc
STRIP	= $(PRE)strip
AS		= $(PRE)as
LINK	= $(PRE)link

#编译优化等级
CFLAGS  = -O2
#源码路径
VPATH	= ./
#头文件路径
INCPATH	=  -I../appApi/src/src
#包含库的路径
LFLAGS = -L./
#包含库
LIBS = -lapp -liconv -ljpeg
#.o文件
OBJ		= ./
#生成库路径以及名称
LOCAL_TEST	= ./aptt
#minicom下装路径
MINICOM_PATH = ../rel

#系统更新进程
TARGETS +=$(LOCAL_TEST) 

.PHONY:all
all:$(TARGETS)

$(LOCAL_TEST): $(OBJ)/apiTest.o
	$(CC) $(LFLAGS) $(LIBS) -I include -lpthread -lm $^ -o $@   
	$(STRIP) $@
#	mv $@ $(MINICOM_PATH)
	@echo "End LOCAL_TEST\n"


.PHONY:clean
clean:
	rm -rf $(OBJ)/*.o
	rm -rf $(LOCAL_TEST)
