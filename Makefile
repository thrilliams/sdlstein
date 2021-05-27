CC=clang
CFLAGS=-std=gnu11 -Wall $(shell pkg-config --cflags --libs sdl2) $(shell sdl2-config --cflags --libs)
ASAN_CFLAGS=-O1 -g -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address

main: main.c
	$(CC) $(CFLAGS) main.c -o main

debug: main.c
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) main.c -o main
