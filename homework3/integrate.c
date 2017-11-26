#include "mpi.h"
#include <stdio.h>
#include <math.h>

#define PI 3.1415926535

int main(int argc, char **argv)
{
  long long i;
  long long num_intervals;
  double rect_width;
  double area;
  double sum;
  double _sum;
  double x_middle;

  int size;
  int rank;
  long long int left;
  long long int right;
  long long int block;

  sscanf(argv[1], "%llu", &num_intervals);

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  rect_width = PI / num_intervals;


  MPI_Status status;

  sum = 0;
  _sum = 0;

  block = num_intervals / size + 1;

  left = rank * block + 1;
  right = (rank + 1) * block;

  for(i = left; i <= right && i <= num_intervals; i++) {
    x_middle = (i - 0.5) * rect_width;
    area = sin(x_middle) * rect_width;
    _sum = _sum + area;
  }

  if (rank == 0) {
      sum += _sum;

      for(int j = 1; j < size; j++){
          MPI_Recv(&_sum, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &status);
          sum += _sum;
      }
      printf("The total area is: %f\n", (float)sum);
  }
  else {
      MPI_Send(&_sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}
