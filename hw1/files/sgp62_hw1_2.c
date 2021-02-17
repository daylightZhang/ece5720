/* Stefen Pegels sgp62
 * Row by column matrix multiply
 * In a loop increase matrix dimension n from MIN_SIZE doubling
 * on each pass until MAX_SIZE is reached
 *
 * Timing results are stored in file rbyc.csv in a linear array
 *
 * to compile: gcc -std=gnu99 -O3 -o sgp62_hw1_2  sgp62_hw1_2.c
 * run code: ./sgp62_hw1_2
*/
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// set range for testing
# define MIN_SIZE    1<<6
# define MAX_SIZE    1<<10
# define BILLION     1000000000L

int main(int argc, char **argv){

// loop and other indices
    // your code
// other parameters
    // your code

// open file to record time measurements
  FILE *fp = NULL;
  fp = fopen("rbyc.csv", "w");

// declare matrices
  float **a, **b, **c;

// time measurement variables
  double time;
  struct timespec start, end, ntime;

// get clock resolution
  clock_getres(CLOCK_MONOTONIC, &start);
  printf("resolution of CLOCK_MONOTONIC is %ld ns\n", start.tv_nsec);

// if using random matrices, set seed srand48(1);
  n = MAX_SIZE;

// for a check of correctness use special matrices
// then set matrices to what is needed

// allocate memory and initialize a 
  a = (float **) malloc(n * sizeof(float *));
  for(i = 0; i < n; i++) {
      a[i] = (float *) malloc(n * sizeof(float));
      for(j = 0; j < n; j++) {
          a[i][j] = 1.0*i;
      }
  }
// allocate memory and initialize b 
  b = (float **) malloc(n * sizeof(float *));
  for(i = 0; i < n; i++) {
      b[i] = (float *) malloc(n * sizeof(float));
      for(j = 0; j < n; j++) {
          b[i][j] = 1.0*j;
      }
  }

// allocate memory for c 
    // your code

// ------ loop from MIN_SIZE, doubling the size, up to MAX_SIZE -----

  // start clock
  clock_gettime(CLOCK_MONOTONIC, &start);

  // your code

  // stop clock
  clock_gettime(CLOCK_MONOTONIC, &end);

  // calculate time taken for this size
  // your code

  // record absolute time or
  // scale by the number of operation which is loop^3, otherwise set to 1
  // your code

  // write to file
  // your code


// for sanity check print 8 by 8 upper left submatrix of the product c
// remove for the final code
  printf("size = %4d time = %lf \n",loop, time);
  for(i = 0; i < 8; i++) {
    for(j = 0; j < 8; j++) {
      printf("%8.2e  ",c[i][j]);
    }
    printf("\n");
  }

  fclose(fp); free(a); free(b); free(c);

/*
*  Create one way pipe line with call to popen()
*  need tile.csv file and plot_tile.gp file
*/

  FILE *tp = NULL;
  if (( tp = popen("gnuplot plot_rbyc.gp", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }

// Close the pipe
  pclose(tp);

}
