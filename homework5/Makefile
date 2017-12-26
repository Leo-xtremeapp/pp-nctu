CC=g++
FLAGS=-lOpenCL

all:
	$(CC) image-histogram.cpp -o image-histogram
	$(CC) histogram.cpp $(FLAGS) -o histogram

clean:
	rm -f image-histogram
