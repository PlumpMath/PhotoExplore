CC=gcc # or gcc

FRAMEWORKS:= -framework Foundation -framework Cocoa
LIBRARIES:= -lobjc -lglfw3 

SOURCE=main.m

CFLAGS=-Wall -Werror -m32 -c -g -v $(SOURCE)
LDFLAGS=$(LIBRARIES) $(FRAMEWORKS)
OUT=-o main

all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUT)
