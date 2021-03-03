#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>

#define NS_PER_SECOND  100000000

void *infty_norm(void *arg);

struct NORMDATA
 {
   float **A;
   float *part;
   int   N;
   int   M;
   int   num_thrds;
   int   thrd_id;
 } ;

float norm = 0.0;

pthread_mutex_t mutex_norm = PTHREAD_MUTEX_INITIALIZER;;        
pthread_mutex_t mutex_rowk = PTHREAD_MUTEX_INITIALIZER;;        

/************************ MAIN ******************************/
int main(int argc, char *argv[]) {

/********* general parameters ******** **********************/
  int i,j,k,rc,t;                    // general loop indices
  int N, M, num_thrds;    // matrix dim and # threads loops
  
  M = atoi(argv[1]); 
  N = atoi(argv[2]); 
  num_thrds = atoi(argv[3]); 
  float **A, *part;
  float total_time = 0.0;

/********* timing related declarations **********************/
  struct timespec start, end;     // start and stop timer
  float el_time;                 // elapsed time

/********* thread related declarations **********************/
  pthread_t thread[num_thrds];
  struct NORMDATA thrd_data_array[num_thrds];
  void *status;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


  part = (float *) malloc(num_thrds * sizeof(float *));
  for(i=0;i<num_thrds;i++)
    part[i]=0.0;

// initialize A
  A = (float **) malloc(M * sizeof(float *));
  for(i = 0; i < M; i++) {
      A[i] = (float *) malloc(N * sizeof(float));
      for(j = 0; j < N; j++){ 
          A[i][j] = 1.0*(i-j);
//          printf("%9.2e  ",A[i][j]);
      }
//      printf("\n");
   }

  clock_gettime(CLOCK_MONOTONIC, &start);

for(k=0;k<20;k++){
/************** fill in data for threads********************/
  for(t = 0; t < num_thrds; t++) {   
    thrd_data_array[t].A = A;
    thrd_data_array[t].part = part;
    thrd_data_array[t].M = M;
    thrd_data_array[t].N = N;
    thrd_data_array[t].thrd_id = t;
    thrd_data_array[t].num_thrds = num_thrds;
    rc = pthread_create(&thread[t], NULL, infty_norm, (void *)
       &thrd_data_array[t]);
  }
  for (t=0; t<num_thrds; t++){
    pthread_join(thread[t], &status);
//    printf("part[%d] = %10.3e\n",t, part[t]);
      if(part[t] > norm)
        norm =part[t];   
   }
 
  clock_gettime(CLOCK_MONOTONIC, &end);

  el_time = NS_PER_SECOND*(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
  el_time = el_time / NS_PER_SECOND;
  total_time += el_time;

}
el_time = total_time/20;
  printf("norm computed = %10.3e\n",norm);
  norm = N*(N-1)/2;
  printf("true norm = %10.3e\n",norm);
  printf("execution time %10.3e \n",el_time);
  return 0; 
}

void *infty_norm(void *arg){

   struct NORMDATA *my_data = (struct NORMDATA*) arg;
   int i, j, start, end, chunk;    /* start, end and length to work on */
   int M, N, num_thrds, tid;
   float **A, *part;              /* pointers to subvectors */
   float temp, mynorm = 0.0;         /* local norm */

/* read data for me */
   tid = (*my_data).thrd_id;
   M = (*my_data).M;
   N = (*my_data).N;
   A = (*my_data).A;
   part = (*my_data).part;
   num_thrds = (*my_data).num_thrds;

   chunk = M/num_thrds;
   start = chunk*tid; end   = start + chunk;
//   printf("tid = %d, (%d,%d)\n", tid,start,end);

   for(i=start; i < end; i++){
      temp = 0.0;
      for(j=0; j< N; j++)
        temp += abs(A[i][j]);
      if (temp > mynorm)
        mynorm = temp;
   }
   part[tid] = mynorm;
//   printf("tid = %d, mynorm = %10.3e\n", tid, part[tid]);

/*
   pthread_mutex_lock(&mutex_norm); 
   if(mynorm>norm)
     norm = mynorm;
   pthread_mutex_unlock(&mutex_norm);
*/

   pthread_exit(NULL);
};

