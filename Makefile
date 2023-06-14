CC=g++
CFLAGS=-Wall -Wextra -std=c++20
BINARY=slc

all: main.cpp
	$(CC) $(CFLAGS) $^ -o $(BINARY)
