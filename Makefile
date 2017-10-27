CC=gcc
CFLAGS=-pthread -std=gnu99 -O2 -s

all:
	$(CC) pi.c $(CFLAGS) -o pi

clean:
	rm pi