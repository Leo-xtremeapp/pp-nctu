/**********************************************************************
 * DESCRIPTION:
 *   Serial Concurrent Wave Equation - C Version
 *   This program implements the concurrent wave equation
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAXPOINTS 1000000
#define MAXSTEPS 1000000
#define MINPOINTS 20
#define PI 3.14159265

// cjyeh
#define BLOCK_SIZE 1024

void check_param(void);
void update (void);
void printfinal (void);

int nsteps,                 	 /* number of time steps */
    tpoints, 	     		         /* total points along string */
    rcode;                  	 /* generic return code */

float  values[MAXPOINTS+2], 	 /* values at time t */
       old_val[MAXPOINTS+2], 	 /* values at time (t-dt) */
       new_val[MAXPOINTS+2]; 	 /* values at time (t+dt) */

// cjyeh
float  *values_d;

/**********************************************************************
 *	Checks input values from parameters
 *********************************************************************/
void check_param(void)
{
   char tchar[20];

   /* check number of points, number of iterations */
   while ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) {
      printf("Enter number of points along vibrating string [%d-%d]: "
           ,MINPOINTS, MAXPOINTS);
      scanf("%s", tchar);
      tpoints = atoi(tchar);
      if ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS))
         printf("Invalid. Please enter value between %d and %d\n",
                 MINPOINTS, MAXPOINTS);
   }
   while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
      printf("Enter number of time steps [1-%d]: ", MAXSTEPS);
      scanf("%s", tchar);
      nsteps = atoi(tchar);
      if ((nsteps < 1) || (nsteps > MAXSTEPS))
         printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS);
   }

   printf("Using points = %d, steps = %d\n", tpoints, nsteps);
}

/* run_parallel */
__global__ void run_parallel(float *values_d, int tpoints, int nsteps)
{
    int i, k;
    float x, fac, tmp;
    float dtime, c, dx, tau, sqtau;
    float value, new_val, old_val;

    /* init_line() */
    fac = 2.0 * PI;
    k = 1 + blockIdx.x * BLOCK_SIZE + threadIdx.x;
    tmp = tpoints - 1;
    x = (k - 1) / tmp;
    value = sin (fac * x);
    old_val = value;

    /* do_math() */
    dtime = 0.3;
    c = 1.0;
    dx = 1.0;
    tau = (c * dtime / dx);
    sqtau = tau * tau;

    /* update() */
    if(k <= tpoints) {
      for (i = 1; i<= nsteps; i++) {
        if ((k == 1) || (k  == tpoints))
          new_val = 0.0;
        else
          new_val = (2.0 * value) - old_val + (sqtau * -2.0 * value);
        old_val = value;
        value = new_val;
      }
      values_d[k] = value;
    }
}

/**********************************************************************
 *     Print final results
 *********************************************************************/
void printfinal()
{
   int i;

   for (i = 1; i <= tpoints; i++) {
      printf("%6.4f ", values[i]);
      if (i % 10 == 0)
         printf("\n");
   }
}

/**********************************************************************
 *	Main program
 *********************************************************************/
int main(int argc, char *argv[])
{
    // var
    int size;
    int block_num;

    sscanf(argv[1], "%d", &tpoints);
  	sscanf(argv[2], "%d", &nsteps);
  	check_param();

    size = (tpoints + 1) * sizeof(float);
    cudaMalloc((void**) &values_d, size);

    printf("Initializing points on the line...\n");
  	//init_line();
  	printf("Updating all points for all time steps...\n");
  	//update();

    block_num = tpoints / BLOCK_SIZE + !(tpoints % BLOCK_SIZE == 0);

    run_parallel<<<block_num, BLOCK_SIZE>>>(values_d, tpoints, nsteps);

    cudaMemcpy(values, values_d, size, cudaMemcpyDeviceToHost);
    cudaFree(values_d);

    printf("Printing final results...\n");
  	printfinal();
  	printf("\nDone.\n\n");

	return 0;
}
