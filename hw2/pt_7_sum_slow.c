
/*  A Pthreads version of a program to sum up the elements of an array.
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define array_size 10000000
#define no_threads 8

int a[array_size];
int global_index = 0;
long int sum = 0;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *slave(void *thrd_id) {
    void *threadID;
    long tid = (long) thrd_id;             /* user thread ID     */
    int local_index; 
    long int partial_sum = 0;
    int no_ele = (int) array_size;
    do {
        pthread_mutex_lock(&mutex1);
          local_index = global_index;
          global_index++;
        pthread_mutex_unlock(&mutex1);

        if (local_index < no_ele)
            partial_sum += a[local_index];
    } while (local_index < no_ele);

    threadID = pthread_self();
    pthread_mutex_lock(&mutex1);
    sum += partial_sum;
    printf("Partial sum from sysID%10x usrID %ld is %ld\n",
            (unsigned int) threadID, tid, sum);
    pthread_mutex_unlock(&mutex1);

    pthread_exit(NULL);
}

int main() {

/* for timing */
 struct timespec tstart={0,0}, tend={0,0};

    long int no_el;
    long i;
    pthread_t thread[no_threads];
    pthread_attr_t attr;
    pthread_mutex_init(&mutex1, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < array_size; i++)
        a[i] = i+1;

    clock_gettime(CLOCK_MONOTONIC, &tstart);

    for (i = 0; i < no_threads; i++)
        if (pthread_create(&thread[i], &attr, slave, (void *) i) != 0)
            perror("Pthread create fails");

    for (i = 0; i < no_threads; i++)
        if (pthread_join(thread[i], NULL) != 0)
            perror("Pthread join fails");

    clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("computation took %.5f seconds\n",
    ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
    ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));

    printf("The pthread sum of 1 to %i is %ld\n", array_size, sum);
    no_el = (long int) array_size;
    sum = (no_el+1)*no_el/2;
    printf("The correct sum is %ld\n", sum);
    printf("Max long int is %ld, max int is %d\n", LONG_MAX, INT_MAX);

    pthread_mutex_destroy(&mutex1);
    pthread_exit(NULL);
}

