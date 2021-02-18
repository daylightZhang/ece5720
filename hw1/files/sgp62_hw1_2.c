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
# define min_exp     6
# define max_exp     10
# define MIN_SIZE    1<<min_exp
# define MAX_SIZE    1<<max_exp
# define BILLION     1000000000L



int main(int argc, char **argv){

// loop and other indices
  int logsize = min_exp;

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
  int n = MAX_SIZE;

// allocate memory and initialize a 
  a = (float **) malloc(n * sizeof(float *));
  for(int i = 0; i < n; i++) {
      a[i] = (float *) malloc(n * sizeof(float));
      for(int j = 0; j < n; j++) {
          a[i][j] = 1.0*i;
      }
  }
// allocate memory and initialize b 
  b = (float **) malloc(n * sizeof(float *));
  for(int i = 0; i < n; i++) {
      b[i] = (float *) malloc(n * sizeof(float));
      for(int j = 0; j < n; j++) {
          b[i][j] = 1.0*j;
      }
  }

// allocate memory for c 
  c = (float **) malloc(n * sizeof(float *));
  for(int i = 0; i < n; i++){
    c[i] = (float *) malloc(n * sizeof(float));
    for(int j = 0; j < n; j++) {
          c[i][j] = 1.0;
      }
  }

// ------ loop from MIN_SIZE, doubling the size, up to MAX_SIZE -----

  for(int n = MIN_SIZE; n <= MAX_SIZE; n += n){

    // start clock
    fprintf(fp, "%d, ", logsize);
    logsize++;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // your code
    for(int i = 0; i < n; i++){
      for(int j = 0; j < n; j++){
        for(int k = 0; k < n; k++){
          c[i][j] += a[i][k] * b[k][j];
        }
      }
    }

    // stop clock
    clock_gettime(CLOCK_MONOTONIC, &end);

    // calculate time taken for this size
    ntime.tv_sec = end.tv_sec - start.tv_sec;
    ntime.tv_nsec = end.tv_nsec - start.tv_nsec;
    float diff = ntime.tv_sec * BILLION + ntime.tv_nsec;

    // write to file
    fprintf(fp, "%1.3e, ", diff);
    fprintf(fp, "\n");
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
