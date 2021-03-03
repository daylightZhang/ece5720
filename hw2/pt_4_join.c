/************************************************************************
* pthread_create followed by pthread_join                               *
* pthread_exit returns status for pthread_join                          *
*************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 8

void *SomeWork(void *thr_id) {
  int i,t;
  long tid = (long) thr_id;
  double sum = 0.0;
  srand48(tid); /* set seed to tid */

  for (i=0; i<10; i++) {
    sum = sum + drand48();
  }
  printf("Thread %ld done. Result = %10.2e\n",tid, sum);
  pthread_exit((void*) thr_id);       /* returns user's thread ID after join */
}

int main (int argc, char *argv[]) {
  pthread_t thread[NUM_THREADS];
  pthread_attr_t attr;
  int rc; long t;
  void *status;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for(t=0; t<NUM_THREADS; t++) {
    rc = pthread_create(&thread[t], &attr, SomeWork, (void *)t);
  }

  for(t=0; t<NUM_THREADS; t++) {
    rc = pthread_join(thread[t], &status);  
    printf("Main completed join with thread %ld, status = %ld\n",t,(long)status);
  }
  printf("Main thread: program completed. Exiting.\n");

  pthread_attr_destroy(&attr);
  pthread_exit(NULL);
  return 1;
}
