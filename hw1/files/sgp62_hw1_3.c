/* Stefen Pegels sgp62
 * Blocked matrix multiplication, c = a*b,
 *    subblocks of a, b and c are cached.
 * There are two outer loops, one over size of matrices, the other
 * over size of blocks (tiles).
 * Outer loop increases matrix dimension n from MIN_SIZE doubling
 * on each pass until MAX_SIZE is reached.
 * Inner loop increases block size b_s from MIN_BLOCK doubling
 * on each pass untill MAX_BLOCK is reached.
 * It is assumed that n is divisible by b_s.
 * NOTE: for MAX_SIZE = 2^13 it takes many minutes to finish
 *
 * Timing results are stored in file the tile.csv as a 2D array where
 * rows correspond to growing sizes of blocks, and columns correspond
 * to growing dimensions of the matrices plus one. Make the first column
 * the column of indices against which other columns are plotted.
 * 
 * //Could do matrix size, or log of block size
 * compile: gcc -std=gnu99 -O3 -o sgp62_hw1_3 sgp62_hw1_3.c 
 * run code:   ./sgp62_hw1_3
 *
 *
 * Additions:
 * (a) add the case when matrix dimension is not divisible by block size
*/
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// for benchmarking various sizes of matrices and blocks
// set sizes here, otherwise read from the command line
# define min_exp     6
# define max_exp     13
# define MIN_SIZE    1<<min_exp
# define MAX_SIZE    1<<max_exp
# define MIN_BLOCK   1<<3
# define MAX_BLOCK   1<<5
# define BILLION 1000000000

int main(int argc, char *argv[])
{

// loop and other indices 
   int logarray = min_exp;
   int n = MAX_SIZE;
   int bl = MAX_BLOCK;
   int block_p = 0;
   
// variables for timings
   struct timespec start, end;
   float sec, nsec, exec_time;

// open file for recording time measurements
   FILE *fp = NULL;
   fp = fopen("tile.csv", "w");
 
// define subblocks and allocate memory 
   static float **tile_a, **tile_b, **tile_c;

// define matrices
   float **a, **b, **c;

// a and b are set in such a way that it easy to check
// for corretness of the product see
// once verified that the code is correct, initialize as needed

// initialize a
  a = (float **) malloc(n * sizeof(float *));
  for(int i = 0; i < n; i++) {
      a[i] = (float *) malloc(n * sizeof(float));
      for(int j = 0; j < n; j++) {
          a[i][j] = i*1.0;
      }
  }

// initialize b
  b = (float **) malloc(n * sizeof(float *));
  for(int i = 0; i < n; i++) {
      b[i] = (float *) malloc(n * sizeof(float));
      for(int j = 0; j < n; j++) {
          b[i][j] = 1.0*j;
      }
  }

// allocate space for c, use malloc or  calloc
   c = (float **) malloc(n * sizeof(float *));
   for(int i = 0; i < n; i++){
      c[i] = (float *) malloc(n * sizeof(float));
      for(int j = 0; j < n; j++) {
         c[i][j] = 1.0;
      }
   }

   //malloc for tiles
   tile_a = (float **) malloc(bl * sizeof(float *));
   for(int i = 0; i < n; i++){
      tile_a[i] = (float *) malloc(bl * sizeof(float));
      for(int j = 0; j < n; j++) {
         tile_a[i][j] = 0.0;
      }
   }

   tile_b = (float **) malloc(bl * sizeof(float *));
   for(int i = 0; i < n; i++){
      tile_b[i] = (float *) malloc(bl * sizeof(float));
      for(int j = 0; j < n; j++) {
         tile_b[i][j] = 0.0;
      }
   }

   tile_c = (float **) malloc(bl * sizeof(float *));
   for(int i = 0; i < n; i++){
      tile_c[i] = (float *) malloc(bl * sizeof(float));
      for(int j = 0; j < n; j++) {
         tile_c[i][j] = 0.0;
      }
   }



// ------- loop from MIN_SIZE, doubling the size, up to MAX_SIZE ----
   for(int dim_n = MIN_SIZE; dim_n <= MAX_SIZE; dim_n += dim_n){
      fprintf(fp, "%d, ", logarray);
      logarray++;
      // ------- loop from MIN_BLOCK, doubling the size, up to MAX_BLOCK ----
      for(int b_s = MIN_BLOCK; b_s <= MAX_BLOCK; b_s += b_s){
         // Multiply a * b = c

         // load subblocks of a,b,c to cache, and perform in cache multiplication
         // accumulate products
         // start the clock
         block_p = dim_n / b_s;
         clock_gettime(CLOCK_MONOTONIC, &start); 	
         for (int ib=0; ib<dim_n; ib=b_s+ib){
            for (int jb=0; jb<dim_n; jb=b_s+jb){
               // load subblock c(ib,jb) into cache as tile_c, done once per each subblock of c
               for(int i = 0; i < b_s; i++){
                  for(int j = 0; j < b_s; j++){
                     tile_c[i][j] = c[i+ib][j+jb];
                  }
               }
               //**tile_c = c[ib][jb]; // Seg fault
               // subblocks of a and b are loaded number of subblocks times
               for (int kb=0; kb<block_p; kb++){
                  // load subblock a(ib, kb) into cache as tile_a
                  //c[ib][jb] = a[ib][kb] * b[kb][jb];
                  for(int i = 0; i < b_s; i++){
                     for(int k = 0; k < b_s; k++){
                        tile_a[i][k] = a[i+ib][k+kb];
                     }
                  }
                  for(int i = 0; i < b_s; i++){
                     for(int j = 0; j < b_s; j++){
                        tile_b[i][j] = b[i+kb][j+jb];
                     }
                  }
                  //**tile_a = a[ib][kb];
                  // load subblock b(kb,jb) into cache as tile_b
                  //**tile_b = b[kb][jb];
                  // find product tile_c(i,j)
                  for(int i = 0; i < b_s; i++){
                     for(int j = 0; j < b_s; j++){
                        for(int k = 0; k < b_s; k++){
                           tile_c[i][j] += tile_a[i][k] * tile_b[k][j];
                        }
                     }
                  }
                  //**tile_c += **tile_b * **tile_c;
                  // store tile_c(i,j) back to main memory
                  //c[ib][jb] = **tile_c;
                  for(int i = 0; i < b_s; i++){
                     for(int j = 0; j < b_s; j++){
                        c[i+ib][j+jb] = tile_c[i][j];
                     }
                  }
               }
            }
         }
         // stop the clock and measure the multiplication time
         clock_gettime(CLOCK_MONOTONIC, &end);
         sec = end.tv_sec - start.tv_sec;
         nsec = end.tv_nsec - start.tv_nsec;
         exec_time = sec * BILLION + nsec;
         // write the measurement to file "tile.csv"
         fprintf(fp, "%1.3e, ", exec_time);

      }// end of block size loop
      fprintf(fp, "\n");
   }// end of matrix size loop

// close the file and free memory
fclose(fp); free(a); free(b); free(c);

/*
*  Create one way pipe line with call to popen()
*  need tile.csv file and plot_tile.gp file
*/

  FILE *tp = NULL;
  if (( tp = popen("gnuplot plot_tile.gp", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }
// Close the pipe
  pclose(tp);
     
   return 0;
}
