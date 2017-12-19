CC=gcc
CFLAGS=-pthread -std=gnu99 -O2 -s

all:
	$(CC) aes.c $(CFLAGS) -o aes
	$(CC) aes_pthread.c $(CFLAGS) -o aes_pthread

clean:
	rm -f aes
	rm -f aes_pthread
