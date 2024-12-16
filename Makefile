CC      = gcc
CFLAGS  = -std=gnu11 -Os -g -Wall -Wconversion -Wextra -Wpedantic
SRC     = src

all: build

build: main.c $(SRC)/path_def.c $(SRC)/strf.c $(SRC)/fort.c
	$(CC) ${CFLAGS} -o ricw $^
	chmod 775 ricw

install: all
	cp ricw /usr/local/bin/
	chmod +x /usr/local/bin/ricw

clean:
	rm -f ./ricw

uninstall:
	rm -f /usr/local/bin/ricw
