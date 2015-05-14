#author lichao 2013.5.8

PRE		= arm-none-linux-gnueabi-
CC		= $(PRE)gcc
STRIP		= $(PRE)strip
AS		= $(PRE)as
LINK		= $(PRE)link

#¿¿¿¿¿¿
CFLAGS  = -O2
#¿¿¿¿
VPATH	= ./src ./
#¿¿¿¿¿
INCPATH	=  -I ./inc -I ../appLib/appApi/inc -I ../appLib/rel/osinc -I ../appLib/appApi/src/src/
#¿¿¿¿¿¿
LFLAGS = -L ../appLib/rel/
#¿¿¿
LIBS = -lapp -liconv -ljpeg -liw
#.o¿¿
OBJ		= ./obj
#Éú³É¿âÂ·¾¶ÒÔ¼°Ãû³Æ
LOCAL_TEST	= ./posapp.bin

OBJECTS  += $(OBJ)/cfg.o
OBJECTS  += $(OBJ)/comms.o
OBJECTS  += $(OBJ)/evecom.o
OBJECTS  += $(OBJ)/Global.o
OBJECTS  += $(OBJ)/iso8583.o
OBJECTS  += $(OBJ)/logon.o
OBJECTS  += $(OBJ)/mag.o
OBJECTS  += $(OBJ)/manager.o
OBJECTS  += $(OBJ)/msg.o
OBJECTS  += $(OBJ)/pinpad.o
OBJECTS  += $(OBJ)/print.o
OBJECTS  += $(OBJ)/save.o
OBJECTS  += $(OBJ)/trans.o
OBJECTS  += $(OBJ)/util.o
OBJECTS  += $(OBJ)/xdata.o
OBJECTS  += $(OBJ)/main.o
OBJECTS  += $(OBJ)/menu.o


#ÏµÍ³¸üÐÂ½ø³Ì
TARGETS +=$(LOCAL_TEST) 

.PHONY:all
all:$(TARGETS)

$(LOCAL_TEST): $(OBJECTS)
	$(CC) $(LFLAGS) $(LIBS) -I include -lpthread -lm $^ -o $@
	$(STRIP) $@
	@echo "End POSAPP"

$(OBJ)/%.o : %.c
	$(CC) -fPIC -c $(CFLAGS) $(INCPATH) -c $< -o $@


.PHONY:clean
clean:
	rm -rf $(OBJ)/*.o
	rm -rf $(LOCAL_TEST)

