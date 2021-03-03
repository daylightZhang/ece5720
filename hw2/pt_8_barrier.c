
//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
//#include "pthreadbarrier.h"

#define num_thrds 4

pthread_barrier_t mybarrier;

void* threadFn(void *id_ptr) {
  long thread_id = (long) id_ptr;

  int wait_sec = 1 + rand() % 4;
  printf("thread %ld: Wait for %d seconds.\n", thread_id, wait_sec);
  sleep(wait_sec);
  printf("thread %ld: I'm ready \n", thread_id);

  pthread_barrier_wait(&mybarrier);

  printf("thread %ld left the barrier\n", thread_id);
  pthread_exit(NULL);
}


int main() {
  long i;
  pthread_t ids[num_thrds];
  int short_ids[num_thrds];

  srand(time(NULL));
  pthread_barrier_init(&mybarrier, NULL, num_thrds + 1);

  for (i=0; i < num_thrds; i++) {
    pthread_create(&ids[i], NULL, threadFn, (void*) i);
  }

  printf("main() is ready.\n");

  sleep(1);

  pthread_barrier_wait(&mybarrier);
  printf("main() left the barrier.\n");

  for (i=0; i < num_thrds; i++) {
    pthread_join(ids[i], NULL);
  }

  pthread_barrier_destroy(&mybarrier);

  return 0;
}
