#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

typedef unsigned long long unlong;

// global
unlong count_in_circle;
unlong round_per_thread;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* tossDart() {
	// var
	unlong count = 0;

	// init
	srand(time(NULL));

	for (unlong i = 0; i < round_per_thread; i++) {
		// var
		double x = (double) rand() / (RAND_MAX + 1.0);
		double y = (double) rand() / (RAND_MAX + 1.0);

		if (x * x + y * y <= 1)
			count++;
	}

	// critical section
	pthread_mutex_lock(&lock);
	count_in_circle += count;
	pthread_mutex_unlock(&lock);

	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "usage: ./pi <num_of_cpu> <num_of_toss>\n");
		exit(1);
	}

	// var
	double pi_estimate;
	double elapsed_time;
	struct timeval t1, t2;
	unlong num_of_cpu = atoll(argv[1]);
	unlong num_of_toss = atoll(argv[2]);
	pthread_attr_t attr;
	pthread_t* threads = malloc(num_of_cpu * sizeof(pthread_t));

	// init
	count_in_circle = 0;
	round_per_thread = num_of_toss / num_of_cpu;
	pthread_attr_init(&attr);
	pthread_mutex_init(&lock, NULL);

	// start measurement
	gettimeofday(&t1, NULL);

	for (unlong i = 0; i < num_of_cpu; i++) {
		if (pthread_create(&threads[i], &attr, tossDart, (void *) NULL)) {
			printf("Error: pthread_create()\n");
			exit(1);
		}
	}

	for (unlong i = 0; i < num_of_cpu; i++) {
		pthread_join(threads[i], NULL);
	}

	// end measurement
	gettimeofday(&t2, NULL);

	free(threads);
	pthread_mutex_destroy(&lock);

	// calculate result
	pi_estimate = (double)count_in_circle * 4 / num_of_toss;
	elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000;
	elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000;

	printf("PI: %lf\n", pi_estimate);
	printf("Elapsed Time: %lf seconds\n", elapsed_time / 1000);

	return 0;
}