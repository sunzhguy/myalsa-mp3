include ./build.cfg

TAR_FILE=demo-pcm



OBJ_FILES=aenc.c

LIB_PATH= /home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/lib
LIB_FILES=-lasound

INC_PATH=/home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/include/
CFLAGS+=-I$(INC_PATH)


all:
	$(CC) $(CFLAGS) -o $(TAR_FILE) $(OBJ_FILES) -L$(LIB_PATH) $(LIB_FILES) -lpthread

clean:
	rm -rf $(TAR_FILE)
