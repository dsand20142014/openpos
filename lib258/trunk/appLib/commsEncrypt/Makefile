PRE		= arm-none-linux-gnueabi-
CC		= $(PRE)gcc
STRIP	= $(PRE)strip
AS		= $(PRE)as
LINK	= $(PRE)link

#编译优化等级
CFLAGS  = -O0
#源码路径
VPATH	= ./
#头文件路径
INCPATH	= -I./ -I../osApi/include/
#包含库的路径
LFLAGS = -L ../rel/ 
#包含库
LIBS = -lapp
#.o文件
OBJ		= ./
#生成库路径以及名称
NAME	= ../rel/libcommsencrypt.so
#版本
VER = 1.0.4

LIBAPP	= $(NAME).$(VER)

OBJECTS  += $(OBJ)/CommsEncrypt.o


TARGETS  = $(LIBAPP)

.PHONY:all
all:$(TARGETS)

$(LIBAPP): $(OBJECTS)
	$(CC) -shared $(LFLAGS) $(LIBS) -lpthread  $^ -o $@
	$(STRIP) $@
	@echo "End COMMSENCRYPT"

$(OBJ)/%.o : %.c
	$(CC) -fPIC -c $(CFLAGS) $(INCPATH) -c $< -o $@

.PHONY:clean
clean:
	rm -rf ./*.o
	rm -rf $(NAME).*

