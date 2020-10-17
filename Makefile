RM = rm -rf
CC = gcc
CFLAGS = -std=c99 -g -D_POSIX_C_SOURCE=200809L

default: all

all: bin client

bin:
	mkdir -p bin/

client: systems-assignment-gli.c
	$(CC) $< $(CFLAGS) -o bin/socket

soc: socket.c
	$(CC) $< $(CFLAGS) -o bin/socket

clean:
	$(RM) bin/

.PHONY: client clean
