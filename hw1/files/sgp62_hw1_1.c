/* Stefen Pegels, sgp62
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

    compile: gcc -std=gnu99 -o sgp62_hw1_1 sgp62_hw1_1.c -lm
    run: ./sgp62_hw1_cache                          
*/
#define _POSIX_C_SOURCE 199309L //Used since multiple structs and macros were undefined without it, such as CLOCK_MONOTONIC and timespec
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define K 		10	        // run each stride K times
#define BILLION 	1000000000
#define min_exp         2              // smallest array tested
#define max_exp         5              // use as large max_exp as 
                                        // the system will tolerate

int main(int argc, char **argv) {

// define MIN_SIZE and MAZ_SIZE
  int MAX_SIZE = 1 << max_exp;
  int MIN_SIZE = 1 << min_exp;

// for starting and stopping timers
  struct timespec start, end; //,temp_time; 

// dynamic memory allocation for A
  float *A;
  A = (float *) malloc(MAX_SIZE*sizeof(float));
  if(!A) {
    printf("Memory not allocated, malloc returns NULL!\n");
    exit(1);
  }

// dynamic memory allocation for T
  float *T;
  T = (float *) malloc(MAX_SIZE*sizeof(float));
  if(!T) {
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

  //float Time1 = clock_gettime(CLOCK_MONOTONIC, &start);

	
  // Main loop,  double the array length starting from min length
  for(int n = MIN_SIZE; n <= MAX_SIZE; n += n){ //NOTE: what is for array A[0:n-1]? For each of the arrays? Each array is A at a different n, as n goes from MIN_SIZE to MAX_SIZE
    //Double the stride
    for(int s = 1; s <= MAX_SIZE; s += s){
      if(s <= n/2){
        //start the timer
        float Tstart = clock_gettime(CLOCK_MONOTONIC, &start);
        //printf("%.6f \n", Tstart);
        // repeat K times the current stride s for averaging
        for(int j = 0; j < K*s; j++) {
          for(int i = 0; i < n; i+=s) {
            //Access A[i]
            *(A+i) += 0;
          }
        }
        //Stop Timer, and
        // compute the average access time in K repetitions
        *(T+n*MAX_SIZE) = log2f(n);
        //printf("%1.3e ", *(T + n*MAX_SIZE + s));
        *(T + n*MAX_SIZE + s) = (clock_gettime(CLOCK_MONOTONIC, &end) - Tstart) / (n*K);
        //printf("%1.3e ", *(T + n*MAX_SIZE + s));
      }
      else{
        // Fill in zeroes
        *(T + n*MAX_SIZE + s) = 0.00;
        //printf("%1.3e ", *(T + n*MAX_SIZE + s));
      }
    } // end of loop over strides
  } // end of loop over subarrays
/*
  float Time2 = clock_gettime(CLOCK_MONOTONIC, &end);
  temp_time.tv_sec = end.tv_sec - start.tv_sec;
  temp_time.tv_nsec = end.tv_nsec - start.tv_nsec;
  long diff = temp_time.tv_sec * 1000000000 + temp_time.tv_nsec;
  printf("%.3f \n", (double) diff);
*/


// write to file
  fwrite(&T, sizeof(T), 1, tp);




// close file and free A
  fclose(tp); free(A); free(T);

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
