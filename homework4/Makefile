CC=gcc
NVCC=nvcc

all:
	$(CC) serial_wave.c -lm -o serial_wave
	$(NVCC) parallel_wave.cu -o parallel_wave

clean:
	rm serial_wave
	rm parallel_wave
