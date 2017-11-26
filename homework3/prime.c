#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int isprime(int n) {
  int i, squareroot;
  if (n > 10) {
    squareroot = (int) sqrt(n);
    for (i = 3; i <= squareroot; i = i + 2)
      if ((n % i) == 0)
        return 0;
    return 1;
  }
  else
    return 0;
}

int main(int argc, char *argv[])
{
  int pc;
  int _pc;
  int foundone;
  int _foundone;
  long long int n, limit;

  // cjyeh
  int size;
  int rank;
  long long int left;
  long long int right;
  long long int block;

  sscanf(argv[1], "%llu", &limit);
  printf("Starting. Numbers to be scanned = %lld\n", limit);

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  printf("%d %d\n", size, rank);

  pc = 4;
  _pc = 0;
  limit -= (limit & 1) ? 2 : 1;
  block = limit / size + 1;

  left = rank * block + 1;
  right = (rank + 1) * block;

  for (n = left; n < right && n < limit; n += 2) {
    if (isprime(n)) {
      _pc++;
      _foundone = n;
    }
  }

  MPI_Status status;

  printf("rank %d, _pc = %d, _foundone = %d\n", rank, _pc, _foundone);

  if (rank == 0) {
      pc += _pc;
      foundone = _foundone;

      for(int i = 1; i < size; i++){
          MPI_Recv(&_pc, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
          pc += _pc;
          MPI_Recv(&_foundone, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
          foundone = foundone < _foundone ? _foundone : foundone;
      }
      printf("Done. Largest prime is %d Total primes %d\n", foundone, pc);
  }
  else {
      MPI_Send(&_pc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&_foundone, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}
