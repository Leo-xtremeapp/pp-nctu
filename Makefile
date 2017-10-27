CC=gcc
CFLAGS=-pthread

all:
	$(CC) pi.c $(CFLAGS) -o pi

clean:
	rm pi