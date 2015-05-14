include $(SANDAPP_SDK_PATH)/poscfg/config.rules

#CC= /usr/local/arm-linux/usr/bin/arm-linux-gcc
#STRIP= /usr/local/arm-linux/usr/bin/arm-linux-strip
#AS   = /usr/local/arm-linux/usr/bin/arm-linux-as
#LINK = /usr/local/arm-linux/usr/bin/arm-linux-link
#LIBPATH= ../../LIB

CFLAGS=-g -O0

#INCLUDE = -I/usr/local/pslib/include -I../appinc -I../EMVInc -I../InterfaceInc -I../QEMVInc -I../VisaWaveInc
INCLUDE  = -I/mnt/hgfs/imxlib258/osinc/ -I../appinc -I../emvinc -I../interfaceinc -I../qemvinc -I../visawaveinc

#LFLAGS = -L$(LIBPATH) -lapp  -lSMC_CHK -lpthread -lcommsencrypt
LFLAGS = -L$(LIBPATH)  -lpthread  -lSDL -lSDL_ttf -liconv -lfreetype -lz -lSDL_image
#LFLAGS = -L/mnt/hgfs/imxlib258/lib/  -lpthread  -lSDL -lSDL_ttf -liconv -lfreetype -lz -lSDL_image
LIBS= -lapp -lemv -lvisawave -lcommsencrypt
#    -lec -lqemv 
TARGET_DIR = ../../bin/
OBJECT_DIR = ../../obj/

TARGET_FILES = 104

SOURCE_FILES = $(wildcard *.c)
OBJECT_FILES = $(patsubst %.c, %.o, $(SOURCE_FILES))

$(addprefix $(OBJECT_DIR), %.o) : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@
	

$(addprefix $(TARGET_DIR), $(TARGET_FILES)) : $(addprefix $(OBJECT_DIR), $(OBJECT_FILES))
	$(CC) $(CFLAGS)   $(LFLAGS) $(LIBS) $(INCLUDE) $^ -o $@
	$(STRIP) -s $(addprefix $(TARGET_DIR), $(TARGET_FILES))
	
.PHONY : clean
clean :
	rm -rf ../../obj/*.o
	rm -rf $(TARGET_DIR)$(TARGET_FILES)