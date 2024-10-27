CC 		= gcc
CFLAGS 	= -std=gnu11 -Os -g -Wall -Wconversion -Wextra -Wpedantic

all:
	$(CC) ${CFLAGS} -c pathdec.c
	$(CC) ${CFLAGS} -c strf.c
	$(CC) ${CFLAGS} -c main.c
	$(CC) ${CFLAGS} -o ricw pathdec.o strf.o main.o
