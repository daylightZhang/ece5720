/*****************************************************************************
*   pass multiple arguments as a structure.                                  *
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 3

// thrd_data definition
struct thrd_data 
{
   int  thrd_id, sum;           /* some random computation for sum */
   char *text;                  /* text for printing               */
};


// no computation, only printing of received data 
void *PrintHello(void *thread_arg)  /* thread_arg comes as a pointer to struct */
{
   int taskid, sum;
   char *hello_msg;
   struct thrd_data *my_data = (struct thrd_data*) thread_arg;

/* read data for me */  
   taskid = (*my_data).thrd_id;    
   sum = (*my_data).sum; 
   hello_msg = (*my_data).text;

   printf("Thread %d: %s,  Sum=%d\n", taskid, hello_msg, sum);
   pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

pthread_t threads[NUM_THREADS];
int rc, t, sum = 0;

// make thrd_data_array global
struct thrd_data thrd_data_array[NUM_THREADS];

// global array for text
char *text[NUM_THREADS];          

// set text
text[0] = "ECE2300"; text[1] = "ECE4750"; text[2] = "ECE5720_Spring_21"; 

for(t = 0; t < NUM_THREADS; t++) {   /* fill in data for threads to print */
  sum = t+t;
  thrd_data_array[t].thrd_id = t;
  thrd_data_array[t].sum = sum;
  thrd_data_array[t].text= text[t];

/* transmit pointer &thrd_data_array[t] to structure for thread t */
  rc = pthread_create(&threads[t], NULL, PrintHello, (void *) 
       &thrd_data_array[t]); 
  if (rc) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
    }
  }
pthread_exit(NULL);
}
