CC = gcc
CFLAGS := -ggdb3 -O2 -Wall -std=c11
CFLAGS += -Wno-unused-function -Wvla

# Flags for FUSE
LDLIBS := $(shell pkg-config fuse --cflags --libs)

# Name for the filesystem!
FS_NAME := fisopfs

all: build
	
build: $(FS_NAME)
$(FS_NAME): fisopfs.c fs.o

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

docker-build:
	./dock build

docker-run:
	./dock run

docker-exec:
	./dock exec

clean:
	rm -rf $(EXEC) *.o core vgcore.* $(FS_NAME)

.PHONY: all build clean format docker-build docker-run docker-exec
