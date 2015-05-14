PRE		= arm-none-linux-gnueabi-
#PRE		= arm-linux-
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
NAME		= ./dial.dm
#版本
VER	=	4.1.4
#release path
REL = ../rel/

DIAL	=$(NAME).$(VER)

#进程
TARGETS +=$(DIAL) 

.PHONY:all
all:$(TARGETS)

$(DIAL): $(OBJ)/dial.o
	$(CC) $(LFLAGS) -I include -Wall -lrt -lpthread -lm $^ -o $@   
	$(STRIP) $@
	mv $@ $(REL)
	@echo "End DIAL\n"


.PHONY:clean
clean:
	rm -rf $(OBJ)/*.o
	rm -rf $(REL)$(NAME).*
