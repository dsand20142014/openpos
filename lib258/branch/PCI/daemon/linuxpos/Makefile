#QC430 = y

PROJECT = ps4000
CC = arm-none-linux-gnueabi-gcc 
LINK = arm-none-linux-gnueabi-gcc
VPATH= ossrc 

OBJPATH  = obj

NAME		= linuxpos.dm
VER	=	1.2.5
#release path
REL_PATH = ../rel/

TARGET = $(NAME).$(VER)

INCPATH  = -I osinclude -I`pwd`/../../appLib/appApi/inc/ -I./../../appLib/appApi/src/src
CFLAGS = -mlittle-endian -O0 -Wall -D_GNU_SOURCE=1 -DREENTRANT 

ifeq ($(QC430_v01),y)
CFLAGS += -DQC430_v01
endif

ifeq ($(PROJECT), ps4000)
CFLAGS += -D_PS4000
PS4000DIR = ../../appLib/
LIBPATH  = -L`pwd`/$(PS4000DIR)/rel
endif

LIBS  = -lrt -ldl -lpthread -lapp -ljpeg -liconv -liw
LIBS += -lm -liw
LFLAGS =  -Wl $(LIBS) $(LIBPATH)

OBJS += $(OBJPATH)/main.o 
OBJS += $(OBJPATH)/sys_linux.o 
#OBJS += $(OBJPATH)/osmenu.o //xl del 20130510

.PHONY:all
all:$(TARGET)
$(TARGET):$(OBJS)
	$(LINK) $(LFLAGS) $^ -o $@ 
	arm-none-linux-gnueabi-strip $@
	mv $(TARGET) $(REL_PATH)
	@echo "End linuxpos"

$(OBJPATH)/%.o : %.c
	$(CC) $(CFLAGS) $(INCPATH) -c $< -o $@ 

.PHONY:clean
clean:
	rm -f $(TARGET)
	rm -f $(OBJPATH)/*
	rm -rf $(REL_PATH)$(NAME).*
