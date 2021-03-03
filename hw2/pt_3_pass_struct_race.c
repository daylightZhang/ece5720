/*********************************************************
* compute pi from the foemula:                           *
* pi = 4*(1 - 1/3 + 1/5 -1/7 + 1/9 + ......)             *
* decide on the number of terms and number of threads    *
* divide work equally among threads                      *
*********************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>

//#define NUM_THRDS 2
//#define N         1<<20

double PI = 3.1415926535897932;
double sum = 0.0;
int flag = 1;
struct thrd_data
{
  int n;
  int num_thrds;
  int thrd_id;
};

void* pt_pi(void* thrd_id){
  struct thrd_data *my_data = (struct thrd_data*) thrd_id;
  int n = (*my_data).n;
  int num_thrds = (*my_data).num_thrds;
  int my_id = (*my_data).thrd_id;
  double s, error;
  long i;
  long chunk = n/num_thrds;
  long my_first_i = chunk*my_id;
  long my_last_i = my_first_i + chunk;

  if (my_first_i % 2 == 0)
    s = 1.0;
  else
    s = -1.0;

  for(i = my_first_i; i < my_last_i; i++, s = -s){
      sum += s/(2*i+1);
    }
      
  error = 4*sum-PI;
  printf("thread %d, sum = %12.6e\n", my_id,error);
  return NULL;
}

int main(int argc, char *argv[]) {
  int n = atoi(argv[1]);
  int num_thrds = atoi(argv[2]);
 
  pthread_t threads[num_thrds];
  int t;
  int rc;

  struct thrd_data thrd_data_array[num_thrds];

for(t = 0; t < num_thrds; t++) {
  thrd_data_array[t].n = n;
  thrd_data_array[t].num_thrds = num_thrds;
  thrd_data_array[t].thrd_id = t;
  rc = pthread_create(&threads[t], NULL, pt_pi, (void *) &thrd_data_array[t]);
  }
pthread_exit(NULL);
}


