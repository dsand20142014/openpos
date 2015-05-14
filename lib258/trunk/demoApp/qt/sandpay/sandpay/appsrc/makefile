#CC= /usr/local/arm-linux/usr/bin/arm-linux-gcc
#STRIP= /usr/local/arm-linux/usr/bin/arm-linux-strip
#AS   = /usr/local/arm-linux/usr/bin/arm-linux-as
#LINK = /usr/local/arm-linux/usr/bin/arm-linux-link
#CFLAGS= -g -O0
#LIBPATH= /usr/local/pslib/lib/24xx

include $(SANDAPP_SDK_PATH)/poscfg/config.rules

INCLUDE 		= -I$(OSINC) -I../appinc
LFLAGS = -L$(LIBPATH) -lapp  -lSMC_CHK  -lcommsencrypt -lVikang


#-lpthread
LIBS= -lSDL -lSDL_ttf -liconv -lfreetype -lz -lSDL_image
TARGET_DIR = ../bin/
OBJECT_DIR = ../obj/

TARGET_FILES = 194

SOURCE_FILES = $(wildcard *.c)
OBJECT_FILES = $(patsubst %.c, %.o, $(SOURCE_FILES))

$(addprefix $(OBJECT_DIR), %.o) : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

$(addprefix $(TARGET_DIR), $(TARGET_FILES)) : $(addprefix $(OBJECT_DIR), $(OBJECT_FILES))
	$(CC) $(CFLAGS) $(LFLAGS) $(LIBS) $(INCLUDE) $^ -o $@
	$(STRIPS) -s $(addprefix $(TARGET_DIR), $(TARGET_FILES))
	
.PHONY : clean
clean :
	rm -rf ../OBJ/*.o
	rm -rf $(TARGET_DIR)$(TARGET_FILES)
