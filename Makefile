CC=cc
CFLAGS=-Wall -Wextra -std=c2x
BINARY=slc

all: main.c
	$(CC) $(CFLAGS) $^ -o $(BINARY)
