PRE		= arm-none-linux-gnueabi-
#PRE		= 
CC		= $(PRE)gcc
STRIP	= $(PRE)strip
AS		= $(PRE)as
LINK	= $(PRE)link

#编译优化等级
CFLAGS  = -O2
#源码路径
VPATH	= ./
#头文件路径
INCPATH	= -I./
#包含库的路径
LFLAGS = 
#包含库
LIBS =
#.o文件
OBJ		= .
#生成库路径以及名称
DIAL		= ./sln.dm.0
#release path
MINICOM_PATH = ../rel/

#进程
TARGETS +=$(DIAL) 

.PHONY:all
all:$(TARGETS)

$(DIAL): $(OBJ)/ln.o
	$(CC) $(LFLAGS) -I include -lpthread -lm $^ -o $@   
	$(STRIP) $@
	mv $@ $(MINICOM_PATH)
	@echo "End LOCAL_SERVICE\n"


.PHONY:clean
clean:
	rm -rf $(OBJ)/*.o

