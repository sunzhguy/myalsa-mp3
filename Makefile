include ./build.cfg

TOP_DIR = .
INSTALL_DIR = /bin

PROJECT = demo-pcm-mp3
CFLAGS = -O2 -Wall -g

SOURCE = aenc.c
SOURCE += mp3_encoder/bitstream.c
SOURCE += mp3_encoder/coder.c
SOURCE += mp3_encoder/formatBitstream.c
SOURCE += mp3_encoder/huffman.c
SOURCE += mp3_encoder/layer3.c
SOURCE += mp3_encoder/loop.c
SOURCE += mp3_encoder/utils.c
SOURCE += mp3_encoder/wave.c

SOURCE += ring_buffer/kring_buffer.c

LIB_PATH= /home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/lib
LIB_FILES=-lasound

INC_PATH=/home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/include/
CFLAGS+=-I$(INC_PATH)




SUBDIRS := $(shell find -type d)

#$(shell mkdir -p $(TOP_DIR)$(INSTALL_DIR))

$(shell for val in $(SUBDIRS);do \
mkdir -p $(TOP_DIR)$(INSTALL_DIR)/$${val}; \
done;)

OBJS = $(SOURCE:%.c=$(TOP_DIR)$(INSTALL_DIR)/%.o)

$(TOP_DIR)$(INSTALL_DIR)/%o:%c

$(TOP_DIR)$(INSTALL_DIR)/$(PROJECT):$(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)  -lpthread  -lasound

$(TOP_DIR)$(INSTALL_DIR)/%.o:%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@  


clean:
	-rm -rf $(TOP_DIR)$(INSTALL_DIR)

