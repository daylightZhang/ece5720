/* Jacobi iteration for Laplace equation using pthreads

     gcc -O3 jacobi.c -lpthread
     a.out gridSize num_thrds numIters

*/

//#define _REENTRANT
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <limits.h>
#include "pthreadbarrier.h"

#define MAXGRID 258   /* maximum grid size, including boundaries */
#define MAX_THRDS 8  /* maximum number of worker threads */
#define BILLION         1000000000

void *Worker(void *);
void InitializeGrids();

struct timespec start,finish;
long ntime, stime;

pthread_barrier_t mybarrier;
int numArrived = 0;       /* count of the number who have arrived */

int gridSize, num_thrds, numIters, stripSize;
double maxDiff[MAX_THRDS];
double grid1[MAXGRID][MAXGRID], grid2[MAXGRID][MAXGRID];


/* main() 
  read command line, 
  initialize grids, 
  create threads
  print the results 
*/

int main(int argc, char *argv[]) {
  /* thread ids and attributes */
  pthread_t workerid[MAX_THRDS];
  pthread_attr_t attr;

  long i; int j;
  double maxdiff = 0.0, ex_time;

  FILE *results;

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* read command line and initialize grids */
  gridSize = atoi(argv[1]);
  num_thrds = atoi(argv[2]);
  numIters = atoi(argv[3]);

  stripSize = gridSize/num_thrds;

  InitializeGrids();

  /* initialize barrier variable */
  pthread_barrier_init(&mybarrier, NULL, num_thrds);

  clock_gettime(CLOCK_REALTIME,&start);

  /* create the workers */
  for (i = 0; i < num_thrds; i++)
    pthread_create(&workerid[i], &attr, Worker, (void *) i);

  /* join the workers   */
  for (i = 0; i < num_thrds; i++)
    pthread_join(workerid[i], NULL);

 clock_gettime(CLOCK_REALTIME,&finish);

  /* find max distance between consecutive iterations */
  for (i = 0; i < num_thrds; i++)
    if (maxdiff < maxDiff[i])
      maxdiff = maxDiff[i];
  printf("number of iterations:  %d\nmaximum difference:  %e\n",
          numIters, maxdiff);

  /* calculate elapsed time */
 ntime = finish.tv_nsec - start.tv_nsec;
 stime = (int) finish.tv_sec - (int) start.tv_sec;
 ex_time = (double)( BILLION*(finish.tv_sec - start.tv_sec) +
             (finish.tv_nsec - start.tv_nsec) );
  printf("elapsed time:  %10.2e, %ld, %ld\n", ex_time,stime,ntime);

  /* write computed solution to file */
  results = fopen("results", "w");
  for (i = 1; i <= gridSize; i++) {
    for (j = 1; j <= gridSize; j++) {
      fprintf(results, "%10.2e ", grid2[i][j]);
    }
    fprintf(results, "\n");
  }
}


/* Each Worker computes values in one strip of the grids.
   The main worker loop does two computations to avoid copying from
   one grid to the other. */ 

void *Worker(void *arg) {
  int myid = (int) arg;
  double maxdiff, temp;
  int i, j, iters;
  int first, last;

  printf("worker %d has started\n", myid);

  /* determine first and last rows of my strip of the grids */
  first = myid*stripSize + 1;
  last = first + stripSize - 1;

  for (iters = 1; iters <= numIters; iters++) {
    /* use 5 point stencil, update my points */
    for (i = first; i <= last; i++) {
      for (j = 1; j <= gridSize; j++) {
        grid2[i][j] = (grid1[i-1][j] + grid1[i+1][j] + 
                       grid1[i][j-1] + grid1[i][j+1]) * 0.25;
      }
    }
  pthread_barrier_wait(&mybarrier);
    /* update my points again */
    for (i = first; i <= last; i++) {
      for (j = 1; j <= gridSize; j++) {
        grid1[i][j] = (grid2[i-1][j] + grid2[i+1][j] +
               grid2[i][j-1] + grid2[i][j+1]) * 0.25;
      }
    }

  /* wait for all */
  pthread_barrier_wait(&mybarrier);
  }

  /* compute the maximum difference in my strip and set global variable */
  maxdiff = 0.0;
  for (i = first; i <= last; i++) {
    for (j = 1; j <= gridSize; j++) {
      temp = grid1[i][j]-grid2[i][j];
      if (temp < 0)
        temp = -temp;
      if (maxdiff < temp)
        maxdiff = temp;
    }
  }

  /* record max diff in global maxDiff[] */
  maxDiff[myid] = maxdiff;

  return 0;
}

void InitializeGrids() 
{
  /* initialize the grids (grid1 and grid2)
     set boundaries to 1.0 and interior points to 0.0  */
  int i, j;
  for (i = 0; i <= gridSize+1; i++)
    for (j = 0; j <= gridSize+1; j++) {
      grid1[i][j] = 0.0;
      grid2[i][j] = 0.0;
    }
  for (i = 0; i <= gridSize+1; i++) {
    grid1[i][0] = 1.0;
    grid1[i][gridSize+1] = 1.0;
    grid2[i][0] = 1.0;
    grid2[i][gridSize+1] = 1.0;
  }
  for (j = 0; j <= gridSize+1; j++) {
    grid1[0][j] = 1.0;
    grid2[0][j] = 1.0;
    grid1[gridSize+1][j] = 1.0;
    grid2[gridSize+1][j] = 1.0;
  }
}
