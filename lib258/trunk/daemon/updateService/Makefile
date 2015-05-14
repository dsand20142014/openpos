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
INCPATH	= -I./
#包含库的路径
LFLAGS = 
#包含库
LIBS = ./updateService.so
#.o文件
OBJ		= .
#生成库路径以及名称
UPDT_S_REL	= ./updateService.dm.3.1.4
UPDT_S_DBG	= ./upd_2.0
UPDT_S_DEL	= ./updateService
#minicom下装路径
REL_PATH = ../rel
DBG_PATH = /minicom_upload/

#批量处理
TARGETS += 

#.PHONY:TARGETS
rel:$(UPDT_S_REL)
dbg:$(UPDT_S_DBG)

$(UPDT_S_REL): $(OBJ)/updateService.o $(OBJ)/param.o
	$(CC) $(LFLAGS) -I include -Wall -lrt -lpthread -lm $^ -o $@   
	$(STRIP) $@
	mv $@ $(REL_PATH)
	@echo "End LOCAL_SERVICE\n"

$(UPDT_S_DBG): $(OBJ)/updateService.o $(OBJ)/param.o
	$(CC) $(LFLAGS) -I include -lpthread -lm $^ -o $@   
	$(STRIP) $@
	mv $@ $(DBG_PATH)
	@echo "End LOCAL_SERVICE\n"

.PHONY:clean
clean:
	rm -rf $(OBJ)/*.o
	rm -rf $(REL_PATH)/$(UPDT_S_DEL)*
