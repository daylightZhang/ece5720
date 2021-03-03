/*
 * trylock.c
 *
 * Demonstrate a simple use of pthread_mutex_trylock. The
 * counter_thread updates a shared counter at intervals, and a
 * monitor_thread occasionally reports the current value of the
 * counter -- but only if the mutex is not already locked by
 * counter_thread.
 * from Butenhof
 */
#include <pthread.h>
#include "errors.h"

#define SPIN 10000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long counter;
time_t end_time;

/*
 * Thread start routine that repeatedly locks a mutex and
 * increments a counter.
 */
void *counter_thread (void *arg)
{
    int status;
    int spin;

    /*
     * Until end_time, increment the counter each
     * second. Instead of just incrementing the counter, it
     * sleeps for another second with the mutex locked, to give
     * monitor_thread a reasonable chance of running.
     */
    while (time (NULL) < end_time)
    {
        status = pthread_mutex_lock (&mutex);
        for (spin = 0; spin < SPIN; spin++)
            counter++;
        status = pthread_mutex_unlock (&mutex);
        sleep (1);
    }
    printf ("Counter is %#lx\n", counter);
    return NULL;
}

/*
 * Thread start routine to "monitor" the counter. Every 3
 * seconds, try to lock the mutex and read the counter. If the
 * trylock fails, skip this cycle.
 */
void *monitor_thread (void *arg)
{
    int status;
    int misses = 0;

    /*
     * Loop until end_time, checking the counter every 3
     * seconds.
     */
    while (time (NULL) < end_time)
    {
        sleep (3);
        status = pthread_mutex_trylock (&mutex);
        if (status != EBUSY)
        {
            printf ("Counter is %ld\n", counter/SPIN);
            status = pthread_mutex_unlock (&mutex);
        } else
            misses++;           /* Count "misses" on the lock */
    }
    printf ("Monitor thread missed update %d times.\n", misses);
    return NULL;
}

int main (int argc, char *argv[])
{
    int status;
    pthread_t counter_thread_id;
    pthread_t monitor_thread_id;

    end_time = time (NULL) + 9;        /* Run for a while*/
    status = pthread_create ( &counter_thread_id, NULL, counter_thread, NULL);
    status = pthread_create ( &monitor_thread_id, NULL, monitor_thread, NULL);
    status = pthread_join (counter_thread_id, NULL);
    status = pthread_join (monitor_thread_id, NULL);
    return 0;
}
