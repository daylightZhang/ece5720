#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define NUM_THREADS 4            /* this can be set in main()     */

void *PrintHello(void *threadid)  /* takes a single void * argument */
{                                 /* must return void *             */
   long tid = (long) threadid;

   int i,j;
   int wait_time = 1 + tid%3;
   sleep(wait_time);
   printf("Hello World! It's me, thread #%ld! Slept %d sec\n", tid,wait_time);
   pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS]; /* array of NUM_THREADS thread handles */
   int rc;                         
   long t;                         /* user assigned thread id             */
   for(t=0; t<NUM_THREADS; t++){
     printf("In main: creating thread %ld\n", t);
     rc = pthread_create(&threads[t], NULL, PrintHello, (void *) t);
     if (rc){                     
       printf("ERROR; return code from pthread_create() is %d\n", rc);
     }
   }
   pthread_exit(NULL);
}

