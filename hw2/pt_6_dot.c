/*****************************************************************************
*   Use of mutex to compute dot product
*   Data is available to all threads 
*   Each thread works on a different part of the data. 
*   The main thread waits for all the threads to complete 
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>

#define NUM_THRDS 8
#define VEC_LEN 1000000
#define MAX_LEN NUM_THRDS*VEC_LEN
#define NS_PER_SECOND  100000000 

typedef struct 
 {
   double *a, *b, sum;
   int     veclen; 
 } DOTDATA; 

DOTDATA dotstr;                   /* global struct, accessible to all */
pthread_mutex_t mutexsum;         /* global mutex,  accessible to all */

double A[MAX_LEN], B[MAX_LEN];    /* vectors for dot product */

// timing rtoutine
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td);

void *dotprod(void *arg)
{
   long offset = (long) arg;   /* subvector id on which this thread works */ 
   int i, start, end, len ;    /* start, end and length to work on */ 
   double *x, *y;              /* pointers to subvectors */
   double mysum = 0.0;         /* local sum  (dot product) */
     
   len = dotstr.veclen;
   start = offset*len; end   = start + len;
   x = dotstr.a; y = dotstr.b;

   for (i=start; i<end ; i++) 
      mysum += (x[i] * y[i]);

   pthread_mutex_lock (&mutexsum);  /* lock global sum for update */
   dotstr.sum += mysum;
   pthread_mutex_unlock (&mutexsum);/* unlock to make it accessible */

   pthread_exit((void*) 0);
}

int main (int argc, char *argv[])
{
struct timespec start,finish,delta;  /* variables for timing */
int rc, ntime, stime;

pthread_t threads[NUM_THRDS];

long i,j;
void *status;
pthread_attr_t attr;
  
for (i=0; i<VEC_LEN*NUM_THRDS; i++) {      /* populate vectors */
  A[i]=1.1/((double) (i+2));
  B[i]= (double) rand()/100000.0;
  }

dotstr.veclen = VEC_LEN;                   /* info for threads */
dotstr.a = A; dotstr.b = B; 
dotstr.sum=0;

pthread_mutex_init(&mutexsum, NULL);
         
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

clock_gettime(CLOCK_REALTIME,&start);

for(i=0;i<NUM_THRDS;i++) {
   rc = pthread_create(&threads[i], &attr, dotprod, (void *)i); 
}

for(i=0;i<NUM_THRDS;i++) {
  pthread_join(threads[i], &status);
  }

clock_gettime(CLOCK_REALTIME,&finish);
sub_timespec(start, finish, &delta);

pthread_attr_destroy(&attr);
pthread_mutex_destroy(&mutexsum);

//ntime = finish.tv_nsec - start.tv_nsec;
//stime = finish.tv_sec -  start.tv_sec;

//printf("main(): Created %d threads. Time %d, nsec %d\n", NUM_THRDS, stime, ntime);
printf("%d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);
printf("main(): Created %lu threads. \n", (unsigned long int) NUM_THRDS);

printf ("Sum =  %f \n", dotstr.sum);
printf ("vec length =  %d \n", MAX_LEN);
pthread_exit(NULL);
}   

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}
