/* 
cache access time by different strides
(1) Set up a linear array of MAX_SIZE floats. 
(2) In a loop, examine access to subarrays of sizes from MIN_SIZE, 
    doubling the size in each pass through the loop, till size MAX_SIZE
(3) For each size of subarrays access their elements with varying length
    strides from stride 1, doubling the stride, until half of the size
    of the current subarray.
(4) Repeate (3) so the current subarray was accessed K*current_size times
    where K is used to average K runs

(5) Store time measurements in an SxL array where
      S is the number of different strides probed plus one
      L is the number of different subarrays used
      (for plotting results, make the first column logarithms base 2 
       of strides)

(6) use gnuplot to plot timing results, "gnuplot cahce_plots.gp"
    where the script cache_plots.gp is provided

    compile: gcc -std=gnu99 -o netID_hw1_1 netID_hw1_1.c 
    run: ./netID_hw1_cache                          
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define K 		10	        // run each stride K times
#define BILLION 	1000000000
#define min_exp         16              // smallest array tested
#define max_exp         26              // use as large max_exp as 
                                        // the system will tolerate

int main(int argc, char **argv) {

// define MIN_SIZE and MAZ_SIZE

// for starting and stopping timers
  struct timespec start, end; 

// dynamic memory allocation for A
  float *A;
  A = (float *) malloc(MAX_SIZE*sizeof(float));
  if(!A) {
    printf("Memory not allocated, malloc returns NULL!\n");
    exit(1);
    }

// open file for writing timing results
  FILE *tp = NULL;                    
  tp = fopen("time_cache.csv", "w"); // file where time measurements
                                     // are stored

// get clock resolution
  clock_getres(CLOCK_MONOTONIC, &start);
  printf("resolution of CLOCK_MONOTONIC is %ld ns\n", start.tv_nsec);
	
// Main loop,  double the array length starting from min length

// Loop over strides

// start timer
   clock_gettime(CLOCK_MONOTONIC, &start);

// repeat K times the current stride s for averaging
      for (j = 0; j < K*s; j++) { 
        for (i = 0; i < n; i+=s)
          /* access A[i] */
      }
// stop timer
   clock_gettime(CLOCK_MONOTONIC, &end);

// compute the average access time in K repetitions

// write to file

// end of loop over strides

// end of loop over subarrays

// close file and free A
  fclose(tp); free(A);

/* -------- this part is to create a gnuplot *.eps graph ----------- */
/* the name is set to cache_plots.eps but you are free to change it  */                           

  FILE *fp = NULL;            // script for gnuplot which generates
                              // eps graph of time measurements

/* Create one way pipe line with call to popen() */
  if (( fp = popen("gnuplot plot_cache.gp", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }

/* Close the pipe */
  pclose(fp); 

  return(0);

}
