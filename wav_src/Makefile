include ./build.cfg

TAR_FILE=demo-wav

SRC_FILES=$(shell find ./ -name "*.c" -print)
OBJ_FILES=$(patsubst %.c,%.o,$(SRC_FILES))

LIB_PATH=../lib/
LIB_PATH+=-L/home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/lib
LIB_FILES=-lasound

INC_PATH=../include/
INC_PATH+=-I/home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/include/
CFLAGS+=-I$(INC_PATH)


all:$(TAR_FILE)
$(TAR_FILE):$(OBJ_FILES)
	$(CC) $(CFLAGS) -o $(TAR_FILE) $(OBJ_FILES) -L$(LIB_PATH) $(LIB_FILES) -lpthread

clean:
	rm -rf $(OBJ_FILES)
	rm -rf $(TAR_FILE)
