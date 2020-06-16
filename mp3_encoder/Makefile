#!/bin/sh
# GCC tools
#CC = gcc
CFLAGS = -c -O2 -Wall
LD = $(CC)
LDFLAGS = -o $@

# Files
OBJS = bitstream.o huffman.o layer3.o loop.o main.o wave.o coder.o utils.o

# Rules

shine.exe: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf ./*.o shine.exe

# Header dependencies

bitstream.o: types.h
huffman.o: types.h
layer3.o: types.h
coder.o: types.h table1.h
loop.o: types.h table2.h
main.o: types.h
wave.o: types.h
