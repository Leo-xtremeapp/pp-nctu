CC=gcc
CFLAGS=-pthread -std=gnu99 -O2 -s

all:
	$(CC) aes_serial.c $(CFLAGS) -o aes_serial
	$(CC) aes_parallel.c $(CFLAGS) -o aes_parallel

clean:
	rm -f aes_serial
	rm -f aes_parallel
