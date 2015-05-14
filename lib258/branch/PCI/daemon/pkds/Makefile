CC = arm-none-linux-gnueabi-gcc 
LINK = arm-none-linux-gnueabi-gcc
CFLAGS = -O2
VPATH= ossrc 

OBJPATH  = .

NAME = pkds.dm
VER = 0.1
#release path
REL_PATH = ../rel/

TARGET = $(NAME).$(VER)

INCPATH  = -I./../../appLib/appApi/inc/ -I./../../appLib/appApi/src/src

OBJS += $(OBJPATH)/pkds.o 
OBJS += $(OBJPATH)/drvs.o

.PHONY:all
all:$(TARGET)
$(TARGET):$(OBJS)
	$(LINK) $(LFLAGS) $^ -o $@ 
	arm-none-linux-gnueabi-strip $@
	cp $(TARGET) $(REL_PATH)
	@echo "End PKDS"

$(OBJPATH)/%.o : %.c
	$(CC) $(CFLAGS) $(INCPATH) -c $< -o $@ 

.PHONY:clean
clean:
	rm -f $(TARGET)
	rm -f $(OBJPATH)/*.o
	rm -rf $(REL_PATH)$(NAME).*
