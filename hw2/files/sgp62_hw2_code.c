//Stefen Pegels sgp62
/***********************************************************************************  
* Gaussian elimonation with partial pivoting on an N by N matrix
*   (1) uses cyclic by row distribution for the elimination steps
*   (2) only a single thread finds a pivot row and swaps it
*       if necessary
*   (3) backsubstitution perfomed by 
*       (a) the main thread if there is only "few" rhs vectors
*           you can set a single rhs by demanding that the solution x
*           is all 1s, then b = A*x
*       (b) collectively by all threads if there are "many" rhs vectors
*           you can set N rhs vectors which are columns of an identity
*           matrix (this will be the case (c) below)
*       (c) finding the inverse will require N rhs vectors
*
*   Possible improvements/changes (not required for this assignment)
*   (1) threads cooperate to find the pivot - requires a mutex which is
*       costly, most likely not worth doing
*   (2) threads cooperate in a single  backsubstitution - must use tiling, 
*       barriers and locks
*       a single thread most likely is faster 
*   (3) after the elimination part is done, one could set strictly lower 
*       traingular part to zero, however to check for correctness you
*       may want to retain numerical values 
*       to see whether they are indeed close to zeroes 
*   (4) consider the case when matrix dimension is not divisible by
*       number of threads
*
*   Benchmarking is done for a range of matrix dimesions and different 
*   number of threads.
*     (a) The outer loop increases matrix dimension N from MIN_DIM, doubling
*         on each pass until MAX_DIM is reached
*     (b) The inner loop increases the number of threads from MIN_THRS to 
*         MAX_THRS doubling on each pass 
*   It is assumed that N is divisible by num_thrs. Feel free to add the case 
*   when N is not divisible by num_thrs.
*
*   NOTE: for MAX_DIM = 2^13 it takes many minutes to finish.
*
*   compile: gcc -std=gnu99 -O3 -o mat_inv sgp62_hw2_code.c -lpthread -lm
*   run: ./mat_inv
*
*   alternatively use input from the command line and run as
*        ./my_pt_gauss #1 #2 where
*        #1 is the (square) matrix dimension
*        #2 is the number is threads 
************************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
//#include "pthreadbarrier.h" Include error?

// set dimensions for testing
# define MIN_DIM     1<<8        // min dimension of the matrices
# define MAX_DIM     1<<8        // max dimension
# define MIN_THRS    1           // min size of a tile
# define MAX_THRS    8           // max size of a tile
# define Nrhs        256         // number of rhs vectors
# define BILLION 1000000000L
# define MILLION 1000000L

// can be used when N is not divisible by num_thrs
#define min(a, b) (((a) < (b)) ? (a) : (b))

void data_A_b(int N, float** A, float** b);            // create data
void *triangularize(void *arg);                        // triangularization
void *backSolve(void *arg);                            // backsubstitution
float error_check(float** A, float** x, float** b, int N, int nrhs, float res_error);       // check residual ||A*x-b||_2

pthread_barrier_t barrier;   // used to synchronize threads

// create a global structure visible to all threads,
// the stucture carries all necessary info
struct Thread_Data {
        float** A;           // pointer to matrix A
        float** b;           // pointer to rhs vectors b
        float** x;           // pointer to solution vectors
        int N;               // dimension of A
        int nrhs;            // number of rhs vectors
        int thrs_used;       // number of threads
} thread_data;

/************************* main ********************************/

