CC=g++
FLAGS=-lOpenCL -O3

all:
	$(CC) image-histogram.cpp -o image-histogram
	$(CC) histogram.cpp $(FLAGS) -o histogram

clean:
	rm -f image-histogram
	rm -f histogram
	rm -f 0556563.out
