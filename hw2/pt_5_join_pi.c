/*************************************************************************
* probabilistic way to compute pi by 'throwing darts'                    *
* one generates a random point (x,y), with 0<= x,y <= 1, if the distance *
* to (0,0) is less than 1 then we have a hit, we 'throw' TOTAL_T times   *
* pi is approximately 4*hits/TOTAL_T                                     *
*************************************************************************/


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define num_threads 8
#define TOTAL_T 10000000

int sample_points = TOTAL_T; 

void *compute_pi(void*);

int main() {
  int taskids[num_threads];
  long t;
  pthread_t p_threads[num_threads];
  pthread_attr_t attr;
  void *status;
  int i, tot_hits = 0;
  int rc, sample_points = TOTAL_T; 
  double computed_pi;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for (t=0; t<num_threads; t++){
    pthread_create(&p_threads[t], NULL, compute_pi, (void*) t);
printf("thread %d created\n", i);
  }

  for (i=0; i<num_threads; i++){
    pthread_join(p_threads[i], &status);
    tot_hits+= (long)status;
  }
  computed_pi = 4*(double) tot_hits/((double) (sample_points*num_threads));
  printf("approximation to pi is %lf\n", computed_pi);
}

void *compute_pi(void* ptr) {
  long task_id = (long) ptr;
  int i; long  my_hits = 0; 
  double rand_x, rand_y;

  srand48((long)task_id);
  for (i = 0; i < sample_points; i++) {
    rand_x = drand48(); rand_y = drand48();
    if ((rand_x * rand_x)+ (rand_y * rand_y ) < 1.00)
      my_hits ++;
  }
  printf("from thread %ld, my_hits = %ld\n",task_id, my_hits);
  pthread_exit((void *)my_hits);     //return local count
}