int main(int argc, char *argv[]) {
 
/******** loop indices, other helper variables ************/

/********* file writing declarations **********************/
// would like to benchmark for a range of sizes and different
// number of threads, and record timings to a file

  FILE *fp = NULL;
  fp = fopen("Gauss_solver.csv", "w");

/********* timing related declarations **********************/
  struct timeval start, end, temptime;     // start and stop timer
  float el_time;                 // elapsed time


// ---- loop over matrix dimensions N, doubling the sizes at each pass ---
  for(int N = MIN_DIM; N <= MAX_DIM; N += N){

// ---- loop over num_thrs, doubling the sizes at each pass ----
    for(int num_thrs = MIN_THRS; num_thrs <= MAX_THRS; num_thrs += num_thrs){

/********* thread related declarations **********************/
// redefined after each pass in the num_thrs loop
      pthread_t threads[num_thrs];
      pthread_barrier_init(&barrier, NULL, num_thrs);

// Allocate memory for A
      float **A = (float **)malloc(N*sizeof(float*));
      for (int q=0; q < N; q++)
        A[q] = (float*)malloc(N*sizeof(float));

// Allocate memory for b and x, 
// for Nrhs = 1 a single rhs, for Nrhs = N for inversion
      float** b = (float**) malloc(sizeof(float*)*N);
      for (int q=0; q < N; q++)
        b[q] = (float*)malloc(Nrhs*sizeof(float));

      float** x = (float**) malloc(sizeof(float*)*N);
      for (int q=0; q < N; q++)
        x[q] = (float*)malloc(Nrhs*sizeof(float));

// set members in thread_data to pass to threads 
// like thread_data.A = A, etc.
      thread_data.A = A;
      thread_data.b = b;
      thread_data.x = x;
      thread_data.N = N;
      thread_data.nrhs = Nrhs;
      thread_data.thrs_used = num_thrs;
      

// used to pass the thread ids to the pthread function, 
      int *index = malloc (num_thrs*sizeof (uintptr_t));
      for(int ii = 0; ii < num_thrs; ii++) {
        index[ii] = ii;
      }

// populate A and b so the solution x is all 1s
      data_A_b(N,A,b);

// start timer
      gettimeofday(&start, NULL);

// activate threads for triangularization of A and update of b
      for(int t = 0; t < num_thrs; t++){
        pthread_create(&threads[t], NULL, triangularize, (void *)t);
      }

// terminate threads --join???
      for (int i=0; i < num_thrs; i++) {
        pthread_join(threads[i], NULL);
      }

// stop timer
      gettimeofday(&end, NULL);

// get triangularization execution time 
      temptime.tv_sec = end.tv_sec - start.tv_sec;
      temptime.tv_usec = end.tv_usec - start.tv_usec;
      el_time = temptime.tv_sec * MILLION + temptime.tv_usec;

// barrier synchronization 
      pthread_barrier_wait(&barrier);
// write execution time to the file
      fprintf(fp, "%1.3e, ", el_time);
// backsubstitution, A is now upper triangular, b has changed too

// activate threads for backsubstitution 
      for(int t = 0; t < num_thrs; t++){
        pthread_create(&threads[t], NULL, backSolve, (void *)t);
      }
// terminate threads -- join threads maybe?
      for (int i=0; i < num_thrs; i++) {
        pthread_join(threads[i], NULL);
      }

// stop timer
      gettimeofday(&end, NULL);
      

/* sanity check, to see whether the right solution is found
      if (N <= 8){
        printf("printing x...\n");
        for(i=0;i<Nrhs;i++){
          for(j=0;j<N;j++)
          printf("%10.2e",x[i][j]);
        printf("\n");
      }
   }
*/

// get the total execution time
      temptime.tv_sec = end.tv_sec - start.tv_sec;
      temptime.tv_usec = end.tv_usec - start.tv_usec;
      el_time = temptime.tv_sec * MILLION + temptime.tv_usec;
// check the residual error
      float res_error;
      fprintf("Residual Error: %1.3e, ", error_check(A,x,b,N,Nrhs,res_error));
      free(A); free(b); free(x);

    } // end of num_thrs loop <-------------------


  } // end of N loop <--------------------

  fclose(fp);

/*
*  Create one way pipe line with call to popen()
*  need Gauss_solver.csv file and plot_gauss.gp file
*/

  FILE *tp = NULL;
  if (( tp = popen("gnuplot plot_gauss.gp", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }
// Close the pipe
  pclose(tp);

// this part is for Mac OS only, do not use under linux
  FILE *fpo = NULL;
  if (( fpo = popen("open gauss_plots.eps", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }
  pclose(fpo);

  return 0;
}

void data_A_b(int N, float** A, float** b){
  int i, j;

  for (i=0; i<N; i++){
    for (j=0; j<N; j++)
      A[i][j] = 1.0/(1.0*i + 1.0*j + 1.0);

    A[i][i] = A[i][i] + 1.0;
  }

/* create b, either as columns of the identity matrix, or */
/* when Nrhs = 1, assume x all 1s and set b = A*x         */

  if(Nrhs == 1){
    for(int i = 0; i < N; i++){
      for(int k = 0; k < N; k++){
        b[i][0] += A[i][k];
      }
    }
  }
  else{
    for(int i = 0; i < N; i++){
      for(int k = 0; k < N; k++){
        b[i][k] = (i == k)? 1.0 : 0.0;
      }
    }
  }

}

void *triangularize(void *arg) {
  int myid = *((int*)arg);
  int i, piv_indx, thrs_used, N;
  /* other variables */

                             
// copy from global thread_data to local data
  N = thread_data.N;
  thrs_used = thread_data.thrs_used;
  float **A = thread_data.A;
  float **b = thread_data.b;
  float **x = thread_data.x;

// thread myid finds index piv_indx of pivot row in column i
// and next swaps rows i and  piv_indx 

  for(i = 0; i<N; i++) {
    if ((i%thrs_used) == (int) myid) {
       /* your code for finding pivot */
       /* your code for swapping rows i and piv_indx in A and b */
      }

// all threads wait until swapping of row i and piv_indx are done

    pthread_barrier_wait(&barrier);

// rows i+1 to N can be updated independently by threads 
// based on cyclic distribution of rows among threads

   /* your code */

// wait for all
    pthread_barrier_wait(&barrier);
  }
    pthread_barrier_wait(&barrier);
  
  return 0;
}

void *backSolve(void *arg){
  int myid = *((int*)arg);

// copy global thread_data to local data

// thread myid performs backsubstitution for Nrhs/thrs_used rhs
// column cyclic distribution

  for(k= myid;k < nrhs; k += thrs_used){  // loop over # rhs

    /* your backsubstitution code */ 

    }
}

float error_check(float** A, float** x, float** b, int N, int nrhs, float res_error){

  //res_error argument? Where to store/output result?

/************************************************************************ 
 * compute residual r = b - A*x, compute ||r||_2 = sqrt(sum_i(r[i]*r[i]))
 * compute ||x||_2 = sqrt(sum_i(x[i]*x[i])), 
 * ||A||_F = sqrt(sum_i(sum_j(a[i][j]*a[i][j])))
 * compute normalized residual error res_error 
 *   res_error =  ||r||_2/(||A||_F*||x||_2)
 * in single precision it should be close to 1.0e-6
 * in double precision it should be close to 1.0e-15
 *************************************************************************/

   return res_error*
}

void printData(int N, double** A){
  if (N <= 16){
    for (int x=0; x<N; x++){
      printf("| ");
      for(int y=0; y<N; y++)
        printf("% 5.2e ", A[x][y]);
      printf("|\n");
      }
    }
  else{
    printf("\nMatrix and vector too large to print out.\n");
  }
}

